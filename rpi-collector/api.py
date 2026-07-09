"""
api.py — Flask REST API for querying stored aquarium data
Runs on port 5050 (configurable in config.py).
"""
import datetime
import logging
import re

from flask import Flask, jsonify, request, abort, Response
import db
import collector
import config
import requests

log = logging.getLogger(__name__)
app = Flask(__name__, static_folder="static", static_url_path="/static")

# ── frontend & proxy ───────────────────────────────────────────────────────

@app.route("/")
def index():
    return app.send_static_file("index.html")

@app.route("/api/config")
def get_config():
    """Return frontend-needed configuration."""
    return jsonify({"esp32_ip": config.ESP32_IP})


def _rewrite_esp32_html(html: str) -> str:
    """
    Rewrite relative URL references in ESP32-served HTML so that all
    internal navigation and fetch() calls route through /proxy/ instead of
    hitting the ESP32 directly from the browser.

    This is necessary because the Docker WebUI is served over HTTPS via a
    reverse proxy, and Chrome's Private Network Access (PNA) policy blocks
    direct browser-to-ESP32 (192.168.x.x) connections from secure contexts.
    By proxying page content and rewriting URLs, every request stays within
    the Docker host's network — no PNA preflight issues.
    """
    esp32_base = f"http://{config.ESP32_IP}"

    # 1. Inject <base> tag so any URLs we miss still resolve via proxy origin
    #    (placed right after <head> as a safety net).
    html = re.sub(
        r'(<head[^>]*>)',
        r'\1<base href="/proxy/">',
        html, count=1, flags=re.IGNORECASE
    )

    # 2. Rewrite absolute ESP32 URLs (e.g. http://192.168.x.x/path) → /proxy/path
    html = html.replace(esp32_base, '/proxy')

    # 3. Rewrite JS navigation patterns:
    #    window.location.href = '/path'  →  window.location.href = '/proxy/path'
    html = re.sub(
        r"(window\.location\.href\s*=\s*['\x22])/((?!proxy/)[^'\x22]*)",
        r'\g<1>/proxy/\2',
        html
    )

    # 4. Rewrite fetch('/path', ...) → fetch('/proxy/path', ...)
    html = re.sub(
        r"(fetch\(['\x22])/((?!proxy/)[^'\x22]*)",
        r'\g<1>/proxy/\2',
        html
    )

    # 5. Rewrite href="/path" attributes → href="/proxy/path"
    html = re.sub(
        r'(href=["\'])/((?!proxy/)[^"\'])',
        r'\g<1>/proxy/\2',
        html
    )

    # 6. Rewrite action="/path" form attributes → action="/proxy/path"
    html = re.sub(
        r'(action=["\'])/((?!proxy/)[^"\'])',
        r'\g<1>/proxy/\2',
        html
    )

    # 7. Rewrite src="/path" attributes → src="/proxy/path"
    html = re.sub(
        r'(src=["\'])/((?!proxy/)[^"\'])',
        r'\g<1>/proxy/\2',
        html
    )

    return html


@app.route("/proxy/<path:subpath>", methods=["GET", "POST", "DELETE"])
def proxy(subpath):
    """Forward requests to the ESP32 main web server (port 80).

    For HTML page responses the body is rewritten so that all relative
    internal links and fetch() API calls are redirected back through this
    proxy endpoint — preventing browser Private Network Access (PNA) blocks
    that occur when the WebUI is served over HTTPS.
    """
    esp32_url = f"http://{config.ESP32_IP}:80/{subpath}"
    try:
        if request.method == "GET":
            resp = requests.get(esp32_url, params=request.args, timeout=10)
        elif request.method == "POST":
            # Forward both JSON and form data
            if request.is_json:
                resp = requests.post(esp32_url, json=request.json, timeout=10)
            else:
                resp = requests.post(esp32_url, data=request.form, timeout=10)
            
            # Immediately poll the ESP32 status after any POST so the local DB is up-to-date
            # for any subsequent frontend refresh.
            if resp.ok:
                try:
                    collector._poll_status()
                except Exception as e:
                    log.error(f"Force poll failed: {e}")
        elif request.method == "DELETE":
            resp = requests.delete(esp32_url, params=request.args, timeout=10)

        content_type = resp.headers.get('Content-Type', '')

        # The ESP32 does not typically specify a charset, so requests defaults to ISO-8859-1.
        # Force UTF-8 so characters like the degree symbol (°) render correctly.
        resp.encoding = 'utf-8'

        # For HTML responses rewrite URLs so all sub-requests stay in-proxy.
        if 'text/html' in content_type:
            rewritten = _rewrite_esp32_html(resp.text)
            return Response(rewritten, status=resp.status_code, content_type='text/html; charset=utf-8')

        # For JSON responses return a parsed jsonify.
        try:
            return jsonify(resp.json()), resp.status_code
        except ValueError:
            return resp.text, resp.status_code
            
    except requests.exceptions.RequestException as e:
        log.error(f"Proxy error to {esp32_url}: {e}")
        return jsonify({"error": "ESP32 unreachable or timed out."}), 502

