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
    if (!isoString) return '--';
    
    let d;
    // Check if it matches DD/MM/YYYY HH:MM:SS format from ESP32
    const match = isoString.match(/^(\d{2})\/(\d{2})\/(\d{4}) (\d{2}):(\d{2}):(\d{2})$/);
    if (match) {
        // match: 1=DD, 2=MM, 3=YYYY, 4=HH, 5=mm, 6=ss
        d = new Date(match[3], match[2] - 1, match[1], match[4], match[5], match[6]);
    } else {
        d = new Date(isoString);
    }
    
    if (isNaN(d.getTime())) return isoString;
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
                        unit: 'hour',
                        stepSize: 2,
                        tooltipFormat: 'MMM D, HH:mm',
                        displayFormats: {
                            minute: 'HH:mm',
                            hour:   'HH:mm',
                            day:    'MMM D'
                        }
                    },
                    grid: { color: 'rgba(255, 255, 255, 0.05)' },
                    border: { display: false },
                    ticks: {
                        maxRotation: 0,
                        autoSkip: true,
                        autoSkipPadding: 20
                    }
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

// ── Chart helpers ──────────────────────────────────────────────────────────

/**
 * Downsample an array to at most maxPoints entries by picking every Nth point.
 * Always keeps the first and last point so the visible range stays intact.
 */
function _downsample(arr, maxPoints) {
    if (arr.length <= maxPoints) return arr;
    const step = Math.ceil(arr.length / maxPoints);
    const result = [];
    for (let i = 0; i < arr.length; i += step) result.push(arr[i]);
    // Ensure the last real point is always included
    if (result[result.length - 1] !== arr[arr.length - 1]) {
        result.push(arr[arr.length - 1]);
    }
    return result;
}

/**
 * Return midnight boundaries (local time) between start and end as Date objects.
 */
function _midnightsBetween(start, end) {
    const midnights = [];
    // Start from the midnight after 'start'
    const d = new Date(start);
    d.setHours(24, 0, 0, 0); // next local midnight
    while (d <= end) {
        midnights.push(new Date(d));
        d.setDate(d.getDate() + 1);
    }
    return midnights;
}

// Persist day-boundary info so the afterDraw plugin can read it
let _dayBoundaries = []; // array of Date objects at local midnight
let _showDayLines = false;

// Register a Chart.js plugin that draws vertical dotted day-separator lines
// and date labels rendered INSIDE the chart area (near the top of each line)
// to avoid colliding with Chart.js's own tick labels on the x-axis.
Chart.register({
    id: 'dayBoundaryLines',
    afterDraw(chart) {
        if (!_showDayLines || _dayBoundaries.length === 0) return;
        const { ctx, scales: { x, y } } = chart;
        const top    = y.top;
        const bottom = y.bottom;

        const LABEL_FONT   = '11px Outfit, sans-serif';
        const LABEL_PAD_X  = 6;   // horizontal padding inside the pill
        const LABEL_PAD_Y  = 3;   // vertical padding inside the pill
        const LABEL_TOP    = top + 8; // distance from the top of the plot area

        ctx.save();

        for (const midnight of _dayBoundaries) {
            const xPx = x.getPixelForValue(midnight.getTime());
            if (xPx < x.left || xPx > x.right) continue;

            // ── Vertical dotted line spanning the full plot area ──
            ctx.setLineDash([4, 6]);
            ctx.strokeStyle = 'rgba(148, 163, 184, 0.28)';
            ctx.lineWidth   = 1;
            ctx.beginPath();
            ctx.moveTo(xPx, top);
            ctx.lineTo(xPx, bottom);
            ctx.stroke();

            // ── Date label pill drawn inside the chart near the top ──
            ctx.setLineDash([]); // solid for the pill
            ctx.font = LABEL_FONT;
            const label     = midnight.toLocaleDateString(undefined, { month: 'short', day: 'numeric' });
            const textW     = ctx.measureText(label).width;
            const pillW     = textW + LABEL_PAD_X * 2;
            const pillH     = 14 + LABEL_PAD_Y * 2;
            const pillX     = xPx - pillW / 2;
            const pillY     = LABEL_TOP;
            const radius    = 4;

            // Pill background
            ctx.beginPath();
            ctx.moveTo(pillX + radius, pillY);
            ctx.lineTo(pillX + pillW - radius, pillY);
            ctx.arcTo(pillX + pillW, pillY, pillX + pillW, pillY + radius, radius);
            ctx.lineTo(pillX + pillW, pillY + pillH - radius);
            ctx.arcTo(pillX + pillW, pillY + pillH, pillX + pillW - radius, pillY + pillH, radius);
            ctx.lineTo(pillX + radius, pillY + pillH);
            ctx.arcTo(pillX, pillY + pillH, pillX, pillY + pillH - radius, radius);
            ctx.lineTo(pillX, pillY + radius);
            ctx.arcTo(pillX, pillY, pillX + radius, pillY, radius);
            ctx.closePath();
            ctx.fillStyle   = 'rgba(14, 30, 58, 0.82)';
            ctx.fill();
            ctx.strokeStyle = 'rgba(148, 163, 184, 0.30)';
            ctx.lineWidth   = 1;
            ctx.stroke();

            // Pill text
            ctx.fillStyle   = 'rgba(186, 208, 232, 0.90)';
            ctx.textAlign   = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText(label, xPx, pillY + pillH / 2);
        }

        ctx.restore();
    }
});

