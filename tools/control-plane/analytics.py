"""Time-series analytics for cluster metrics."""

from __future__ import annotations

import json
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
ANALYTICS_FILE = ROOT / "data" / "analytics.json"
MAX_POINTS = 500


def _load() -> list:
    if not ANALYTICS_FILE.exists():
        return []
    return json.loads(ANALYTICS_FILE.read_text())


def record(metrics: dict) -> None:
    ANALYTICS_FILE.parent.mkdir(parents=True, exist_ok=True)
    points = _load()
    points.append({
        "ts": time.time(),
        "load_percent": metrics.get("load_percent", 0),
        "power_w": metrics.get("power_w", 0),
        "temp_c": metrics.get("temp_c", 0),
        "app_count": metrics.get("app_count", 0),
    })
    if len(points) > MAX_POINTS:
        points = points[-MAX_POINTS:]
    ANALYTICS_FILE.write_text(json.dumps(points, indent=2) + "\n")


def query(limit: int = 100) -> dict:
    points = _load()
    return {"points": points[-limit:], "count": len(points)}
