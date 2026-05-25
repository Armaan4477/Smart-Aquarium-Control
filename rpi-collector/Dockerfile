# ── Build stage ────────────────────────────────────────────────────────────
# Matches Pi's Python 3.13; creates a venv and installs deps into it.
FROM python:3.13-slim AS builder

WORKDIR /app

# Create venv inside the project directory
RUN python -m venv .venv

COPY requirements.txt .

# Install into the venv (no system-wide changes)
RUN .venv/bin/pip install --no-cache-dir --upgrade pip \
 && .venv/bin/pip install --no-cache-dir -r requirements.txt

# ── Runtime stage ──────────────────────────────────────────────────────────
FROM python:3.13-slim

WORKDIR /app

# Copy only the pre-built venv from the builder stage
COPY --from=builder /app/.venv /app/.venv

# Copy application source
COPY config.py collector.py db.py api.py main.py ./

# Put the venv's bin/ first so python / gunicorn resolve from it
ENV PATH="/app/.venv/bin:$PATH" \
    VIRTUAL_ENV="/app/.venv" \
    PYTHONUNBUFFERED=1

# Data volume – SQLite DB is stored here, persists across restarts
VOLUME ["/data"]

EXPOSE 5050

# Gunicorn: 1 worker (SQLite WAL handles concurrent reads fine),
# 4 threads, bind to 0.0.0.0:5050
CMD ["gunicorn", \
     "--workers=1", \
     "--threads=4", \
     "--bind=0.0.0.0:5050", \
     "--timeout=30", \
     "--access-logfile=-", \
     "--error-logfile=-", \
     "main:app"]

