"""
main.py — Application entry point
Initialises the DB, starts collector threads, then runs the Flask API
with Gunicorn (in Docker) or Flask dev server (local testing).
"""
import logging
import os
import sys

# Ensure /data directory exists (in case the volume isn't mounted yet)
os.makedirs("/data", exist_ok=True)

logging.basicConfig(
    stream=sys.stdout,
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
    datefmt="%Y-%m-%dT%H:%M:%S",
)

import db
import collector
import config

db.init_db()
collector.start()

# Import the Flask app for Gunicorn (CMD in Dockerfile calls gunicorn main:app)
from api import app  # noqa: E402

if __name__ == "__main__":
    # Local dev: python main.py
    app.run(host=config.API_HOST, port=config.API_PORT, debug=True, use_reloader=False)
