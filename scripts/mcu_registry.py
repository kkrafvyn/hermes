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
    spec = resolve_build(target_id)
    if not spec or not spec.get("env"):
        return None
    env = spec["env"]
    path = spec.get("path", "hardware")
    return f"pio run -e {env} -d {path} -t upload"


def deploy_approach(target_id: str) -> str:
    """direct_pio | flash_alias | sdk_port | coprocessor | unknown"""
    cfg = all_targets().get(target_id, {})
    wa = workaround_for(target_id)
    if wa and wa.get("kind") == "coprocessor":
        return "coprocessor"
    if cfg.get("tier") == "port" or (wa and wa.get("kind") == "stub_port"):
        return "sdk_port"
    if cfg.get("tier") == "alias" or (wa and wa.get("kind") in ("flash_alias", "runtime_alias")):
        return "flash_alias"
    if target_id in firmware_build_map():
        return "direct_pio"
    if wa and wa.get("use_target"):
        return "flash_alias"
    return "unknown"


def resolve_build(target_id: str) -> dict | None:
    """PIO env/path used to compile firmware for target_id (direct or alias)."""
    wa = workaround_for(target_id)
    if wa and wa.get("kind") == "coprocessor":
        return None
    if target_id in firmware_build_map():
        bm = firmware_build_map()[target_id]
        return {
            "approach": "direct_pio",
            "register_as": target_id,
            "builds_as": target_id,
            "env": bm["env"],
            "path": bm.get("path", "hardware"),
            "tool": bm.get("tool", "platformio"),
        }
    if wa and wa.get("use_target"):
        ut = wa["use_target"]
        bm = firmware_build_map().get(ut)
        if not bm:
            return None
        return {
            "approach": "flash_alias",
            "register_as": target_id,
            "builds_as": ut,
            "env": wa.get("env") or bm["env"],
            "path": wa.get("path") or bm.get("path", "hardware"),
            "tool": "platformio",
            "alias_summary": wa.get("summary", ""),
        }
    return None


PORT_STUB_FILES = [
    "hardware/stubs/generic_node.c",
    "hardware/stubs/hermes_stub_sensor.c",
    "hardware/stubs/hermes_stub_sensor.h",
]

COPROCESSOR_SETUP = {
    "cc2530": {"firmware": "Z-Stack coordinator", "doc": "hardware/stubs/ti_cc/README.md"},
    "cc2531": {"firmware": "Z-Stack USB coordinator", "doc": "hardware/stubs/ti_cc/README.md"},
    "cc2652": {"firmware": "TI Z-Stack 3 coordinator", "doc": "hardware/stubs/ti_cc/README.md"},
    "cc26xx": {"firmware": "TI SimpleLink coprocessor", "doc": "hardware/stubs/ti_cc/README.md"},
    "efr32mg12": {"firmware": "Silicon Labs Zigbee NCP (Spinel)", "doc": "hardware/stubs/efr32/README.md"},
    "efr32mg21": {"firmware": "EFR32 MG21 Zigbee NCP", "doc": "hardware/stubs/efr32/README.md"},
}


