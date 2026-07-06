#ifndef PAGE_AUTO_REBOOT_H
#define PAGE_AUTO_REBOOT_H

#include <Arduino.h>

const char autoRebootPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <script src="/theme.js"></script>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Scheduled Auto-Reboot</title>
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
        
        .container {
            padding: 20px;
            max-width: 800px;
            margin: auto;
        }
        
        .card {
            background: var(--card-color);
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            padding: 25px;
            margin-bottom: 25px;
        }
        .card h3 {
            color: var(--primary-color);
            font-size: 1.4rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
            margin-bottom: 20px;
        }

        .form-group {
            margin-bottom: 20px;
        }
        .form-group label {
            display: block;
            margin-bottom: 8px;
            font-weight: 500;
            color: var(--text-color);
        }
        .form-control {
            width: 100%;
            padding: 12px;
            border: 1px solid #ddd;
            border-radius: var(--border-radius);
            font-size: 1rem;
            transition: var(--transition);
            background-color: white;
            color: #333;
        }
        [data-theme="dark"] .form-control {
            background-color: #333;
            color: white;
            border-color: #555;
        }
        .form-control:focus {
            border-color: var(--primary-color);
            outline: none;
            box-shadow: 0 0 0 2px var(--primary-light);
        }

        .days-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(130px, 1fr));
            gap: 10px;
            margin-top: 10px;
        }
        
        .day-checkbox {
            display: flex;
            align-items: center;
            gap: 8px;
            cursor: pointer;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: var(--border-radius);
            transition: var(--transition);
            white-space: nowrap;
        }
        [data-theme="dark"] .day-checkbox {
            border-color: #555;
            background-color: #2a2a2a;
        }
        .day-checkbox:hover {
            border-color: var(--primary-color);
            background-color: var(--primary-light);
        }
        [data-theme="dark"] .day-checkbox:hover {
            background-color: var(--primary-dark);
        }
        
        .time-picker {
            display: flex;
            gap: 10px;
            align-items: center;
        }
        .time-picker input {
            width: 80px;
        }
        
        .switch-container {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 15px;
            background: var(--background-color);
            border-radius: var(--border-radius);
            margin-bottom: 20px;
            border: 1px solid #ddd;
        }
        [data-theme="dark"] .switch-container {
            border-color: #555;
            background-color: #2a2a2a;
        }
        
        .switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
        }
        .switch input { 
            opacity: 0;
            width: 0;
            height: 0;
        }
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0; left: 0; right: 0; bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }
        .slider:before {
            position: absolute;
            content: "";
            height: 26px; width: 26px;
            left: 4px; bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }
        input:checked + .slider {
            background-color: var(--success-color);
        }
        input:checked + .slider:before {
            transform: translateX(26px);
        }

        .button {
            display: inline-block;
            padding: 12px 24px;
            background-color: var(--primary-color);
            color: white;
            text-decoration: none;
            border-radius: var(--border-radius);
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
        }
        
        .status-msg {
            padding: 10px;
            border-radius: var(--border-radius);
            margin-top: 15px;
            display: none;
        }
        .status-success {
            background-color: rgba(76, 175, 80, 0.1);
            color: var(--success-color);
            border: 1px solid var(--success-color);
        }
        .status-error {
            background-color: rgba(244, 67, 54, 0.1);
            color: var(--error-color);
            border: 1px solid var(--error-color);
        }
    </style>
