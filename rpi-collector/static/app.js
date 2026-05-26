// Configuration
const REFRESH_INTERVAL_MS = 60000; // 60 seconds
let tempChart = null;

// DOM Elements
const els = {
    connStatus: document.getElementById('conn-status'),
    connText: document.getElementById('conn-text'),
    valIntTemp: document.getElementById('val-int-temp'),
    valExtTemp: document.getElementById('val-ext-temp'),
    relay1: document.getElementById('relay-toggle-1'),
    relay2: document.getElementById('relay-toggle-2'),
    relay3: document.getElementById('relay-toggle-3'),
    valUptime: document.getElementById('val-uptime'),
    lastUpdated: document.getElementById('last-updated'),
    timeRange: document.getElementById('time-range'),
    refreshBtn: document.getElementById('refresh-btn'),
    logsBody: document.getElementById('logs-body'),
    logSearch: document.getElementById('log-search'),
    errorBanner: document.getElementById('system-error-banner'),
    errorText: document.getElementById('system-error-text')
};

// State
let currentLogs = [];

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    initChart();
    fetchData();
    
    // Event Listeners
    els.refreshBtn.addEventListener('click', forceFetchData);
    els.timeRange.addEventListener('change', fetchChartData);
    els.logSearch.addEventListener('input', renderLogs);
    
    // Auto-refresh
    setInterval(fetchData, REFRESH_INTERVAL_MS);
    
    // Initialize external links
    initializeLinks();
});

async function initializeLinks() {
    try {
        const res = await fetch('/api/config');
        if (!res.ok) throw new Error('Failed to fetch config');
        const config = await res.json();
        
        const ip = config.esp32_ip;
        document.getElementById('link-main-sched').href = `http://${ip}/mainSchedules`;
        document.getElementById('link-temp-sched').href = `http://${ip}/tempschedules`;
        document.getElementById('link-temp-ctrl').href = `http://${ip}/tempcontrol`;
        document.getElementById('link-disp-ctrl').href = `http://${ip}/displayctrl`;
    } catch (e) {
        console.error("Could not initialize links:", e);
    }
}

// Set Connection Status
function setConnectionStatus(status) {
    els.connStatus.className = 'dot ' + status;
    switch(status) {
        case 'loading': els.connText.textContent = 'Updating...'; break;
        case 'online': els.connText.textContent = 'Live'; break;
        case 'error': els.connText.textContent = 'Offline'; break;
    }
}

// Format Uptime
function formatUptime(seconds, days) {
    if (days > 0) return `${days}d ${Math.floor((seconds % 86400) / 3600)}h`;
    const h = Math.floor(seconds / 3600);
    const m = Math.floor((seconds % 3600) / 60);
    if (h > 0) return `${h}h ${m}m`;
    return `${m}m`;
}

// Format Date
function formatDateTime(isoString) {
    const d = new Date(isoString);
    return d.toLocaleString(undefined, { 
        month: 'short', day: 'numeric', 
        hour: '2-digit', minute: '2-digit', second: '2-digit'
    });
}

// Force Fetch Data (Ping ESP32)
async function forceFetchData() {
    const originalText = els.refreshBtn.textContent;
    els.refreshBtn.textContent = '...';
    els.refreshBtn.disabled = true;
    setConnectionStatus('loading');
    try {
        const res = await fetch('/api/force_refresh', { method: 'POST' });
        if (!res.ok) throw new Error('Force refresh request failed');
        await fetchData();
    } catch (e) {
        console.error("Force refresh failed:", e);
        alert("Failed to force refresh. Check connection.");
        setConnectionStatus('error');
    } finally {
        els.refreshBtn.textContent = originalText;
        els.refreshBtn.disabled = false;
    }
}

// Fetch All Data
async function fetchData() {
    setConnectionStatus('loading');
    try {
        const [isOffline] = await Promise.all([
            fetchLatestStatus(),
            fetchChartData(),
            fetchLogs()
        ]);
        
        if (isOffline) {
            setConnectionStatus('error');
        } else {
            setConnectionStatus('online');
        }
        
        els.lastUpdated.textContent = new Date().toLocaleTimeString();
    } catch (error) {
        console.error("Fetch error:", error);
        setConnectionStatus('error');
    }
}

