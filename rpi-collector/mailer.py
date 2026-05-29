import smtplib
import datetime
import logging
import socket
import threading
import time
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.application import MIMEApplication
import config
import db

log = logging.getLogger(__name__)

# ── Internet connectivity check ─────────────────────────────────────────────

def _check_internet(host: str = "8.8.8.8", port: int = 53, timeout: float = 3.0) -> bool:
    """Return True if we can reach the internet (DNS via TCP to Google's resolver)."""
    try:
        socket.setdefaulttimeout(timeout)
        socket.socket(socket.AF_INET, socket.SOCK_STREAM).connect((host, port))
        return True
    except OSError:
        return False


# ── Pending-email queue (used when internet is unavailable at send time) ────

_pending_lock: threading.Lock = threading.Lock()
_pending_queue: list[dict] = []   # each entry: {subject, plain, html, attachment_bytes, attachment_name}


def _enqueue(subject: str, body_plain: str, body_html: str,
             attachment_bytes: bytes | None, attachment_name: str) -> None:
    """Add an email to the pending queue to be sent once internet is available."""
    with _pending_lock:
        _pending_queue.append({
            "subject": subject,
            "body_plain": body_plain,
            "body_html": body_html,
            "attachment_bytes": attachment_bytes,
            "attachment_name": attachment_name,
        })
    log.info("Email queued (no internet): %s  [queue length=%d]", subject, len(_pending_queue))


def _retry_pending() -> None:
    """Background loop: drain the pending queue as soon as internet is available."""
    while True:
        time.sleep(15)  # check every 15 s
        with _pending_lock:
            if not _pending_queue:
                continue
            if not _check_internet():
                log.debug("Internet still unavailable — %d email(s) remain queued.", len(_pending_queue))
                continue
            # Internet is back — send all queued emails
            log.info("Internet restored. Sending %d queued email(s).", len(_pending_queue))
            to_send = list(_pending_queue)
            _pending_queue.clear()

        for item in to_send:
            _send_now(
                item["subject"],
                item["body_plain"],
                item["body_html"],
                item["attachment_bytes"],
                item["attachment_name"],
            )


# Start the retry thread at module load (daemon so it dies with the process)
_retry_thread = threading.Thread(target=_retry_pending, name="mailer_retry", daemon=True)
_retry_thread.start()


# ── shared helpers ─────────────────────────────────────────────────────────

def _now_str() -> str:
    return datetime.datetime.now().astimezone().strftime("%d %b %Y, %H:%M:%S %Z")


def _send_now(subject: str, body_plain: str, body_html: str, attachment_bytes: bytes | None = None,
              attachment_name: str = "logs.txt") -> None:
    """Low-level helper that builds and sends a MIME email (no internet check)."""
    if not getattr(config, "EMAIL_SENDER_ACCOUNT", None) or \
       not getattr(config, "EMAIL_SENDER_PASSWORD", None):
        log.warning("Email not sent: Missing SMTP credentials in config.py")
        return

    msg = MIMEMultipart("mixed")
    msg["Subject"] = subject
    msg["From"] = f"Aquarium Control <{config.EMAIL_SENDER_ACCOUNT}>"
    msg["To"] = f"User <{getattr(config, 'EMAIL_RECIPIENT', '')}>"

    # Attach plain + HTML as an "alternative" part
    alt = MIMEMultipart("alternative")
    alt.attach(MIMEText(body_plain, "plain", "utf-8"))
    alt.attach(MIMEText(body_html,  "html",  "utf-8"))
    msg.attach(alt)

    if attachment_bytes is not None:
        part = MIMEApplication(attachment_bytes, Name=attachment_name)
        part["Content-Disposition"] = f'attachment; filename="{attachment_name}"'
        msg.attach(part)

    try:
        log.info("Sending email: %s", subject)
        with smtplib.SMTP_SSL(
            getattr(config, "SMTP_HOST", "smtp.gmail.com"),
            getattr(config, "SMTP_PORT", 465),
        ) as server:
            server.login(config.EMAIL_SENDER_ACCOUNT, config.EMAIL_SENDER_PASSWORD)
            server.send_message(msg)
        log.info("Email sent successfully.")
    except Exception as exc:
        log.error("Failed to send email: %s", exc)


