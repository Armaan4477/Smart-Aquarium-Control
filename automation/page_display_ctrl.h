#ifndef PAGE_DISPLAY_CTRL_H
#define PAGE_DISPLAY_CTRL_H

#include <Arduino.h>

const char displayCtrlPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <script src="/theme.js"></script>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Display Control</title>
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
        .card:hover { box-shadow: 0 5px 15px rgba(0,0,0,0.15); }
        .card h3 {
            color: var(--primary-color);
            font-size: 1.4rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
            margin-bottom: 20px;
        }
        /* Status badge */
        .status-row {
            display: flex;
            align-items: center;
            gap: 12px;
            margin-bottom: 8px;
        }
        .status-badge {
            display: inline-block;
            padding: 6px 16px;
            border-radius: 20px;
            font-weight: 600;
            font-size: 0.95rem;
            letter-spacing: 0.5px;
            transition: var(--transition);
        }
        .status-badge.on  { background: #e8f5e9; color: var(--success-color); border: 1.5px solid var(--success-color); }
        .status-badge.off { background: #fce4ec; color: var(--error-color);   border: 1.5px solid var(--error-color); }
        .mode-label { color: var(--text-light); font-size: 0.9rem; }
        /* Schedule form */
        .form-row {
            display: flex;
            flex-wrap: wrap;
            gap: 16px;
            margin-bottom: 18px;
            align-items: flex-end;
        }
        .form-group { display: flex; flex-direction: column; flex: 1; min-width: 120px; }
        .form-group label {
            font-size: 0.85rem;
            color: var(--text-light);
            margin-bottom: 5px;
            font-weight: 500;
        }
        .form-group input[type=time],
        .form-group input[type=number] {
            padding: 10px 12px;
            border: 1.5px solid #ddd;
            border-radius: var(--border-radius);
            font-size: 1rem;
            transition: var(--transition);
            width: 100%;
        }
        .form-group input:focus {
            outline: none;
            border-color: var(--primary-color);
            box-shadow: 0 0 0 3px rgba(25,118,210,0.12);
        }
        .toggle-row {
            display: flex;
            align-items: center;
            gap: 12px;
            margin-bottom: 18px;
        }
        .toggle-row label { font-size: 1rem; cursor: pointer; user-select: none; }
        input[type=checkbox] {
            width: 18px; height: 18px;
            accent-color: var(--primary-color);
            cursor: pointer;
        }
        /* Override buttons */
        .override-group {
            display: grid;
            grid-template-columns: 1fr 1fr 1fr;
            gap: 12px;
            margin-top: 4px;
        }
        .override-btn {
            padding: 14px 8px;
            border: 2px solid transparent;
            border-radius: var(--border-radius);
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: var(--transition);
            background: #f5f7fa;
            color: var(--text-color);
        }
        .override-btn:hover { transform: translateY(-2px); box-shadow: 0 4px 12px rgba(0,0,0,0.12); }
        .override-btn.active-schedule { background: var(--primary-light); border-color: var(--primary-color); color: var(--primary-dark); }
        .override-btn.active-on       { background: #e8f5e9; border-color: var(--success-color); color: #2e7d32; }
        .override-btn.active-off      { background: #fce4ec; border-color: var(--error-color);   color: #c62828; }
        /* Save button */
        .save-btn {
            width: 100%;
            padding: 14px;
            background: var(--primary-color);
            color: white;
            border: none;
            border-radius: var(--border-radius);
            font-size: 1.05rem;
            font-weight: 600;
            cursor: pointer;
            transition: var(--transition);
            box-shadow: var(--shadow);
            margin-top: 6px;
        }
        .save-btn:hover { background: var(--primary-dark); transform: translateY(-2px); box-shadow: 0 4px 15px rgba(0,0,0,0.2); }
        .save-btn:active { transform: translateY(1px); }
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
        @media (max-width: 600px) {
            .override-group { grid-template-columns: 1fr; }
            .form-row { flex-direction: column; align-items: stretch; }
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
    </style>
</head>
<body>
    <header>
        <h1>Display Control</h1>
        <p>Manage OLED display on/off schedule</p>
    </header>
    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Dashboard</button>
        </div>

        <!-- Status Card -->
        <div class="card">
            <h3>Current Status</h3>
            <div class="status-row">
                <span>Display:</span>
                <span class="status-badge" id="displayBadge">...</span>
            </div>
            <div class="mode-label" id="modeLabel">Loading...</div>
        </div>

        <!-- Override Card -->
        <div class="card">
            <h3>Override</h3>
            <p style="color:var(--text-light);font-size:0.9rem;margin-bottom:16px;">Instantly control the display, or let the schedule decide.</p>
            <div class="override-group">
                <button class="override-btn" id="btn-follow" onclick="setOverride(0)">Follow Schedule</button>
                <button class="override-btn" id="btn-on"     onclick="setOverride(1)">Force ON</button>
                <button class="override-btn" id="btn-off"    onclick="setOverride(2)">Force OFF</button>
            </div>
        </div>

        <!-- Schedule Card -->
        <div class="card">
            <h3>Schedule</h3>
            <div class="form-row">
                <div class="form-group">
                    <label for="onTime">Turn ON at</label>
                    <input type="time" id="onTime" value="08:00">
                </div>
                <div class="form-group">
                    <label for="offTime">Turn OFF at</label>
                    <input type="time" id="offTime" value="22:00">
                </div>
            </div>
            <button class="save-btn" onclick="saveSchedule()">Save Schedule</button>
        </div>

    </div>
    <div id="toast"></div>
    <script>
        let currentData = {};
        let isEditing = false;

        const modeNames = ['Following schedule', 'Forced ON', 'Forced OFF'];

        function pad(n) { return String(n).padStart(2, '0'); }

        function showToast(msg, type) {
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.className = 'show ' + (type || '');
            setTimeout(() => { t.className = ''; }, 3000);
        }

        function applyData(d) {
            currentData = d;
            // Status badge
            const badge = document.getElementById('displayBadge');
            badge.textContent = d.displayOn ? 'ON' : 'OFF';
            badge.className = 'status-badge ' + (d.displayOn ? 'on' : 'off');
            // Mode label
            document.getElementById('modeLabel').textContent = modeNames[d.overrideMode] || '';
            // Override buttons
            document.getElementById('btn-follow').className = 'override-btn' + (d.overrideMode === 0 ? ' active-schedule' : '');
            document.getElementById('btn-on').className     = 'override-btn' + (d.overrideMode === 1 ? ' active-on'       : '');
            document.getElementById('btn-off').className    = 'override-btn' + (d.overrideMode === 2 ? ' active-off'      : '');
            // Schedule fields
            if (!isEditing) {
                document.getElementById('onTime').value  = pad(d.onHour)  + ':' + pad(d.onMinute);
                document.getElementById('offTime').value = pad(d.offHour) + ':' + pad(d.offMinute);
            }
        }

        function loadData() {
            fetch('/display/schedule')
                .then(r => r.json())
                .then(applyData)
                .catch(() => showToast('Failed to load settings', 'error'));
        }

        function setOverride(mode) {
            fetch('/display/schedule/save', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ overrideMode: mode })
            })
            .then(r => r.json())
            .then(d => {
                if (d.status === 'success') {
                    showToast('Override updated', 'success');
                    setTimeout(loadData, 400);
                } else { showToast('Error: ' + (d.error || 'unknown'), 'error'); }
            })
            .catch(() => showToast('Request failed', 'error'));
        }

        function saveSchedule() {
            const onParts  = document.getElementById('onTime').value.split(':');
            const offParts = document.getElementById('offTime').value.split(':');
            fetch('/display/schedule/save', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    onHour:    parseInt(onParts[0]),
                    onMinute:  parseInt(onParts[1]),
                    offHour:   parseInt(offParts[0]),
                    offMinute: parseInt(offParts[1]),
                    enabled:   true
                })
            })
            .then(r => r.json())
            .then(d => {
                if (d.status === 'success') {
                    showToast('Schedule saved', 'success');
                    isEditing = false;
                    setTimeout(loadData, 400);
                } else { showToast('Error: ' + (d.error || 'unknown'), 'error'); }
            })
            .catch(() => showToast('Request failed', 'error'));
        }

        document.getElementById('onTime').addEventListener('input', () => isEditing = true);
        document.getElementById('offTime').addEventListener('input', () => isEditing = true);

        // Refresh status every 5 s
        loadData();
        setInterval(loadData, 5000);

        function goBack() {
            window.history.back();
        }
    </script>
</body>
</html>
)html";

#endif // PAGE_DISPLAY_CTRL_H
