# Smart Aquarium Control System

![License](https://img.shields.io/badge/license-MIT-blue.svg)

A comprehensive, dual-component aquarium automation and monitoring system. This project ensures your aquatic ecosystem remains stable by providing real-time equipment control, scheduled automation, localized alerting mechanisms and long-term historical data collection.

## Project Structure

This repository is split into two primary components:

1. **[ESP32 Controller (`/automation`)](./automation/README.md)**
   The core hardware controller built on the ESP32 platform. It handles all real-time sensor monitoring (temperature), equipment scheduling (wave makers, lights, air pumps), physical overrides and serves the local web-based control dashboard.

2. **[Raspberry Pi Collector (`/rpi-collector`)](./rpi-collector/README.md)**
   A complementary data logging service running on a Raspberry Pi via Docker. It routinely polls the ESP32 for sensor readings and system states, stores them in a local SQLite database and provides a RESTful API for historical data querying.

## Key System Features

- **Automated Equipment Scheduling:** Manage wave makers, primary/secondary lights and air pumps through weekly recurring and temporary one-time schedules. Includes a dedicated one-click **Feeding Mode** to temporarily pause water circulation.
- **Maintenance Mode:** Temporarily pause automated background polling and email alerts via the data collector to safely perform tank maintenance without triggering false alarms.
- **Configuration & Firmware Management:** Built-in Backup/Restore functionality with auto-reboot, custom NTP time synchronization, scheduled firmware updates, and robust Over-The-Air (OTA) update support with seamless UI state management for firmware upgrades directly from the dashboard.
- **Wi-Fi Provisioning System:** Built-in fallback Access Point (AP) mode for initial network setup and recovery. Features frictionless, open AP connectivity without forced captive portal redirection, allowing easy Wi-Fi configuration without hardcoding credentials.
- **Persistent Dark Mode:** A modern, customizable Web UI with a built-in dark mode that remembers your preference.
- **Precision Environmental Monitoring:** Dual-sensor setup for tracking water temperature, alongside ambient temperature and humidity (via DHT22).
- **Robust Hardware Controls:** Local OLED status display and physical override switches for immediate, web-free access.
- **Historical Data Logging:** Automated data collection every 60 seconds, stored in an SQLite database for trend analysis.
- **Reliable Alerting & Error Tracking:** Features dynamic UI pop-ups for immediate visual feedback, alongside automated email notifications for system startups, sensor failures and periodic health checks.
- **High Stability:** Optimized FreeRTOS task scheduling ensures reliable long-term execution and prevents Watchdog Timer (WDT) resets.
- **Local Network Independence:** Fully offline-optimized, self-hosted web interface that operates entirely over your LAN without reliance on external cloud services or CDNs.

## Getting Started

Please refer to the specific component documentation for installation and setup instructions:

- **Hardware & ESP32 Setup:** See the [ESP32 Controller Documentation](./automation/README.md)
- **Data Collector Setup:** See the [Raspberry Pi Collector Documentation](./rpi-collector/README.md)

---

## License

This project is open-source and available under the [MIT License](LICENSE).