@app.post("/api/force_refresh")
def force_refresh():
    """Manually force a fresh poll of status and logs from ESP32."""
    try:
        collector._poll_status()
        collector._poll_logs()
        return jsonify({"status": "ok"})
    except Exception as e:
        log.error(f"Force refresh failed: {e}")
        return jsonify({"error": str(e)}), 500

@app.get("/api/email_config")
def get_email_config():
    """Return email notification configuration."""
    is_enabled = db.get_state("email_enabled", "1")
    return jsonify({"email_enabled": is_enabled == "1"})

@app.post("/api/email_config")
def set_email_config():
    """Update email notification configuration."""
    data = request.json
    if data and "email_enabled" in data:
        db.set_state("email_enabled", "1" if data["email_enabled"] else "0")
    return jsonify({"status": "ok"})


@app.get("/api/maintenance")
def get_maintenance():
    """Return maintenance mode status."""
    until = db.get_state("maintenance_mode_until")
    if until:
        try:
            until_dt = datetime.datetime.strptime(until, "%Y-%m-%dT%H:%M:%SZ")
            if datetime.datetime.utcnow() < until_dt:
                return jsonify({"active": True, "until": until})
        except ValueError:
            pass
    return jsonify({"active": False, "until": None})

@app.post("/api/maintenance")
def set_maintenance():
    """Update maintenance mode."""
    data = request.json
    if not data:
        return jsonify({"error": "Invalid request"}), 400
    
    action = data.get("action")
    if action == "stop":
        db.set_state("maintenance_mode_until", "")
        return jsonify({"active": False, "until": None})
    elif action == "start":
        hours = _parse_int(data.get("hours"), default=0, min_v=0, max_v=72)
        minutes = _parse_int(data.get("minutes"), default=0, min_v=0, max_v=59)
        
        if hours == 0 and minutes == 0:
            return jsonify({"error": "Duration must be greater than 0"}), 400
            
        duration = datetime.timedelta(hours=hours, minutes=minutes)
        until_dt = datetime.datetime.utcnow() + duration
        until_str = until_dt.strftime("%Y-%m-%dT%H:%M:%SZ")
        db.set_state("maintenance_mode_until", until_str)
        return jsonify({"active": True, "until": until_str})
        
    return jsonify({"error": "Invalid action"}), 400


# ── helpers ────────────────────────────────────────────────────────────────

def _row_to_dict(row) -> dict:
    return dict(row)


def _parse_int(val, default: int, min_v: int = 1, max_v: int = 1000) -> int:
    try:
        return max(min_v, min(max_v, int(val)))
    except (TypeError, ValueError):
        return default


# ── status & temperature ────────────────────────────────────────────────────

@app.get("/temperature")
def get_temperature():
    """
    Return historical temperature readings.
    Query params:
      limit=N        max rows (default 100, max 1000)
      since=ISO      only rows after this UTC datetime (e.g. 2026-05-01T00:00:00Z)
    """
    limit = _parse_int(request.args.get("limit"), default=100, max_v=100000)
    since = request.args.get("since")

    query = "SELECT * FROM status_readings"
    params: list = []

    if since:
        query += " WHERE collected_at > ?"
        params.append(since)

    query += " ORDER BY collected_at DESC LIMIT ?"
    params.append(limit)

    with db.get_conn() as conn:
        rows = conn.execute(query, params).fetchall()

    return jsonify([_row_to_dict(r) for r in rows])


@app.get("/temperature/latest")
def get_temperature_latest():
    """Return the single most recent status reading."""
    with db.get_conn() as conn:
        row = conn.execute(
            "SELECT * FROM status_readings ORDER BY collected_at DESC LIMIT 1"
        ).fetchone()
    if not row:
        return jsonify({"error": "No data yet"}), 404
    
    result = _row_to_dict(row)
    
    # If sensors are in an error state, do not return the stale temperature/humidity
    active_errs = result.get("active_errors") or 0
    if active_errs & (1 << 2):
        result["internal_c"] = None
    if active_errs & (1 << 3):
        result["external_c"] = None
        result["external_hum"] = None

    result["is_offline"]     = collector.uptime_state.get("is_offline", False)
    result["uptime_pending"] = collector.uptime_state.get("uptime_pending", False)
    result["docker_disabled"] = collector.uptime_state.get("docker_disabled", False)
    return jsonify(result)


