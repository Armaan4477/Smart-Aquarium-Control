"""
collector.py — Background polling loop
Polls the ESP32 every POLL_STATUS_INTERVAL seconds (status + temperatures)
and every POLL_LOGS_INTERVAL seconds (system logs) then stores the results
in SQLite.
"""
import time
import logging
import threading
import datetime
import requests

import config
import db
import mailer

log = logging.getLogger(__name__)

# Shared state exposed to the API for /status health check
last_status_poll: dict = {"time": None, "ok": None, "error": None}
last_logs_poll:   dict = {"time": None, "ok": None, "error": None}

# Email tracking state
email_tracking_state = {
    "last_active_errors": 0,
    # Tracks which 90-minute slot index (since midnight) was last emailed.
    # Initialised to the *current* slot so we don't fire immediately on start.
    "last_periodic_email_slot": None,
}

# Uptime and ping tracking state
# uptime_seconds is restored from the persistent DB on module load so that a
# container restart does NOT reset the counter back to zero.
# uptime_pending is True from container startup until we receive at least one
# successful health ping — during this window the API returns None for uptime
# so the frontend can display '--' instead of a stale/wrong value.
_persisted_uptime = db.get_state("uptime_seconds", default="0")
uptime_state = {
    "rpi_uptime_seconds": int(_persisted_uptime),
    "failed_ping_count": 0,
    "is_offline": False,
    "uptime_pending": True,   # True until first successful ping after (re)start
    "went_offline_at": None,  # wall-clock time when is_offline first became True
}

_ESP32_STATUS_URL = f"http://{config.ESP32_IP}:{config.ESP32_PORT}/api/status"
_ESP32_LOGS_URL   = f"http://{config.ESP32_IP}:{config.ESP32_PORT}/api/logs"
_AUTH = (config.ESP32_USER, config.ESP32_PASS)


# ── helpers ────────────────────────────────────────────────────────────────

def _now_iso() -> str:
    return datetime.datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")


def is_maintenance_mode() -> bool:
    until = db.get_state("maintenance_mode_until")
    if not until:
        return False
    try:
        until_dt = datetime.datetime.strptime(until, "%Y-%m-%dT%H:%M:%SZ")
        return datetime.datetime.utcnow() < until_dt
    except ValueError:
        return False


def _fetch(url: str) -> dict | None:
    """GET a JSON endpoint; returns parsed dict or None on any error."""
    try:
        resp = requests.get(url, auth=_AUTH, timeout=config.REQUEST_TIMEOUT)
        resp.raise_for_status()
        return resp.json()
    except requests.exceptions.ConnectionError:
        log.warning("ESP32 unreachable: %s", url)
    except requests.exceptions.Timeout:
        log.warning("Timeout fetching: %s", url)
    except requests.exceptions.HTTPError as exc:
        log.warning("HTTP error %s from %s", exc.response.status_code, url)
    except ValueError:
        log.warning("Non-JSON response from %s", url)
    return None


# ── poll tasks ─────────────────────────────────────────────────────────────

def _poll_status():
    """Fetch /api/status and store in status_readings."""
    if is_maintenance_mode():
        return
    global last_status_poll
    data = _fetch(_ESP32_STATUS_URL)
    now  = _now_iso()

    if data is None:
        last_status_poll = {"time": now, "ok": False, "error": "fetch_failed"}
        return

    try:
        uptime_sec = uptime_state["rpi_uptime_seconds"]
        uptime_days = uptime_sec // 86400

        with db.get_conn() as conn:
            conn.execute(
                """INSERT INTO status_readings
                       (collected_at, esp32_time,
                        internal_c, external_c,
                        relay1, relay2, relay3,
                        override1, override2,
                        active_errors, acknowledged_errors,
                        uptime_seconds, uptime_days, time_synced)
                   VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)""",
                (
                    now,
                    data.get("timestamp"),
                    data.get("internal_c"),
                    data.get("external_c"),
                    1 if data.get("relay1")         else 0,
                    1 if data.get("relay2")         else 0,
                    1 if data.get("relay3")         else 0,
                    1 if data.get("override1")      else 0,
                    1 if data.get("override2")      else 0,
                    int(data.get("active_errors", 0)),
                    int(data.get("acknowledged_errors", 0)),
                    uptime_sec,
                    uptime_days,
                    1 if data.get("time_synced")    else 0,
                ),
            )
        last_status_poll = {"time": now, "ok": True, "error": None}
        log.debug("Status stored: %.2f°C int / %.2f°C ext",
                  data.get("internal_c", 0), data.get("external_c", 0))

    except Exception as exc:
        log.error("DB write error (status): %s", exc)
        last_status_poll = {"time": now, "ok": False, "error": str(exc)}


