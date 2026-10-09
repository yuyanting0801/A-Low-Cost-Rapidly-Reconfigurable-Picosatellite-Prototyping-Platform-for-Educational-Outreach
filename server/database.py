"""
Database module: SQLite setup and sensor storage (history table + in-memory cache).
"""

import sqlite3
import time
from collections import deque
from config import DB_PATH, MAX_HISTORY

# In-memory ring buffer of the most recent readings
sensor_history = deque(maxlen=MAX_HISTORY)


def init_db():
    """Create the SQLite sensor history table if it does not exist yet."""
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute("""CREATE TABLE IF NOT EXISTS sensor_history
                 (id INTEGER PRIMARY KEY AUTOINCREMENT,
                  timestamp INTEGER,
                  temperature REAL,
                  humidity REAL,
                  uvi REAL,
                  lux REAL,
                  ax REAL,
                  ay REAL,
                  az REAL)""")
    conn.commit()
    conn.close()
    print("Database initialized")


def store_sensor_data(data, ts=None):
    """Append one reading to the SQLite history table and the in-memory ring buffer."""
    if ts is None:
        ts = int(time.time() * 1000)
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute("""INSERT INTO sensor_history
                 (timestamp, temperature, humidity, uvi, lux, ax, ay, az)
                 VALUES (?, ?, ?, ?, ?, ?, ?, ?)""",
              (ts, data["temperature"], data["humidity"],
               data["uvi"], data["lux"],
               data["ax"], data["ay"], data["az"]))
    conn.commit()
    conn.close()
    sensor_history.append({
        "timestamp": ts,
        "temperature": data["temperature"],
        "humidity": data["humidity"],
        "uvi": data["uvi"],
        "lux": data["lux"],
        "ax": data["ax"],
        "ay": data["ay"],
        "az": data["az"],
    })