// Fetch Latest Status
async function fetchLatestStatus() {
    const res = await fetch('/temperature/latest');
    if (!res.ok) throw new Error('Status fetch failed');
    const data = await res.json();
    
    // Temps
    els.valIntTemp.textContent = data.internal_c !== null ? `${data.internal_c.toFixed(1)}°C` : '--°C';
    els.valExtTemp.textContent = data.external_c !== null ? `${data.external_c.toFixed(1)}°C` : '--°C';
    
    // Uptime
    // Show '--' when offline OR when the container just restarted and hasn't
    // yet received its first health ping (uptime_pending = true).
    if (data.is_offline || data.uptime_pending) {
        els.valUptime.textContent = "--";
    } else {
        els.valUptime.textContent = formatUptime(data.uptime_seconds, data.uptime_days);
    }
    
    // Error Banner
    if (data.has_error) {
        let msg = "System Error Active";
        if (data.temp_error && data.ext_temp_error) {
            msg = "Critical: Both internal and external temperature sensors failed!";
        } else if (data.temp_error) {
            msg = "Critical: Internal temperature sensor failed!";
        } else if (data.ext_temp_error) {
            msg = "Critical: External temperature sensor failed!";
        }
        els.errorText.textContent = msg;
        els.errorBanner.classList.remove('hidden');
    } else {
        els.errorBanner.classList.add('hidden');
    }
    
    // Relays
    updateRelayBadge(els.relay1, document.getElementById('override-1'), data.relay1, data.override1);
    updateRelayBadge(els.relay2, document.getElementById('override-2'), data.relay2, data.override2);
    // Assuming relay 3 uses override 1 logic per backend implementation
    updateRelayBadge(els.relay3, document.getElementById('override-3'), data.relay3, data.override1); 
    
    return data.is_offline;
}

function updateRelayBadge(relayEl, overrideEl, state, override) {
    if (state === 1) {
        relayEl.classList.add('on');
        relayEl.textContent = 'ON';
    } else {
        relayEl.classList.remove('on');
        relayEl.textContent = 'OFF';
    }
    
    if (override === 1) {
        overrideEl.classList.remove('hidden');
    } else {
        overrideEl.classList.add('hidden');
    }
}

// Control Relays
async function toggleRelay(relayNumber) {
    const el = document.getElementById(`relay-toggle-${relayNumber}`);
    const originalText = el.textContent;
    el.textContent = '...';
    try {
        const res = await fetch(`/proxy/relay/${relayNumber}`, { method: 'POST' });
        if (!res.ok) throw new Error('Failed to toggle relay');
        // Immediately fetch status to update UI
        await fetchLatestStatus();
    } catch (e) {
        console.error(e);
        el.textContent = originalText;
        alert("Error toggling relay. Check connection.");
    }
}