def _send(subject: str, body_plain: str, body_html: str, attachment_bytes: bytes | None = None,
          attachment_name: str = "logs.txt") -> None:
    """Send an email, queuing it if internet is currently unavailable."""
    if _check_internet():
        _send_now(subject, body_plain, body_html, attachment_bytes, attachment_name)
    else:
        _enqueue(subject, body_plain, body_html, attachment_bytes, attachment_name)


# ── CSS / base template ────────────────────────────────────────────────────

_BASE_STYLE = """
  /* ── Reset ── */
  body, table, td, p, a { -webkit-text-size-adjust:100%; -ms-text-size-adjust:100%; }
  body { margin:0; padding:0; background:#0a1628;
         font-family:'Segoe UI',Helvetica,Arial,sans-serif; color:#cbd5e1; }
  /* ── Outer centering shell ── */
  .email-shell { width:100%; background:#0a1628; }
  /* ── Card ── */
  .wrapper { width:100%; max-width:620px; margin:0 auto;
             background:#0f2040; border-radius:16px;
             overflow:hidden; box-shadow:0 8px 32px rgba(0,0,0,0.5); }
  /* ── Header ── */
  .header { padding:28px 32px 24px;
            background:linear-gradient(135deg,#0e3460 0%,#1a5276 100%);
            border-bottom:1px solid #1e3a5f; }
  .header-fish { font-size:28px; }
  .header h1 { margin:8px 0 4px; font-size:20px; font-weight:700;
               color:#e0f2fe; letter-spacing:0.3px; }
  .header p  { margin:0; font-size:13px; color:#94a3b8; }
  /* ── Badges ── */
  .badge { display:inline-block; padding:4px 12px; border-radius:99px;
           font-size:12px; font-weight:600; margin-top:10px; }
  .badge-ok     { background:#0d3321; color:#4ade80; border:1px solid #166534; }
  .badge-warn   { background:#3b1f00; color:#fbbf24; border:1px solid #92400e; }
  .badge-error  { background:#3b0a0a; color:#f87171; border:1px solid #991b1b; }
  .badge-offline{ background:#1e1b4b; color:#a5b4fc; border:1px solid #3730a3; }
  /* ── Sections ── */
  .section { padding:24px 32px; border-bottom:1px solid #1e3a5f; }
  .section:last-child { border-bottom:none; }
  .section-title { font-size:11px; font-weight:700; letter-spacing:1.2px;
                   text-transform:uppercase; color:#64748b; margin-bottom:14px; }
  /* ── Data grid (table-based for email safety) ── */
  .grid { display:table; width:100%; border-collapse:collapse; }
  .row  { display:table-row; }
  .cell-label { display:table-cell; padding:7px 12px 7px 0; font-size:13px;
                color:#64748b; width:45%; vertical-align:middle; }
  .cell-value { display:table-cell; padding:7px 0; font-size:13px;
                color:#e2e8f0; font-weight:600; vertical-align:middle; }
  /* ── Pills ── */
  .pill { display:inline-block; padding:2px 10px; border-radius:99px;
          font-size:12px; font-weight:600; }
  .pill-on      { background:#0d3321; color:#4ade80; }
  .pill-off     { background:#1e293b; color:#64748b; }
  .pill-active  { background:#1c1917; color:#fb923c; }
  .pill-inactive{ background:#1e293b; color:#64748b; }
  .pill-error   { background:#3b0a0a; color:#f87171; }
  .pill-ok      { background:#0d3321; color:#4ade80; }
  /* ── Temperature display ── */
  .temp-big   { font-size:32px; font-weight:800; color:#38bdf8; }
  .temp-unit  { font-size:16px; color:#7dd3fc; }
  .temp-row   { display:inline-block; margin-right:24px; text-align:center; }
  .temp-label { font-size:11px; color:#64748b; margin-top:2px; }
  /* ── Attachment notice ── */
  .attach-notice { background:#081428; border:1px solid #1e3a5f; border-radius:8px;
                   padding:14px 18px; display:flex; align-items:center; gap:12px; }
  .attach-icon { font-size:22px; flex-shrink:0; }
  .attach-text { font-size:13px; color:#94a3b8; line-height:1.6; }
  .attach-text strong { color:#e2e8f0; }
  /* ── Footer ── */
  .footer { padding:18px 32px; background:#081428;
            text-align:center; font-size:11px; color:#334155; }
  /* ── Responsive: full-width card on small screens ── */
  @media only screen and (max-width:660px) {
    .email-shell { padding:0 !important; }
    .wrapper     { border-radius:0 !important; }
    .header,
    .section,
    .footer      { padding-left:20px !important; padding-right:20px !important; }
    .temp-big    { font-size:26px !important; }
  }
"""


