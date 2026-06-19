"""Persistent storage for control plane state."""

from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
DATA_DIR = ROOT / "data"
STATE_FILE = DATA_DIR / "cluster_state.json"
CHECKPOINT_DIR = DATA_DIR / "checkpoints"


def ensure_dirs() -> None:
    DATA_DIR.mkdir(exist_ok=True)
    CHECKPOINT_DIR.mkdir(exist_ok=True)


def save_state(state: dict) -> None:
    ensure_dirs()
    STATE_FILE.write_text(json.dumps(state, indent=2) + "\n")


def load_state() -> dict | None:
    if not STATE_FILE.exists():
        return None
    return json.loads(STATE_FILE.read_text())


def save_checkpoint(app_name: str, data: dict) -> Path:
    ensure_dirs()
    path = CHECKPOINT_DIR / f"{app_name}.json"
    path.write_text(json.dumps(data, indent=2) + "\n")
    return path


def load_checkpoint(app_name: str) -> dict | None:
    path = CHECKPOINT_DIR / f"{app_name}.json"
    if not path.exists():
        return None
    return json.loads(path.read_text())
