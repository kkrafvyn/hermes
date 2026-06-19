"""HERMES unified sensor catalog — supported on all MCU targets."""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
CATALOG_PATH = ROOT / "sensors" / "catalog.json"


def _mcu_targets() -> list[str]:
    try:
        sys.path.insert(0, str(ROOT / "scripts"))
        from mcu_registry import sensor_mcu_ids  # noqa: E402
        return sensor_mcu_ids()
    except Exception:
        pass
    cat = load_catalog()
    return cat.get("mcu_targets", MCU_TARGETS)

MCU_TARGETS = ["esp8266", "esp32", "esp32s3", "esp32c3", "stm32h7", "stm32f4", "nrf52840"]


def load_catalog() -> dict:
    return json.loads(CATALOG_PATH.read_text())


def list_sensors() -> list[dict]:
    cat = load_catalog()
    sensors = cat.get("sensors", [])
    return [{**s, "mcu_targets": _mcu_targets()} for s in sensors]


def get_sensor(sensor_id: str) -> dict | None:
    for s in list_sensors():
        if s["id"] == sensor_id:
            return s
    return None


def validate_sensor_list(sensor_ids: list | str) -> tuple[list[str], str | None]:
    if isinstance(sensor_ids, str):
        sensor_ids = [x.strip() for x in sensor_ids.split(",") if x.strip()]
    if not sensor_ids:
        return [], None
    known = {s["id"] for s in list_sensors()}
    bad = [s for s in sensor_ids if s not in known]
    if bad:
        return [], f"unknown sensors: {', '.join(bad)}"
    return list(sensor_ids), None


def catalog_summary() -> dict:
    cat = load_catalog()
    mcus = _mcu_targets()
    return {
        "version": cat.get("version", "1.0.0"),
        "mcu_targets": mcus,
        "mcu_count": len(mcus),
        "sensor_count": len(cat.get("sensors", [])),
        "sensors": list_sensors(),
    }
