"""Minimal WebSocket support for stdlib HTTP server."""

from __future__ import annotations

import base64
import hashlib
import json
import struct
import threading
import time

WS_MAGIC = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

_clients: list = []
_lock = threading.Lock()


def accept_key(key: str) -> str:
    digest = hashlib.sha1((key + WS_MAGIC).encode()).digest()
    return base64.b64encode(digest).decode()


def _frame_text(payload: str) -> bytes:
    data = payload.encode()
    length = len(data)
    if length < 126:
        header = struct.pack("!BB", 0x81, length)
    elif length < 65536:
        header = struct.pack("!BBH", 0x81, 126, length)
    else:
        header = struct.pack("!BBQ", 0x81, 127, length)
    return header + data


def register(client) -> None:
    with _lock:
        _clients.append(client)


def unregister(client) -> None:
    with _lock:
        if client in _clients:
            _clients.remove(client)


def broadcast(data: dict) -> None:
    payload = json.dumps(data)
    frame = _frame_text(payload)
    dead = []
    with _lock:
        for client in _clients:
            try:
                client.wfile.write(frame)
                client.wfile.flush()
            except Exception:
                dead.append(client)
        for client in dead:
            if client in _clients:
                _clients.remove(client)


def pump_status(get_status, interval: float = 2.0) -> None:
    while True:
        try:
            broadcast(get_status())
        except Exception:
            pass
        time.sleep(interval)