def _html_wrap(header_html: str, content_html: str, footer_note: str = "") -> str:
    """Wrap content in the base email shell."""
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Aquarium Control</title>
  <style>{_BASE_STYLE}</style>
</head>
<body>
  <!-- Outer shell centres the card on desktop; collapses to full-width on mobile -->
  <table class="email-shell" cellpadding="0" cellspacing="0" role="presentation"
         style="width:100%;background:#0a1628;padding:32px 16px;">
    <tr><td align="center">
      <div class="wrapper">
        {header_html}
        {content_html}
        <div class="footer">
          🐠 Smart Aquarium Control System &nbsp;·&nbsp; Automated notification
          {"&nbsp;·&nbsp; " + footer_note if footer_note else ""}
        </div>
      </div>
    </td></tr>
  </table>
</body>
</html>"""


# ── send_email_report ──────────────────────────────────────────────────────

def send_email_report(trigger: str, status_data: dict) -> None:
    """Constructs and sends the styled HTML status-report email."""

    # ── Fetch logs ──
    log_entries: list[str] = []
    try:
        with db.get_conn() as conn:
            cursor = conn.execute(
                "SELECT esp32_time, message FROM log_entries ORDER BY id DESC LIMIT 40"
            )
            for row in reversed(cursor.fetchall()):
                esp32_time, message = row
                log_entries.append(f"[{esp32_time}] {message}")
    except Exception as exc:
        log.error("Failed to fetch logs for email: %s", exc)
        log_entries.append("Failed to load logs.")

    logs_text = "\n".join(log_entries)

    # ── Extract fields ──
    now_str      = _now_str()
    internal_c   = status_data.get("internal_c") or 0.0
    external_c   = status_data.get("external_c") or 0.0
    relay1       = "ON"     if status_data.get("relay1")         else "OFF"
    relay2       = "ON"     if status_data.get("relay2")         else "OFF"
    relay3       = "ON"     if status_data.get("relay3")         else "OFF"
    override1    = "Active" if status_data.get("override1")      else "Inactive"
    override2    = "Active" if status_data.get("override2")      else "Inactive"
    has_error    = bool(status_data.get("has_error"))
    temp_error   = bool(status_data.get("temp_error"))
    ext_err      = bool(status_data.get("ext_temp_error"))
    uptime_days  = status_data.get("uptime_days",    0)
    uptime_secs  = status_data.get("uptime_seconds", 0)
    uptime_hours = uptime_secs // 3600
    uptime_mins  = (uptime_secs % 3600) // 60

    error_label  = "Error Present" if has_error else "No Errors"
    badge_cls    = "badge-error"   if has_error else "badge-ok"

    def relay_pill(val: str) -> str:
        cls = "pill-on" if val == "ON" else "pill-off"
        return f'<span class="pill {cls}">{val}</span>'

    def override_pill(val: str) -> str:
        cls = "pill-active" if val == "Active" else "pill-inactive"
        return f'<span class="pill {cls}">{val}</span>'

    error_pill = (f'<span class="pill pill-error">{error_label}</span>'
                  if has_error else
                  f'<span class="pill pill-ok">{error_label}</span>')

    temp_flags = []
    if temp_error: temp_flags.append('<span class="pill pill-error">Temp Sensor Error</span>')
    if ext_err:    temp_flags.append('<span class="pill pill-error">Ext Temp Sensor Error</span>')
    temp_flags_html = " ".join(temp_flags) if temp_flags else '<span class="pill pill-ok">Sensors OK</span>'

    # ── HTML body ──
    header_html = f"""
    <div class="header">
      <div class="header-fish">🐠</div>
      <h1>Aquarium Control System</h1>
      <p>Status Report &nbsp;·&nbsp; {now_str}</p>
      <span class="badge {badge_cls}">{trigger}</span>
    </div>"""

    content_html = f"""
    <div class="section">
      <div class="section-title">Temperature</div>
      <div style="text-align:center">
        <div class="temp-row">
          <div class="temp-big">{internal_c:.1f}<span class="temp-unit"> °C</span></div>
          <div class="temp-label">Internal</div>
        </div>
        <div class="temp-row">
          <div class="temp-big">{external_c:.1f}<span class="temp-unit"> °C</span></div>
          <div class="temp-label">External</div>
        </div>
      </div>
      <div style="margin-top:12px;text-align:center">{temp_flags_html}</div>
    </div>

    <div class="section">
      <div class="section-title">Relays &amp; Overrides</div>
      <div class="grid">
        <div class="row">
          <div class="cell-label">Relay 1 (WaveMaker)</div>
          <div class="cell-value">{relay_pill(relay1)}</div>
        </div>
        <div class="row">
          <div class="cell-label">Relay 2 (Light)</div>
          <div class="cell-value">{relay_pill(relay2)}</div>
        </div>
        <div class="row">
          <div class="cell-label">Relay 3 (Air Pump)</div>
          <div class="cell-value">{relay_pill(relay3)}</div>
        </div>
        <div class="row">
          <div class="cell-label">Override 1</div>
          <div class="cell-value">{override_pill(override1)}</div>
        </div>
        <div class="row">
          <div class="cell-label">Override 2</div>
          <div class="cell-value">{override_pill(override2)}</div>
        </div>
      </div>
    </div>

    <div class="section">
      <div class="section-title">System Health</div>
      <div class="grid">
        <div class="row">
          <div class="cell-label">Error Status</div>
          <div class="cell-value">{error_pill}</div>
        </div>
        <div class="row">
          <div class="cell-label">ESP32 Uptime</div>
          <div class="cell-value">{uptime_days}d {uptime_hours}h {uptime_mins}m</div>
        </div>
      </div>
    </div>

    <div class="section">
      <div class="section-title">System Logs</div>
      <div class="attach-notice">
        <div class="attach-icon">📎</div>
        <div class="attach-text">
          <strong>logs.txt</strong> is attached to this email.<br>
          It contains the latest 40 log entries from the ESP32 in chronological order.
        </div>
      </div>
    </div>"""

    html = _html_wrap(header_html, content_html)

    # ── Plain-text fallback ──
    plain = (
        f"Aquarium Control System Report\n"
        f"Event: {trigger}\n"
        f"Timestamp: {now_str}\n\n"
        f"System Status:\n"
        f"  Internal Temperature : {internal_c:.1f} °C\n"
        f"  External Temperature : {external_c:.1f} °C\n"
        f"  Relay 1 (WaveMaker)  : {relay1}\n"
        f"  Relay 2 (Light)      : {relay2}\n"
        f"  Relay 3 (Air Pump)   : {relay3}\n"
        f"  Override 1           : {override1}\n"
        f"  Override 2           : {override2}\n"
        f"  Error Status         : {error_label}\n"
        f"  Uptime               : {uptime_days}d {uptime_hours}h {uptime_mins}m\n\n"
        f"Full logs are attached as logs.txt"
    )

    subject = f"{getattr(config, 'EMAIL_SUBJECT', 'Aquarium Control')} – {trigger}"
    _send(subject, plain, html, logs_text.encode("utf-8"), "logs.txt")


# ── send_offline_email ─────────────────────────────────────────────────────

def send_offline_email() -> None:
    """Sends a styled alert email when the ESP32 goes offline."""
    now_str = _now_str()

    header_html = f"""
    <div class="header">
      <div class="header-fish">⚠️</div>
      <h1>Aquarium Control System</h1>
      <p>Connectivity Alert &nbsp;·&nbsp; {now_str}</p>
      <span class="badge badge-offline">ESP32 Offline</span>
    </div>"""

    content_html = f"""
    <div class="section">
      <div class="section-title">Alert Details</div>
      <div class="grid">
        <div class="row">
          <div class="cell-label">Event</div>
          <div class="cell-value" style="color:#a5b4fc;font-weight:700;">ESP32 Offline</div>
        </div>
        <div class="row">
          <div class="cell-label">Detected at</div>
          <div class="cell-value">{now_str}</div>
        </div>
        <div class="row">
          <div class="cell-label">Missed pings</div>
          <div class="cell-value">3 consecutive (≈ 30 s)</div>
        </div>
      </div>
    </div>
    <div class="section" style="text-align:center;padding:32px;">
      <div style="font-size:48px;margin-bottom:12px;">🔌</div>
      <div style="font-size:15px;color:#94a3b8;line-height:1.7;">
        The Aquarium Control System (ESP32) has stopped responding.<br>
        Please check the device and its network connection.
      </div>
    </div>"""

    html  = _html_wrap(header_html, content_html)
    plain = (
        f"Aquarium Control System — ESP32 Offline\n"
        f"Timestamp: {now_str}\n\n"
        f"The ESP32 has been offline for 3 consecutive health pings (~30 s).\n"
        f"Please check the device and its network connection."
    )

    subject = f"{getattr(config, 'EMAIL_SUBJECT', 'Aquarium Control')} – ESP32 Offline"
    _send(subject, plain, html)


# ── send_online_email ──────────────────────────────────────────────────────

def send_online_email() -> None:
    """Sends a styled confirmation email when the ESP32 comes back online."""
    now_str = _now_str()

    header_html = f"""
    <div class="header">
      <div class="header-fish">✅</div>
      <h1>Aquarium Control System</h1>
      <p>Connectivity Update &nbsp;·&nbsp; {now_str}</p>
      <span class="badge badge-ok">ESP32 Back Online</span>
    </div>"""

    content_html = f"""
    <div class="section">
      <div class="section-title">Recovery Details</div>
      <div class="grid">
        <div class="row">
          <div class="cell-label">Event</div>
          <div class="cell-value" style="color:#4ade80;font-weight:700;">ESP32 Back Online</div>
        </div>
        <div class="row">
          <div class="cell-label">Restored at</div>
          <div class="cell-value">{now_str}</div>
        </div>
      </div>
    </div>
    <div class="section" style="text-align:center;padding:32px;">
      <div style="font-size:48px;margin-bottom:12px;">🐠</div>
      <div style="font-size:15px;color:#94a3b8;line-height:1.7;">
        The Aquarium Control System (ESP32) is back online<br>and responding to health pings normally.
      </div>
    </div>"""

    html  = _html_wrap(header_html, content_html)
    plain = (
        f"Aquarium Control System — ESP32 Back Online\n"
        f"Timestamp: {now_str}\n\n"
        f"The ESP32 is back online and responding to health pings."
    )

    subject = f"{getattr(config, 'EMAIL_SUBJECT', 'Aquarium Control')} – ESP32 Back Online"
    _send(subject, plain, html)
