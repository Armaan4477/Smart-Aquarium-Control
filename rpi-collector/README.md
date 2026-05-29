# Smart Aquarium – RPi Collector

A self-contained Docker service that runs on your Raspberry Pi, polls the ESP32 every 60 seconds, and stores all readings in a local SQLite database. Exposes a REST API on port **5050** for querying historical data from any device on your LAN.

---

## Quick start

```bash
# On the Raspberry Pi — copy the rpi-collector/ folder across first
cd rpi-collector/

# Build and start (first run downloads the base image, ~takes 2–3 min on a Pi)
docker compose up -d --build

# Watch live logs
docker compose logs -f
```

---

## API endpoints

All endpoints are served from `http://<rpi-ip>:5050`.

### Temperature

| Method | Path | Description |
|---|---|---|
| `GET` | `/temperature/latest` | Most recent reading (temps + relays + errors) |
| `GET` | `/temperature?limit=N&since=ISO` | Historical readings |
| `GET` | `/temperature/range?hours=N` | Min / max / avg over last N hours |

**Example response – `/temperature/latest`**
```json
{
  "id": 142,
  "collected_at": "2026-05-25T13:55:00Z",
  "esp32_time": "25/05/2026 19:25:00",
  "internal_c": 26.50,
  "external_c": 24.10,
  "relay1": 1,
  "relay2": 1,
  "relay3": 0,
  "override1": 0,
  "override2": 0,
  "has_error": 0,
  "temp_error": 0,
  "ext_temp_error": 0,
  "uptime_seconds": 86400,
  "uptime_days": 1,
  "time_synced": 1
}
```

---

### Relays, overrides & errors

| Method | Path | Description |
|---|---|---|
| `GET` | `/relays/latest` | Current relay / override / error snapshot |
| `GET` | `/relays?limit=N&since=ISO` | Historical snapshots |

---

### System logs

| Method | Path | Description |
|---|---|---|
| `GET` | `/logs?limit=N` | Latest N log entries |
| `GET` | `/logs?since=ISO` | Entries after a datetime |
| `GET` | `/logs?search=TEXT` | Substring search on message |

**Example response – `/logs?limit=3`**
```json
[
  {
    "id": 55,
    "collected_at": "2026-05-25T13:50:00Z",
    "esp32_id": 17,
    "esp32_time": "25/05/2026 19:20:00",
    "message": "Relay 2 turned ON by schedule."
  }
]
```

---

### Collector health

| Method | Path | Description |
|---|---|---|
| `GET` | `/health` | Poll times, DB row counts, config |

---

## Configuration

Edit [`config.py`](config.py) before building:

| Key | Default | Notes |
|---|---|---|
| `ESP32_IP` | `192.168.29.7` | Static IP of the ESP32 |
| `POLL_STATUS_INTERVAL` | `60` | Seconds between temperature + relay polls |
| `POLL_LOGS_INTERVAL` | `60` | Seconds between log polls |
| `RETENTION_DAYS` | `30` | Records older than this are auto-purged |
| `API_PORT` | `5050` | LAN port for the query API |

After editing, rebuild: `docker compose up -d --build`

---

## ESP32 endpoints added

Two new endpoints were added to `automation.ino` (nothing else was changed):

| Route | What it returns |
|---|---|
| `GET /api/status` | Calibrated temps, relay states, override flags, error flags, uptime |
| `GET /api/logs` | System logs (same as existing `/logs/data`) |

Both reuse `checkAuthentication()` — the RPi IP `192.168.29.3` is already in the `allowedIPs` whitelist so no credentials are needed from the container.

---

## Data files

The SQLite database is stored in `rpi-collector/data/aquarium.db` (bind-mounted into the container). It survives container restarts and rebuilds. Back it up with:

```bash
cp rpi-collector/data/aquarium.db ~/aquarium-backup-$(date +%F).db
```
