"""Simulate blueprint app behavior from UI config (no C code edits)."""

from __future__ import annotations

from typing import Any

from blueprint_handlers import INIT, TICK

NAME_KEYS = (
    "zone_name", "home_name", "network_name", "hub_name",
    "greenhouse_name", "site_name", "system_name", "station_name",
    "facility_name", "building_name", "barn_name", "pool_name",
    "charger_name", "tracker_name", "machine_name", "trap_name",
    "gate_name", "care_site_name", "lot_name", "hive_name", "coop_name",
    "silo_name", "room_name", "garage_name", "meter_name", "cellar_name",
    "gen_name", "circuit_name", "line_name", "tank_name", "vessel_name",
    "unit_name", "batch_name", "lift_name",
)


def _sensor_ids(value) -> list[str]:
    if isinstance(value, list):
        return [str(x).strip() for x in value if str(x).strip()]
    if isinstance(value, str):
        return [x.strip() for x in value.split(",") if x.strip()]
    return []


def sensor_connectivity_metrics(config: dict) -> dict[str, Any]:
    local = _sensor_ids(config.get("sensors"))
    remote = _sensor_ids(config.get("remote_sensors"))
    mesh_on = bool(config.get("mesh_enabled"))
    mesh_nodes = int(config.get("mesh_max_nodes", 4) or 0)
    return {
        "local_sensors": len(local),
        "remote_sensor_types": len(remote),
        "mesh_enabled": mesh_on,
        "mesh_max_nodes": mesh_nodes if mesh_on else 0,
        "connected_sensors": len(local) + (len(remote) * mesh_nodes if mesh_on else 0),
    }


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
    out = fn(config, runtime) if fn else dict(runtime)
    out.update(sensor_connectivity_metrics(config))
    return out
