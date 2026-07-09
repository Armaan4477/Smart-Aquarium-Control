# Smart Aquarium – RPi Collector

![Platform](https://img.shields.io/badge/platform-Docker-blue.svg)

This directory contains a self-contained Docker service intended to run on a Raspberry Pi or similar local server. It routinely polls the ESP32 controller every 60 seconds and stores all readings in a local SQLite database. It also exposes a REST API on port **5050** for querying historical data from any device on your LAN and acts as a secure reverse-proxy to serve the ESP32 WebUI without cross-origin issues.

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
| `ESP32_USER` | `Admin` | HTTP Basic Auth username (default unless changed in ESP32 WebUI) |
| `ESP32_PASS` | `Admin` | HTTP Basic Auth password (default unless changed in ESP32 WebUI) |
| `POLL_STATUS_INTERVAL` | `60` | Seconds between temperature, humidity + relay status polls |
| `POLL_LOGS_INTERVAL` | `60` | Seconds between retrieving system logs |
| `POLL_ERROR_INTERVAL` | `30` | Seconds between error checks & email alerts |
| `PING_INTERVAL` | `10` | Seconds between health check pings |
| `RETENTION_DAYS` | `30` | Records older than this are automatically purged |
| `API_PORT` | `5050` | LAN port for the historical query API |
| `SMTP_HOST` | `smtp.gmail.com`| SMTP server address for RPi email notifications |
| `SMTP_PORT` | `465` | SMTP server port |
| `EMAIL_SENDER_ACCOUNT`| (empty) | Your sender email address |
| `EMAIL_SENDER_PASSWORD`| (empty) | Your app-specific email password |
| `EMAIL_RECIPIENT`| (empty) | Destination email address for alerts |
| `EMAIL_SUBJECT` | `Aquarium Control Logs` | Subject line for notification emails |

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
| `GET` | `/temperature/latest` | Most recent reading (temps, humidity + relays + errors) |
| `GET` | `/temperature?limit=N&since=ISO` | Historical readings |
| `GET` | `/temperature/range?hours=N` | Min / max / avg temps & humidity over last N hours |

**Example response – `/temperature/latest`**
```json
{
  "id": 142,
  "collected_at": "2026-05-25T13:55:00Z",
  "esp32_time": "25/05/2026 19:25:00",
  "internal_c": 26.50,
  "external_c": 24.10,
  "external_hum": 60.5,
  "relay1": 1,
  "relay2": 1,
  "relay3": 0,
  "override1": 0,
  "override2": 0,
  "active_errors": 0,
  "acknowledged_errors": 0,
  "uptime_seconds": 86400,
  "uptime_days": 1,
  "time_synced": 1,
  "is_offline": false,
  "uptime_pending": false,
  "docker_disabled": false
}
```

*Note: If a sensor is in an active error state, `internal_c`, `external_c`, or `external_hum` will be returned as `null` to prevent displaying stale data.*

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
| `GET` | `/api/maintenance` | Return maintenance mode status (active state and until time) |
| `POST`| `/api/maintenance` | Update maintenance mode (start or stop) |
| `ANY` | `/proxy/<path>` | Proxies ESP32 WebUI, dynamically rewriting HTML/JS to bypass Chrome's Private Network Access (PNA) restrictions |

---

## ESP32 Integration

The data collector interacts with two dedicated endpoints on the ESP32 firmware (`automation.ino`) for background data aggregation:
- `GET /api/status`: Returns calibrated temps, humidity, relay states, override flags, error bitmasks (`active_errors`, `acknowledged_errors`) and time sync state.
- `GET /api/logs`: Returns system logs.

Additionally, the `/proxy/<path>` endpoint seamlessly routes all interactive WebUI features—such as firmware OTA updates, scheduled reboots, and configuration restores—directly to the ESP32. This ensures all management tasks can be performed securely through the Docker host without triggering browser CORS or Private Network Access (PNA) blocks.

**Security**: Ensure your Raspberry Pi's IP address (e.g., `192.168.29.3`) is included in the `allowedIPs` whitelist within the ESP32 code so the collector can poll without authentication. If the IP is not whitelisted, the collector will use the `ESP32_USER` and `ESP32_PASS` credentials configured in `config.py` for HTTP Basic Authentication.
