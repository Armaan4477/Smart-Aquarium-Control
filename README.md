# Automated ESP32 Aquarium Controller

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-green.svg)
![Framework](https://img.shields.io/badge/framework-Arduino-orange.svg)

A comprehensive, ESP32-based aquarium automation system designed to monitor and control essential aquarium equipment via a robust local web interface. This project ensures your aquatic ecosystem remains stable, providing real-time data, scheduled automation, and alerting mechanisms.

## Key Features

- **Responsive Web Dashboard**: Manage your aquarium from any device with WebSocket-powered near real-time updates for relay states and sensor data.
- **Advanced Scheduling System**:
  - **Regular Schedules**: Recurring day-of-week ON/OFF schedules with automatic conflict detection, safely stored in EEPROM.
  - **Temporary Schedules**: One-time, auto-expiring schedules (up to 2 per relay) for ad-hoc equipment control.
- **Precision Dual-Temperature Monitoring**:
  - Internal and External DS18B20 sensors ensure water and ambient temperatures are constantly tracked.
  - Software-based sensor calibration with offsets saved to EEPROM for high accuracy.
- **4-Channel Relay Control**:
  - **Relay 1**: Wave Maker
  - **Relay 2**: Main Light (supports startup color-cycle sequences)
  - **Relay 3**: Air Pump
  - **Relay 4**: Secondary Light Control (synchronized with Relay 2)
- **Local OLED Display**: 128x64 I2C OLED screen for at-a-glance status monitoring, complete with customizable display schedules and physical override modes to prevent light pollution at night.
- **Physical Manual Overrides**: Hardware switches to temporarily bypass automation:
  - **Switch 1**: Overrides Wave Maker (Relay 1) & Air Pump (Relay 3)
  - **Switch 2**: Overrides Main Light (Relay 2 & 4)
- **Comprehensive Logging & Alerts**:
  - Persistent event logging stored on LittleFS.
  - Automated email notifications for system startups, periodic status checks, and sensor errors.
  - Dedicated hardware LED for immediate visual error indication.
- **Robust Timekeeping**: Automatic NTP time synchronization with built-in retry logic.

---

## Hardware Requirements

- **Microcontroller**: ESP32 Development Board
- **Relays**: 4-Channel Relay Module (5V/3.3V compatible)
- **Sensors**: 2x DS18B20 Temperature Sensors (Waterproof recommended for internal)
- **Display**: 128x64 I2C OLED Display (SSD1306)
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
| **Internal Temp Sensor** | `GPIO 26` | Requires 4.7kΩ pull-up resistor |
| **External Temp Sensor** | `GPIO 27` | Requires 4.7kΩ pull-up resistor |
| **OLED SDA** | `GPIO 21` | I2C Data |
| **OLED SCL** | `GPIO 22` | I2C Clock |

---

## Installation & Setup

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/yourusername/Smart-Aquarium-Control.git
   ```
2. **Open the Project**: Load the `automation` folder in the Arduino IDE or PlatformIO.
3. **Install Dependencies**: Ensure the following libraries are installed:
   - `WiFi`, `WebServer`, `WebSocketsServer`, `WiFiUDP`
   - `ArduinoJson`, `EEPROM`, `LittleFS`, `WiFiClientSecure`
   - `ReadyMail` (for SMTP emails)
   - `OneWire`, `DallasTemperature`
   - `TimeLib`, `Ticker`
   - `Adafruit GFX Library`, `Adafruit SSD1306` (for OLED)
4. **Configure Credentials**: Update the following variables in `automation.ino`:
   ```cpp
   const char* ssid = "Your_WiFi_SSID";
   const char* password = "Your_WiFi_Password";
   const char* emailSenderAccount = "your_email@gmail.com";
   const char* emailSenderPassword = "your_app_password";
   const char* emailRecipient = "recipient_email@example.com";
   const char* authUsername = "admin";
   const char* authPassword = "your_password";
   ```
5. **Review Deployment Settings**: 
   - Update the `allowedIPs` list to match your local network devices.
   - Adjust sensor addresses (`sensorAddress`, `externalSensorAddress`).
6. **Upload**: Flash the code to your ESP32.
7. **Access**: Navigate to the ESP32's assigned IP address in your web browser.

---

## Usage Guide

### Web Interface
The intuitive web dashboard provides complete control over your aquarium. Navigate through dedicated pages to:
- **Dashboard**: View live sensor readings and manually toggle equipment.
- **Schedules**: Create and manage recurring weekly schedules.
- **Temp Schedules**: Set up one-time, expiring temporary schedules.
- **Temp Control**: Monitor raw temperature data and calibrate internal/external sensors.
- **Display Control**: Configure OLED screen behavior, including operating hours and manual overrides (Always On / Always Off / Schedule).
- **System Logs**: Review historical events, errors, and system warnings.

### Physical Manual Overrides
Physical switches allow you to instantly override automated schedules without accessing the web interface:
- **Switch 1**: Force-enables the Wave Maker & Air Pump (Relays 1 & 3).
- **Switch 2**: Force-enables the Main Light (Relay 2).
Releasing the switch immediately returns control to the scheduling engine.

### Monitoring and Alerts
- **Startup Sequence**: Reconciles current time with schedules upon successful NTP sync.
- **Sensor Polling**: Internal temps update every 20s, external every 60s.
- **Health Checks**: Schedule verification runs every second.
- **Email Reporting**: Automated status emails are dispatched periodically and upon critical sensor failures.

---

## License

This project is open-source and available under the [MIT License](LICENSE).