def deploy_plan(target_id: str) -> dict:
    """Full deploy plan for one registry target."""
    cfg = all_targets().get(target_id)
    if not cfg:
        return {"error": f"unknown mcu: {target_id}"}

    approach = deploy_approach(target_id)
    wa = workaround_for(target_id)
    base = {
        "id": target_id,
        "name": cfg.get("name", target_id),
        "family": cfg.get("family", ""),
        "tier": cfg.get("tier", ""),
        "approach": approach,
        "notes": cfg.get("notes", ""),
        "boards": cfg.get("boards", []),
    }

    if approach == "direct_pio":
        spec = resolve_build(target_id)
        port_flag = f" --upload-port PORT" if spec else ""
        return {
            **base,
            "build": spec,
            "steps": [
                f"pio run -e {spec['env']} -d {spec['path']} -t upload{port_flag}",
                "python run.py",
                f"python tools/serial-bridge/bridge.py PORT --register --node-id N --name \"{base['name']}\" --node-type {target_id}",
            ],
            "flash_command": flash_command(target_id),
        }

    if approach == "flash_alias":
        spec = resolve_build(target_id)
        if not spec:
            return {**base, "error": "no alias build resolved"}
        ref = firmware_build_map().get(spec["builds_as"], {})
        return {
            **base,
            "build": spec,
            "workaround": wa,
            "alias_summary": wa.get("summary", "") if wa else "",
            "steps": [
                f"# Flash {spec['builds_as']} firmware onto {target_id} hardware",
                f"pio run -e {spec['env']} -d {spec['path']} -t upload --upload-port PORT",
                "python run.py",
                f"python tools/serial-bridge/bridge.py PORT --register --node-id N "
                f"--name \"{base['name']}\" --node-type {target_id}",
            ],
            "flash_command": flash_command(target_id),
            "reference_build": f"pio run -e {ref.get('env', spec['env'])} -d {ref.get('path', spec['path'])}",
        }

    if approach == "sdk_port":
        ref_env = (wa or {}).get("env", "pic32")
        ref_path = (wa or {}).get("use_target", "pic32")
        ref_spec = firmware_build_map().get(ref_path, {})
        return {
            **base,
            "workaround": wa,
            "stub_files": PORT_STUB_FILES,
            "compile_defines": [
                f"-DHERMES_STUB_MCU_NAME=\"{target_id}\"",
                f"-DHERMES_STUB_FAMILY=\"{cfg.get('family', 'generic')}\"",
                "-DHERMES_NODE_ID=1",
            ],
            "steps": [
                f"Copy stub files from hardware/stubs/ into your {target_id} SDK project",
                f"Define HERMES_STUB_MCU_NAME={target_id}, HERMES_STUB_FAMILY={cfg.get('family')}, HERMES_NODE_ID",
                "Wire printf to UART 115200; call hermes_stub_handle_command() on RX",
                "python run.py",
                f"python tools/serial-bridge/bridge.py PORT --register --node-type {target_id}",
                f"# Protocol reference: pio run -e {ref_spec.get('env', ref_env)} -d {ref_spec.get('path', 'hardware')}",
            ],
            "doc": cfg.get("path", "hardware/stubs") + "/README.md",
        }

    if approach == "coprocessor":
        setup = COPROCESSOR_SETUP.get(target_id, {"firmware": "vendor radio stack", "doc": "hardware/stubs/README.md"})
        return {
            **base,
            "workaround": wa,
            "coprocessor": setup,
            "steps": [
                f"Flash {setup['firmware']} (not HERMES node firmware)",
                "Connect UART/USB to PC",
                "python run.py",
                "python tools/serial-bridge/bridge.py PORT --register --node-type " + target_id,
                "Run App Studio apps on a separate PIO node (esp32, nrf52840, …)",
            ],
            "doc": setup.get("doc", "hardware/stubs/README.md"),
        }

    return {**base, "error": "no deploy path defined"}


def deploy_approaches_summary() -> dict:
    """All targets grouped by deploy approach."""
    groups: dict[str, list[dict]] = {
        "direct_pio": [],
        "flash_alias": [],
        "sdk_port": [],
        "coprocessor": [],
        "unknown": [],
    }
    for tid in sorted(all_targets()):
        approach = deploy_approach(tid)
        entry = {
            "id": tid,
            "name": all_targets()[tid].get("name", tid),
            "flash_command": flash_command(tid),
            "deploy_url": f"/api/mcus/{tid}/deploy",
        }
        groups.get(approach, groups["unknown"]).append(entry)
    return {
        "approaches": {
            "direct_pio": {
                "description": "PlatformIO unified firmware — full hermes_sensor.cpp",
                "count": len(groups["direct_pio"]),
                "targets": groups["direct_pio"],
            },
            "flash_alias": {
                "description": "Flash sibling PIO build; register node as real MCU type",
                "count": len(groups["flash_alias"]),
                "targets": groups["flash_alias"],
            },
            "sdk_port": {
                "description": "Port hardware/stubs/generic_node.c in vendor SDK",
                "count": len(groups["sdk_port"]),
                "targets": groups["sdk_port"],
            },
            "coprocessor": {
                "description": "Zigbee/radio dongle — cluster link only",
                "count": len(groups["coprocessor"]),
                "targets": groups["coprocessor"],
            },
        },
        "cli": {
            "list": "python scripts/flash_mcu.py --list",
            "flash": "python scripts/flash_mcu.py <mcu_id> --port COM4",
            "plan": "python scripts/flash_mcu.py <mcu_id> --plan-only",
            "coprocessor": "python scripts/setup_coprocessor.py <mcu_id>",
        },
    }


def all_targets() -> dict:
    return load_registry().get("targets", {})


def supported_targets() -> dict:
    return {k: v for k, v in all_targets().items() if v.get("tier") in ("supported", "pio")}


def firmware_build_map() -> dict[str, dict]:
    """Map for scripts/build_all.py — PIO/IDF buildable targets only."""
    out: dict[str, dict] = {}
    for name, cfg in all_targets().items():
        tool = cfg.get("tool", "platformio")
        if tool in ("stub", "alias", "port", "coprocessor"):
            continue
        if not cfg.get("env"):
            continue
        if tool not in ("platformio", "esp-idf"):
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
                "approach": deploy_approach(k),
                "deploy_url": f"/api/mcus/{k}/deploy",
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
