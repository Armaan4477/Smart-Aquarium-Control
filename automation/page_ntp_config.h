#ifndef PAGE_NTP_CONFIG_H
#define PAGE_NTP_CONFIG_H

#include <Arduino.h>

const char ntpConfigPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <script src="/theme.js"></script>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>NTP Settings</title>
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

        [data-theme="dark"] input,
        [data-theme="dark"] select,
        [data-theme="dark"] textarea {
            background-color: #333 !important;
            color: white !important;
            border-color: #555 !important;
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
        .card:hover { box-shadow: 0 5px 15px rgba(0,0,0,0.15); }
        .card h3 {
            color: var(--primary-color);
            font-size: 1.4rem;
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
            margin-bottom: 20px;
        }
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
        .form-group input[type=text] {
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
        
        .action-buttons {
            display: flex;
            gap: 10px;
            margin-top: 15px;
        }
        
        .test-btn, .save-btn {
            flex: 1;
            padding: 14px;
            color: white;
            border: none;
            border-radius: var(--border-radius);
            font-size: 1.05rem;
            font-weight: 600;
            cursor: pointer;
            transition: var(--transition);
            box-shadow: var(--shadow);
        }
        
        .test-btn {
            background: var(--accent-color);
        }
        
        .test-btn:hover {
            background: #0288D1;
            transform: translateY(-2px);
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        }
        
        .save-btn {
            background: var(--primary-color);
        }
        
        .save-btn:hover:not(:disabled) { 
            background: var(--primary-dark); 
            transform: translateY(-2px); 
            box-shadow: 0 4px 15px rgba(0,0,0,0.2); 
        }
        
        .save-btn:active:not(:disabled), .test-btn:active:not(:disabled) { transform: translateY(1px); }
        
        .save-btn:disabled, .test-btn:disabled {
            background: #BDBDBD;
            cursor: not-allowed;
            transform: none;
            box-shadow: none;
            opacity: 0.7;
        }
        
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
        #toast.warning { background: var(--warning-color); color: #333; }
        
        @media (max-width: 600px) {
            .form-row { flex-direction: column; align-items: stretch; }
            .action-buttons { flex-direction: column; }
        }
        
        @media (max-width: 768px) {
            [data-theme="dark"] {
                --background-color: #000000;
                --card-color: #000000;
            }
        }
    </style>
</head>
<body>
    <header>
        <h1>NTP Settings</h1>
        <p>Configure Time Synchronization Server</p>
    </header>

    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Device Settings</button>
        </div>

        <div class="card">
            <h3>NTP Server Configuration</h3>
            
            <div class="form-row">
                <div class="form-group" style="flex: 1 1 100%;">
                    <label for="ntpServer">NTP Server Address (e.g., pool.ntp.org)</label>
                    <input type="text" id="ntpServer" placeholder="Enter NTP server IP or hostname" oninput="checkSaveState()">
                </div>
            </div>
            
            <p style="font-size: 0.9rem; color: var(--text-light); margin-bottom: 15px;">
                You can use a global server like <strong>pool.ntp.org</strong> or a local NTP server on your network.
            </p>

            <div class="action-buttons">
                <button class="test-btn" onclick="testNtp()">Test Server</button>
                <button class="save-btn" onclick="saveNtpConfig()">Save Settings</button>
            </div>
        </div>
    </div>
    
    <div id="toast"></div>

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
        
        let originalServer = '';
        let lastTestedServer = '';

        function checkSaveState() {
            const currentVal = document.getElementById('ntpServer').value.trim();
            const saveBtn = document.querySelector('.save-btn');
            
            if (currentVal !== originalServer && currentVal !== lastTestedServer) {
                saveBtn.disabled = true;
                saveBtn.title = "Test server connection before saving";
            } else {
                saveBtn.disabled = false;
                saveBtn.title = "";
            }
        }
        
        function loadConfig(retryCount) {
            retryCount = retryCount || 0;
            fetch('/api/ntpConfig', { credentials: 'include' })
                .then(res => {
                    if (!res.ok) throw new Error('HTTP ' + res.status);
                    return res.json();
                })
                .then(data => {
                    originalServer = data.server || 'pool.ntp.org';
                    document.getElementById('ntpServer').value = originalServer;
                    checkSaveState();
                })
                .catch(err => {
                    console.error('Failed to load NTP config (attempt ' + (retryCount + 1) + ')', err);
                    if (retryCount < 2) {
                        setTimeout(() => loadConfig(retryCount + 1), 1000);
                    } else {
                        showToast('Failed to load configuration', 'error');
                    }
                });
        }
        
        function testNtp() {
            const serverStr = document.getElementById('ntpServer').value.trim();
            if (!serverStr) {
                showToast('Please enter an NTP server', 'warning');
                return;
            }
            
            const btn = document.querySelector('.test-btn');
            btn.textContent = "Testing...";
            btn.disabled = true;
            
            fetch('/api/testNtp', {
                method: 'POST',
                credentials: 'include',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ server: serverStr })
            })
            .then(res => res.json())
            .then(data => {
                if (data.success) {
                    showToast('Server test successful! Connection established.', 'success');
                    lastTestedServer = serverStr;
                    checkSaveState();
                } else {
                    showToast('Server test failed: ' + (data.error || 'Unknown error'), 'error');
                }
            })
            .catch(err => {
                console.error('Test error', err);
                showToast('Error testing server', 'error');
            })
            .finally(() => {
                btn.textContent = "Test Server";
                btn.disabled = false;
            });
        }
        
        function saveNtpConfig() {
            const serverStr = document.getElementById('ntpServer').value.trim();
            if (!serverStr) {
                showToast('Please enter an NTP server', 'warning');
                return;
            }
            
            const btn = document.querySelector('.save-btn');
            btn.textContent = "Saving...";
            btn.disabled = true;
            
            fetch('/api/ntpConfig', {
                method: 'POST',
                credentials: 'include',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ server: serverStr })
            })
            .then(res => res.json())
            .then(data => {
                if (data.success) {
                    showToast('Configuration saved and applied!', 'success');
                    originalServer = serverStr;
                    checkSaveState();
                } else {
                    showToast('Failed to save configuration: ' + (data.error || 'Unknown error'), 'error');
                }
            })
            .catch(err => {
                console.error('Save error', err);
                showToast('Error saving configuration', 'error');
            })
            .finally(() => {
                btn.textContent = "Save Settings";
                btn.disabled = false;
            });
        }
        
        window.addEventListener('load', () => loadConfig(0));
    </script>
</body>
</html>
)html";

#endif // PAGE_NTP_CONFIG_H
