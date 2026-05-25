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

log = logging.getLogger(__name__)

# Shared state exposed to the API for /status health check
last_status_poll: dict = {"time": None, "ok": None, "error": None}
last_logs_poll:   dict = {"time": None, "ok": None, "error": None}

_ESP32_STATUS_URL = f"http://{config.ESP32_IP}:{config.ESP32_PORT}/api/status"
_ESP32_LOGS_URL   = f"http://{config.ESP32_IP}:{config.ESP32_PORT}/api/logs"
_AUTH = (config.ESP32_USER, config.ESP32_PASS)


# ── helpers ────────────────────────────────────────────────────────────────

def _now_iso() -> str:
    return datetime.datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")


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
    global last_status_poll
    data = _fetch(_ESP32_STATUS_URL)
    now  = _now_iso()

    if data is None:
        last_status_poll = {"time": now, "ok": False, "error": "fetch_failed"}
        return

    try:
        with db.get_conn() as conn:
            conn.execute(
                """INSERT INTO status_readings
                       (collected_at, esp32_time,
                        internal_c, external_c,
                        relay1, relay2, relay3,
                        override1, override2,
                        has_error, temp_error, ext_temp_error,
                        uptime_seconds, uptime_days, time_synced)
                   VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)""",
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
                    1 if data.get("has_error")      else 0,
                    1 if data.get("temp_error")     else 0,
                    1 if data.get("ext_temp_error") else 0,
                    data.get("uptime_seconds"),
                    data.get("uptime_days"),
                    1 if data.get("time_synced")    else 0,
                ),
            )
        last_status_poll = {"time": now, "ok": True, "error": None}
        log.debug("Status stored: %.2f°C int / %.2f°C ext",
                  data.get("internal_c", 0), data.get("external_c", 0))
    except Exception as exc:
        log.error("DB write error (status): %s", exc)
        last_status_poll = {"time": now, "ok": False, "error": str(exc)}


def _poll_logs():
    """Fetch /api/logs and store any new entries in log_entries."""
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
        (_poll_status, config.POLL_STATUS_INTERVAL),
        (_poll_logs,   config.POLL_LOGS_INTERVAL),
        (_purge_loop,  None),          # _purge_loop handles its own sleep
    ]:
        fn   = target if interval is None else (lambda t=target, i=interval: _run_every(i, t))
        name = target.__name__
        t = threading.Thread(target=fn, name=name, daemon=True)
        t.start()
        log.info("Collector thread started: %s (interval=%s s)", name, interval)
