"""HERMES sensor mesh — remote nodes aggregated from board/gateway telemetry."""

from __future__ import annotations

import json
import threading
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
MESH_FILE = ROOT / "data" / "mesh_nodes.json"

_lock = threading.Lock()
_nodes: list[dict] = []


def _load() -> None:
    global _nodes
    if not MESH_FILE.exists():
        _nodes = []
        return
    try:
        _nodes = json.loads(MESH_FILE.read_text())
    except json.JSONDecodeError:
        _nodes = []


def _save() -> None:
    MESH_FILE.parent.mkdir(parents=True, exist_ok=True)
    MESH_FILE.write_text(json.dumps(_nodes, indent=2) + "\n")


def ingest_telemetry(telemetry: dict) -> None:
    """Merge mesh block from board heartbeat into control-plane state."""
    mesh = telemetry.get("mesh")
    if not mesh:
        return
    nodes = mesh.get("nodes") or []
    readings = {r.get("node_id"): r for r in (mesh.get("readings") or [])}
    now = time.time()
    with _lock:
        by_id = {n.get("node_id"): n for n in _nodes}
        for n in nodes:
            nid = n.get("node_id")
            if nid is None:
                continue
            entry = by_id.get(nid, {})
            entry.update(n)
            entry["last_seen"] = now
            if nid in readings:
                entry["readings"] = readings[nid].get("readings", [])
            by_id[nid] = entry
        _nodes[:] = list(by_id.values())
        _save()


def ingest_line(line: str) -> None:
    """Parse mesh JSON line from serial bridge."""
    try:
        msg = json.loads(line)
    except json.JSONDecodeError:
        return
    if "mesh" not in msg:
        return
    ingest_telemetry({"mesh": {
        "nodes": [msg] if msg.get("node_id") else [],
        "readings": [msg] if msg.get("sensors") else [],
    }})


def list_nodes() -> list[dict]:
    with _lock:
        return list(_nodes)


def status_summary() -> dict:
    nodes = list_nodes()
    online = sum(1 for n in nodes if n.get("online"))
    return {
        "enabled": bool(nodes),
        "node_count": len(nodes),
        "online_count": online,
        "nodes": nodes,
    }


_load()