// Fetch Chart Data
async function fetchChartData() {
    const minutes = parseInt(els.timeRange.value); // raw value is already minutes
    const hours   = minutes / 60;

    // Determine how many raw rows to fetch
    let limit = 2000;
    if (hours > 24)  limit = 5000;
    if (hours > 72)  limit = 10000;
    if (hours > 168) limit = 20000;
    if (minutes === 0) limit = 100000; // All time

    // ── Accurate 'since' calculation ──
    // Use Date.now() minus the exact millisecond offset to avoid the
    // setHours() roll-over bug that caused ~6h of missing data on 24h view.
    let sinceIso = "";
    let sinceDate = null;
    if (minutes > 0) {
        sinceDate = new Date(Date.now() - minutes * 60 * 1000);
        sinceIso  = sinceDate.toISOString();
    }

    const res = await fetch(`/temperature?limit=${limit}${sinceIso ? `&since=${sinceIso}` : ''}`);
    if (!res.ok) throw new Error('Chart data fetch failed');
    const data = await res.json();

    // API returns DESC order — Chart.js time axis needs ASC
    data.reverse();

    // ── Build raw point arrays ──
    const rawInternal = [];
    const rawExternal = [];
    data.forEach(row => {
        const t = new Date(row.collected_at);
        rawInternal.push({ x: t, y: row.internal_c });
        rawExternal.push({ x: t, y: row.external_c });
    });

    // ── Downsample for multi-day views ──
    // Keep max 300 visible points so the chart is readable without clutter.
    // For 1h / 12h views keep all points (they're few enough already).
    const MAX_POINTS = hours <= 12 ? rawInternal.length : 300;
    const internalData = _downsample(rawInternal, MAX_POINTS);
    const externalData = _downsample(rawExternal, MAX_POINTS);

    // ── Day-boundary lines (only when span > 1 day) ──
    _showDayLines = hours > 24;
    if (_showDayLines && rawInternal.length > 0) {
        const chartStart = sinceDate || rawInternal[0].x;
        const chartEnd   = rawInternal[rawInternal.length - 1].x;
        _dayBoundaries   = _midnightsBetween(chartStart, chartEnd);
    } else {
        _dayBoundaries = [];
    }

    // ── Configure x-axis tick density based on range ──
    // For multi-day views skip hourly ticks and show fewer, cleaner labels.
    let xUnit, xStepSize;
    if (hours <= 2) {
        xUnit = 'minute'; xStepSize = 15;
    } else if (hours <= 12) {
        xUnit = 'hour'; xStepSize = 1;
    } else if (hours <= 24) {
        xUnit = 'hour'; xStepSize = 2;
    } else if (hours <= 72) {
        xUnit = 'hour'; xStepSize = 6;
    } else if (hours <= 168) {
        xUnit = 'hour'; xStepSize = 12;
    } else {
        xUnit = 'day'; xStepSize = 1;
    }

    tempChart.options.scales.x.time.unit     = xUnit;
    tempChart.options.scales.x.time.stepSize = xStepSize;

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
        tdTime.textContent = formatDateTime(log.esp32_time || log.collected_at);
        
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
