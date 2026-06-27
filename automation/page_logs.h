#ifndef PAGE_LOGS_H
#define PAGE_LOGS_H

#include <Arduino.h>

const char logsPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <script src="/theme.js"></script>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>System Logs</title>
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
        [data-theme="dark"] .temperature-item {
            background-color: #2C2C2C !important;
        }

        [data-theme="dark"] .temperature-item:hover {
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

        .logs-table {
            width: 100%;
            border-collapse: separate;
            border-spacing: 0;
            background-color: var(--card-color);
            box-shadow: var(--shadow);
            border-radius: var(--border-radius);
            overflow: hidden;
            margin-bottom: 30px;
        }

        .logs-table th, .logs-table td {
            padding: 15px;
            text-align: left;
        }

        .logs-table th {
            background-color: var(--primary-color);
            color: white;
            font-weight: 500;
        }

        .logs-table tr:nth-child(even) {
            background-color: #f9f9f9;
        }

        .logs-table tr {
            transition: var(--transition);
            border-bottom: 1px solid #eee;
        }

        .logs-table tr:last-child {
            border-bottom: none;
        }

        .logs-table tr:hover {
            background-color: #f1f1f1;
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

        .refresh-button {
            float: right;
            background-color: var(--success-color);
        }

        .refresh-button:hover {
            background-color: #388E3C;
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

        @media (max-width: 768px) {
            .logs-table {
                font-size: 14px;
            }
            
            .logs-table th, .logs-table td {
                padding: 10px;
            }
            
            .container {
                padding: 10px;
            }
            
        }

        .loading {
            display: none;
            text-align: center;
            padding: 20px;
        }

        .loading-spinner {
            border: 4px solid #f3f3f3;
            border-top: 4px solid var(--primary-color);
            border-radius: 50%;
            width: 40px;
            height: 40px;
            animation: spin 1s linear infinite;
            margin: 0 auto;
        }

        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
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
        <h1>System Logs</h1>
        <p>View recent activity and system events</p>
    </header>
    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Dashboard</button>
            <button onclick="refreshLogs()" class="button refresh-button">Refresh Logs</button>
        </div>
        <div id="loading" class="loading">
            <div class="loading-spinner"></div>
            <p>Loading logs...</p>
        </div>
        <table class="logs-table">
            <thead>
                <tr>
                    <th>ID</th>
                    <th>Timestamp</th>
                    <th>Message</th>
                </tr>
            </thead>
            <tbody id="logsTableBody">
            </tbody>
        </table>
    </div>
    <script>
        function loadLogs(showSpinner = true) {
            if (showSpinner) {
                document.getElementById('loading').style.display = 'block';
            }
            
            fetch('/logs/data')
                .then(response => response.json())
                .then(data => {
                    const tableBody = document.getElementById('logsTableBody');
                    tableBody.innerHTML = '';
                    
                    if (data.logs && Array.isArray(data.logs)) {
                        data.logs.reverse().forEach(log => {
                            const row = tableBody.insertRow();
                            row.insertCell(0).textContent = log.id;
                            row.insertCell(1).textContent = log.timestamp;
                            row.insertCell(2).textContent = log.message;
                        });
                    }
                    
                    if (showSpinner) {
                        document.getElementById('loading').style.display = 'none';
                    }
                })
                .catch(error => {
                    console.error('Error loading logs:', error);
                    if (showSpinner) {
                        document.getElementById('loading').style.display = 'none';
                    }
                });
        }

        function refreshLogs() {
            loadLogs(true);
        }

        function goBack() {
            window.history.back();
        }

        loadLogs(true);
        setInterval(() => loadLogs(false), 10000);
    </script>
</body>
</html>
)html";

#endif // PAGE_LOGS_H