@app.get("/temperature/range")
def get_temperature_range():
    """
    Return min/max/avg temperatures and humidity over a time window.
    Query params:
      hours=N    look-back window in hours (default 24, max 720)
    """
    hours = _parse_int(request.args.get("hours"), default=24, min_v=1, max_v=720)
    since = (datetime.datetime.utcnow() - datetime.timedelta(hours=hours)).strftime(
        "%Y-%m-%dT%H:%M:%SZ"
    )
    with db.get_conn() as conn:
        row = conn.execute(
            """SELECT
                   COUNT(*)          AS count,
                   MIN(internal_c)   AS internal_min,
                   MAX(internal_c)   AS internal_max,
                   AVG(internal_c)   AS internal_avg,
                   MIN(external_c)   AS external_min,
                   MAX(external_c)   AS external_max,
                   AVG(external_c)   AS external_avg,
                   MIN(external_hum) AS humidity_min,
                   MAX(external_hum) AS humidity_max,
                   AVG(external_hum) AS humidity_avg
               FROM status_readings
               WHERE collected_at > ?""",
            (since,),
        ).fetchone()
    result = _row_to_dict(row)
    result["window_hours"] = hours
    # round averages
    for key in ("internal_avg", "external_avg", "humidity_avg"):
        if result.get(key) is not None:
            result[key] = round(result[key], 2)
    return jsonify(result)


# ── relay & errors ─────────────────────────────────────────────────────────

@app.get("/relays")
def get_relays():
    """
    Return historical relay + override + error state snapshots.
    Query params:
      limit=N    (default 100)
      since=ISO
    """
    limit = _parse_int(request.args.get("limit"), default=100)
    since = request.args.get("since")

    query  = """SELECT collected_at, esp32_time,
                       relay1, relay2, relay3,
                       override1, override2,
                       active_errors, acknowledged_errors,
                       uptime_seconds, uptime_days, time_synced
                FROM status_readings"""
    params: list = []
    if since:
        query += " WHERE collected_at > ?"
        params.append(since)
    query += " ORDER BY collected_at DESC LIMIT ?"
    params.append(limit)

    with db.get_conn() as conn:
        rows = conn.execute(query, params).fetchall()
    return jsonify([_row_to_dict(r) for r in rows])


@app.get("/relays/latest")
def get_relays_latest():
    """Return the current relay / override / error / uptime snapshot."""
    with db.get_conn() as conn:
        row = conn.execute(
            """SELECT collected_at, esp32_time,
                      relay1, relay2, relay3,
                      override1, override2,
                      active_errors, acknowledged_errors,
                      uptime_seconds, uptime_days, time_synced
               FROM status_readings ORDER BY collected_at DESC LIMIT 1"""
        ).fetchone()
    if not row:
        return jsonify({"error": "No data yet"}), 404
    
    result = _row_to_dict(row)
    result["is_offline"]     = collector.uptime_state.get("is_offline", False)
    result["uptime_pending"] = collector.uptime_state.get("uptime_pending", False)
    return jsonify(result)


# ── logs ───────────────────────────────────────────────────────────────────

@app.get("/logs")
def get_logs():
    """
    Return stored log entries.
    Query params:
      limit=N      (default 100, max 1000)
      since=ISO
      search=TEXT  case-insensitive substring filter on message
    """
    limit  = _parse_int(request.args.get("limit"), default=100)
    since  = request.args.get("since")
    search = request.args.get("search", "").strip()

    clauses: list[str] = []
    params:  list      = []

    if since:
        clauses.append("collected_at > ?")
        params.append(since)
    if search:
        clauses.append("LOWER(message) LIKE ?")
        params.append(f"%{search.lower()}%")

    where = ("WHERE " + " AND ".join(clauses)) if clauses else ""
    query = f"SELECT * FROM log_entries {where} ORDER BY collected_at DESC LIMIT ?"
    params.append(limit)

    with db.get_conn() as conn:
        rows = conn.execute(query, params).fetchall()
    return jsonify([_row_to_dict(r) for r in rows])


# ── collector health ───────────────────────────────────────────────────────

@app.get("/health")
def get_health():
    """
    Collector health — last poll times, DB row counts, config summary.
    """
    with db.get_conn() as conn:
        status_count = conn.execute("SELECT COUNT(*) FROM status_readings").fetchone()[0]
        log_count    = conn.execute("SELECT COUNT(*) FROM log_entries").fetchone()[0]
        oldest_status = conn.execute(
            "SELECT MIN(collected_at) FROM status_readings"
        ).fetchone()[0]
        oldest_log = conn.execute(
            "SELECT MIN(collected_at) FROM log_entries"
        ).fetchone()[0]

    return jsonify({
        "esp32_ip":              config.ESP32_IP,
        "poll_status_interval":  config.POLL_STATUS_INTERVAL,
        "poll_logs_interval":    config.POLL_LOGS_INTERVAL,
        "retention_days":        config.RETENTION_DAYS,
        "db_status_rows":        status_count,
        "db_log_rows":           log_count,
        "oldest_status_reading": oldest_status,
        "oldest_log_entry":      oldest_log,
        "last_status_poll":      collector.last_status_poll,
        "last_logs_poll":        collector.last_logs_poll,
    })


# ── entry point (used by main.py / gunicorn) ────────────────────────────────

if __name__ == "__main__":
    app.run(host=config.API_HOST, port=config.API_PORT, debug=False)
