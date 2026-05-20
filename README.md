# Automated ESP Aquarium Controller

This project is an ESP32-based aquarium control system that automates and monitors key aquarium equipment from a local web interface.

## Features

- **Web Dashboard + WebSocket Live Updates**: Monitor relay states and sensor data in near real-time
- **4 Relay Outputs**:
  - Relay 1: Wave Maker
  - Relay 2: Light (supports startup color-cycle toggle sequence, linked with Relay 4)
  - Relay 3: Air Pump
  - Relay 4: Secondary Light Control (synchronized with Relay 2)
- **Regular Schedules**: Day-of-week recurring ON/OFF schedules with conflict detection
- **Temporary Schedules**: One-time schedules that are auto-removed after execution (up to 2 per relay)
- **Dual Temperature Monitoring**:
  - Internal DS18B20 sensor
  - External DS18B20 sensor
- **Sensor Calibration**: Adjustable internal and external temperature offsets saved to EEPROM
- **Physical Manual Overrides**:
  - Switch 1: Overrides Relay 1 and Relay 3
  - Switch 2: Overrides Relay 2
- **Event Logging**: Logs are persisted in LittleFS and viewable from the logs page
- **Email Notifications**:
  - Startup notification
  - Periodic status-check emails
  - Sensor error alerts
- **Error Indication**: Dedicated LED indicates active error state
- **NTP Time Synchronization**: NTP-based clock with retry logic

## Hardware Requirements

- ESP32 development board
- 4-channel relay module
- 2x DS18B20 temperature sensors (internal + external)
- 2 switches for manual override
- 1 status/error LED
- Power supply
- Aquarium equipment to control (Wave Maker, Lights, Air Pump)

## Wiring

- Relay 1 (Wave Maker): GPIO18
- Relay 2 (Light): GPIO19
- Relay 3 (Air Pump): GPIO21
- Relay 4 (Secondary Light Control): GPIO25
- Switch 1 (Override 1): GPIO23
- Switch 2 (Override 2): GPIO22
- Error LED: GPIO2
- Internal Temperature Sensor (DS18B20): GPIO26
- External Temperature Sensor (DS18B20): GPIO27

## Installation

1. Clone this repository
2. Open the project in Arduino IDE or PlatformIO
3. Install required libraries:
   - WiFi
   - WebServer
   - WebSocketsServer
   - WiFiUDP
   - ArduinoJson
   - EEPROM
   - LittleFS
   - ReadyMail
   - WiFiClientSecure
   - OneWire
   - DallasTemperature
   - TimeLib
   - Ticker
4. Configure your WiFi credentials and email settings in the code
5. Upload the code to your ESP32
6. Access the web interface via the ESP32's IP address

## Configuration

Edit these parameters in the code to match your setup:

```cpp
const char* ssid = "Your_WiFi_SSID";
const char* password = "Your_WiFi_Password";
const char* emailSenderAccount = "your_email@gmail.com";
const char* emailSenderPassword = "your_app_password";
const char* emailRecipient = "recipient_email@example.com";
const char* authUsername = "admin";
const char* authPassword = "your_password";
```

Also review these deployment settings in the sketch:
- Allowed IP list (`allowedIPs`)
- Sensor calibration defaults (`sensorCalibration`)
- NTP and timezone offsets
- Relay and switch pin mapping

## Usage

### Web Interface

Access the web interface by navigating to the ESP32's IP address in a web browser. From here you can:
- Control equipment manually
- View internal and external temperatures
- Calibrate both sensors
- Create and manage schedules
- Set up temporary one-time schedules
- View system logs
- View raw sensor readings
- Clear error states

### Scheduling Options

#### Regular Schedules
- Create schedules that run on specified days of the week
- Set start and end times for each schedule
- Enable/disable schedules as needed
- Conflicting schedules for the same relay/day window are rejected
- Saved to EEPROM so they persist across restart

#### Temporary Schedules
- Create one-time schedules that automatically expire after execution
- Set either start time, end time, or both
- Each relay can have up to 2 temporary schedules at a time
- Stored in RAM (not persisted after reboot)

### Manual Overrides

- Toggle Switch 1 to override and force-enable Wave Maker + Air Pump (Relay 1 and 3)
- Toggle Switch 2 to override and force-enable Light (Relay 2)
- Release the switch to remove override and return control to schedule/manual logic

### Monitoring and Alerts

- Startup schedule reconciliation runs once after time sync
- Internal temperature updates every 20 seconds
- External temperature updates every 60 seconds
- Status checks run every second for schedule execution
- Status-check emails are sent periodically after startup

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.