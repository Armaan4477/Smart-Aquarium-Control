#ifndef PAGE_MAIN_H
#define PAGE_MAIN_H

#include <Arduino.h>

const char mainPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <script src="/theme.js"></script>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Aquarium Control</title>
    <style>
        :root {
            --primary-color: #1976D2;
            --primary-dark: #0D47A1;
            --primary-light: #BBDEFB;
            --accent-color: #03A9F4;
            --success-color: #4CAF50;
            --warning-color: #FFC107;
            --lightbtn-color: #94730eff;
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
        [data-theme="dark"] .temperature-item {
            background-color: #2C2C2C !important;
        }

        [data-theme="dark"] .temperature-item:hover {
            background-color: #3C3C3C !important;
        }

        .temperature-item.error-state {
            background-color: rgba(244, 67, 54, 0.15) !important;
        }

        [data-theme="dark"] .temperature-item.error-state {
            background-color: rgba(244, 67, 54, 0.25) !important;
        }
        
        [data-theme="dark"] .dismiss-btn {
            background-color: #333 !important;
            color: var(--error-color) !important;
        }

        [data-theme="dark"] .dismiss-btn:hover {
            background-color: #444 !important;
        }

        [data-theme="dark"] .dismiss-all:hover {
            background-color: #333 !important;
        }
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--background-color);
            color: var(--text-color);
            line-height: 1.6;
        }

        header {
            background: linear-gradient(135deg, var(--primary-color), var(--primary-dark));
            color: white;
            padding: 20px;
            text-align: center;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            position: relative;
            z-index: 10;
        }

        header h1 {
            margin: 0;
            font-size: 2rem;
            letter-spacing: 0.5px;
        }

        .container {
            padding: 20px;
            max-width: 1000px;
            margin: auto;
        }

        .time-container {
            margin: 20px 0;
            padding: 20px;
            background-color: var(--card-color);
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            text-align: center;
        }

        #time {
            font-size: 2.5rem;
            font-weight: bold;
            color: var(--primary-color);
            margin: 10px 0;
            transition: var(--transition);
        }

        #day {
            font-size: 1.5rem;
            color: var(--text-light);
            margin: 5px 0;
        }

        #date {
            font-size: 1.5rem;
            color: var(--text-light);
            margin: 5px 0;
        }

        .temperature-container {
            margin: 20px 0;
            padding: 20px;
            background-color: var(--card-color);
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            transition: var(--transition);
        }

        .temperature-container:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .temperature-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            align-items: center;
        }

        .temperature-item {
            text-align: center;
            padding: 15px;
            background-color: #f8f9fa;
            border-radius: var(--border-radius);
            transition: var(--transition);
        }

        .temperature-item:hover {
            background-color: #e9ecef;
        }

        .temperature-label {
            font-size: 1rem;
            color: var(--text-light);
            margin-bottom: 8px;
            font-weight: 500;
        }

        .temperature-value {
            font-size: 1.8rem;
            font-weight: bold;
            color: var(--primary-color);
        }

        .temperature-item.internal {
            border-left: 4px solid var(--success-color);
        }

        .temperature-item.external {
            border-left: 4px solid var(--accent-color);
        }

        .control-section {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            margin-bottom: 25px;
            transition: var(--transition);
        }

        .control-section:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .control-section h3 {
            color: var(--primary-color);
            margin-bottom: 20px;
            font-size: 1.5rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
            text-align: center;
        }

        .relay-buttons {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
            gap: 15px;
            margin-bottom: 10px;
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

        .button {
            padding: 15px;
            border: none;
            border-radius: var(--border-radius);
            font-size: 1.1rem;
            font-weight: 500;
            cursor: pointer;
            transition: var(--transition);
            text-align: center;
            box-shadow: var(--shadow);
            background-color: var(--primary-color);
            color: white;
        }

        .button:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        }

        .button:active {
            transform: translateY(1px);
        }

        .button.on {
            background-color: var(--success-color);
        }

        .button.off {
            background-color: var(--error-color);
        }

        .button.override {
            background-color: var(--warning-color);
            color: #333;
        }

        .button.override:hover {
            background-color: #e6ac00;
            color: #333;
        }

        .button.paused {
            background-color: #FF9800;
            color: #fff;
        }

        .nav-button {
            background-color: var(--primary-color);
        }

        .nav-button:hover {
            background-color: var(--primary-dark);
        }

        .special-button {
            background-color: var(--lightbtn-color);
            color: #333;
        }

        .special-button:hover {
            background-color: #64532bff;
        }

        #errorSection {
            text-align: center;
            margin: 20px 0;
            color: white;
            background-color: var(--error-color);
            padding: 20px;
            border-radius: var(--border-radius);
            display: none;
            animation: pulse 2s infinite;
            box-shadow: 0 4px 10px rgba(244, 67, 54, 0.3);
        }

        @keyframes pulse {
            0% { box-shadow: 0 0 0 0 rgba(244, 67, 54, 0.4); }
            70% { box-shadow: 0 0 0 10px rgba(244, 67, 54, 0); }
            100% { box-shadow: 0 0 0 0 rgba(244, 67, 54, 0); }
        }

        @media (max-width: 768px) {
            .relay-buttons {
                grid-template-columns: 1fr;
            }
            
            .navigation-buttons {
                grid-template-columns: 1fr;
            }

            .navigation-buttons .nav-full {
                grid-column: 1;
            }

            .temperature-grid {
                grid-template-columns: 1fr;
                gap: 15px;
            }

            .temperature-value {
                font-size: 1.5rem;
            }
            
            #time {
                font-size: 2rem;
            }
            
            #day, #date {
                font-size: 1.2rem;
            }
            
            .container {
                padding: 10px;
            }
            
            .control-section {
                padding: 15px;
                margin-bottom: 15px;
            }
            
            .control-section h3 {
                font-size: 1.3rem;
            }
        }


        #errorSection h3 {
            color: #fff;
            border-bottom: 2px solid rgba(255,255,255,0.3);
            margin-bottom: 15px;
            padding-bottom: 10px;
        }
        .error-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            background: rgba(0,0,0,0.15);
            padding: 10px 15px;
            border-radius: 8px;
            margin-bottom: 10px;
            font-size: 1.1rem;
        }
        .dismiss-btn {
            background-color: #fff;
            color: var(--error-color);
            padding: 5px 15px;
            font-size: 0.9rem;
            margin: 0;
        }
        .dismiss-btn:hover {
            background-color: #f1f1f1;
            transform: scale(1.05);
        }
        .dismiss-all {
            background-color: transparent;
            color: #fff;
            border: 2px solid #fff;
            width: 100%;
        }
        .dismiss-all:hover {
            background-color: #fff;
            color: var(--error-color);
        }

        /* Toast */
        #toast {
            position: fixed;
            bottom: 28px;
            left: 50%;
            transform: translateX(-50%) translateY(80px);
            background: #323232;
            color: white;
            padding: 12px 28px;
            border-radius: 24px;
            font-size: 0.95rem;
            opacity: 0;
            transition: all 0.35s ease;
            z-index: 1000;
            pointer-events: none;
        }
        #toast.show { opacity: 1; transform: translateX(-50%) translateY(0); }
        #toast.success { background: var(--success-color); }
        #toast.error   { background: var(--error-color); }
    </style>
