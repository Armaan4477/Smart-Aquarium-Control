#ifndef PAGE_DEVICE_SETTINGS_H
#define PAGE_DEVICE_SETTINGS_H

#include <Arduino.h>

const char deviceSettingsPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <script src="/theme.js"></script>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Device Settings</title>
    <style>
        :root {
            --primary-color: #1976D2;
            --primary-dark: #0D47A1;
            --primary-light: #BBDEFB;
            --accent-color: #03A9F4;
            --success-color: #4CAF50;
            --warning-color: #FFC107;
            --error-color: #F44336;
            --text-color: #333;
            --text-light: #757575;
            --background-color: #f5f7fa;
            --card-color: #ffffff;
            --border-radius: 8px;
            --shadow: 0 2px 10px rgba(0,0,0,0.1);
            --transition: all 0.3s ease;
        }
        [data-theme="dark"] {
            --primary-color: #2196F3;
            --primary-dark: #1976D2;
            --primary-light: #0D47A1;
            --accent-color: #03A9F4;
            --success-color: #4CAF50;
            --warning-color: #FFD54F;
            --error-color: #F44336;
            --text-color: #E0E0E0;
            --text-light: #9E9E9E;
            --background-color: #121212;
            --card-color: #1E1E1E;
            --shadow: 0 2px 10px rgba(0,0,0,0.5);
            --lightbtn-color: #bfa13a;
        }

        /* Dark Mode Overrides for Hardcoded Colors */
        [data-theme="dark"] .temperature-item,
        [data-theme="dark"] .raw-data-item {
            background-color: #2C2C2C !important;
        }

        [data-theme="dark"] .temperature-item:hover,
        [data-theme="dark"] .raw-data-item:hover {
            background-color: #3C3C3C !important;
        }
        
        /* Table overrides (logs and schedules) */
        [data-theme="dark"] .logs-table tr:nth-child(even),
        [data-theme="dark"] .logs-table tr:nth-child(odd),
        [data-theme="dark"] .logs-table tr,
        [data-theme="dark"] .schedule-table tr {
            background-color: #2C2C2C !important;
            color: var(--text-color) !important;
        }
        
        [data-theme="dark"] .logs-table tr:nth-child(even) {
            background-color: #242424 !important;
        }

        [data-theme="dark"] .logs-table tr:hover,
        [data-theme="dark"] .schedule-table tr:hover {
            background-color: #3C3C3C !important;
        }

        [data-theme="dark"] .schedule-table tr.disabled {
            background-color: #222 !important;
            opacity: 0.8 !important;
        }
        
        [data-theme="dark"] .temporary-indicator {
            background-color: #4A3B00 !important;
            color: #FFD54F !important;
        }

        [data-theme="dark"] .status-badge.on,
        [data-theme="dark"] .override-btn.active-on {
            background-color: #1b4332 !important;
            color: #4CAF50 !important;
            border-color: #4CAF50 !important;
        }

        [data-theme="dark"] .status-badge.off,
        [data-theme="dark"] .override-btn.active-off {
            background-color: #641220 !important;
            color: #F44336 !important;
            border-color: #F44336 !important;
        }

        [data-theme="dark"] .override-btn {
            background-color: #333 !important;
            color: var(--text-color) !important;
        }

        [data-theme="dark"] .override-btn:hover {
            background-color: #444 !important;
        }

        [data-theme="dark"] .override-btn.active-schedule {
            background-color: var(--primary-dark) !important;
            color: white !important;
        }

        [data-theme="dark"] #clearErrorBtn,
        [data-theme="dark"] .dismiss-btn {
            background-color: #333 !important;
            color: var(--error-color) !important;
        }

        [data-theme="dark"] #clearErrorBtn:hover,
        [data-theme="dark"] .dismiss-btn:hover {
            background-color: #444 !important;
        }

        [data-theme="dark"] .dismiss-all:hover {
            background-color: #333 !important;
        }

        [data-theme="dark"] input,
        [data-theme="dark"] select,
        [data-theme="dark"] textarea {
            background-color: #333 !important;
            color: white !important;
            border-color: #555 !important;
        }
        
        [data-theme="dark"] .modal-content {
            background-color: var(--card-color) !important;
        }

        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--background-color);
            color: var(--text-color);
            line-height: 1.6;
            padding-bottom: 40px;
        }
        header {
            background: linear-gradient(135deg, var(--primary-color), var(--primary-dark));
            color: white;
            padding: 20px;
            text-align: center;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            position: relative;
            z-index: 10;
            margin-bottom: 30px;
        }
        header h1 { margin: 0; font-size: 2rem; letter-spacing: 0.5px; }
        header p { margin: 5px 0 0; opacity: 0.85; font-size: 0.95rem; }
        .button {
            display: inline-block;
            padding: 12px 24px;
            background-color: var(--primary-color);
            color: white;
            text-decoration: none;
            border-radius: var(--border-radius);
            margin: 5px 0 20px 0;
            transition: var(--transition);
            border: none;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            box-shadow: var(--shadow);
            text-align: center;
        }
        .button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }
        .header-actions {
            margin-bottom: 20px;
            overflow: hidden;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 10px;
        }
        .container {
            padding: 20px;
            max-width: 1200px;
            margin: auto;
        }
        .card {
            background: var(--card-color);
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            padding: 25px;
            margin-bottom: 25px;
            transition: var(--transition);
        }
        .card h3 {
            color: var(--primary-color);
            font-size: 1.4rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
            margin-bottom: 20px;
        }
        
        .section-subtitle {
            color: var(--text-color);
            font-size: 1.1rem;
            margin-bottom: 12px;
            font-weight: 600;
            opacity: 0.9;
        }
        
        .navigation-buttons {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
            margin-bottom: 10px;
        }

        .navigation-buttons .nav-full {
            grid-column: 1 / -1;
        }

        .nav-button {
            background-color: var(--primary-color);
            padding: 15px;
            border: none;
            border-radius: var(--border-radius);
            font-size: 1.1rem;
            font-weight: 500;
            cursor: pointer;
            transition: var(--transition);
            text-align: center;
            box-shadow: var(--shadow);
            color: white;
            width: 100%;
        }

        .nav-button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        }
        
        .nav-button:active {
            transform: translateY(1px);
        }

        @media (max-width: 768px) {
            .navigation-buttons {
                grid-template-columns: 1fr;
            }
            .navigation-buttons .nav-full {
                grid-column: 1;
            }
        }
    </style>
