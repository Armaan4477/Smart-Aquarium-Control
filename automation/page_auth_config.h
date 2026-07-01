#ifndef PAGE_AUTH_CONFIG_H
#define PAGE_AUTH_CONFIG_H

#include <Arduino.h>

const char authConfigPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <script src="/theme.js"></script>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Authentication Settings</title>
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
        .form-group input {
            padding: 12px;
            border: 1px solid #ddd;
            border-radius: 6px;
            font-size: 1rem;
            transition: var(--transition);
        }
        .form-group input:focus {
            outline: none;
            border-color: var(--primary-color);
            box-shadow: 0 0 0 2px rgba(25,118,210,0.2);
        }
        
        [data-theme="dark"] .form-group input {
            background-color: #333 !important;
            color: white !important;
            border-color: #555 !important;
        }

        .save-btn {
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
        .save-btn:hover {
            background-color: #3d8b40;
            transform: translateY(-2px);
        }
        .save-btn:disabled {
            background-color: #a5d6a7;
            cursor: not-allowed;
            transform: none;
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
        <h1>Authentication Settings</h1>
        <p>Manage device login credentials</p>
    </header>

    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Device Settings</button>
        </div>

        <div class="card">
            <h3>Login Credentials</h3>
            
            <div class="form-row">
                <div class="form-group" style="flex: 1 1 100%;">
                    <label for="authUsername">Username</label>
                    <input type="text" id="authUsername" placeholder="Enter new username">
                </div>
            </div>
            
            <div class="form-row">
                <div class="form-group" style="flex: 1 1 100%;">
                    <label for="authPassword">Password</label>
                    <div class="password-wrapper">
                        <input type="password" id="authPassword" placeholder="Enter new password">
                        <span class="password-toggle" id="togglePassword">
                            <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" id="eyeIcon">
                                <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path>
                                <circle cx="12" cy="12" r="3"></circle>
                            </svg>
                        </span>
                    </div>
                </div>
            </div>
            <p style="font-size: 0.9rem; color: var(--text-light); margin-bottom: 20px;">
                Note: Updating these credentials will log out all current sessions. 
                If credentials are forgotten, they fallback to "Admin" / "Admin" on EEPROM corruption or clearance.
            </p>
            <button class="save-btn" onclick="saveAuthConfig()">Save Settings</button>
        </div>
    </div>
    
    <div id="toast"></div>

    <script>
        document.addEventListener('DOMContentLoaded', function() {
            const togglePassword = document.getElementById('togglePassword');
            const passwordInput = document.getElementById('authPassword');
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

        function goBack() {
            window.history.back();
        }
        
        function showToast(msg, type='success') {
            const t = document.getElementById('toast');
            t.textContent = msg;
            t.className = type + ' show';
            setTimeout(() => t.className = '', 3000);
        }
        
        function loadConfig(retryCount) {
            retryCount = retryCount || 0;
            fetch('/api/authConfig', { credentials: 'include' })
                .then(res => {
                    if (!res.ok) throw new Error('HTTP ' + res.status);
                    return res.json();
                })
                .then(data => {
                    document.getElementById('authUsername').value = data.username || '';
                    document.getElementById('authPassword').value = data.password || '';
                })
                .catch(err => {
                    console.error('Failed to load auth config (attempt ' + (retryCount + 1) + ')', err);
                    if (retryCount < 2) {
                        setTimeout(() => loadConfig(retryCount + 1), 1000);
                    } else {
                        showToast('Failed to load configuration', 'error');
                    }
                });
        }
        
        function saveAuthConfig() {
            const username = document.getElementById('authUsername').value.trim();
            const password = document.getElementById('authPassword').value.trim();

            if (!username || !password) {
                showToast('Username and password cannot be empty', 'error');
                return;
            }

            if (username.length > 31 || password.length > 31) {
                showToast('Username and password must be 31 characters or less', 'error');
                return;
            }

            const data = {
                username: username,
                password: password
            };
            
            const btn = document.querySelector('.save-btn');
            btn.textContent = "Saving...";
            btn.disabled = true;
            
            fetch('/api/authConfig', {
                method: 'POST',
                credentials: 'include',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(data)
            })
            .then(res => res.json())
            .then(data => {
                if (data.success) {
                    showToast('Configuration saved successfully! Reloading in 3s...', 'success');
                    setTimeout(() => {
                        window.location.reload();
                    }, 3000);
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

#endif // PAGE_AUTH_CONFIG_H
