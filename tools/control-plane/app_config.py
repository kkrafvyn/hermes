"""Persist per-app UI configuration under data/apps/."""

from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
APPS_DIR = ROOT / "data" / "apps"


def _app_dir(name: str) -> Path:
    d = APPS_DIR / name
    d.mkdir(parents=True, exist_ok=True)
    return d


def save_config(app_name: str, blueprint_id: str, config: dict, runtime: dict | None = None) -> Path:
    d = _app_dir(app_name)
    payload = {
        "app_name": app_name,
        "blueprint": blueprint_id,
        "config": config,
    }
    if runtime is not None:
        payload["runtime"] = runtime
    path = d / "config.json"
    existing = load_config(app_name)
    if existing and runtime is None and existing.get("runtime"):
        payload["runtime"] = existing["runtime"]
    path.write_text(json.dumps(payload, indent=2) + "\n")
    return path


def load_config(app_name: str) -> dict | None:
    path = APPS_DIR / app_name / "config.json"
    if not path.exists():
        return None
    return json.loads(path.read_text())


def save_runtime(app_name: str, runtime: dict) -> None:
    data = load_config(app_name)
    if not data:
        return
    data["runtime"] = runtime
    (APPS_DIR / app_name / "config.json").write_text(json.dumps(data, indent=2) + "\n")


def list_configs() -> dict[str, dict]:
    out: dict[str, dict] = {}
    if not APPS_DIR.exists():
        return out
    for path in APPS_DIR.glob("*/config.json"):
        out[path.parent.name] = json.loads(path.read_text())
    return out
