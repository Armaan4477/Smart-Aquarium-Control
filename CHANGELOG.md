# Smart Aquarium Control - Version History

This document outlines the major version rewrites, feature additions and historical progress of the Smart Aquarium Control project.

---

## v19.x (Jul 2026)

### v19.2 (02 Jul 2026) *(Current)*
- Bug fixes and minor stability enhancements.

### v19.1 (01 Jul 2026)
- Stability improvements.
- General bug fixes.

### v19.0 (01 Jul 2026)
- **Dynamic IP Allowlist**: Replaced hardcoded IP lists with dynamic WebUI IP discovery for better network flexibility.

---

## v18.0 (01 Jul 2026)
- **Dynamic TimeSync System**: Moved time synchronization server to its own dynamic system for enhanced reliability.

---

## v17.0 (30 Jun 2026)
- **Wi-Fi Rewrite**: Comprehensive rework of the Wi-Fi logic for better connectivity.
- **Reset Device**: Added reset functionality from the WebUI.

---

## v16.0 (29 Jun 2026)
- **OTA Rollback**: Added manual rollback capability to the Over-The-Air update system.
- **Manual Time Synchronization**: Users can now trigger time sync manually.
- **Self-hosted AP Mode**: Added fallback Access Point mode for local setup and hosting.
- **Authentication System v2**: Upgraded the authentication mechanism for enhanced security.

---

## v15.0 (28 Jun 2026)
- **OTA Firmware Updates**: Implemented seamless Over-The-Air firmware updates.

---

## v14.x (Jun 2026)

### v14.1 (27 Jun 2026)
- **Dark Theme**: Added a modern dark theme option to the ESP32 WebUI.

### v14.0 (27 Jun 2026)
- **Feeding Mode**: Implemented (and rewritten) the feeding mode logic.

---

## v13.x (Jun 2026)

### v13.1 (26 Jun 2026)
- **WebUI Separation**: Separated HTML pages into their own distinct files for better maintainability.
- **Schedule Editor**: Added detailed logic to edit existing schedules.

### v13.0 (26 Jun 2026)
- **Error Handling Overhaul**: Comprehensive rework of error detection and logging.
- **Maintenance Mode**: Introduced maintenance mode for the Docker container.

---

## v12.0 (25 Jun 2026)
- **Docker Management**: Integrated Docker container support for external deployment.
- **Docker Controls**: Added overall system enable/disable handling from the container.

---

## v11.x (May 2026)

### v11.1 (26 May 2026)
- **Email System Redesign**: Comprehensive rework of email notifications.
- **Dashboard Improvements**: Refined the dashboard interface.

### v11.0 (25 May 2026)
- **Dashboard Interface**: Introduced a new high-level dashboard.

---

## v10.x (May 2026)

### v10.1 (25 May 2026)
- **Docker Support**: Initial Docker support integrated into the project.

### v10.0 (21 May 2026)
- **Local OLED Display**: Added support for an OLED screen to display status and temperature locally.

---

## v9.0 (26 Jul 2025)
- **External Sensor Framework**: Initial setup to support various external sensors in the aquarium.

---

## v8.x (Jul 2025)

### v8.1 (12 Jul 2025)
- **HTML/WebUI Rewrite**: Completely redid all HTML pages for better structure and UI.

### v8.0 (12 Jul 2025)
- **Temporary Schedules**: Logic implemented for running one-off/temporary schedules.

---

## v7.0 (14 Jun 2025)
- **ESP32 Internal RTC**: Migrated timekeeping to use the ESP32's Internal Real-Time Clock for far better time reliability.

---

## v6.x (Feb - Mar 2025)

### v6.1 (14 Mar 2025)
- **Temperature Sensor**: Implemented temperature sensor reading logic.

### v6.0 (24 Feb 2025)
- **Email Notifications**: System added to email logs and critical alerts.

---

## v5.x (Jan 2025)

### v5.2 (07 Jan 2025)
- **Automatic Logging**: System logs data periodically (every 90 mins).
- **Timestamp Improvements**: Formatted timestamps introduced into logs.

### v5.1 (05 Jan 2025)
- **Persistent Log Storage**: Storing logs persistently across reboots.
- **Log Viewer**: Added a dedicated UI to view historical logs.
- **Log Limits**: Implemented maximum log limit handling.

### v5.0 (04 Jan 2025)
- **Day-based Schedules**: Schedule structure upgraded to support active days.
- **Initial OTA**: First implementation logic for Over-The-Air updates (later refined).

---

## v4.x (Dec 2024 - Jan 2025)

### v4.1 (01 Jan 2025)
- **Calendar Support**: Base date and calendar-level scheduling added.

### v4.0 (31 Dec 2024)
- **Logging System**: Added on-page webpage logger.
- **Watchdog**: System watchdog implementation for stability.
- **Password Authentication**: Basic password logic added.
- **Schedule Conflict Detection**: Prevents adding overlapping schedules.
- **Major WebUI Improvements**: Refined UI interactions, warnings for empty fields, success dialogues.

---

## v3.0 (30 Dec 2024)
- **Physical Override**: Switch integration for physical override control.
- **Fail-safe Operation**: System won't break if server time sync is unavailable.
- **Physical Error Indication**: LED triggers to indicate system errors.

---

## v2.0 (29 Dec 2024)
- **Persistent Scheduling**: Storing schedules into permanent memory.
- **Desktop ↔ ESP32 Sync**: Synchronized interactions between ESP32 and Desktop app.
- **WebUI State Sync**: Active sessions now change states and display current relay states in real-time.

---

## v1.0 (28 Dec 2024)
- **Initial Working Controller**: Base logic for the ESP32 Aquarium Controller.
- **Desktop Application**: Created base for the desktop Python app.
- **Basic Timekeeping**: Added foundational time tracking to the codebase.
