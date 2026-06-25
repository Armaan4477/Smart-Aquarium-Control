# Smart Aquarium Control System

![License](https://img.shields.io/badge/license-MIT-blue.svg)

A comprehensive, dual-component aquarium automation and monitoring system. This project ensures your aquatic ecosystem remains stable by providing real-time equipment control, scheduled automation, localized alerting mechanisms and long-term historical data collection.

## Project Structure

This repository is split into two primary components:

1. **[ESP32 Controller (`/automation`)](./automation/README.md)**
   The core hardware controller built on the ESP32 platform. It handles all real-time sensor monitoring (temperature), equipment scheduling (wave makers, lights, air pumps), physical overrides and serves the local web-based control dashboard.

2. **[Raspberry Pi Collector (`/rpi-collector`)](./rpi-collector/README.md)**
   A complementary data logging service running on a Raspberry Pi via Docker. It routinely polls the ESP32 for sensor readings and system states, storing them in a local SQLite database and providing a RESTful API for historical data querying.

## Key System Features

- **Automated Equipment Scheduling:** Manage wave makers, lights and air pumps through weekly recurring and temporary one-time schedules.
- **Precision Temperature Monitoring:** Dual-sensor setup for tracking both water and ambient temperatures.
- **Robust Hardware Controls:** Local OLED status display and physical override switches for immediate, web-free access.
- **Historical Data Logging:** Automated data collection every 60 seconds, stored in an SQLite database for trend analysis.
- **Reliable Alerting:** Automated email notifications for system startups, sensor failures and periodic health checks.
- **Local Network Independence:** Operates entirely over your local network without reliance on external cloud services.

## Getting Started

Please refer to the specific component documentation for installation and setup instructions:

- **Hardware & ESP32 Setup:** See the [ESP32 Controller Documentation](./automation/README.md)
- **Data Collector Setup:** See the [Raspberry Pi Collector Documentation](./rpi-collector/README.md)

---

## License

This project is open-source and available under the [MIT License](LICENSE).