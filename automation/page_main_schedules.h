#ifndef PAGE_MAIN_SCHEDULES_H
#define PAGE_MAIN_SCHEDULES_H

#include <Arduino.h>

const char mainSchedules[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <script src="/theme.js"></script>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Schedules</title>
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
            --success-color: #81C784;
            --warning-color: #FFD54F;
            --error-color: #E57373;
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
            color: #74c69d !important;
            border-color: #74c69d !important;
        }

        [data-theme="dark"] .status-badge.off,
        [data-theme="dark"] .override-btn.active-off {
            background-color: #641220 !important;
            color: #ff8fa3 !important;
            border-color: #ff8fa3 !important;
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
            border: none;
            margin-top: 20px;
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

        .schedule-table td:nth-child(4) {
            flex: 2.5;
        }

        .schedule-table td:nth-child(1)::before { content: "Relay"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; white-space: nowrap; }
        .schedule-table td:nth-child(2)::before { content: "On Time"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; white-space: nowrap; }
        .schedule-table td:nth-child(3)::before { content: "Off Time"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; white-space: nowrap; }
        .schedule-table td:nth-child(4)::before { content: "Days"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; white-space: nowrap; }
        .schedule-table td:nth-child(5)::before { content: "Status"; font-weight: 600; color: var(--text-light); font-size: 0.85rem; margin-bottom: 5px; text-transform: uppercase; letter-spacing: 0.5px; white-space: nowrap; }

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

        .action-button.edit { background-color: var(--accent-color); color: white; }
        .action-button.edit:hover { background-color: #0288D1; }

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

        @media (max-width: 768px) {

            .container {
                padding: 10px;
            }

            .buttons {
                grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
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
    </style>
</head>
<body>
    <header>
        <h1>Schedules</h1>
        <p>Manage daily recurring schedules</p>
    </header>
    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Dashboard</button>
        </div>

        <div class="schedule-form">
            <h3>Add Schedule</h3>
            <label for="relaySelect">Select Relay:</label>
            <select id="relaySelect">
                <option value="" disabled selected>Select Relay</option>
                <option value="1">WaveMaker</option>
                <option value="2">Light</option>
                <option value="3">Air Pump</option>
            </select>
            <div id="relayError" class="error">Please select a relay.</div>

            <label for="onTime">Start Time:</label>
            <input type="time" id="onTime" placeholder="On Time">
            <div id="onTimeError" class="error">Please enter a start time.</div>

            <label for="offTime">End Time:</label>
            <input type="time" id="offTime" placeholder="Off Time">
            <div id="offTimeError" class="error">Please enter an end time.</div>

            <label>Select Days:</label>
            <div class="day-checkboxes">
                <label>
                    <input type="checkbox" value="0" class="dayCheckbox">
                    <span class="checkmark"></span> Sun
                </label>
                <label>
                    <input type="checkbox" value="1" class="dayCheckbox">
                    <span class="checkmark"></span> Mon
                </label>
                <label>
                    <input type="checkbox" value="2" class="dayCheckbox">
                    <span class="checkmark"></span> Tue
                </label>
                <label>
                    <input type="checkbox" value="3" class="dayCheckbox">
                    <span class="checkmark"></span> Wed
                </label>
                <label>
                    <input type="checkbox" value="4" class="dayCheckbox">
                    <span class="checkmark"></span> Thu
                </label>
                <label>
                    <input type="checkbox" value="5" class="dayCheckbox">
                    <span class="checkmark"></span> Fri
                </label>
                <label>
                    <input type="checkbox" value="6" class="dayCheckbox">
                    <span class="checkmark"></span> Sat
                </label>
            </div>
            <div id="dayError" class="error2">Please select at least one day.</div>

            <button id="addScheduleBtn" onclick="addSchedule()">Add Schedule</button>
        </div>
        <table class="schedule-table" id="scheduleTable">
            <tr>
                <th>Relay</th>
                <th>On Time</th>
                <th>Off Time</th>
                <th>Days</th>
                <th>Status</th>
                <th>Action</th>
            </tr>
        </table>

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

        let editingScheduleId = null;
        let allSchedules = [];

        function addSchedule() {
            document.getElementById('relayError').style.display = 'none';
            document.getElementById('onTimeError').style.display = 'none';
            document.getElementById('offTimeError').style.display = 'none';

            const relay = document.getElementById('relaySelect').value;
            const onTime = document.getElementById('onTime').value;
            const offTime = document.getElementById('offTime').value;
            const dayCheckboxes = document.querySelectorAll('.dayCheckbox');
            let days = Array(7).fill(false);
            dayCheckboxes.forEach(cb => {
                if(cb.checked) {
                    days[parseInt(cb.value)] = true;
                }
            });
            let hasError = false;

            if (relay === "") {
                document.getElementById('relayError').style.display = 'block';
                hasError = true;
            }
            if (!onTime) {
                document.getElementById('onTimeError').style.display = 'block';
                hasError = true;
            }
            if (!offTime) {
                document.getElementById('offTimeError').style.display = 'block';
                hasError = true;
            }
            if (days.every(day => day === false)) {
                document.getElementById('dayError').style.display = 'block';
                hasError = true;
            } else {
                document.getElementById('dayError').style.display = 'none';
            }
            if (hasError) {
                return;
            }

            const url = editingScheduleId !== null ? '/schedule/edit' : '/schedule/add';
            const bodyData = { relay, onTime, offTime, days };
            if (editingScheduleId !== null) {
                bodyData.id = editingScheduleId;
            }

            fetch(url, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(bodyData)
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
                loadSchedules(); 
                checkErrorStatus(); 
                showToast(editingScheduleId !== null ? 'Schedule updated successfully!' : 'Schedule added successfully!', 'success');
                if (editingScheduleId !== null) cancelEdit();
            })
            .catch(error => { 
                showToast('Failed to save schedule: ' + error.message, 'error'); 
                checkErrorStatus(); 
            });
        }

        function editSchedulePrompt(index) {
            const schedule = allSchedules[index];
            if (!schedule) return;
            
            editingScheduleId = index;
            document.getElementById('relaySelect').value = schedule.relay || schedule.relayNumber;
            document.getElementById('onTime').value = String(schedule.onHour).padStart(2, '0') + ':' + String(schedule.onMinute).padStart(2, '0');
            document.getElementById('offTime').value = String(schedule.offHour).padStart(2, '0') + ':' + String(schedule.offMinute).padStart(2, '0');
            
            const dayCheckboxes = document.querySelectorAll('.dayCheckbox');
            schedule.daysOfWeek.forEach((active, i) => {
                dayCheckboxes[i].checked = active;
            });
            
            document.getElementById('addScheduleBtn').textContent = 'Save Changes';
            document.querySelector('.schedule-form h3').textContent = 'Edit Schedule';
            
            let cancelBtn = document.getElementById('cancelEditBtn');
            if (!cancelBtn) {
                cancelBtn = document.createElement('button');
                cancelBtn.id = 'cancelEditBtn';
                cancelBtn.textContent = 'Cancel Edit';
                cancelBtn.style.backgroundColor = '#757575';
                cancelBtn.onclick = cancelEdit;
                cancelBtn.style.marginTop = '10px';
                document.getElementById('addScheduleBtn').parentNode.appendChild(cancelBtn);
            }
            cancelBtn.style.display = 'block';
            
            checkFields();
            window.scrollTo(0, 0);
        }
        
        function cancelEdit() {
            editingScheduleId = null;
            document.getElementById('relaySelect').value = '';
            document.getElementById('onTime').value = '';
            document.getElementById('offTime').value = '';
            document.querySelectorAll('.dayCheckbox').forEach(cb => cb.checked = false);
            
            document.getElementById('addScheduleBtn').textContent = 'Add Schedule';
            document.querySelector('.schedule-form h3').textContent = 'Add Schedule';
            
            const cancelBtn = document.getElementById('cancelEditBtn');
            if (cancelBtn) cancelBtn.style.display = 'none';
            
            checkFields();
        }

        function checkErrorStatus() {
            fetch('/error/status')
                .then(response => response.json())
                .then(data => {
                    const errSec = document.getElementById('errorSection');
                    if (!errSec) return;
                    let activeErrors = data.activeErrors || 0;
                    if (activeErrors > 0) {
                                                let html = '<h3>System Errors Detected</h3>';
                        if (activeErrors & 1) html += '<p class="error-row"><span>WiFi Disconnected</span> <button class="button dismiss-btn" onclick="clearError(1)">Dismiss</button></p>';
                        if (activeErrors & 2) html += '<p class="error-row"><span>Time Sync Failed</span> <button class="button dismiss-btn" onclick="clearError(2)">Dismiss</button></p>';
                        if (activeErrors & 4) html += '<p class="error-row"><span>Internal Temperature Sensor Failed</span> <button class="button dismiss-btn" onclick="clearError(4)">Dismiss</button></p>';
                        if (activeErrors & 8) html += '<p class="error-row"><span>External Temperature Sensor Failed</span> <button class="button dismiss-btn" onclick="clearError(8)">Dismiss</button></p>';
                        html += '<div style="margin-top: 15px;"><button class="button dismiss-btn dismiss-all" onclick="clearError(\'all\')">Dismiss All</button></div>';
                        errSec.innerHTML = html;
                        errSec.style.display = 'block';
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


        function deleteSchedule(id) {
            fetch('/schedule/delete?id=' + id, { method: 'DELETE', headers: { 'Content-Type': 'application/json' } })
                .then(response => response.ok ? response.json() : { status: 'error' })
                .then(data => { 
                    if (data.status === 'success') { 
                        loadSchedules(); 
                        checkErrorStatus(); 
                        showToast('Schedule deleted successfully', 'success');
                    } else { 
                        throw new Error('Failed to delete schedule'); 
                    } 
                })
                .catch(error => { showToast('Failed to delete schedule: ' + error.message, 'error'); checkErrorStatus(); });
        }

        function loadSchedules() {
            fetch('/schedules')
                .then(response => response.json())
                .then(schedules => {
                    allSchedules = schedules;
                    const table = document.getElementById('scheduleTable');
                    table.innerHTML = `<tr>
                        <th>Relay</th>
                        <th>On Time</th>
                        <th>Off Time</th>
                        <th>Days</th>
                        <th>Status</th>
                        <th>Action</th>
                    </tr>`;
                    let dayNames = ["Sun","Mon","Tue","Wed","Thu","Fri","Sat"];
                    schedules.forEach((schedule, index) => {
                        const row = table.insertRow();
                        // Support both schedule.relay and schedule.relayNumber depending on API response
                        let relayVal = schedule.relay !== undefined ? schedule.relay : schedule.relayNumber;
                        let relayName = "Unknown";
                        if (relayVal == 1) relayName = "WaveMaker";
                        else if (relayVal == 2) relayName = "Light";
                        else if (relayVal == 3) relayName = "Air Pump";
                        
                        row.insertCell(0).textContent = relayName;
                        row.insertCell(1).textContent = `${String(schedule.onHour).padStart(2, '0')}:${String(schedule.onMinute).padStart(2, '0')}`;
                        row.insertCell(2).textContent = `${String(schedule.offHour).padStart(2, '0')}:${String(schedule.offMinute).padStart(2, '0')}`;
                        
                        const activeDays = [];
                        schedule.daysOfWeek.forEach((active, i) => {
                            if (active) activeDays.push(dayNames[i]);
                        });
                        row.insertCell(3).textContent = activeDays.join(", ");
                        
                        row.insertCell(4).textContent = schedule.enabled ? 'Active' : 'Inactive';
                        const actionCell = row.insertCell(5);
                        const toggleBtn = document.createElement('button');
                        toggleBtn.textContent = schedule.enabled ? 'Deactivate' : 'Activate';
                        toggleBtn.className = 'action-button ' + (schedule.enabled ? 'deactivate' : 'activate');
                        toggleBtn.onclick = () => toggleSchedule(index, !schedule.enabled);
                        
                        const editBtn = document.createElement('button');
                        editBtn.textContent = 'Edit';
                        editBtn.className = 'action-button edit';
                        editBtn.onclick = () => editSchedulePrompt(index);
                        
                        const deleteBtn = document.createElement('button');
                        deleteBtn.textContent = 'Delete';
                        deleteBtn.className = 'action-button delete';
                        deleteBtn.onclick = () => deleteSchedule(index);
                        
                        actionCell.appendChild(toggleBtn);
                        actionCell.appendChild(editBtn);
                        actionCell.appendChild(deleteBtn);
                    });
                })
                .catch(() => checkErrorStatus());
        }

        function toggleSchedule(id, enabled) {
            fetch('/schedule/update', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ id, enabled })
            })
            .then(response => response.ok ? response.json() : response.json().then(data => { throw new Error(data.error); }))
            .then(() => { 
                loadSchedules(); 
                checkErrorStatus(); 
                showToast(enabled ? 'Schedule activated' : 'Schedule deactivated', 'success');
            })
            .catch(error => { showToast('Failed to update schedule: ' + error.message, 'error'); checkErrorStatus(); });
        }

        function checkFields() {
            const relay = document.getElementById('relaySelect').value;
            const onTime = document.getElementById('onTime').value;
            const offTime = document.getElementById('offTime').value;
            const addBtn = document.getElementById('addScheduleBtn');
            const dayCheckboxes = document.querySelectorAll('.dayCheckbox');
            const oneDayChecked = Array.from(dayCheckboxes).some(cb => cb.checked);

            if (relay && onTime && offTime && oneDayChecked) {
                addBtn.classList.add('ready');
            } else {
                addBtn.classList.remove('ready');
            }
        }

        document.getElementById('relaySelect').addEventListener('change', checkFields);
        document.getElementById('onTime').addEventListener('input', checkFields);
        document.getElementById('offTime').addEventListener('input', checkFields);
        document.querySelectorAll('.dayCheckbox').forEach(cb => cb.addEventListener('change', checkFields));

        loadSchedules();
    </script>
</body>
</html>
)html";

#endif // PAGE_MAIN_SCHEDULES_H
