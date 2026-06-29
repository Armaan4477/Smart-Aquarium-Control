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


        /* Table overrides */
        [data-theme="dark"] .logs-table tr:nth-child(even),
        [data-theme="dark"] .logs-table tr:nth-child(odd),
        [data-theme="dark"] .logs-table tr {
            background-color: #2C2C2C !important;
            color: var(--text-color) !important;
        }
        
        [data-theme="dark"] .logs-table tr:nth-child(even) {
            background-color: #242424 !important;
        }

        [data-theme="dark"] .logs-table tr:hover {
            background-color: #3C3C3C !important;
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
