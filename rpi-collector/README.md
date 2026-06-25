# Smart Aquarium – RPi Collector

![Platform](https://img.shields.io/badge/platform-Docker-blue.svg)

This directory contains a self-contained Docker service intended to run on a Raspberry Pi or similar local server. It routinely polls the ESP32 controller every 60 seconds and stores all readings in a local SQLite database. It also exposes a REST API on port **5050** for querying historical data from any device on your LAN.

---

## Setup & Quick Start

1. **Transfer Files**: Copy this `rpi-collector/` directory to your Raspberry Pi.
2. **Configure Environment**: Edit `config.py` (see [Configuration](#configuration) section below) to point to your ESP32's IP address.
3. **Deploy Docker Container**:
   Navigate into the directory and build the container:
   ```bash
   cd rpi-collector/
   
   # Build and start (first run downloads the base image, ~takes 2–3 min on a Pi)
   docker compose up -d --build
   ```
4. **Monitor Service**:
   ```bash
   # Watch live logs to ensure successful polling
   docker compose logs -f
   ```

---

## Configuration

Before building or running the container, ensure the variables in [`config.py`](config.py) match your network setup:

| Key | Default | Notes |
|---|---|---|
| `ESP32_IP` | `192.168.29.7` | Static IP of the ESP32 Controller |
| `ESP32_PORT` | `82` | Dedicated API server port on ESP32 |
| `POLL_STATUS_INTERVAL` | `60` | Seconds between temperature + relay status polls |
| `POLL_LOGS_INTERVAL` | `60` | Seconds between retrieving system logs |
| `POLL_ERROR_INTERVAL` | `30` | Seconds between error checks & email alerts |
| `PING_INTERVAL` | `10` | Seconds between health check pings |
| `RETENTION_DAYS` | `30` | Records older than this are automatically purged |
| `API_PORT` | `5050` | LAN port for the historical query API |
| `SMTP_HOST` | `smtp.gmail.com`| SMTP server address for RPi email notifications |
| `EMAIL_SENDER_ACCOUNT`| (empty) | Your sender email address |
| `EMAIL_SENDER_PASSWORD`| (empty) | Your app-specific email password |

**Note**: If you modify `config.py` while the container is running, you must rebuild it for changes to take effect: `docker compose up -d --build`

---

## Data Management

The SQLite database is stored in `rpi-collector/data/aquarium.db` (bind-mounted into the container). This ensures your data survives container restarts and image rebuilds. 

It's highly recommended to back up this database periodically:
```bash
cp rpi-collector/data/aquarium.db ~/aquarium-backup-$(date +%F).db
```

---

## REST API Endpoints

The collector exposes a local API for data visualization and querying. All endpoints are served from `http://<rpi-ip>:5050`.

### Temperature & Sensor Data

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
  "time_synced": 1,
  "is_offline": false,
  "uptime_pending": false
}
```

### Relays, Overrides & Errors

| Method | Path | Description |
|---|---|---|
| `GET` | `/relays/latest` | Current relay / override / error snapshot |
| `GET` | `/relays?limit=N&since=ISO` | Historical snapshots |

### System Logs

| Method | Path | Description |
|---|---|---|
| `GET` | `/logs?limit=N` | Latest N log entries |
| `GET` | `/logs?since=ISO` | Entries after a datetime |
| `GET` | `/logs?search=TEXT` | Substring search on message |

### Collector Health & Management

| Method | Path | Description |
|---|---|---|
| `GET` | `/health` | Poll times, DB row counts, configuration state |
| `POST` | `/api/force_refresh` | Manually force a fresh poll of status and logs from ESP32 |
| `GET` | `/api/config` | Frontend configuration (ESP32 IP, etc.) |
| `GET` | `/api/email_config` | Returns whether RPi email notifications are enabled |
| `POST`| `/api/email_config` | Enable or disable RPi email notifications |
| `ANY` | `/proxy/<path>` | Forwards requests to the ESP32 main web server (port 80) |

---

## ESP32 Integration

The data collector interacts with two dedicated endpoints on the ESP32 firmware (`automation.ino`):
- `GET /api/status`: Returns calibrated temps, relay states, override flags, error flags, and uptime.
- `GET /api/logs`: Returns system logs.

**Security**: Ensure your Raspberry Pi's IP address (e.g., `192.168.29.3`) is included in the `allowedIPs` whitelist within the ESP32 code so the collector can poll without authentication.
