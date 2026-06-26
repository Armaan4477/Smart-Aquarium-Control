#ifndef PAGE_EMAIL_CONFIG_H
#define PAGE_EMAIL_CONFIG_H

#include <Arduino.h>

const char emailConfigPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Email Settings</title>
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
        .form-group input[type=email],
        .form-group input[type=text],
        .form-group input[type=password] {
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
        .password-wrapper {
            position: relative;
            display: flex;
            align-items: center;
        }
        .password-wrapper input {
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
    </style>
</head>
<body>
    <header>
        <h1>Email Settings</h1>
        <p>Manage system notifications and credentials</p>
    </header>

    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Dashboard</button>
        </div>

        <div class="card">
            <h3>Configuration</h3>
            <div class="toggle-row">
                <input type="checkbox" id="emailEnabled">
                <label for="emailEnabled">Enable Email Notifications</label>
            </div>
            
            <div class="form-row">
                <div class="form-group" style="flex: 1 1 100%;">
                    <label for="senderAccount">Sender Email</label>
                    <input type="email" id="senderAccount" placeholder="e.g. your_email@gmail.com">
                </div>
            </div>
            
            <div class="form-row">
                <div class="form-group" style="flex: 1 1 100%;">
                    <label for="senderPassword">Sender App Password</label>
                    <div class="password-wrapper">
                        <input type="password" id="senderPassword" placeholder="e.g. abcd efgh ijkl mnop">
                        <span class="password-toggle" id="togglePassword">
                            <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" id="eyeIcon">
                                <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path>
                                <circle cx="12" cy="12" r="3"></circle>
                            </svg>
                        </span>
                    </div>
                </div>
            </div>
            
            <div class="form-row">
                <div class="form-group" style="flex: 1 1 100%;">
                    <label for="recipient">Receiver Email</label>
                    <input type="email" id="recipient" placeholder="e.g. receiver_email@gmail.com">
                </div>
            </div>

            <button class="save-btn" onclick="saveEmailConfig()">Save Settings</button>
        </div>
    </div>
    
    <div id="toast"></div>

    <script>
        document.addEventListener('DOMContentLoaded', function() {
            const togglePassword = document.getElementById('togglePassword');
            const passwordInput = document.getElementById('senderPassword');
            const eyeIcon = document.getElementById('eyeIcon');

            togglePassword.addEventListener('click', function () {
                const type = passwordInput.getAttribute('type') === 'password' ? 'text' : 'password';
                passwordInput.setAttribute('type', type);
                
                if (type === 'text') {
                    eyeIcon.innerHTML = '<path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"></path><line x1="1" y1="1" x2="23" y2="23"></line>';
                } else {
                    eyeIcon.innerHTML = '<path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle>';
                }
            });
        });

        function goBack() { window.history.back(); }
        
        function showToast(msg, type='success') {
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.className = type + ' show';
            setTimeout(() => t.className = '', 3000);
        }
        
        function loadConfig(retryCount) {
            retryCount = retryCount || 0;
            fetch('/api/emailConfig', { credentials: 'include' })
                .then(res => {
                    if (!res.ok) throw new Error('HTTP ' + res.status);
                    return res.json();
                })
                .then(data => {
                    document.getElementById('emailEnabled').checked = data.enabled;
                    document.getElementById('senderAccount').value = data.senderAccount || '';
                    document.getElementById('senderPassword').value = data.senderPassword || '';
                    document.getElementById('recipient').value = data.recipient || '';
                })
                .catch(err => {
                    console.error('Failed to load email config (attempt ' + (retryCount + 1) + ')', err);
                    if (retryCount < 2) {
                        setTimeout(() => loadConfig(retryCount + 1), 1000);
                    } else {
                        showToast('Failed to load configuration', 'error');
                    }
                });
        }
        
        function saveEmailConfig() {
            const data = {
                enabled: document.getElementById('emailEnabled').checked,
                senderAccount: document.getElementById('senderAccount').value,
                senderPassword: document.getElementById('senderPassword').value,
                recipient: document.getElementById('recipient').value
            };
            
            const btn = document.querySelector('.save-btn');
            btn.textContent = "Saving...";
            btn.disabled = true;
            
            fetch('/api/emailConfig', {
                method: 'POST',
                credentials: 'include',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(data)
            })
            .then(res => res.json())
            .then(data => {
                if (data.success) {
                    showToast('Configuration saved successfully!', 'success');
                } else {
                    showToast('Failed to save configuration', 'error');
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

#endif // PAGE_EMAIL_CONFIG_H
