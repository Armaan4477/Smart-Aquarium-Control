# ESP32 Aquarium Controller

![Platform](https://img.shields.io/badge/platform-ESP32-green.svg)
![Framework](https://img.shields.io/badge/framework-Arduino-orange.svg)

This directory contains the firmware for the ESP32-based hardware controller. It handles real-time equipment scheduling, temperature monitoring, manual overrides and serves a robust local web interface.

## Key Features

- **Responsive Web Dashboard**: Manage your aquarium from any device with WebSocket-powered near real-time updates for relay states and sensor data. Fully optimized for offline loading with no external dependencies, features dynamic main page pop-ups for critical system alerts and errors and includes a **Persistent Dark Mode** saved directly to EEPROM.
- **Feeding Mode**: A dedicated 5-minute one-click pause for the wave maker and air pump to allow for calm water during feeding, complete with a live countdown timer on the dashboard.
- **Advanced Scheduling System**:
  - **Regular Schedules**: Recurring day-of-week ON/OFF schedules with automatic conflict detection, safely stored in EEPROM.
  - **Temporary Schedules**: One-time, auto-expiring schedules (up to 2 per relay) for ad-hoc equipment control.
- **Precision Temperature and Humidity Monitoring**:
  - Water temperature sensor (DS18B20) and Ambient temperature/humidity sensor (DHT22).
  - Software-based sensor calibration with offsets saved to EEPROM for high accuracy.
- **4-Channel Relay Control**:
  - **Relay 1**: Wave Maker
  - **Relay 2**: Main Light (supports startup color-cycle sequences)
  - **Relay 3**: Air Pump
  - **Relay 4**: Secondary Light Control (synchronized with Relay 2)
- **Local OLED Display**: 128x64 I2C OLED screen for at-a-glance status monitoring. Features customizable display schedules, physical override modes to prevent light pollution and dynamic error state visualization (displays blinking "E" for active errors and "--" for acknowledged errors).
- **Physical Manual Overrides**: Hardware switches to temporarily bypass automation:
  - **Switch 1**: Overrides Wave Maker (Relay 1) & Air Pump (Relay 3)
  - **Switch 2**: Overrides Main Light (Relay 2 & 4)
- **Comprehensive Logging & Alerts**:
  - Persistent event logging stored on LittleFS.
  - Automated email notifications for system startups, periodic status checks and sensor errors (Email functionality is disabled by default to prevent startup errors until safely configured via the UI).
  - Dedicated hardware LED and WebUI pop-ups for immediate visual error indication.
- **Modular Web Interface**: Web pages are compartmentalized into separate header files (`page_*.h`) for clean code organization and maintainability.
- **Robust Timekeeping**: Automatic NTP time synchronization with built-in retry logic.
- **System Stability**: Optimized FreeRTOS task scheduling to prevent Watchdog Timer (WDT) resets, ensuring continuous and reliable long-term operation.

---

## Hardware Requirements

- **Microcontroller**: ESP32 Development Board
- **Relays**: 4-Channel Relay Module (5V/3.3V compatible)
- **Sensors**: 1x DS18B20 Temperature Sensor (Waterproof recommended for water temperature) and 1x DHT22 Temperature/Humidity Sensor
- **Display**: 128x64 I2C OLED Display (SH1106)
- **Inputs/Outputs**: 
  - 2x Physical Switches (for manual overrides)
  - 1x Status/Error LED (with appropriate current-limiting resistor)
- **Power**: Adequate power supply for the ESP32 and relay board
- **Target Equipment**: Wave Maker, Aquarium Lights, Air Pump

---

## Wiring Diagram

| Component | Pin | Notes |
| :--- | :--- | :--- |
| **Relay 1 (Wave Maker)** | `GPIO 18` | |
| **Relay 2 (Light)** | `GPIO 19` | |
| **Relay 3 (Air Pump)** | `GPIO 23` | |
| **Relay 4 (Secondary Light)**| `GPIO 25` | |
| **Switch 1 (Override 1)** | `GPIO 33` | Connect to GND via switch |
| **Switch 2 (Override 2)** | `GPIO 32` | Connect to GND via switch |
| **Error LED** | `GPIO 2` | |
| **Water Temp Sensor** | `GPIO 26` | Requires 4.7kΩ pull-up resistor |
| **Ambient DHT22 Sensor** | `GPIO 27` | Measures ambient temp & humidity |
| **OLED SDA** | `GPIO 21` | I2C Data |
| **OLED SCL** | `GPIO 22` | I2C Clock |

---

## Installation & Setup

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/yourusername/Smart-Aquarium-Control.git
   ```
2. **Open the Project**: Load this `automation` folder in the Arduino IDE or PlatformIO.
3. **Install Dependencies**: Ensure the following libraries are installed:
   - `WiFi`, `WebServer`, `DNSServer`, `WebSocketsServer`, `WiFiUDP`
   - `ArduinoJson`, `EEPROM`, `LittleFS`, `WiFiClientSecure`
   - `ReadyMail` (for SMTP emails)
   - `OneWire`, `DallasTemperature`
   - `DHT sensor library`, `Adafruit Unified Sensor`
   - `TimeLib`, `Ticker`
   - `Adafruit GFX Library`, `Adafruit SH110X` (for OLED)
   - `Update` (for OTA updates)
4. **Review Deployment Settings**: 
   - Update the `allowedIPs` list to match your local network devices.
   - Adjust sensor addresses (`sensorAddress`, `externalSensorAddress`).
5. **Upload**: Flash the code to your ESP32.
6. **Initial Setup (Wi-Fi Provisioning)**: Connect your device to the ESP32's fallback access point (`ESP32_Aquarium` / `aquarium123`). Navigate to `http://192.168.4.1` in your browser.
7. **Configure Wi-Fi**: Go to the **WiFi Config** page in the Web UI to scan for networks and save your local Wi-Fi credentials. The ESP32 will reboot and connect to your network.
8. **Access**: Once connected to your local network, navigate to the ESP32's assigned IP address in your web browser.

---

## Usage Guide

### Web Interface
The intuitive web dashboard provides complete control over your aquarium (including a built-in Dark Mode toggle). Navigate through dedicated pages to:
- **Dashboard**: View live sensor readings and manually toggle equipment. Relay buttons automatically turn **yellow** to clearly indicate when a physical or manual override is active. Also features a **Feeding Mode** button to temporarily pause water circulation for 5 minutes.
- **Device Settings**: A central hub to manage system configuration, hardware setup, security, and maintenance, including syncing time, scheduled auto-reboots, and factory resets.
- **Schedules**: Create and manage recurring weekly schedules.
- **Temp Schedules**: Set up one-time, expiring temporary schedules.
- **Temp Control**: Monitor raw temperature data and calibrate water/ambient sensors.
- **WiFi Config**: Manage Wi-Fi credentials and connection settings directly from the web interface. Features network scanning and dynamic AP fallback management.
- **Auth Config**: Update Web UI login credentials (default is `Admin` / `Admin`, saved persistently to EEPROM).
- **Email Config**: Manage SMTP email credentials directly from the web interface (saved persistently to EEPROM).
- **NTP Config**: Configure custom time synchronization (NTP) servers and test connectivity directly from the web interface.
- **Docker Config**: Configure connection settings for the Raspberry Pi data collector.
- **Auto Reboot Config**: Configure scheduled automatic system reboots to maintain long-term stability.
- **Display Control**: Configure OLED screen behavior, including operating hours and manual overrides.
- **System Logs**: Review historical events, errors and system warnings.
- **Backup & Restore**: Easily backup and restore the full EEPROM configuration (schedules, settings) to a JSON file. The system will automatically reboot after a successful restore to apply configurations seamlessly.
- **OTA Updates**: Securely perform Over-The-Air (OTA) firmware updates directly from the dashboard without needing physical access to the ESP32. Features an intelligent update-in-progress state (disabling UI elements like the rollback button to prevent errors) and supports **Scheduled Firmware Updates** to safely apply updates automatically at midnight.

### Physical Manual Overrides
Physical switches allow you to instantly override automated schedules without accessing the web interface:
- **Switch 1**: Force-enables the Wave Maker & Air Pump (Relays 1 & 3).
- **Switch 2**: Force-enables the Main Light (Relay 2 & 4).

**Note**: These switches function as temporary overrides. Releasing the switch immediately returns control to the scheduling engine.

### Monitoring and Alerts
- **Startup Sequence**: Reconciles current time with schedules upon successful NTP sync.
- **Sensor Polling**: Water temps update every 20s, ambient every 60s.
- **Health Checks**: Schedule verification runs every second.
- **Email Reporting**: Automated status emails are dispatched periodically and upon critical sensor failures.
- **Fallback Access Point**: If WiFi fails to connect, the ESP32 hosts a fallback AP (`ESP32_Aquarium` / `aquarium123`). This mode provides full Web UI access and allows for safe OTA updates even when the local network is down. When connected to this AP, the Web UI automatically bypasses login authentication for seamless access.
