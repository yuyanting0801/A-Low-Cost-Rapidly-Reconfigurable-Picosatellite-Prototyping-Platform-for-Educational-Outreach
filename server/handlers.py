"""
WebSocket and HTTP request handlers.
- ws_handler: serves all three WebSocket roles (sender / receiver / frontend)
- HTTP handlers: latest sensor reading, capture trigger, static page
- start_http_server: aiohttp routes and CORS
"""

import json
import time
import asyncio
import sqlite3
import aiohttp
from aiohttp import web
from websockets.exceptions import ConnectionClosed
from config import CAPTURE_URL, INDEX_HTML_PATH, DB_PATH
from database import sensor_history, store_sensor_data
from connection_manager import manager


async def ws_handler(websocket, path):
    """Handle one WebSocket connection; the client declares its role (sender / receiver /
    frontend) in the first JSON message, within 5 s."""
    try:
        msg = await asyncio.wait_for(websocket.recv(), timeout=5)
        data = json.loads(msg)
        client_type = data.get("type")
    except asyncio.TimeoutError:
        print("Client did not send an identity message; closing connection")
        await websocket.close()
        return
    except Exception as e:
        print(f"Identity handshake failed: {e}")
        await websocket.close()
        return

    if client_type == "sender":
        print("Sender connected")
        manager.set_sender(websocket)
        try:
            async for message in websocket:
                if isinstance(message, str):
                    try:
                        sensor = json.loads(message)
                        if sensor.get("type") == "sensor":
                            now_sec = int(time.time())
                            sensor["timestamp"] = now_sec
                            store_sensor_data(sensor, ts=now_sec * 1000)
                            await manager.broadcast_to_frontends(json.dumps(sensor))
                    except Exception as e:
                        print(f"Sensor data parse error: {e}")
                elif isinstance(message, bytes):
                    # Video frame (first byte 0x01) - forward to every frontend
                    await manager.broadcast_to_frontends(message, is_binary=True)
        except ConnectionClosed:
            pass
        finally:
            if manager.sender == websocket:
                manager.sender = None
            print("Sender disconnected")

    elif client_type == "receiver":
        print("Receiver connected")
        manager.set_receiver(websocket)
        try:
            async for message in websocket:
                if isinstance(message, bytes):
                    # Photo frame (first byte 0x02) - forward to every frontend
                    await manager.broadcast_to_frontends(message, is_binary=True)
        except ConnectionClosed:
            pass
        finally:
            if manager.receiver == websocket:
                manager.receiver = None
            print("Receiver disconnected")

    elif client_type == "frontend":
        print("Frontend connected")
        manager.add_frontend(websocket)
        if sensor_history:
            history_msg = json.dumps({"type": "history", "data": list(sensor_history)})
            try:
                await websocket.send(history_msg)
            except Exception:
                pass
        try:
            async for message in websocket:
                if isinstance(message, str):
                    try:
                        cmd = json.loads(message)
                        if cmd.get("type") == "capture":
                            await manager.send_to_sender(json.dumps({"type": "capture"}))
                    except Exception as e:
                        print(f"Command parse error: {e}")
        except ConnectionClosed:
            pass
        finally:
            manager.remove_frontend(websocket)
            print("Frontend disconnected")


async def handle_sensor_latest(request):
    """HTTP GET /api/sensor/latest - most recent stored reading."""
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute("""SELECT timestamp, temperature, humidity, uvi, lux, ax, ay, az
                 FROM sensor_history ORDER BY timestamp DESC LIMIT 1""")
    row = c.fetchone()
    conn.close()
    if row:
        return web.json_response({
            "timestamp": row[0], "temperature": row[1], "humidity": row[2],
            "uvi": row[3], "lux": row[4], "ax": row[5], "ay": row[6], "az": row[7]
        })
    return web.json_response({"error": "No data"}, status=500)


async def handle_capture(request):
    """HTTP POST /api/capture - proxy a capture request to the sender device."""
    async with aiohttp.ClientSession() as session:
        try:
            async with session.post(CAPTURE_URL, timeout=aiohttp.ClientTimeout(total=5)) as resp:
                if resp.status == 200:
                    return web.json_response({"status": "ok"})
                return web.json_response({"status": "error"}, status=500)
        except Exception as e:
            return web.json_response({"status": "error", "message": str(e)}, status=500)


async def handle_index(request):
    """HTTP GET / - serve the frontend page."""
    try:
        with open(INDEX_HTML_PATH, "rb") as f:
            return web.Response(body=f.read(), content_type="text/html")
    except FileNotFoundError:
        return web.Response(body=b"<h1>index.html not found</h1>", content_type="text/html", status=404)


def start_http_server():
    """Build the aiohttp app: routes (/, /api/sensor/latest, /api/capture) with permissive CORS."""
    app = web.Application()
    cors = aiohttp_cors.setup(app, defaults={
        "*": aiohttp_cors.ResourceOptions(allow_credentials=True, expose_headers="*", allow_headers="*")
    })
    app.router.add_get("/api/sensor/latest", handle_sensor_latest)
    app.router.add_post("/api/capture", handle_capture)
    app.router.add_get("/", handle_index)
    for route in list(app.router.routes()):
        cors.add(route)
    return app
