#ifndef PAGE_TEMP_CTRL_H
#define PAGE_TEMP_CTRL_H

#include <Arduino.h>

const char tempctrl[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <script src="/theme.js"></script>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sensor Calibration</title>
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
            margin-bottom: 30px;
        }

        header h1 {
            margin: 0;
            font-size: 2rem;
            letter-spacing: 0.5px;
        }

        .container {
            padding: 20px;
            max-width: 1200px;
            margin: auto;
        }

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

        .button:active {
            transform: translateY(1px);
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

        .calibration-section {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            margin-bottom: 25px;
            transition: var(--transition);
        }

        .calibration-section:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .calibration-section h3 {
            color: var(--primary-color);
            margin-bottom: 15px;
            font-size: 1.5rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
        }

        .calibration-grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 20px;
            margin-bottom: 20px;
        }

        .calibration-item label {
            display: block;
            margin-bottom: 8px;
            font-weight: 500;
            color: var(--text-color);
        }

        .calibration-item input[type="number"] {
            width: 100%;
            padding: 12px;
            margin: 8px 0 20px 0;
            border-radius: var(--border-radius);
            border: 1px solid #ddd;
            font-size: 1rem;
            transition: var(--transition);
        }

        .calibration-item input[type="number"]:focus {
            outline: none;
            border-color: var(--primary-color);
            box-shadow: 0 0 0 3px var(--primary-light);
        }

        .calibration-note {
            background-color: #e3f2fd;
            border-left: 4px solid var(--primary-color);
            padding: 12px;
            margin-bottom: 20px;
            border-radius: 4px;
            font-size: 0.9rem;
            color: #1565c0;
        }

        .raw-data-section {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            margin-bottom: 25px;
            transition: var(--transition);
        }

        .raw-data-section:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .raw-data-section h3 {
            color: var(--primary-color);
            margin-bottom: 15px;
            font-size: 1.5rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
        }

        .raw-data-grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 20px;
            margin-bottom: 20px;
        }

        .raw-data-item {
            text-align: center;
            padding: 15px;
            background-color: #f8f9fa;
            border-radius: var(--border-radius);
            transition: var(--transition);
        }

        .raw-data-item:hover {
            background-color: #e9ecef;
        }

        .raw-data-label {
            font-size: 1rem;
            color: var(--text-light);
            margin-bottom: 8px;
            font-weight: 500;
        }

        .raw-data-value {
            font-size: 1.8rem;
            font-weight: bold;
            color: var(--accent-color);
        }

        .raw-data-item.internal {
            border-left: 4px solid var(--success-color);
        }

        .raw-data-item.external {
            border-left: 4px solid var(--accent-color);
        }

        .calibration-save-button {
            width: 100%;
            padding: 12px 24px;
            background-color: var(--accent-color);
            color: white;
            border: none;
            border-radius: var(--border-radius);
            cursor: pointer;
            transition: var(--transition);
            font-weight: 500;
            font-size: 1rem;
            box-shadow: var(--shadow);
        }

        .calibration-save-button:hover {
            background-color: #0288D1;
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }

        .calibration-save-button:active {
            transform: translateY(1px);
        }

        .changed-indicator {
            background-color: #fff3cd !important;
            border-left: 4px solid var(--warning-color) !important;
        }

        .calibration-save-button.changes-pending {
            background-color: var(--warning-color) !important;
            color: #333 !important;
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0% { transform: scale(1); }
            50% { transform: scale(1.05); }
            100% { transform: scale(1); }
        }

        @media (max-width: 768px) {
            .container {
                padding: 10px;
            }
            
            .calibration-grid {
                grid-template-columns: 1fr;
                gap: 15px;
            }

            .raw-data-grid {
                grid-template-columns: 1fr;
                gap: 15px;
            }

            .calibration-section {
                padding: 15px;
            }

            .raw-data-section {
                padding: 15px;
            }

            .calibration-section h3 {
                font-size: 1.3rem;
            }

            .raw-data-section h3 {
                font-size: 1.3rem;
            }

            .raw-data-value {
                font-size: 1.5rem;
            }
            
            .calibration-save-button {
                padding: 12px;
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
        @media (max-width: 768px) {
            [data-theme="dark"] {
                --background-color: #000000;
                --card-color: #000000;
            }
            [data-theme="dark"] .temperature-item:not(.error-state),
            [data-theme="dark"] .raw-data-item,
            [data-theme="dark"] .logs-table tr,
            [data-theme="dark"] .logs-table tr:nth-child(even),
            [data-theme="dark"] .logs-table tr:nth-child(odd),
            [data-theme="dark"] .schedule-table tr,
            [data-theme="dark"] .schedule-table tr:nth-child(even),
            [data-theme="dark"] .schedule-table tr:nth-child(odd) {
                background-color: #000000 !important;
            }
            [data-theme="dark"] .temperature-item:not(.error-state):hover,
            [data-theme="dark"] .raw-data-item:hover,
            [data-theme="dark"] .logs-table tr:hover,
            [data-theme="dark"] .schedule-table tr:hover {
                background-color: #111111 !important;
            }
        }
    </style>
</head>
<body>
    <header>
        <h1>Sensor Calibration</h1>
        <p>Manage sensor's calibration</p>
    </header>
    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Device Settings</button>
        </div>

        <div class="raw-data-section">
            <h3>Raw Sensor Data</h3>
            <div class="raw-data-grid">
                <div class="raw-data-item internal">
                    <div class="raw-data-label">Internal Sensor (Raw)</div>
                    <div class="raw-data-value" id="internal-raw-temp">--</div>
                </div>
                <div class="raw-data-item external">
                    <div class="raw-data-label">External Sensor (Raw)</div>
                    <div class="raw-data-value" id="external-raw-temp">--</div>
                </div>
                <div class="raw-data-item external">
                    <div class="raw-data-label">External Humidity (Raw)</div>
                    <div class="raw-data-value" id="external-raw-hum">--</div>
                </div>
            </div>
        </div>

        <div class="calibration-section">
            <h3>Sensor Calibration</h3>
            <div class="calibration-note">
                Calibration allows you to adjust sensor readings to match a reference thermometer. 
                Positive values increase the reading, negative values decrease it. Range: -10.00°C to +10.00°C
            </div>
            
            <div class="calibration-grid">
                <div class="calibration-item">
                    <label for="internal-calibration">Internal Sensor Offset (°C):</label>
                    <input type="number" id="internal-calibration" min="-10" max="10" step="0.01" value="0.00">
                </div>
                <div class="calibration-item">
                    <label for="external-calibration">External Sensor Offset (°C):</label>
                    <input type="number" id="external-calibration" min="-10" max="10" step="0.01" value="0.00">
                </div>
                <div class="calibration-item">
                    <label for="external-hum-calibration">External Hum Offset (%):</label>
                    <input type="number" id="external-hum-calibration" min="-100" max="100" step="0.01" value="0.00">
                </div>
            </div>
            
            <div class="temp-buttons">
                <button class="calibration-save-button" id="save-calibration-settings" onclick="saveCalibrationSettings()">Save Calibration</button>
            </div>
        </div>
    </div>
    <div id="toast"></div>
    <script>
        function showToast(msg, type) {
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.className = 'show ' + (type || '');
            setTimeout(() => { t.className = ''; }, 3000);
        }

        let socket = new WebSocket('ws://' + window.location.hostname + ':81/');
        let userChangedCalibration = false;
        let lastSavedCalibration = {};

        socket.onopen = () => {
            console.log('WebSocket connected');
            loadCalibrationSettings();
        };
        
        socket.onmessage = (event) => {
            try {
                let data = JSON.parse(event.data);
                if (data.internalRawTemp !== undefined) {
                    document.getElementById('internal-raw-temp').textContent = data.internalRawTemp.toFixed(2) + ' °C';
                }
                if (data.externalRawTemp !== undefined) {
                    document.getElementById('external-raw-temp').textContent = data.externalRawTemp.toFixed(2) + ' °C';
                }
            } catch (e) {
                console.error('WebSocket error:', e);
            }
        };

        socket.onclose = () => console.log('WebSocket disconnected');
        socket.onerror = () => console.log('WebSocket error');

        function goBack() {
          if (userChangedCalibration) {
                if (!confirm('You have unsaved changes. Are you sure you want to leave?')) {
                    return;
                }
            }
            window.history.back();
        }

        function checkForCalibrationChanges() {
            const currentInternal = parseFloat(document.getElementById('internal-calibration').value);
            const currentExternal = parseFloat(document.getElementById('external-calibration').value);
            const currentExternalHum = parseFloat(document.getElementById('external-hum-calibration').value);
            
            const hasChanges = (
                Math.abs(currentInternal - lastSavedCalibration.internalOffset) > 0.001 ||
                Math.abs(currentExternal - lastSavedCalibration.externalOffset) > 0.001 ||
                Math.abs(currentExternalHum - (lastSavedCalibration.externalHumidityOffset || 0)) > 0.001
            );
            
            userChangedCalibration = hasChanges;
            updateCalibrationUI();
        }

        function updateCalibrationUI() {
            const saveButton = document.getElementById('save-calibration-settings');
            const calibrationSection = document.querySelector('.calibration-section');
            
            if (userChangedCalibration) {
                saveButton.classList.add('changes-pending');
                saveButton.textContent = 'Save Changes';
                calibrationSection.classList.add('changed-indicator');
            } else {
                saveButton.classList.remove('changes-pending');
                saveButton.textContent = 'Save Calibration';
                calibrationSection.classList.remove('changed-indicator');
            }
        }

        function loadCalibrationSettings() {
            fetch('/calibration/settings')
                .then(response => response.json())
                .then(data => {
                    // Only update if user hasn't made changes
                    if (!userChangedCalibration) {
                        document.getElementById('internal-calibration').value = data.internalOffset.toFixed(2);
                        document.getElementById('external-calibration').value = data.externalOffset.toFixed(2);
                        if (data.externalHumidityOffset !== undefined) {
                            document.getElementById('external-hum-calibration').value = data.externalHumidityOffset.toFixed(2);
                        }
                    }
                    
                    // Always update the saved state
                    lastSavedCalibration = {
                        internalOffset: data.internalOffset,
                        externalOffset: data.externalOffset,
                        externalHumidityOffset: data.externalHumidityOffset || 0
                    };
                })
                .catch(error => {
                    console.error('Error loading calibration settings:', error);
                });
        }

        function saveCalibrationSettings() {
            const internalOffset = parseFloat(document.getElementById('internal-calibration').value);
            const externalOffset = parseFloat(document.getElementById('external-calibration').value);
            const externalHumOffset = parseFloat(document.getElementById('external-hum-calibration').value);
            
            if (internalOffset < -10 || internalOffset > 10 || externalOffset < -10 || externalOffset > 10 || externalHumOffset < -100 || externalHumOffset > 100) {
                showToast('Calibration offsets out of bounds!', 'error');
                return;
            }
            
            const settings = {
                internalOffset: internalOffset,
                externalOffset: externalOffset,
                externalHumidityOffset: externalHumOffset
            };
            
            fetch('/calibration/save', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(settings)
            })
            .then(async response => {
                if (!response.ok) {
                    let errMsg = 'Server error';
                    try {
                        const data = await response.json();
                        if (data.error) errMsg = data.error;
                    } catch (e) {}
                    throw new Error(errMsg);
                }
                return response.json();
            })
            .then(data => {
                showToast('Calibration settings saved successfully!', 'success');
                // Update saved state and reset change tracking
                lastSavedCalibration = { ...settings };
                userChangedCalibration = false;
                updateCalibrationUI();
                loadCalibrationSettings();
            })
            .catch(error => {
                showToast('Failed to save calibration: ' + error.message, 'error');
            });
        }

        function loadRawSensorData() {
            fetch('/temperature/raw')
                .then(response => response.json())
                .then(data => {
                    if (data.internalRaw !== undefined) {
                        document.getElementById('internal-raw-temp').textContent = data.internalRaw.toFixed(2) + ' °C';
                    }
                    if (data.externalRaw !== undefined) {
                        document.getElementById('external-raw-temp').textContent = data.externalRaw.toFixed(2) + ' °C';
                    }
                    if (data.externalHumRaw !== undefined) {
                        document.getElementById('external-raw-hum').textContent = data.externalHumRaw.toFixed(2) + ' %';
                    }
                })
                .catch(error => {
                    console.error('Error loading raw sensor data:', error);
                });
        }
        
        document.getElementById('internal-calibration').addEventListener('input', checkForCalibrationChanges);
        document.getElementById('external-calibration').addEventListener('input', checkForCalibrationChanges);

        // Initialize everything
        loadCalibrationSettings();
        loadRawSensorData();
        
        setInterval(() => {
            if (!userChangedCalibration) {
                loadCalibrationSettings();
            }
            loadRawSensorData();
        }, 10000);
    </script>
</body>
</html>
)html";

#endif // PAGE_TEMP_CTRL_H
