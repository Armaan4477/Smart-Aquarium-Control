#ifndef PAGE_BACKUP_RESTORE_H
#define PAGE_BACKUP_RESTORE_H

#include <Arduino.h>

const char page_backup_restore[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <script src="/theme.js"></script>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Backup & Restore</title>
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
        .btn-block {
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
        .btn-block.danger {
            background: var(--warning-color);
            color: #333;
        }
        .btn-block:hover { background: var(--primary-dark); transform: translateY(-2px); box-shadow: 0 4px 15px rgba(0,0,0,0.2); }
        .btn-block.danger:hover { background: #e6ac00; color: #333; }
        .btn-block:active { transform: translateY(1px); }
        
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
        
        input[type="file"] {
            display: none;
        }
        
        .modal {
            display: none;
            position: fixed;
            z-index: 1000;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            background-color: rgba(0,0,0,0.5);
            align-items: center;
            justify-content: center;
        }
        .modal.show { display: flex; }
        .modal-content {
            background-color: var(--card-color);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: var(--shadow);
            width: 90%;
            max-width: 500px;
            text-align: center;
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
        <h1>Backup & Restore</h1>
        <p>Save and load your complete configuration</p>
    </header>

    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Device Settings</button>
        </div>

        <div class="card">
            <h3>Backup Configuration</h3>
            <p style="margin-bottom: 15px;">Download a complete backup of all schedules, sensor calibrations, and system settings to a JSON file.</p>
            <button class="btn-block" onclick="downloadBackup()">Download Backup</button>
        </div>
        
        <div class="card">
            <h3>Restore Configuration</h3>
            <p style="margin-bottom: 15px;">Upload a previously downloaded JSON backup. <strong>Warning: This will overwrite your current configuration entirely.</strong></p>
            <input type="file" id="fileInput" accept=".json">
            <button class="btn-block danger" onclick="triggerRestore()">Upload Restore File</button>
        </div>
    </div>
    
    <div id="toast"></div>
    
    <div id="versionModal" class="modal">
        <div class="modal-content">
            <h3 style="color: var(--warning-color); border-bottom: none; margin-bottom: 10px;">Version Mismatch Warning</h3>
            <p id="modalMessage" style="margin-bottom: 15px;"></p>
            <div id="modalDetails" style="text-align: left; background: var(--background-color); padding: 15px; border-radius: var(--border-radius); margin-bottom: 20px; font-size: 0.9em; max-height: 200px; overflow-y: auto;"></div>
            <div style="display: flex; gap: 10px;">
                <button class="btn-block" onclick="closeModal()">Cancel</button>
                <button class="btn-block danger" onclick="proceedWithRestore()">Proceed Anyway</button>
            </div>
        </div>
    </div>

    <script>
        function goBack() {
            window.history.back();
        }
        
        function showToast(msg, type='success') {
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.className = type + ' show';
            setTimeout(() => t.className = '', 3000);
        }
        
        function downloadBackup() {
            fetch('/api/backup')
                .then(res => res.blob())
                .then(blob => {
                    const url = window.URL.createObjectURL(blob);
                    const a = document.createElement('a');
                    a.style.display = 'none';
                    a.href = url;
                    a.download = 'aquarium_backup.json';
                    document.body.appendChild(a);
                    a.click();
                    window.URL.revokeObjectURL(url);
                    showToast('Backup downloaded successfully', 'success');
                })
                .catch(err => {
                    console.error('Download error', err);
                    showToast('Failed to download backup', 'error');
                });
        }
        
        function triggerRestore() {
            document.getElementById('fileInput').click();
        }
        
        let pendingRestoreContent = null;

        function closeModal() {
            document.getElementById('versionModal').classList.remove('show');
            pendingRestoreContent = null;
            document.getElementById('fileInput').value = '';
        }

        function proceedWithRestore() {
            document.getElementById('versionModal').classList.remove('show');
            if (pendingRestoreContent) {
                executeRestore(pendingRestoreContent);
            }
        }
        
        function executeRestore(content) {
            fetch('/api/restore', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: content
            })
            .then(res => res.ok ? res.json() : res.json().then(data => { throw new Error(data.error || 'Unknown error'); }))
            .then(data => {
                showToast('Configuration restored successfully. Device is restarting...', 'success');
                setTimeout(() => window.location.href = '/', 5000);
            })
            .catch(err => {
                console.error('Restore error', err);
                showToast('Failed to restore: ' + err.message, 'error');
            });
            pendingRestoreContent = null;
            document.getElementById('fileInput').value = '';
        }
        
        document.getElementById('fileInput').addEventListener('change', function(e) {
            const file = e.target.files[0];
            if (!file) return;
            
            const reader = new FileReader();
            reader.onload = function(e) {
                const content = e.target.result;
                fetch('/api/restore/check', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: content
                })
                .then(res => res.ok ? res.json() : res.json().then(data => { throw new Error(data.error || 'Unknown error'); }))
                .then(data => {
                    if (data.versionMismatch) {
                        pendingRestoreContent = content;
                        const msg = `You are trying to restore a backup from version <strong>${data.backupVersion}</strong> onto firmware version <strong>${data.currentVersion}</strong>.`;
                        document.getElementById('modalMessage').innerHTML = msg;
                        
                        let detailsHtml = '';
                        if (data.ignoredFields && data.ignoredFields.length > 0) {
                            detailsHtml += '<strong>The following fields from the backup are unknown and will be ignored:</strong><ul style="margin-bottom: 10px; padding-left: 20px;">';
                            data.ignoredFields.forEach(f => detailsHtml += `<li>${f}</li>`);
                            detailsHtml += '</ul>';
                        }
                        if (data.missingFields && data.missingFields.length > 0) {
                            detailsHtml += '<strong>The following fields are missing from the backup and will retain their current values:</strong><ul style="margin-bottom: 10px; padding-left: 20px;">';
                            data.missingFields.forEach(f => detailsHtml += `<li>${f}</li>`);
                            detailsHtml += '</ul>';
                        }
                        document.getElementById('modalDetails').innerHTML = detailsHtml;
                        document.getElementById('versionModal').classList.add('show');
                    } else {
                        executeRestore(content);
                    }
                })
                .catch(err => {
                    console.error('Check error', err);
                    showToast('Failed to parse backup file: ' + err.message, 'error');
                    document.getElementById('fileInput').value = '';
                });
            };
            reader.readAsText(file);
        });
    </script>
</body>
</html>
)html";

#endif // PAGE_BACKUP_RESTORE_H