def _current_email_slot() -> int:
    """Return the index of the current 90-minute slot since midnight (0-based).

    Slots are fixed to wall-clock time anchored at 00:00 local time:
      slot 0  → 00:00 – 01:29
      slot 1  → 01:30 – 02:59
      slot 2  → 03:00 – 04:29
      …and so on (16 slots per day).
    """
    now = datetime.datetime.now()
    minutes_since_midnight = now.hour * 60 + now.minute
    return minutes_since_midnight // 90


def _poll_errors():
    """Fetch /api/status to check for errors and trigger emails."""
    if is_maintenance_mode():
        return
    data = _fetch(_ESP32_STATUS_URL)
    if data is None:
        return

    # --- Error-transition email logic ---
    current_active_errors = int(data.get("active_errors", 0))
    last_errors = email_tracking_state["last_active_errors"]
    
    new_errors = current_active_errors & ~last_errors
    email_tracking_state["last_active_errors"] = current_active_errors

    # --- Fixed-schedule periodic email logic ---
    # Determine which 90-minute wall-clock slot we are currently in.
    current_slot = _current_email_slot()

    # Initialise last_periodic_email_slot to the current slot on first run so
    # we don't immediately fire an email on startup.
    if email_tracking_state["last_periodic_email_slot"] is None:
        email_tracking_state["last_periodic_email_slot"] = current_slot

    periodic_due = current_slot != email_tracking_state["last_periodic_email_slot"]

    # If the ESP32 is currently offline, skip the periodic status email for
    # this slot (the offline alert email already notified the user). Mark the
    # slot as consumed so we don't double-send once it comes back.
    if periodic_due and uptime_state["is_offline"]:
        log.info(
            "Skipping periodic status email for slot %d – ESP32 is offline "
            "(offline alert already sent).",
            current_slot,
        )
        email_tracking_state["last_periodic_email_slot"] = current_slot
        periodic_due = False

    needs_email = (new_errors > 0) or periodic_due

    if needs_email:
        # Re-fetch the latest status so the email contains the freshest data
        fresh_data = _fetch(_ESP32_STATUS_URL)
        if fresh_data is None:
            log.warning("Could not refresh status data before sending email; using existing data.")
            fresh_data = data

        uptime_sec = uptime_state["rpi_uptime_seconds"]
        email_data = {
            **fresh_data,
            "uptime_seconds": uptime_sec % 86400,
            "uptime_days": uptime_sec // 86400,
        }

        if new_errors & (1 << 0):
            mailer.send_email_report("WiFi Disconnected Error", email_data)
        if new_errors & (1 << 1):
            mailer.send_email_report("Time Sync Error", email_data)
        if new_errors & (1 << 2):
            mailer.send_email_report("Internal Temperature Sensor Error", email_data)
        if new_errors & (1 << 3):
            mailer.send_email_report("External Temperature Sensor Error", email_data)
        
        if periodic_due:
            mailer.send_email_report("Status Check", email_data)
            email_tracking_state["last_periodic_email_slot"] = current_slot


def _poll_logs():
    """Fetch /api/logs and store any new entries in log_entries."""
    if is_maintenance_mode():
        return
    global last_logs_poll
    data = _fetch(_ESP32_LOGS_URL)
    now  = _now_iso()

    if data is None:
        last_logs_poll = {"time": now, "ok": False, "error": "fetch_failed"}
        return

    entries = data.get("logs", [])
    if not entries:
        last_logs_poll = {"time": now, "ok": True, "error": None}
        return

    try:
        with db.get_conn() as conn:
            # Avoid duplicating entries that are already stored from a
            # previous poll cycle by checking the combination of
            # esp32_id + esp32_time + message.
            for entry in entries:
                esp32_id  = entry.get("id")
                esp32_ts  = entry.get("timestamp")
                message   = entry.get("message", "")
                exists = conn.execute(
                    """SELECT 1 FROM log_entries
                       WHERE esp32_id=? AND esp32_time=? AND message=?
                       LIMIT 1""",
                    (esp32_id, esp32_ts, message),
                ).fetchone()
                if not exists:
                    conn.execute(
                        """INSERT INTO log_entries
                               (collected_at, esp32_id, esp32_time, message)
                           VALUES (?,?,?,?)""",
                        (now, esp32_id, esp32_ts, message),
                    )
        last_logs_poll = {"time": now, "ok": True, "error": None}
        log.debug("Logs stored: %d entries", len(entries))
    except Exception as exc:
        log.error("DB write error (logs): %s", exc)
        last_logs_poll = {"time": now, "ok": False, "error": str(exc)}


