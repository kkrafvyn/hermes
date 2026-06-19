"""HERMES app blueprints — UI-configurable app types without code edits."""

from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
BLUEPRINT_DIR = ROOT / "blueprints"

from blueprint_engine import display_label  # noqa: E402
from sensors import validate_sensor_list  # noqa: E402


def list_blueprints() -> list[dict]:
    items = []
    for path in sorted(BLUEPRINT_DIR.glob("*.json")):
        data = json.loads(path.read_text())
        items.append({
            "id": data["id"],
            "name": data["name"],
            "description": data.get("description", ""),
            "version": data.get("version", "1.0.0"),
            "memory": data.get("memory", "64KB"),
            "field_count": len(data.get("fields", [])),
        })
    return items


def get_blueprint(blueprint_id: str) -> dict | None:
    path = BLUEPRINT_DIR / f"{blueprint_id}.json"
    if not path.exists():
        return None
    return json.loads(path.read_text())


def default_config(blueprint: dict) -> dict:
    cfg: dict = {}
    for field in blueprint.get("fields", []):
        cfg[field["key"]] = field.get("default")
    return cfg


def validate_config(blueprint: dict, config: dict) -> tuple[dict | None, str | None]:
    """Merge user config with defaults and validate. Returns (config, error)."""
    merged = default_config(blueprint)
    merged.update(config or {})

    for field in blueprint.get("fields", []):
        key = field["key"]
        val = merged.get(key)
        ftype = field.get("type", "text")

        if field.get("required") and (val is None or val == ""):
            return None, f"{field['label']} is required"

        if ftype == "number":
            try:
                val = float(val) if "." in str(val) else int(val)
            except (TypeError, ValueError):
                return None, f"{field['label']} must be a number"
            if "min" in field and val < field["min"]:
                return None, f"{field['label']} must be >= {field['min']}"
            if "max" in field and val > field["max"]:
                return None, f"{field['label']} must be <= {field['max']}"
            merged[key] = val
        elif ftype == "toggle":
            merged[key] = bool(val) if not isinstance(val, str) else val.lower() in ("1", "true", "yes", "on")
        elif ftype == "node":
            try:
                merged[key] = int(val)
            except (TypeError, ValueError):
                return None, f"{field['label']} must be a valid node id"
        elif ftype == "sensor_list":
            if isinstance(val, str):
                val = [x.strip() for x in val.split(",") if x.strip()]
            if not isinstance(val, list):
                return None, f"{field['label']} must be a sensor list"
            validated, err = validate_sensor_list(val)
            if err:
                return None, err
            merged[key] = validated
        else:
            merged[key] = str(val) if val is not None else ""

    return merged, None


def display_name(blueprint_id: str, config: dict) -> str:
    label = display_label(config)
    if label:
        return label
    bp = get_blueprint(blueprint_id)
    return bp["name"] if bp else blueprint_id
