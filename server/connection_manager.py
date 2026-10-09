"""
Connection manager: tracks the sender, receiver and frontend WebSocket clients.
Each frontend has its own bounded queue so a slow client cannot grow server memory.
"""

import asyncio
from websockets.exceptions import ConnectionClosed
from typing import Set, Dict, Optional


class ConnectionManager:
    def __init__(self):
        self.sender: Optional[websockets.WebSocketServerProtocol] = None
        self.receiver: Optional[websockets.WebSocketServerProtocol] = None
        self.frontends: Set[websockets.WebSocketServerProtocol] = set()
        self.frontend_queues: Dict[websockets.WebSocketServerProtocol, asyncio.Queue] = {}
        self._send_tasks: Set[asyncio.Task] = set()

    def set_sender(self, ws):
        self.sender = ws

    def set_receiver(self, ws):
        self.receiver = ws

    def add_frontend(self, ws):
        self.frontends.add(ws)
        from config import MAX_FRONTEND_QUEUE_SIZE
        queue = asyncio.Queue(maxsize=MAX_FRONTEND_QUEUE_SIZE)
        self.frontend_queues[ws] = queue
        task = asyncio.create_task(self._frontend_sender(ws, queue))
        self._send_tasks.add(task)
        task.add_done_callback(self._send_tasks.discard)

    def remove_frontend(self, ws):
        self.frontends.discard(ws)
        if ws in self.frontend_queues:
            del self.frontend_queues[ws]

    async def _frontend_sender(self, ws, queue):
        """Dedicated task per frontend: takes queued frames and sends them to it."""
        try:
            while True:
                msg, is_binary = await queue.get()
                try:
                    await ws.send(msg)
                except ConnectionClosed:
                    break
                except Exception as e:
                    print(f"Failed to send to frontend: {e}")
                    break
        finally:
            if ws in self.frontends:
                self.remove_frontend(ws)

    async def broadcast_to_frontends(self, message, is_binary=False):
        """Fan out a frame to every frontend without blocking; a full queue drops it."""
        dead = set()
        for ws, queue in self.frontend_queues.items():
            try:
                queue.put_nowait((message, is_binary))
            except asyncio.QueueFull:
                pass
            except Exception:
                dead.add(ws)
        for ws in dead:
            self.remove_frontend(ws)

    async def send_to_sender(self, message):
        if self.sender:
            try:
                await self.sender.send(message)
            except Exception:
                self.sender = None


# Shared singleton used by the handlers
manager = ConnectionManager()
