#ifndef PAGE_OTA_H
#define PAGE_OTA_H

#include <Arduino.h>

const char otaPage[] PROGMEM = R"html(
<!DOCTYPE html>
<html>
<head>
    <script src="/theme.js"></script>
    <link rel="icon" type="image/png" href="/favicon.png">
    <link rel="shortcut icon" type="image/png" href="/favicon.png">
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>OTA Firmware Update</title>
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

        [data-theme="dark"] input[type="file"] {
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
        .button:active {
            transform: translateY(1px);
        }
        .upload-btn {
            width: 100%;
        }
        .button:hover:not(:disabled) {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.15);
        }
        .button:disabled {
            background-color: var(--text-light);
            cursor: not-allowed;
            transform: none;
            box-shadow: none;
        }

        .container {
            padding: 20px;
            max-width: 600px;
            margin: auto;
        }
        
        .card {
            background: var(--card-color);
            border-radius: var(--border-radius);
            padding: 30px;
            box-shadow: var(--shadow);
            margin-bottom: 20px;
            transition: var(--transition);
        }
        
        .card h2 {
            margin-top: 0;
            color: var(--primary-color);
            border-bottom: 2px solid var(--primary-light);
            padding-bottom: 10px;
            margin-bottom: 20px;
        }
        
        .form-group {
            margin-bottom: 25px;
        }
        
        .form-group label {
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: var(--text-color);
        }
        
        input[type="file"] {
            width: 100%;
            padding: 12px;
            border: 1px solid #ddd;
            border-radius: var(--border-radius);
            font-size: 1rem;
            background-color: #fff;
            transition: var(--transition);
            cursor: pointer;
        }
        
        .progress-container {
            width: 100%;
            background-color: #ddd;
            border-radius: var(--border-radius);
            overflow: hidden;
            margin-bottom: 15px;
            display: none;
            height: 20px;
        }
        
        [data-theme="dark"] .progress-container {
            background-color: #444;
        }
        
        .progress-bar {
            height: 100%;
            background-color: var(--success-color);
            width: 0%;
            transition: width 0.3s ease;
        }
        
        #statusMessage {
            text-align: center;
            font-weight: 600;
            margin-top: 15px;
            padding: 10px;
            border-radius: var(--border-radius);
            display: none;
        }
        
        .status-success {
            background-color: rgba(76, 175, 80, 0.1);
            color: var(--success-color);
            border: 1px solid var(--success-color);
        }
        
        .status-error {
            background-color: rgba(244, 67, 54, 0.1);
            color: var(--error-color);
            border: 1px solid var(--error-color);
        }
        
        .info-note {
            background-color: rgba(33, 150, 243, 0.1);
            color: var(--text-color);
            padding: 12px 15px;
            border-radius: var(--border-radius);
            margin-bottom: 20px;
            font-size: 0.95rem;
            border-left: 4px solid var(--primary-color);
        }
        
        .info-note a {
            color: var(--primary-color);
            font-weight: bold;
            text-decoration: none;
        }
        
        .info-note a:hover {
            text-decoration: underline;
        }
        
        .warning-text {
            color: var(--warning-color);
            font-size: 0.9rem;
            margin-top: 10px;
            display: block;
            text-align: center;
        }
        [data-theme="light"] .warning-text {
            color: #d32f2f;
        }
    </style>
</head>
<body>
    <header>
        <h1>Firmware Update</h1>
        <p>Upload a new compiled .bin file via OTA</p>
    </header>
    <div class="container">
        <div class="header-actions">
            <button onclick="goBack()" class="button">Back to Dashboard</button>
        </div>
        <div class="card">
            <h2>Select Firmware</h2>
            
            <div class="info-note">
                <strong>Tip:</strong> It is highly recommended to <a href="/backuprestore">backup your configuration</a> before performing an OTA update.
            </div>

            <form id="uploadForm" enctype="multipart/form-data">
                <div class="form-group">
                    <label for="firmwareFile">Compiled .bin File:</label>
                    <input type="file" id="firmwareFile" name="update" accept=".bin" required>
                </div>
                
                <div class="progress-container" id="progressContainer">
                    <div class="progress-bar" id="progressBar"></div>
                </div>
                
                <div id="statusMessage"></div>
                
                <button type="submit" class="button upload-btn" id="uploadBtn">Upload and Update</button>
                <span class="warning-text">Do not close this page or turn off the device during update!</span>
            </form>
        </div>
    </div>

    <script>
        function goBack() {
            window.history.back();
        }

        const form = document.getElementById('uploadForm');
        const fileInput = document.getElementById('firmwareFile');
        const uploadBtn = document.getElementById('uploadBtn');
        const progressContainer = document.getElementById('progressContainer');
        const progressBar = document.getElementById('progressBar');
        const statusMessage = document.getElementById('statusMessage');

        form.addEventListener('submit', function(e) {
            e.preventDefault();
            
            const file = fileInput.files[0];
            if (!file) {
                showStatus('Please select a file first.', 'error');
                return;
            }
            
            if (!file.name.endsWith('.bin')) {
                showStatus('Invalid file type. Please select a .bin file.', 'error');
                return;
            }

            uploadBtn.disabled = true;
            uploadBtn.innerText = 'Uploading...';
            progressContainer.style.display = 'block';
            statusMessage.style.display = 'none';
            progressBar.style.width = '0%';

            const formData = new FormData();
            formData.append('update', file, file.name);

            const xhr = new XMLHttpRequest();
            
            xhr.upload.addEventListener('progress', function(e) {
                if (e.lengthComputable) {
                    const percentComplete = Math.round((e.loaded / e.total) * 100);
                    progressBar.style.width = percentComplete + '%';
                    if (percentComplete === 100) {
                        uploadBtn.innerText = 'Flashing... (Please wait)';
                    }
                }
            });

            xhr.addEventListener('load', function() {
                if (xhr.status === 200 && xhr.responseText.trim() === 'OK') {
                    showStatus('Update Successful! The device is rebooting. You will be redirected shortly.', 'success');
                    progressBar.style.backgroundColor = 'var(--success-color)';
                    setTimeout(() => {
                        window.location.href = '/';
                    }, 10000); // Wait 10 seconds before redirecting
                } else {
                    showStatus('Update Failed: ' + (xhr.responseText || 'Unknown error'), 'error');
                    resetForm();
                }
            });

            xhr.addEventListener('error', function() {
                showStatus('Network error occurred during upload.', 'error');
                resetForm();
            });

            xhr.open('POST', '/update', true);
            xhr.send(formData);
        });
        
        function showStatus(msg, type) {
            statusMessage.innerText = msg;
            statusMessage.className = 'status-' + type;
            statusMessage.style.display = 'block';
        }
        
        function resetForm() {
            uploadBtn.disabled = false;
            uploadBtn.innerText = 'Upload and Update';
            progressBar.style.backgroundColor = 'var(--error-color)';
        }
    </script>
</body>
</html>
)html";

#endif
