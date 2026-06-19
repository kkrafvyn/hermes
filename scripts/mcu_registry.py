#!/usr/bin/env python3
"""Load HERMES MCU registry and expose build targets."""

from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
REGISTRY_PATH = ROOT / "hardware" / "mcu_registry.json"


def load_registry() -> dict:
    return json.loads(REGISTRY_PATH.read_text())


def workarounds() -> dict:
    return load_registry().get("workarounds", {})


def workaround_for(target_id: str) -> dict | None:
    wa = workarounds().get(target_id)
    if wa:
        return wa
    cfg = all_targets().get(target_id, {})
    inline = cfg.get("workaround")
    return inline if isinstance(inline, dict) else None


def flash_command(target_id: str) -> str | None:
    """PlatformIO flash command for target or its workaround alias."""
    wa = workaround_for(target_id)
    if wa and wa.get("kind") == "coprocessor":
        return None
    build_map = firmware_build_map()
    tid = target_id
    if tid not in build_map and wa and wa.get("use_target"):
        tid = wa["use_target"]
    if tid not in build_map:
        return None
    cfg = build_map[tid]
    if wa:
        cfg = {**cfg, **{k: wa[k] for k in ("env", "path") if k in wa}}
    env = cfg.get("env")
    path = cfg.get("path", "hardware")
    if not env:
        return None
    return f"pio run -e {env} -d {path} -t upload"


def all_targets() -> dict:
    return load_registry().get("targets", {})


def supported_targets() -> dict:
    return {k: v for k, v in all_targets().items() if v.get("tier") in ("supported", "pio")}


def firmware_build_map() -> dict[str, dict]:
    """Map for scripts/build_all.py — PIO/IDF buildable targets only."""
    out: dict[str, dict] = {}
    for name, cfg in all_targets().items():
        tool = cfg.get("tool", "platformio")
        if tool == "stub":
            continue
        if tool == "platformio" and not cfg.get("env"):
            continue
        out[name] = {
            "path": cfg.get("path", "hardware"),
            "env": cfg.get("env"),
            "tool": tool,
            "tier": cfg.get("tier", "supported"),
            "family": cfg.get("family", ""),
            "boards": cfg.get("boards", []),
        }
    return out


def mcu_summary() -> dict:
    reg = load_registry()
    targets = reg.get("targets", {})
    tiers: dict[str, int] = {}
    families: dict[str, int] = {}
    for t in targets.values():
        tier = t.get("tier", "unknown")
        tiers[tier] = tiers.get(tier, 0) + 1
        fam = t.get("family", "other")
        families[fam] = families.get(fam, 0) + 1
    wa_all = workarounds()
    return {
        "version": reg.get("version", "1.0.0"),
        "target_count": len(targets),
        "sensor_mcu_ids": reg.get("sensor_mcu_ids", []),
        "tiers": tiers,
        "families": families,
        "workarounds": wa_all,
        "targets": [
            {
                "id": k,
                "name": v.get("name", k),
                "family": v.get("family", ""),
                "tier": v.get("tier", ""),
                "tool": v.get("tool", ""),
                "boards": v.get("boards", []),
                "notes": v.get("notes", ""),
                "workaround": wa_all.get(k) or v.get("workaround"),
                "flash_command": flash_command(k),
            }
            for k, v in sorted(targets.items())
        ],
    }


def sensor_mcu_ids() -> list[str]:
    reg = load_registry()
    ids = reg.get("sensor_mcu_ids")
    if ids == "__all__" or not ids:
        return sorted(all_targets().keys())
    return list(ids)
