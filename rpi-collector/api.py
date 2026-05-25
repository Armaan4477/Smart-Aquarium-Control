"""
api.py — Flask REST API for querying stored aquarium data
Runs on port 5050 (configurable in config.py).
"""
import datetime
import logging

from flask import Flask, jsonify, request, abort
import db
import collector
import config

log = logging.getLogger(__name__)
app = Flask(__name__, static_folder="static", static_url_path="/static")

# ── frontend ───────────────────────────────────────────────────────────────

@app.route("/")
def index():
    return app.send_static_file("index.html")



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
    limit = _parse_int(request.args.get("limit"), default=100)
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
    return jsonify(_row_to_dict(row))


@app.get("/temperature/range")
def get_temperature_range():
    """
    Return min/max/avg temperatures over a time window.
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
                   AVG(external_c)   AS external_avg
               FROM status_readings
               WHERE collected_at > ?""",
            (since,),
        ).fetchone()
    result = _row_to_dict(row)
    result["window_hours"] = hours
    # round averages
    for key in ("internal_avg", "external_avg"):
        if result[key] is not None:
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
                       has_error, temp_error, ext_temp_error,
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
                      has_error, temp_error, ext_temp_error,
                      uptime_seconds, uptime_days, time_synced
               FROM status_readings ORDER BY collected_at DESC LIMIT 1"""
        ).fetchone()
    if not row:
        return jsonify({"error": "No data yet"}), 404
    return jsonify(_row_to_dict(row))


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