</head>
<body>
    <header>
        <h1>Scheduled Auto-Reboot</h1>
        <p>Configure automatic system restarts for stability</p>
    </header>

    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Device Settings</button>
        </div>

        <div class="card">
            <h3>Configuration</h3>
            
            <form id="rebootForm">
                <div class="switch-container">
                    <div>
                        <strong style="font-size: 1.1rem;">Enable Auto-Reboot</strong>
                        <p style="margin: 0; font-size: 0.9rem; opacity: 0.8;">Perform a clean reboot on selected days</p>
                    </div>
                    <label class="switch">
                        <input type="checkbox" id="rebootEnabled">
                        <span class="slider"></span>
                    </label>
                </div>
                
                <div class="form-group">
                    <label>Reboot Time (24h)</label>
                    <div class="time-picker">
                        <input type="number" id="rebootHour" class="form-control" min="0" max="23" placeholder="HH" required>
                        <span>:</span>
                        <input type="number" id="rebootMinute" class="form-control" min="0" max="59" placeholder="MM" required>
                    </div>
                    <p style="margin-top: 5px; font-size: 0.85rem; opacity: 0.8;">Time should be specified in your local timezone.</p>
                </div>
                
                <div class="form-group">
                    <label>Reboot Days</label>
                    <div class="days-grid">
                        <label class="day-checkbox">
                            <input type="checkbox" id="day0" value="0"> Sunday
                        </label>
                        <label class="day-checkbox">
                            <input type="checkbox" id="day1" value="1"> Monday
                        </label>
                        <label class="day-checkbox">
                            <input type="checkbox" id="day2" value="2"> Tuesday
                        </label>
                        <label class="day-checkbox">
                            <input type="checkbox" id="day3" value="3"> Wednesday
                        </label>
                        <label class="day-checkbox">
                            <input type="checkbox" id="day4" value="4"> Thursday
                        </label>
                        <label class="day-checkbox">
                            <input type="checkbox" id="day5" value="5"> Friday
                        </label>
                        <label class="day-checkbox">
                            <input type="checkbox" id="day6" value="6"> Saturday
                        </label>
                    </div>
                </div>

                <div style="margin-top: 30px;">
                    <button type="submit" class="button" style="width: 100%;">Save Settings</button>
                </div>
                
                <div id="statusMessage" class="status-msg"></div>
            </form>
        </div>
        
        <div class="card">
            <h3>Information</h3>
            <p style="font-size: 0.95rem; opacity: 0.9;">
                While the internal watchdog and FreeRTOS tasks ensure system stability, scheduling a clean 
                reboot once a week at a time when you are not using the aquarium (e.g., 3:00 AM) can proactively 
                clear out memory fragmentation and ensure months of uninterrupted uptime.
            </p>
        </div>
    </div>

    <script>
        document.addEventListener('DOMContentLoaded', () => {
            fetchConfig();
            
            document.getElementById('rebootForm').addEventListener('submit', function(e) {
                e.preventDefault();
                saveConfig();
            });
        });

        function goBack() {
            window.history.back();
        }

        function fetchConfig() {
            fetch('/api/auto_reboot')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('rebootEnabled').checked = data.enabled;
                    
                    if(data.hour !== undefined) {
                        document.getElementById('rebootHour').value = data.hour.toString().padStart(2, '0');
                    }
                    if(data.minute !== undefined) {
                        document.getElementById('rebootMinute').value = data.minute.toString().padStart(2, '0');
                    }
                    
                    if(data.days) {
                        for(let i=0; i<7; i++) {
                            document.getElementById('day' + i).checked = data.days[i];
                        }
                    }
                })
                .catch(error => showStatus('Error loading configuration: ' + error, false));
        }

        function saveConfig() {
            const formData = new URLSearchParams();
            
            formData.append('enabled', document.getElementById('rebootEnabled').checked ? '1' : '0');
            formData.append('hour', parseInt(document.getElementById('rebootHour').value));
            formData.append('minute', parseInt(document.getElementById('rebootMinute').value));
            
            for(let i=0; i<7; i++) {
                formData.append('day' + i, document.getElementById('day' + i).checked ? '1' : '0');
            }

            fetch('/api/auto_reboot', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: formData.toString()
            })
            .then(response => response.json())
            .then(data => {
                if(data.status === 'success') {
                    showStatus('Configuration saved successfully!', true);
                } else {
                    showStatus('Error: ' + data.error, false);
                }
            })
            .catch(error => showStatus('Error saving configuration: ' + error, false));
        }
        
        function showStatus(message, isSuccess) {
            const msgEl = document.getElementById('statusMessage');
            msgEl.textContent = message;
            msgEl.className = 'status-msg ' + (isSuccess ? 'status-success' : 'status-error');
            msgEl.style.display = 'block';
            
            setTimeout(() => {
                msgEl.style.display = 'none';
            }, 5000);
        }
    </script>
</body>
</html>
)html";

#endif // PAGE_AUTO_REBOOT_H
