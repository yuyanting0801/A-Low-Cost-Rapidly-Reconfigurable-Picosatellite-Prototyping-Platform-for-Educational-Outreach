"""
Entry point: starts the WebSocket server (port 5001, "arduino" subprotocol)
and the HTTP server (port 5000).
"""

import asyncio
from websockets.server import serve
from aiohttp import web
from config import WS_HOST, WS_PORT, HTTP_PORT
from database import init_db
from handlers import ws_handler, start_http_server


async def main():
    init_db()

    ws_server = await serve(
        ws_handler,
        WS_HOST,
        WS_PORT,
        subprotocols=["arduino"],
        ping_interval=None,
        ping_timeout=None,
        close_timeout=5
    )
    print(f"WebSocket server listening on ws://{WS_HOST}:{WS_PORT} (subprotocol: arduino)")

    http_app = start_http_server()
    runner = web.AppRunner(http_app)
    await runner.setup()
    http_site = web.TCPSite(runner, WS_HOST, HTTP_PORT)
    await http_site.start()
    print(f"HTTP server listening on http://{WS_HOST}:{HTTP_PORT}")

    print("System ready, waiting for devices... (video/photo forwarding enabled)")
    await asyncio.Future()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Server stopped")