</head>
<body>
    <header>
        <h1>Aquarium Control Panel</h1>
        <div id="themeToggle" onclick="toggleTheme()" title="Toggle Dark Mode" style="position: absolute; right: 20px; top: 20px; cursor: pointer; color: white;">
            <svg viewBox="0 0 24 24" width="28" height="28" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                <path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"></path>
            </svg>
        </div>
    </header>
    <div class="container">
        <div class="time-container">
            <div id="time">Loading time...</div>
            <div id="day">Loading day...</div>
            <div id="date">Loading date...</div>
        </div>
        
        <div class="temperature-container">
            <div class="temperature-grid">
                <div class="temperature-item internal">
                    <div class="temperature-label">Internal Temperature</div>
                    <div class="temperature-value" id="temperature">-- °C</div>
                </div>
                <div class="temperature-item external">
                    <div class="temperature-label">External Temperature</div>
                    <div class="temperature-value" id="externalTemperature">-- °C</div>
                </div>
            </div>
        </div>

        <div class="control-section">
            <h3>Relay Controls</h3>
            <div class="relay-buttons">
                <button class="button" onclick="toggleRelay(1)" id="btn1">WaveMaker</button>
                <button class="button" onclick="toggleRelay(3)" id="btn3">Air Pump</button>
                <button class="button" onclick="toggleRelay(2)" id="btn2">Light</button>
            </div>
            <div class="relay-buttons" style="margin-top: 15px;">
                <button class="button special-button" onclick="toggleFeedingMode()" id="btnFeedingMode">Feeding Mode (5m)</button>
                <button class="button special-button" onclick="oneClickLight()" id="btnOneClick">Change Light Color</button>
            </div>
        </div>

        <div id="errorSection" style="display: none;" class="control-section"></div>
        
        <div class="control-section">
            <h3>System Navigation</h3>
            <div class="navigation-buttons">
                <button class="button nav-button" onclick="showTempSchedules()">Temporary Schedules</button>
                <button class="button nav-button" onclick="showSchedules()">Main Schedules</button>
                <button class="button nav-button nav-full" onclick="showDeviceSettings()">Device Settings</button>
                <button class="button nav-button nav-full" onclick="showLogs()">System Logs</button>
            </div>
        </div>
    </div>
    <div id="toast"></div>
    <script>
        let currentThemeIsDark = document.documentElement.getAttribute('data-theme') === 'dark';
        
        function toggleTheme() {
            currentThemeIsDark = !currentThemeIsDark;
            document.documentElement.setAttribute('data-theme', currentThemeIsDark ? 'dark' : 'light');
            fetch('/api/themeConfig', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ isDarkMode: currentThemeIsDark })
            }).catch(console.error);
        }

        function showToast(msg, type) {
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.className = 'show ' + (type || '');
            setTimeout(() => { t.className = ''; }, 3000);
        }

        let relayStates = {
            1: false,
            2: false,
            3: false
        };

        let overrideStates = {
            1: false,
            2: false,
            3: false
        };

        let relayNames = {
            1: "WaveMaker",
            2: "Light",
            3: "Air Pump"
        };

        let lastReceivedTemp = '--';
        let lastReceivedExtTemp = '--';
        let currentActiveErrors = 0;
        let currentAckErrors = 0;

        function updateTemperatureDisplay() {
            let tempEl = document.getElementById('temperature');
            let extTempEl = document.getElementById('externalTemperature');
            
            if (tempEl) {
                if ((currentActiveErrors & 4) || (currentAckErrors & 4)) { // ERR_TEMP_INT
                    tempEl.textContent = '-- °C';
                    tempEl.parentElement.classList.add('error-state');
                } else {
                    tempEl.textContent = lastReceivedTemp + ' °C';
                    tempEl.parentElement.classList.remove('error-state');
                }
            }
            if (extTempEl) {
                if ((currentActiveErrors & 8) || (currentAckErrors & 8)) { // ERR_TEMP_EXT
                    extTempEl.textContent = '-- °C';
                    extTempEl.parentElement.classList.add('error-state');
                } else {
                    extTempEl.textContent = lastReceivedExtTemp + ' °C';
                    extTempEl.parentElement.classList.remove('error-state');
                }
            }
        }

        let socket = null;
        let reconnectAttempts = 0;
        const maxReconnectAttempts = 5;
        let reconnectInterval = 1000;
        const maxReconnectInterval = 30000;
        let feedingModeActive = false;
        let feedingModeTimeRemaining = 0;

        setInterval(() => {
            if (feedingModeActive && feedingModeTimeRemaining > 0) {
                feedingModeTimeRemaining--;
                updateFeedingModeUI();
            }
        }, 1000);

        function updateFeedingModeUI() {
            let btn = document.getElementById('btnFeedingMode');
            if (!btn) return;
            if (feedingModeActive) {
                btn.className = 'button paused';
                btn.textContent = 'Feeding Mode (' + Math.floor(feedingModeTimeRemaining / 60) + 'm ' + (feedingModeTimeRemaining % 60) + 's)';
            } else {
                btn.className = 'button special-button';
                btn.textContent = 'Feeding Mode (5m)';
            }
            updateButtonStyle(1);
            updateButtonStyle(3);
        }

        function connectWebSocket() {
            if (socket && (socket.readyState === WebSocket.CONNECTING || socket.readyState === WebSocket.OPEN)) {
                return; // Already connected or connecting
            }
            
            console.log('Attempting WebSocket connection...');
            socket = new WebSocket('ws://' + window.location.hostname + ':81/');
            
            socket.onopen = () => {
                console.log('WebSocket connected');
                reconnectAttempts = 0;
                reconnectInterval = 1000; // Reset interval
                // Request initial data
                getInitialStates();
            };
            
            socket.onmessage = (event) => {
                try {
                    let data = JSON.parse(event.data);
                    
                    if (data.relay1Name) relayNames[1] = data.relay1Name;
                    if (data.relay2Name) relayNames[2] = data.relay2Name;
                    if (data.relay3Name) relayNames[3] = data.relay3Name;
                    
                    if (data.override1 !== undefined) overrideStates[1] = data.override1;
                    if (data.override2 !== undefined) overrideStates[2] = data.override2;
                    // Relay 3 shares override1 (physical switch 1)
                    if (data.override1 !== undefined) overrideStates[3] = data.override1;

                    if (data.relay1 !== undefined) {
                        relayStates[1] = data.relay1;
                        updateButtonStyle(1);
                    }
                    if (data.relay2 !== undefined) {
                        relayStates[2] = data.relay2;
                        updateButtonStyle(2);
                    }
                    if (data.feedingModeActive !== undefined) {
                        feedingModeActive = data.feedingModeActive;
                        if (data.feedingModeTimeRemaining !== undefined) {
                            feedingModeTimeRemaining = data.feedingModeTimeRemaining;
                        }
                        updateFeedingModeUI();
                    }
                    if (data.relay3 !== undefined) {
                        relayStates[3] = data.relay3;
                        updateButtonStyle(3);
                    }
                    if (data.temperature !== undefined) {
                        lastReceivedTemp = data.temperature;
                    }
                    if (data.externalTemperature !== undefined) {
                        lastReceivedExtTemp = data.externalTemperature;
                    }
                    updateTemperatureDisplay();
                } catch (e) {
                    console.error('WebSocket message parsing error:', e);
                }
            };
            
            socket.onclose = (event) => {
                console.log('WebSocket disconnected:', event.code, event.reason);
                socket = null;
                scheduleReconnect();
                checkErrorStatus();
            };
            
            socket.onerror = (error) => {
                console.error('WebSocket error:', error);
                checkErrorStatus();
            };
        }

        function scheduleReconnect() {
        if (reconnectAttempts >= maxReconnectAttempts) {
            console.log('Max reconnection attempts reached');
            return;
        }
        
        reconnectAttempts++;
        console.log(`Scheduling reconnect attempt ${reconnectAttempts} in ${reconnectInterval}ms`);
        
        setTimeout(() => {
            connectWebSocket();
        }, reconnectInterval);
        
        // Exponential backoff
        reconnectInterval = Math.min(reconnectInterval * 1.5, maxReconnectInterval);
    }

    // Initialize connection
    connectWebSocket();

    // Fallback: try to reconnect every 30 seconds if disconnected
    setInterval(() => {
        if (!socket || socket.readyState === WebSocket.CLOSED) {
            console.log('WebSocket check: attempting reconnection');
            reconnectAttempts = 0; // Reset attempts for periodic check
            connectWebSocket();
        }
    }, 30000);

        function updateTime() {
            fetch('/time')
                .then(response => response.text())
                .then(data => {
                    const [time, day, date] = data.split(' ');
                    document.getElementById('time').textContent = time;
                    document.getElementById('day').textContent = day;
                    document.getElementById('date').textContent = date;
                })
                .catch(() => {
                    document.getElementById('time').textContent = "Time unavailable";
                    document.getElementById('day').textContent = "Day unavailable";
                    document.getElementById('date').textContent = "Date unavailable";
                });
        }

        function toggleRelay(relay) {
            fetch('/relay/' + relay, { method: 'POST', headers: { 'Content-Type': 'application/json' } })
                .then(response => response.ok ? response.json() : response.json().then(data => { throw new Error(data.error); }))
                .then(data => {
                    relayStates[relay] = data.state;
                    updateButtonStyle(relay);
                    let stateText = data.state ? "turned ON" : "turned OFF";
                    let relayLabel = relayNames[relay] || ("Relay " + relay);
                    showToast(relayLabel + " " + stateText, "success");
                })
                .catch(error => { showToast(error.message, 'error'); checkErrorStatus(); });
        }

        function toggleFeedingMode() {
            let btn = document.getElementById('btnFeedingMode');
            let action = btn.classList.contains('paused') ? 'stop' : 'start';
            fetch('/api/feeding_mode', { 
                method: 'POST', 
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ action: action })
            })
            .then(response => response.ok ? response.json() : response.json().then(data => { throw new Error(data.error); }))
            .then(data => {
                showToast(action === 'start' ? "Feeding Mode Started" : "Feeding Mode Stopped", "success");
            })
            .catch(error => showToast(error.message, 'error'));
        }

        function updateButtonStyle(relay) {
            const btn = document.getElementById('btn' + relay);
            if (btn) {
                let relayLabel = relayNames[relay] || "Unknown";
                if (overrideStates[relay]) {
                    btn.className = 'button override';
                    btn.textContent = `${relayLabel} (Override)`;
                } else if (relayStates[relay]) {
                    if (feedingModeActive && (relay === 1 || relay === 3)) {
                        btn.className = 'button paused';
                        btn.textContent = `${relayLabel} (Paused)`;
                    } else {
                        btn.className = 'button on';
                        btn.textContent = `${relayLabel} (ON)`;
                    }
                } else {
                    btn.className = 'button off';
                    btn.textContent = `${relayLabel} (OFF)`;
                }
            }
        }

        function getInitialStates() {
            if (socket && socket.readyState === WebSocket.OPEN) {
                return;
            }

            fetch('/relay/status')
                .then(response => response.json())
                .then(data => { 
                    relayStates = data;
                    if (data.override1 !== undefined) {
                        overrideStates[1] = data.override1;
                        overrideStates[3] = data.override1; // relay3 shares override1
                    }
                    if (data.override2 !== undefined) overrideStates[2] = data.override2;
                    if (data.feedingModeActive !== undefined) {
                        feedingModeActive = data.feedingModeActive;
                        if (data.feedingModeTimeRemaining !== undefined) {
                            feedingModeTimeRemaining = data.feedingModeTimeRemaining;
                        }
                        updateFeedingModeUI();
                    }
                    for(let relay in relayStates) {
                        if (relay <= 3) {
                            updateButtonStyle(relay);
                        }
                    }
                    if (data.temperature !== undefined) {
                        lastReceivedTemp = data.temperature;
                    }
                    if (data.externalTemperature !== undefined) {
                        lastReceivedExtTemp = data.externalTemperature;
                    }
                    updateTemperatureDisplay();
                })
                .catch(error => {
                    console.error('Failed to get initial states:', error);
                    checkErrorStatus();
                });
        }

        function checkErrorStatus() {
            fetch('/error/status')
                .then(response => response.json())
                .then(data => {
                    const errSec = document.getElementById('errorSection');
                    if (!errSec) return;
                    currentActiveErrors = data.activeErrors || 0;
                    currentAckErrors = data.acknowledgedErrors || 0;
                    updateTemperatureDisplay();
                    let activeErrors = data.activeErrors || 0;
                    let ackErrors = data.acknowledgedErrors || 0;
                    if (activeErrors > 0 || ackErrors > 0) {
                        let html = '';
                        if (activeErrors > 0) {
                            html += '<h3>System Errors Detected</h3>';
                            if (activeErrors & 1) html += '<p class="error-row"><span>WiFi Disconnected</span> <button class="button dismiss-btn" onclick="clearError(1)">Dismiss</button></p>';
                            if (activeErrors & 2) html += '<p class="error-row"><span>Time Sync Failed</span> <button class="button dismiss-btn" onclick="clearError(2)">Dismiss</button></p>';
                            if (activeErrors & 4) html += '<p class="error-row"><span>Internal Temperature Sensor Failed</span> <button class="button dismiss-btn" onclick="clearError(4)">Dismiss</button></p>';
                            if (activeErrors & 8) html += '<p class="error-row"><span>External Temperature Sensor Failed</span> <button class="button dismiss-btn" onclick="clearError(8)">Dismiss</button></p>';
                            html += '<div style="margin-top: 15px;"><button class="button dismiss-btn dismiss-all" onclick="clearError(\'all\')">Dismiss All</button></div>';
                        }
                        if (ackErrors > 0) {
                            html += '<h3 style="margin-top: ' + (activeErrors > 0 ? '20px' : '0') + ';">Acknowledged Errors</h3>';
                            if (ackErrors & 1) html += '<p class="error-row" style="opacity: 0.7;"><span>WiFi Disconnected</span></p>';
                            if (ackErrors & 2) html += '<p class="error-row" style="opacity: 0.7;"><span>Time Sync Failed</span></p>';
                            if (ackErrors & 4) html += '<p class="error-row" style="opacity: 0.7;"><span>Internal Temperature Sensor Failed</span></p>';
                            if (ackErrors & 8) html += '<p class="error-row" style="opacity: 0.7;"><span>External Temperature Sensor Failed</span></p>';
                        }
                        errSec.innerHTML = html;
                        errSec.style.display = 'block';
                        
                        if (activeErrors === 0 && ackErrors > 0) {
                            errSec.style.backgroundColor = '#ff9800';
                            errSec.style.animation = 'none';
                            errSec.style.boxShadow = '0 4px 10px rgba(255, 152, 0, 0.3)';
                        } else {
                            errSec.style.backgroundColor = 'var(--error-color)';
                            errSec.style.animation = 'pulse 2s infinite';
                            errSec.style.boxShadow = '0 4px 10px rgba(244, 67, 54, 0.3)';
                        }
                    } else {
                        errSec.style.display = 'none';
                    }
                })
                .catch(() => {
                    const errSec = document.getElementById('errorSection');
                    if (errSec) errSec.style.display = 'none';
                });
        }

        function clearError(errId) {
            fetch('/error/clear', { 
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ error_id: errId })
            })
            .then(response => response.ok ? response.json() : { status: 'error' })
            .then(data => { 
                if (data.status === 'success') { 
                    checkErrorStatus(); 
                    if (errId === 'all') {
                        showToast('All errors dismissed', 'success');
                    } else {
                        showToast('Error dismissed', 'success');
                    }
                } 
                else { throw new Error('Failed to clear error'); }
            })
            .catch(error => { showToast('Failed to clear error: ' + error.message, 'error'); });
        }

        function showLogs() {
            window.location.href = '/logs';
        }
        function showDeviceSettings() {
            window.location.href = '/devicesettings';
        }
        function showTempSchedules() {
            window.location.href = '/tempschedules';
        }
        function showSchedules() {
            window.location.href = '/mainSchedules';
        }
        function oneClickLight() {
            fetch('/relay/oneclick', { method: 'POST' })
            .then(response => response.json().then(data => {
                if (!response.ok) throw new Error(data.error);
                showToast('Light colour changed successfully.', 'success');
            }))
            .catch(error => showToast(error.message, 'error'));
        }

        setInterval(updateTime, 1000);
        setInterval(checkErrorStatus, 2000);
        updateTime();
        getInitialStates();
        checkErrorStatus();
        
        document.getElementById('btn1').textContent = `${relayNames[1]} (OFF)`;
        document.getElementById('btn2').textContent = `${relayNames[2]} (OFF)`;
        document.getElementById('btn3').textContent = `${relayNames[3]} (OFF)`;
    </script>
</body>
</html>
)html";

#endif // PAGE_MAIN_H
