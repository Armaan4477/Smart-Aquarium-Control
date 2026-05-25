import smtplib
import datetime
import logging
from email.message import EmailMessage
import config
import db

log = logging.getLogger(__name__)

def send_email_report(trigger: str, status_data: dict) -> None:
    """
    Constructs and sends the email report via SMTP.
    """
    if not getattr(config, 'EMAIL_SENDER_ACCOUNT', None) or not getattr(config, 'EMAIL_SENDER_PASSWORD', None):
        log.warning("Email not sent: Missing SMTP credentials in config.py")
        return

    # Fetch last 40 logs from the database
    log_entries = []
    try:
        with db.get_conn() as conn:
            cursor = conn.execute(
                "SELECT esp32_time, message FROM log_entries ORDER BY id DESC LIMIT 40"
            )
            rows = cursor.fetchall()
            # Reverse to show chronological order
            for row in reversed(rows):
                esp32_time, message = row
                log_entries.append(f"[{esp32_time}] {message}")
    except Exception as exc:
        log.error("Failed to fetch logs for email: %s", exc)
        log_entries.append("Failed to load logs.")

    logs_text = "\n".join(log_entries)

    # Format current time
    formatted_time = datetime.datetime.now().strftime("%H:%M:%S")

    # Extract status details
    internal_c = status_data.get("internal_c", 0.0)
    if internal_c is None:
        internal_c = 0.0
    external_c = status_data.get("external_c", 0.0)
    if external_c is None:
        external_c = 0.0
        
    relay1 = "ON" if status_data.get("relay1") else "OFF"
    relay2 = "ON" if status_data.get("relay2") else "OFF"
    relay3 = "ON" if status_data.get("relay3") else "OFF"
    override1 = "Active" if status_data.get("override1") else "Inactive"
    override2 = "Active" if status_data.get("override2") else "Inactive"
    has_error = "Error Present" if status_data.get("has_error") else "No Errors"
    uptime_days = status_data.get("uptime_days", 0)
    uptime_seconds = status_data.get("uptime_seconds", 0)

    # Construct Body
    body = (
        f"Aquarium Control System Report\n"
        f"Event: {trigger}\n"
        f"Timestamp: {formatted_time}\n\n"
        f"System Status:\n"
        f"Internal Temperature: {internal_c:.1f} °C\n"
        f"External Temperature: {external_c:.1f} °C\n"
        f"Relay 1 (WaveMaker): {relay1}\n"
        f"Relay 2 (Light): {relay2}\n"
        f"Relay 3 (Air Pump): {relay3}\n"
        f"Override 1: {override1}\n"
        f"Override 2: {override2}\n"
        f"Error Status: {has_error}\n"
        f"Uptime: {uptime_days} days, {uptime_seconds} seconds\n\n"
        f"Full logs are attached as logs.txt"
    )

    # Create EmailMessage
    msg = EmailMessage()
    msg['Subject'] = f"{getattr(config, 'EMAIL_SUBJECT', 'Aquarium Control Logs')} - {trigger}"
    msg['From'] = f"Aquarium Control <{config.EMAIL_SENDER_ACCOUNT}>"
    msg['To'] = f"User <{getattr(config, 'EMAIL_RECIPIENT', '')}>"
    msg.set_content(body)

    # Attach logs as .txt file
    msg.add_attachment(
        logs_text.encode('utf-8'),
        maintype='text',
        subtype='plain',
        filename='logs.txt'
    )

    # Send email
    try:
        log.info(f"Sending email report: {trigger}")
        with smtplib.SMTP_SSL(getattr(config, 'SMTP_HOST', 'smtp.gmail.com'), getattr(config, 'SMTP_PORT', 465)) as server:
            server.login(config.EMAIL_SENDER_ACCOUNT, config.EMAIL_SENDER_PASSWORD)
            server.send_message(msg)
        log.info("Email sent successfully.")
    except Exception as exc:
        log.error("Failed to send email: %s", exc)

def send_offline_email() -> None:
    """
    Sends an email when the ESP32 is offline for 3 consecutive pings.
    """
    if not getattr(config, 'EMAIL_SENDER_ACCOUNT', None) or not getattr(config, 'EMAIL_SENDER_PASSWORD', None):
        log.warning("Email not sent: Missing SMTP credentials in config.py")
        return

    formatted_time = datetime.datetime.now().strftime("%H:%M:%S")

    body = (
        f"Aquarium Control System Report\n"
        f"Event: ESP32 Offline\n"
        f"Timestamp: {formatted_time}\n\n"
        f"The Aquarium Control System (ESP32) has been offline for 3 consecutive health pings (30 seconds).\n"
        f"Please check the device and its network connection."
    )

    msg = EmailMessage()
    msg['Subject'] = f"{getattr(config, 'EMAIL_SUBJECT', 'Aquarium Control Logs')} - ESP32 Offline"
    msg['From'] = f"Aquarium Control <{config.EMAIL_SENDER_ACCOUNT}>"
    msg['To'] = f"User <{getattr(config, 'EMAIL_RECIPIENT', '')}>"
    msg.set_content(body)

    try:
        log.info("Sending offline email report")
        with smtplib.SMTP_SSL(getattr(config, 'SMTP_HOST', 'smtp.gmail.com'), getattr(config, 'SMTP_PORT', 465)) as server:
            server.login(config.EMAIL_SENDER_ACCOUNT, config.EMAIL_SENDER_PASSWORD)
            server.send_message(msg)
        log.info("Offline email sent successfully.")
    except Exception as exc:
        log.error("Failed to send offline email: %s", exc)
