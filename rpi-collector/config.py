"""
config.py — Smart Aquarium Collector Configuration
Edit this file to match your network.
"""

# ── ESP32 ──────────────────────────────────────────────────────────────────
ESP32_IP   = "192.168.29.7"
ESP32_PORT = 82   # Dedicated collector API server (Core 0 / emailLoop)

# HTTP Basic Auth credentials (only used if the caller IP is NOT in the
# ESP32 allowedIPs list; the RPi IP 192.168.29.3 is already whitelisted
# so these won't normally be needed, but set them for safety).
ESP32_USER = "armaan"
ESP32_PASS = "Armaan1234"

# ── Polling intervals ──────────────────────────────────────────────────────
POLL_STATUS_INTERVAL = 60   # seconds  — temperature + relay/error/uptime
POLL_LOGS_INTERVAL   = 60   # seconds  — system logs

# ── Data retention ─────────────────────────────────────────────────────────
RETENTION_DAYS = 30          # purge records older than this

# ── Collector API server ───────────────────────────────────────────────────
API_HOST = "0.0.0.0"
API_PORT = 5050

# ── Database ───────────────────────────────────────────────────────────────
DB_PATH = "/data/aquarium.db"

# ── Request timeout ────────────────────────────────────────────────────────
REQUEST_TIMEOUT = 10   # seconds