def _health_ping():
    """Ping ESP32 every 10s to update uptime and check online status."""
    if is_maintenance_mode():
        return
    url = f"http://{config.ESP32_IP}:{config.ESP32_PORT}/api/ping"
    try:
        resp = requests.get(url, auth=_AUTH, timeout=3.0)
        is_docker_disabled = False
        
        if resp.status_code == 403:
            try:
                if resp.json().get("error") == "Docker integration disabled":
                    is_docker_disabled = True
            except ValueError:
                pass
                
        if resp.status_code == 200 or is_docker_disabled:
            uptime_state["docker_disabled"] = is_docker_disabled
            
            # First successful ping after a (re)start — clear the pending flag
            # so the API starts surfacing real uptime values instead of '--'.
            if uptime_state["uptime_pending"]:
                uptime_state["uptime_pending"] = False
                log.info("First health ping received — uptime counter active.")

            uptime_state["rpi_uptime_seconds"] += 10
            uptime_state["failed_ping_count"] = 0

            # Persist updated uptime so a future container restart can resume
            # from the correct value rather than resetting to 0.
            try:
                db.set_state("uptime_seconds", uptime_state["rpi_uptime_seconds"])
            except Exception as exc:
                log.warning("Failed to persist uptime_seconds: %s", exc)

            if uptime_state["is_offline"]:
                # ESP32 has come back online after an offline period.
                # Reset the uptime counter to 0 so it reflects continuous
                # uptime since recovery, not cumulative time that would
                # incorrectly include the offline gap.
                offline_duration = 0
                if uptime_state["went_offline_at"] is not None:
                    offline_duration = int(
                        (datetime.datetime.utcnow() - uptime_state["went_offline_at"]).total_seconds()
                    )
                log.info(
                    "ESP32 is back online after %d s offline. Resetting uptime counter.",
                    offline_duration,
                )
                uptime_state["rpi_uptime_seconds"] = 0
                uptime_state["went_offline_at"] = None
                uptime_state["is_offline"] = False
                try:
                    db.set_state("uptime_seconds", 0)
                except Exception as exc:
                    log.warning("Failed to persist reset uptime_seconds: %s", exc)
                mailer.send_online_email()
        else:
            _handle_ping_failure()
    except requests.exceptions.RequestException:
        _handle_ping_failure()

def _handle_ping_failure():
    uptime_state["failed_ping_count"] += 1
    if uptime_state["failed_ping_count"] >= 3 and not uptime_state["is_offline"]:
        uptime_state["is_offline"] = True
        uptime_state["went_offline_at"] = datetime.datetime.utcnow()
        log.warning("ESP32 is offline. Sending email.")
        mailer.send_offline_email()


# ── purge task ─────────────────────────────────────────────────────────────

def _purge_loop():
    """Run the purge once per hour."""
    while True:
        try:
            db.purge_old_records(config.RETENTION_DAYS)
        except Exception as exc:
            log.error("Purge error: %s", exc)
        time.sleep(3600)


# ── thread runners ─────────────────────────────────────────────────────────

def _run_every(interval: int, fn):
    """Call fn() immediately then repeat every interval seconds."""
    while True:
        try:
            fn()
        except Exception as exc:
            log.error("Unhandled error in %s: %s", fn.__name__, exc)
        time.sleep(interval)


def start():
    """Start all background collector threads (daemon so they die with the process)."""
    for target, interval in [
        (_health_ping, config.PING_INTERVAL),
        (_poll_status, config.POLL_STATUS_INTERVAL),
        (_poll_errors, config.POLL_ERROR_INTERVAL),
        (_poll_logs,   config.POLL_LOGS_INTERVAL),
        (_purge_loop,  None),          # _purge_loop handles its own sleep
    ]:
        fn   = target if interval is None else (lambda t=target, i=interval: _run_every(i, t))
        name = target.__name__
        t = threading.Thread(target=fn, name=name, daemon=True)
        t.start()
        log.info("Collector thread started: %s (interval=%s s)", name, interval)