// Initialize Chart.js
function initChart() {
    const ctx = document.getElementById('tempChart').getContext('2d');
    
    Chart.defaults.color = '#94a3b8';
    Chart.defaults.font.family = "'Outfit', sans-serif";
    
    tempChart = new Chart(ctx, {
        type: 'line',
        data: {
            datasets: [
                {
                    label: 'Internal Temp °C',
                    borderColor: '#38bdf8', // accent-blue
                    backgroundColor: 'rgba(56, 189, 248, 0.1)',
                    borderWidth: 2,
                    pointRadius: 0,
                    pointHoverRadius: 6,
                    fill: true,
                    tension: 0.4,
                    data: []
                },
                {
                    label: 'External Temp °C',
                    borderColor: '#34d399', // accent-green
                    backgroundColor: 'rgba(52, 211, 153, 0.05)',
                    borderWidth: 2,
                    borderDash: [5, 5],
                    pointRadius: 0,
                    pointHoverRadius: 6,
                    fill: false,
                    tension: 0.4,
                    data: []
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            interaction: {
                mode: 'index',
                intersect: false,
            },
            plugins: {
                tooltip: {
                    backgroundColor: 'rgba(15, 23, 42, 0.9)',
                    titleColor: '#f8fafc',
                    bodyColor: '#f8fafc',
                    borderColor: 'rgba(255, 255, 255, 0.1)',
                    borderWidth: 1,
                    padding: 12,
                    boxPadding: 6,
                    usePointStyle: true,
                    callbacks: {
                        label: function(context) {
                            let label = context.dataset.label || '';
                            if (label) {
                                label += ': ';
                            }
                            if (context.parsed.y !== null) {
                                label += context.parsed.y.toFixed(2) + '°C';
                            }
                            return label;
                        }
                    }
                },
                legend: {
                    position: 'top',
                    labels: { usePointStyle: true, boxWidth: 8 }
                }
            },
            scales: {
                x: {
                    type: 'time',
                    time: {
                        tooltipFormat: 'MMM D, HH:mm'
                    },
                    grid: { color: 'rgba(255, 255, 255, 0.05)' },
                    border: { display: false }
                },
                y: {
                    grid: { color: 'rgba(255, 255, 255, 0.05)' },
                    border: { display: false },
                    ticks: {
                        callback: function(value) {
                            // Use 2 dp when the visible Y range is narrow (< 1 °C)
                            // so closely-packed values remain distinguishable.
                            const range = this.max - this.min;
                            const decimals = range < 1 ? 2 : 1;
                            return value.toFixed(decimals) + '°C';
                        }
                    }
                }
            }
        }
    });
}

// Fetch Chart Data
async function fetchChartData() {
    const hours = parseInt(els.timeRange.value) / 60; // minutes to hours
    
    // For large time ranges, we limit points returned to prevent browser lag.
    // In a real prod environment, the API should downsample. We'll fetch a lot and let ChartJS handle it,
    // or just fetch up to limit=1000 which is the API default.
    let limit = 1000;
    if (hours > 24) limit = 3000; // Arbitrary higher limit for longer ranges
    
    // Calculate 'since'
    const sinceDate = new Date();
    sinceDate.setHours(sinceDate.getHours() - hours);
    const sinceIso = sinceDate.toISOString();
    
    const res = await fetch(`/temperature?limit=${limit}&since=${sinceIso}`);
    if (!res.ok) throw new Error('Chart data fetch failed');
    const data = await res.json();
    
    // API returns DESC, Chart.js needs ASC
    data.reverse();
    
    const internalData = [];
    const externalData = [];
    
    data.forEach(row => {
        // Use collected_at for the x-axis
        const t = new Date(row.collected_at);
        internalData.push({ x: t, y: row.internal_c });
        externalData.push({ x: t, y: row.external_c });
    });
    
    tempChart.data.datasets[0].data = internalData;
    tempChart.data.datasets[1].data = externalData;
    tempChart.update();
}

// Fetch Logs
async function fetchLogs() {
    const res = await fetch('/logs?limit=50');
    if (!res.ok) throw new Error('Logs fetch failed');
    currentLogs = await res.json();
    renderLogs();
}

function renderLogs() {
    const searchTerm = els.logSearch.value.toLowerCase();
    
    // Filter
    const filteredLogs = currentLogs.filter(log => 
        log.message.toLowerCase().includes(searchTerm)
    );
    
    // Render
    els.logsBody.innerHTML = '';
    
    if (filteredLogs.length === 0) {
        els.logsBody.innerHTML = '<tr><td colspan="2" style="text-align: center; color: var(--text-secondary);">No logs found.</td></tr>';
        return;
    }
    
    filteredLogs.forEach(log => {
        const tr = document.createElement('tr');
        
        const tdTime = document.createElement('td');
        tdTime.className = 'log-time';
        tdTime.textContent = formatDateTime(log.collected_at);
        
        const tdMsg = document.createElement('td');
        tdMsg.className = 'log-msg';
        tdMsg.textContent = log.message;
        
        // Highlight errors
        if (log.message.toLowerCase().includes('error') || log.message.toLowerCase().includes('fail')) {
            tdMsg.classList.add('error-text');
        }
        
        tr.appendChild(tdTime);
        tr.appendChild(tdMsg);
        els.logsBody.appendChild(tr);
    });
}
