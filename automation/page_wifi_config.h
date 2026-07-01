#ifndef PAGE_WIFI_CONFIG_H
#define PAGE_WIFI_CONFIG_H

#include <Arduino.h>

const char wifiConfigPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <script src="/theme.js"></script>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Wi-Fi Settings</title>
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
            max-width: 800px;
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
        
        .form-row {
            display: flex;
            flex-wrap: wrap;
            gap: 15px;
            margin-bottom: 20px;
        }
        .form-group {
            display: flex;
            flex-direction: column;
            margin-bottom: 5px;
        }
        .form-group label {
            font-weight: 600;
            margin-bottom: 8px;
            color: var(--text-color);
        }
        .form-group input, .form-group select {
            padding: 12px;
            border: 1px solid #ddd;
            border-radius: 6px;
            font-size: 1rem;
            transition: var(--transition);
        }
        .form-group input:focus, .form-group select:focus {
            outline: none;
            border-color: var(--primary-color);
            box-shadow: 0 0 0 2px rgba(25,118,210,0.2);
        }
        
        [data-theme="dark"] .form-group input, [data-theme="dark"] .form-group select {
            background-color: #333 !important;
            color: white !important;
            border-color: #555 !important;
        }

        .save-btn, .scan-btn, .redirect-btn {
            background-color: var(--success-color);
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: var(--border-radius);
            cursor: pointer;
            font-size: 1.1rem;
            font-weight: bold;
            transition: var(--transition);
            width: 100%;
            margin-top: 10px;
        }
        .save-btn:hover, .scan-btn:hover, .redirect-btn:hover {
            background-color: #3d8b40;
            transform: translateY(-2px);
        }
        .save-btn:disabled, .scan-btn:disabled {
            background-color: #a5d6a7;
            cursor: not-allowed;
            transform: none;
        }
        
        .scan-btn {
            background-color: var(--primary-color);
            width: auto;
            margin-top: 0;
            margin-bottom: 15px;
        }
        .scan-btn:hover {
            background-color: var(--primary-dark);
        }
        
        #toast {
            visibility: hidden;
            min-width: 250px;
            background-color: #333;
            color: #fff;
            text-align: center;
            border-radius: 4px;
            padding: 16px;
            position: fixed;
            z-index: 1000;
            left: 50%;
            bottom: 30px;
            transform: translateX(-50%);
            font-size: 16px;
            box-shadow: 0 4px 12px rgba(0,0,0,0.15);
        }
        #toast.show {
            visibility: visible;
            animation: fadein 0.5s, fadeout 0.5s 2.5s;
        }
        #toast.success { background-color: var(--success-color); }
        #toast.error { background-color: var(--error-color); }
        
        @keyframes fadein {
            from {bottom: 0; opacity: 0;}
            to {bottom: 30px; opacity: 1;}
        }
        @keyframes fadeout {
            from {bottom: 30px; opacity: 1;}
            to {bottom: 0; opacity: 0;}
        }

        .password-wrapper {
            position: relative;
            display: flex;
            align-items: center;
        }
        .password-wrapper input {
            width: 100%;
            padding-right: 40px;
        }
        .password-toggle {
            position: absolute;
            right: 12px;
            cursor: pointer;
            color: #666;
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100%;
        }
        .password-toggle:hover {
            color: #333;
        }
        
        .modal {
            display: none;
            position: fixed;
            z-index: 100;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            background-color: rgba(0,0,0,0.5);
            align-items: center;
            justify-content: center;
        }
        .modal-content {
            background-color: var(--card-color);
            margin: auto;
            padding: 30px;
            border-radius: var(--border-radius);
            max-width: 400px;
            text-align: center;
            box-shadow: var(--shadow);
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
        <h1>Wi-Fi Settings</h1>
        <p>Configure device connection & access point</p>
    </header>

    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Device Settings</button>
        </div>

        <div class="card">
            <h3>Home Wi-Fi Network</h3>
            <p style="font-size: 0.9rem; color: var(--text-light); margin-bottom: 15px;">
                Connect the device to your home Wi-Fi network.
            </p>
            
            <button class="scan-btn" id="scanBtn" onclick="scanNetworks()">Scan for Networks</button>

            <div class="form-row" id="networkListRow" style="display:none;">
                <div class="form-group" style="flex: 1 1 100%;">
                    <label for="networkList">Available Networks</label>
                    <select id="networkList" onchange="selectNetwork()"></select>
                </div>
            </div>

            <div class="form-row">
                <div class="form-group" style="flex: 1 1 100%;">
                    <label for="wifiSsid">Wi-Fi Name (SSID)</label>
                    <input type="text" id="wifiSsid" placeholder="Enter Wi-Fi name">
                </div>
            </div>
            
            <div class="form-row">
                <div class="form-group" style="flex: 1 1 100%;">
                    <label for="wifiPassword">Wi-Fi Password</label>
                    <div class="password-wrapper">
                        <input type="password" id="wifiPassword" placeholder="Enter Wi-Fi password">
                        <span class="password-toggle" id="toggleWifiPassword">
                            <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path>
                                <circle cx="12" cy="12" r="3"></circle>
                            </svg>
                        </span>
                    </div>
                </div>
            </div>
        </div>

        <div class="card">
            <h3>Device Access Point (AP) Settings</h3>
            <p style="font-size: 0.9rem; color: var(--text-light); margin-bottom: 15px;">
                Configure the fallback network hosted by the device itself.
            </p>
            
            <div class="form-row">
                <div class="form-group" style="flex: 1 1 100%;">
                    <label for="apSsid">AP Name (SSID)</label>
                    <input type="text" id="apSsid" placeholder="Enter AP name">
                </div>
            </div>
            
            <div class="form-row">
                <div class="form-group" style="flex: 1 1 100%;">
                    <label for="apPassword">AP Password</label>
                    <div class="password-wrapper">
                        <input type="password" id="apPassword" placeholder="Enter AP password">
                        <span class="password-toggle" id="toggleApPassword">
                            <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path>
                                <circle cx="12" cy="12" r="3"></circle>
                            </svg>
                        </span>
                    </div>
                </div>
            </div>
        </div>

        <button class="save-btn" onclick="saveWifiConfig()">Save Settings</button>
    </div>
    
    <div id="toast"></div>

    <div id="successModal" class="modal">
        <div class="modal-content">
            <h2 style="color: var(--success-color); margin-bottom: 15px;">Connection Successful!</h2>
            <p style="margin-bottom: 20px; color: var(--text-color);">The device has successfully connected to your Wi-Fi network.</p>
            <p style="margin-bottom: 20px; font-weight: bold; color: var(--text-color);">New IP Address: <span id="newIpAddress"></span></p>
            <button class="redirect-btn" onclick="redirectToNewIp()">Go to New IP Address</button>
        </div>
    </div>

    <script>
        document.addEventListener('DOMContentLoaded', function() {
            setupPasswordToggle('toggleWifiPassword', 'wifiPassword');
            setupPasswordToggle('toggleApPassword', 'apPassword');
            loadConfig();
        });

        function setupPasswordToggle(toggleId, inputId) {
            const toggle = document.getElementById(toggleId);
            const input = document.getElementById(inputId);

            toggle.addEventListener('click', function () {
                const type = input.getAttribute('type') === 'password' ? 'text' : 'password';
                input.setAttribute('type', type);
                
                if (type === 'text') {
                    toggle.innerHTML = '<svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"></path><line x1="1" y1="1" x2="23" y2="23"></line></svg>';
                } else {
                    toggle.innerHTML = '<svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle></svg>';
                }
            });
        }

        function goBack() {
            window.history.back();
        }
        
        function showToast(msg, type='success') {
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.className = type + ' show';
            setTimeout(() => t.className = '', 3000);
        }
        
        function loadConfig() {
            fetch('/api/wifi/config', { credentials: 'include' })
                .then(res => res.json())
                .then(data => {
                    document.getElementById('wifiSsid').value = data.ssid || '';
                    document.getElementById('apSsid').value = data.apSsid || '';
                })
                .catch(err => {
                    console.error('Failed to load wifi config', err);
                    showToast('Failed to load configuration', 'error');
                });
        }
        
        function scanNetworks() {
            const btn = document.getElementById('scanBtn');
            btn.textContent = "Scanning...";
            btn.disabled = true;

            fetch('/api/wifi/scan', { credentials: 'include' })
                .then(res => res.json())
                .then(data => {
                    const list = document.getElementById('networkList');
                    list.innerHTML = '<option value="">-- Select a Network --</option>';
                    data.forEach(net => {
                        const opt = document.createElement('option');
                        opt.value = net.ssid;
                        opt.textContent = `${net.ssid} (${net.rssi}dBm)`;
                        list.appendChild(opt);
                    });
                    document.getElementById('networkListRow').style.display = 'flex';
                })
                .catch(err => {
                    console.error('Scan error', err);
                    showToast('Error scanning networks', 'error');
                })
                .finally(() => {
                    btn.textContent = "Scan for Networks";
                    btn.disabled = false;
                });
        }

        function selectNetwork() {
            const list = document.getElementById('networkList');
            if (list.value) {
                document.getElementById('wifiSsid').value = list.value;
            }
        }

        let newIp = '';
        function redirectToNewIp() {
            if (newIp) {
                window.location.href = 'http://' + newIp;
            }
        }
        
        function saveWifiConfig() {
            const ssid = document.getElementById('wifiSsid').value.trim();
            const password = document.getElementById('wifiPassword').value.trim();
            const apSsid = document.getElementById('apSsid').value.trim();
            const apPassword = document.getElementById('apPassword').value.trim();

            if (ssid.length > 31 || password.length > 63 || apSsid.length > 31 || apPassword.length > 63) {
                showToast('Fields exceed maximum length limit.', 'error');
                return;
            }

            const data = {
                ssid: ssid,
                password: password,
                apSsid: apSsid,
                apPassword: apPassword
            };
            
            const btn = document.querySelector('.save-btn');
            btn.textContent = "Saving & Connecting...";
            btn.disabled = true;
            
            fetch('/api/wifi/config', {
                method: 'POST',
                credentials: 'include',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(data)
            })
            .then(res => res.json())
            .then(data => {
                if (data.success) {
                    btn.textContent = "Connecting to Wi-Fi...";
                    pollWifiStatus(0);
                } else {
                    showToast(data.error || 'Failed to save configuration', 'error');
                    btn.textContent = "Save Settings";
                    btn.disabled = false;
                }
            })
            .catch(err => {
                // If it fails instantly, the channel might have already switched. Start polling just in case.
                btn.textContent = "Connecting to Wi-Fi...";
                pollWifiStatus(0);
            });
        }

        function pollWifiStatus(attempts) {
            if (attempts > 30) {
                showToast('Wi-Fi connection timed out or failed.', 'error');
                const btn = document.querySelector('.save-btn');
                btn.textContent = "Save Settings";
                btn.disabled = false;
                return;
            }
            
            fetch('/api/wifi/status', { credentials: 'include' })
                .then(res => {
                    if (!res.ok) throw new Error('Network error');
                    return res.json();
                })
                .then(data => {
                    const btn = document.querySelector('.save-btn');
                    if (data.status === 'connected') {
                        newIp = data.ip;
                        document.getElementById('newIpAddress').textContent = newIp;
                        document.getElementById('successModal').style.display = 'flex';
                        btn.textContent = "Save Settings";
                        btn.disabled = false;
                    } else if (data.status === 'failed') {
                        showToast('Failed to connect to Wi-Fi.', 'error');
                        btn.textContent = "Save Settings";
                        btn.disabled = false;
                    } else {
                        setTimeout(() => pollWifiStatus(attempts + 1), 2000);
                    }
                })
                .catch(err => {
                    // AP channel likely changed, causing momentary disconnect. Just retry.
                    setTimeout(() => pollWifiStatus(attempts + 1), 2000);
                });
        }
    </script>
</body>
</html>
)html";

#endif // PAGE_WIFI_CONFIG_H
