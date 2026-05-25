"""
db.py — Database initialisation and helper utilities
"""
import sqlite3
import logging
from config import DB_PATH

log = logging.getLogger(__name__)


def get_conn() -> sqlite3.Connection:
    conn = sqlite3.connect(DB_PATH, check_same_thread=False)
    conn.row_factory = sqlite3.Row
    conn.execute("PRAGMA journal_mode=WAL")
    conn.execute("PRAGMA foreign_keys=ON")
    return conn


def init_db():
    """Create tables if they don't already exist."""
    with get_conn() as conn:
        conn.executescript("""
            CREATE TABLE IF NOT EXISTS status_readings (
                id             INTEGER PRIMARY KEY AUTOINCREMENT,
                collected_at   TEXT    NOT NULL,          -- ISO-8601 UTC
                esp32_time     TEXT,                      -- timestamp from ESP32 (may be null)
                internal_c     REAL,
                external_c     REAL,
                relay1         INTEGER NOT NULL DEFAULT 0,
                relay2         INTEGER NOT NULL DEFAULT 0,
                relay3         INTEGER NOT NULL DEFAULT 0,
                override1      INTEGER NOT NULL DEFAULT 0,
                override2      INTEGER NOT NULL DEFAULT 0,
                has_error      INTEGER NOT NULL DEFAULT 0,
                temp_error     INTEGER NOT NULL DEFAULT 0,
                ext_temp_error INTEGER NOT NULL DEFAULT 0,
                uptime_seconds INTEGER,
                uptime_days    INTEGER,
                time_synced    INTEGER NOT NULL DEFAULT 0
            );

            CREATE TABLE IF NOT EXISTS log_entries (
                id           INTEGER PRIMARY KEY AUTOINCREMENT,
                collected_at TEXT    NOT NULL,
                esp32_id     INTEGER,
                esp32_time   TEXT,
                message      TEXT    NOT NULL
            );

            CREATE INDEX IF NOT EXISTS idx_status_collected
                ON status_readings(collected_at);

            CREATE INDEX IF NOT EXISTS idx_logs_collected
                ON log_entries(collected_at);
        """)
    log.info("Database initialised at %s", DB_PATH)


def purge_old_records(retention_days: int):
    """Delete rows older than retention_days from both tables."""
    cutoff = f"datetime('now', '-{retention_days} days')"
    with get_conn() as conn:
        conn.execute(f"DELETE FROM status_readings WHERE collected_at < {cutoff}")
        conn.execute(f"DELETE FROM log_entries    WHERE collected_at < {cutoff}")
    log.debug("Purge complete (retention=%d days)", retention_days)
