"""
Configuration module: all server constants in one place.
"""

import os

# Bind address and ports
WS_HOST = "0.0.0.0"
WS_PORT = 5001                     # WebSocket, subprotocol "arduino"
HTTP_PORT = 5000                   # static pages and HTTP API

# Device IPs: the static IPs you assigned to the two boards
SENDER_IP = "SENDER_IP"
RECEIVER_IP = "RECEIVER_IP"
CAPTURE_URL = f"http://{SENDER_IP}/capture"

# File paths
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
PARENT_DIR = os.path.dirname(BASE_DIR)                    # satellite/
LIB_DIR = os.path.join(PARENT_DIR, "lib")                 # satellite/lib/
WEB_DIR = os.path.join(PARENT_DIR, "web")                 # satellite/web/
INDEX_HTML_PATH = os.path.join(WEB_DIR, "index.html")

DB_PATH = "sensor_data.db"                                 # relative path, resolved against the working directory

# WebSocket heartbeat
PING_INTERVAL = 20      # seconds between pings
PING_TIMEOUT = 10       # drop the client if no pong arrives in time

# Bounded send queue per frontend
MAX_FRONTEND_QUEUE_SIZE = 20

# In-memory sensor history ring buffer
MAX_HISTORY = 60
