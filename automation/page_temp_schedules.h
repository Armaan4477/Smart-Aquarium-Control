#ifndef PAGE_TEMP_SCHEDULES_H
#define PAGE_TEMP_SCHEDULES_H

#include <Arduino.h>

const char tempschedules[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <script src="/theme.js"></script>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Temporary Schedules</title>
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

        .temp-schedule-form {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            margin-bottom: 25px;
            transition: var(--transition);
        }

        .temp-schedule-form:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .loading-message {
            text-align: center;
            padding: 20px;
            color: var(--text-light);
            font-style: italic;
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

        .card {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            margin-bottom: 25px;
            transition: var(--transition);
        }

        .card:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .card h3 {
            color: var(--primary-color);
            margin-bottom: 15px;
            font-size: 1.5rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
        }

        .schedule-form, .log-section {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            margin-bottom: 25px;
            transition: var(--transition);
        }

        .schedule-form:hover, .log-section:hover {
            box-shadow: 0 5px 15px rgba(0,0,0,0.15);
        }

        .schedule-form h3, .log-section h3 {
            color: var(--primary-color);
            margin-bottom: 15px;
            font-size: 1.5rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
        }

        .schedule-form label {
            display: block;
            margin-bottom: 8px;
            font-weight: 500;
            color: var(--text-color);
        }

        .schedule-form input, .schedule-form select {
            width: 100%;
            padding: 12px;
            margin: 8px 0 20px 0;
            border-radius: var(--border-radius);
            border: 1px solid #ddd;
            font-size: 1rem;
            transition: var(--transition);
        }

        .schedule-form input:focus, .schedule-form select:focus {
            outline: none;
            border-color: var(--primary-color);
            box-shadow: 0 0 0 3px var(--primary-light);
        }

        .schedule-form select {
            appearance: none;
            background-color: #fff;
            background-image: url('data:image/svg+xml;utf8,<svg fill="%23333" height="24" viewBox="0 0 24 24" width="24" xmlns="http://www.w3.org/2000/svg"><path d="M7 10l5 5 5-5z"/><path d="M0 0h24v24H0z" fill="none"/></svg>');
            background-repeat: no-repeat;
            background-position: right 10px center;
            padding-right: 40px;
            cursor: pointer;
        }

        .schedule-form button {
            width: 100%;
            padding: 12px;
            background-color: var(--primary-color);
            color: white;
            border: none;
            border-radius: var(--border-radius);
            font-size: 1.1rem;
            cursor: pointer;
            transition: var(--transition);
            margin-top: 10px;
        }

        .schedule-form button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }

        .schedule-form button:active {
            transform: translateY(1px);
        }

        .schedule-table {
            width: 100%;
            border-collapse: separate;
            border-spacing: 0;
            margin-top: 20px;
            overflow: hidden;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
        }

        .schedule-table {
            width: 100%;
            border: none;
            margin-top: 10px;
        }

        .schedule-table tbody {
            display: flex;
            flex-direction: column;
            gap: 15px;
        }

        .schedule-table tr:first-child {
            display: none;
        }

        .schedule-table tr {
            display: flex;
            flex-direction: row;
            align-items: center;
            background-color: #fff;
            border: 1px solid #ddd;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            padding: 15px 25px;
            transition: var(--transition);
            width: 100%;
            box-sizing: border-box;
            justify-content: space-between;
        }

        .schedule-table tr:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
        }

        .schedule-table td {
            display: flex;
            flex-direction: column;
            align-items: flex-start;
            padding: 0 10px;
            border-bottom: none;
            text-align: left;
            font-size: 1rem;
            color: var(--text-color);
            flex: 1;
        }

        .schedule-table td:last-child {
            flex-direction: row;
            align-items: center;
            justify-content: flex-end;
            gap: 10px;
            padding: 0;
            margin: 0;
            min-width: 200px;
        }

        .schedule-table td:nth-child(1)::before { content: "ID"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; white-space: nowrap; }
        .schedule-table td:nth-child(2)::before { content: "Relay"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; white-space: nowrap; }
        .schedule-table td:nth-child(3)::before { content: "Start Time"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; white-space: nowrap; }
        .schedule-table td:nth-child(4)::before { content: "End Time"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; white-space: nowrap; }

        .action-button {
            flex: 1;
            padding: 12px 16px;
            margin: 0;
            border: none;
            border-radius: 20px;
            cursor: pointer;
            font-weight: 500;
            font-size: 0.95rem;
            transition: var(--transition);
            color: white;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
            text-align: center;
            max-width: 140px;
        }

        .action-button:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.15);
        }
        
        .action-button:active {
            transform: translateY(1px);
        }

        .action-button.activate { background-color: var(--success-color); color: white; }
        .action-button.activate:hover { background-color: #388E3C; }
        
        .action-button.deactivate { background-color: var(--warning-color); color: #333; }
        .action-button.deactivate:hover { background-color: #FFA000; }
        
        .action-button.delete { background-color: var(--error-color); color: white; }
        .action-button.delete:hover { background-color: #D32F2F; }

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

        #clearErrorBtn {
            padding: 12px 24px;
            background-color: white;
            color: var(--error-color);
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            margin-top: 15px;
            transition: var(--transition);
        }

        #clearErrorBtn:hover {
            background-color: #f5f5f5;
            transform: scale(1.05);
        }

        #logSection {
            display: none;
        }

        pre {
            background-color: #f8f9fa;
            padding: 15px;
            border-radius: var(--border-radius);
            max-height: 300px;
            overflow-y: auto;
            font-family: 'Consolas', 'Monaco', monospace;
            border: 1px solid #eee;
            white-space: pre-wrap;
        }

        .error {
            color: var(--error-color);
            display: none;
            margin-top: -15px;
            margin-bottom: 12px;
            font-size: 0.9rem;
            transition: var(--transition);
        }

        .error2 {
            color: var(--error-color);
            display: none;
            margin-top: 2px;
            margin-bottom: 12px;
            font-size: 0.9rem;
            transition: var(--transition);
        }

        .ready {
            background-color: var(--success-color);
            cursor: pointer;
        }



        .day-checkboxes {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            margin-bottom: 20px;
        }

        .day-checkboxes label {
            display: inline-flex;
            align-items: center;
            position: relative;
            padding-left: 30px;
            cursor: pointer;
            font-size: 1rem;
            user-select: none;
            margin-right: 15px;
            margin-bottom: 5px;
        }

        .day-checkboxes input {
            position: absolute;
            opacity: 0;
            cursor: pointer;
            height: 0;
            width: 0;
        }

        .day-checkboxes .checkmark {
            position: absolute;
            top: 0;
            left: 0;
            height: 20px;
            width: 20px;
            background-color: #eee;
            border-radius: 4px;
            transition: var(--transition);
        }

        .day-checkboxes label:hover input ~ .checkmark {
            background-color: #ccc;
        }

        .day-checkboxes input:checked ~ .checkmark {
            background-color: var(--primary-color);
        }

        .day-checkboxes .checkmark:after {
            content: "";
            position: absolute;
            display: none;
            left: 7px;
            top: 3px;
            width: 5px;
            height: 10px;
            border: solid white;
            border-width: 0 2px 2px 0;
            transform: rotate(45deg);
        }

        .day-checkboxes input:checked ~ .checkmark:after {
            display: block;
        }
        
        .limitation-note {
            margin-top: 10px;
            padding: 10px;
            background-color: #fff3cd;
            border-left: 4px solid var(--warning-color);
            color: #856404;
            border-radius: 4px;
            font-size: 0.9rem;
        }

        @media (max-width: 768px) {
            .buttons {
                grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            }
            
            #time {
                font-size: 2rem;
            }
            
            #day, #date {
                font-size: 1.2rem;
            }
            
            .day-checkboxes {
                flex-direction: column;
                gap: 5px;
            }
            
            .day-checkboxes label {
                margin-right: 0;
            }
            
            .card {
                padding: 15px;
            }
            .schedule-form {
                padding: 15px;
            }
            .schedule-table tr {
                flex-direction: column;
                padding: 15px;
            }
            .schedule-table td {
                flex-direction: row;
                justify-content: space-between;
                align-items: center;
                width: 100%;
                border-bottom: 1px solid #eee;
                padding: 12px 0;
                text-align: right;
                flex: none;
            }
            .schedule-table td:last-child {
                border-bottom: none;
                justify-content: center;
                padding-top: 15px;
                margin-top: 5px;
            }
            .schedule-table td::before {
                margin-bottom: 0 !important;
            }
            
            .action-button {
                padding: 10px;
                margin: 3px;
                font-size: 0.9rem;
                flex: 1;
                max-width: 120px;
            }

            .container {
                padding: 10px;
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
        #toast.error   { background: var(--error-color); display: block !important; }
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
        <h1>Temporary Schedules</h1>
        <p>One-time schedules that auto-delete after running</p>
    </header>
    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Dashboard</button>
        </div>

        <div class="schedule-form">
            <h3>Add Temporary Schedule (One-time only)</h3>
            <div class="limitation-note">
                <strong>Note:</strong> Each relay can have a maximum of 2 temporary schedules at a time.
            </div>
            <label for="tempRelaySelect">Select Relay:</label>
            <select id="tempRelaySelect">
                <option value="" disabled selected>Select Relay</option>
                <option value="1">WaveMaker</option>
                <option value="2">Light</option>
                <option value="3">Air Pump</option>
            </select>
            <div id="tempRelayError" class="error">Please select a relay.</div>

            <label for="tempOnTime">Start Time (optional):</label>
            <input type="time" id="tempOnTime" placeholder="On Time">

            <label for="tempOffTime">End Time (optional):</label>
            <input type="time" id="tempOffTime" placeholder="Off Time">
            <div id="tempTimeError" class="error">Please enter at least start time or end time.</div>

            <button id="addTempScheduleBtn" onclick="addTemporarySchedule()">Add Temporary Schedule</button>
        </div>

        <div class="card">
            <h3>Active Temporary Schedules</h3>
            <table class="schedule-table" id="tempScheduleTable">
                <tr>
                    <th>ID</th>
                    <th>Relay</th>
                    <th>Start Time</th>
                    <th>End Time</th>
                    <th>Action</th>
                </tr>
            </table>
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

    function goBack() {
        window.history.back();
    }

    function addTemporarySchedule() {
        document.getElementById('tempRelayError').style.display = 'none';
        document.getElementById('tempTimeError').style.display = 'none';

        const relay = document.getElementById('tempRelaySelect').value;
        const onTime = document.getElementById('tempOnTime').value;
        const offTime = document.getElementById('tempOffTime').value;
        
        let hasError = false;

        if (relay === "") {
            document.getElementById('tempRelayError').style.display = 'block';
            hasError = true;
        }
        
        if (!onTime && !offTime) {
            document.getElementById('tempTimeError').style.display = 'block';
            hasError = true;
        }
        
        if (hasError) {
            return;
        }

        let requestBody = { relay };
        if (onTime) requestBody.onTime = onTime;
        if (offTime) requestBody.offTime = offTime;

        fetch('/temp-schedule/add', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(requestBody)
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
        .then(() => { 
            loadTemporarySchedules(); 
            checkErrorStatus(); 
            document.getElementById('tempRelaySelect').value = '';
            document.getElementById('tempOnTime').value = '';
            document.getElementById('tempOffTime').value = '';
            showToast('Temporary schedule added successfully!', 'success');
        })
        .catch(error => { 
            showToast('Failed to add temporary schedule: ' + error.message, 'error'); 
            checkErrorStatus(); 
        });
    }

    function deleteTemporarySchedule(id) {
        fetch('/temp-schedule/delete?id=' + id, { method: 'DELETE', headers: { 'Content-Type': 'application/json' } })
            .then(response => response.ok ? response.json() : { status: 'error' })
            .then(data => { 
                if (data.status === 'success') { 
                    loadTemporarySchedules(); 
                    checkErrorStatus(); 
                    showToast('Temporary schedule deleted', 'success');
                } else { 
                    throw new Error('Failed to delete temporary schedule'); 
                } 
            })
            .catch(error => { 
                showToast('Failed to delete temporary schedule: ' + error.message, 'error'); 
                checkErrorStatus(); 
            });
    }

    function loadTemporarySchedules() {
        fetch('/temp-schedules')
            .then(response => response.json())
            .then(schedules => {
                const table = document.getElementById('tempScheduleTable');
                table.innerHTML = `<tr>
                    <th>ID</th>
                    <th>Relay</th>
                    <th>Start Time</th>
                    <th>End Time</th>
                    <th>Action</th>
                </tr>`;
                
                schedules.forEach(schedule => {
                    const row = table.insertRow();
                    let relayName = "Unknown";
                    if (schedule.relay == 1) relayName = "WaveMaker";
                    else if (schedule.relay == 2) relayName = "Light";
                    else if (schedule.relay == 3) relayName = "Air Pump";
                    
                    row.insertCell(0).textContent = schedule.id;
                    row.insertCell(1).textContent = relayName;
                    
                    let startTime = schedule.hasOnTime ? 
                        `${String(schedule.onHour).padStart(2, '0')}:${String(schedule.onMinute).padStart(2, '0')}` : 
                        'Not set';
                    row.insertCell(2).textContent = startTime;
                    
                    let endTime = schedule.hasOffTime ? 
                        `${String(schedule.offHour).padStart(2, '0')}:${String(schedule.offMinute).padStart(2, '0')}` : 
                        'Not set';
                    row.insertCell(3).textContent = endTime;
                    
                    const actionCell = row.insertCell(4);
                    const deleteBtn = document.createElement('button');
                    deleteBtn.textContent = 'Delete';
                    deleteBtn.className = 'action-button delete';
                    deleteBtn.onclick = () => deleteTemporarySchedule(schedule.id);
                    actionCell.appendChild(deleteBtn);
                });
            })
            .catch(() => checkErrorStatus());
    }

    function checkErrorStatus() {
        fetch('/error/status')
            .then(response => response.json())
            .then(data => {
                console.log('Error status checked:', data.hasError);
            })
            .catch(() => {
                console.log('Failed to check error status');
            });
    }

    loadTemporarySchedules();
    setInterval(loadTemporarySchedules, 5000);
</script>
</body>
</html>
)html";

#endif // PAGE_TEMP_SCHEDULES_H