</head>
<body>
    <header>
        <h1>Device Settings</h1>
        <p>Manage system configuration and maintenance</p>
    </header>

    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Dashboard</button>
        </div>

        <div class="card">
            <h3>System Management</h3>
            <div class="navigation-buttons">
                <button class="nav-button" onclick="rebootDevice()" style="background-color: var(--error-color);">Reboot Device</button>
            </div>
        </div>

        <div class="card">
            <h3>Configuration Options</h3>
            
            <h4 class="section-subtitle">Hardware Setup</h4>
            <div class="navigation-buttons">
                <button class="nav-button" onclick="showTempControl()">Sensor Calibration</button>
                <button class="nav-button" onclick="showDisplayCtrl()">Display Control</button>
            </div>

            <h4 class="section-subtitle" style="margin-top: 20px;">Services</h4>
            <div class="navigation-buttons">
                <button class="nav-button" onclick="showEmailConfig()">Email Settings</button>
                <button class="nav-button" onclick="showDockerConfig()">Docker Settings</button>
            </div>

            <h4 class="section-subtitle" style="margin-top: 20px;">System Maintenance</h4>
            <div class="navigation-buttons">
                <button class="nav-button" onclick="showBackupRestore()">Backup / Restore</button>
                <button class="nav-button" onclick="showOTAUpdate()">OTA Update</button>
            </div>
        </div>
    </div>
    
    <script>
        function rebootDevice() {
            if(confirm("Are you sure you want to reboot the device?")) {
                fetch('/api/reboot', { method: 'POST' })
                .then(response => {
                    if(response.ok) {
                        alert("Device is rebooting. Please wait a moment and refresh the page.");
                    } else {
                        alert("Failed to initiate reboot.");
                    }
                })
                .catch(error => {
                    alert("Error: " + error);
                });
            }
        }

        function goBack() { window.location.href = '/'; }
        
        function showTempControl() {
            window.location.href = '/tempcontrol';
        }
        function showDisplayCtrl() {
            window.location.href = '/displayctrl';
        }
        function showEmailConfig() {
            window.location.href = '/emailConfig';
        }
        function showDockerConfig() {
            window.location.href = '/dockerConfig';
        }
        function showBackupRestore() {
            window.location.href = '/backuprestore';
        }
        function showOTAUpdate() {
            window.location.href = '/ota';
        }
    </script>
</body>
</html>
)html";

#endif // PAGE_DEVICE_SETTINGS_H
