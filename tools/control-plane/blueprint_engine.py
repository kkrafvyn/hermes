"""Simulate blueprint app behavior from UI config (no C code edits)."""

from __future__ import annotations

from typing import Any

from blueprint_handlers import INIT, TICK

NAME_KEYS = (
    "zone_name", "home_name", "network_name", "hub_name",
    "greenhouse_name", "site_name", "system_name", "station_name",
    "facility_name", "building_name", "barn_name", "pool_name",
    "charger_name", "tracker_name", "machine_name", "trap_name",
    "gate_name", "care_site_name", "lot_name",
)


def display_label(config: dict) -> str | None:
    for key in NAME_KEYS:
        if config.get(key):
            return str(config[key])
    return None


def init_runtime(blueprint_id: str, config: dict) -> dict:
    fn = INIT.get(blueprint_id)
    return fn(config) if fn else {}


def tick(blueprint_id: str, config: dict, runtime: dict) -> dict[str, Any]:
    fn = TICK.get(blueprint_id)
    return fn(config, runtime) if fn else runtime
