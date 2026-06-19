#!/usr/bin/env python3
"""Build and flash any HERMES MCU target (direct PIO, flash alias, or show port/coprocessor plan)."""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))

from pio_env import apply as apply_pio_env  # noqa: E402
from mcu_registry import (  # noqa: E402
    all_targets,
    deploy_approaches_summary,
    deploy_plan,
    resolve_build,
)
from serial_ports import detect_port, list_ports, print_ports  # noqa: E402

apply_pio_env()


def find_pio() -> str | None:
    for p in [
        "pio",
        r"C:\Users\Donfrass\AppData\Local\Programs\Python\Python313\Scripts\pio.exe",
    ]:
        try:
            subprocess.run([p, "--version"], capture_output=True, check=True)
            return p
        except (FileNotFoundError, subprocess.CalledProcessError):
            continue
    return None


def resolve_upload_port(explicit: str | None, auto: bool) -> str | None:
    """Resolve serial port: explicit flag, auto-detect, or None."""
    if explicit:
        return explicit
    if not auto:
        return None
    port = detect_port()
    if port:
        print_ports(selected=port)
        print(f"\nAuto-selected port: {port}")
        return port
    print_ports()
    print("\nNo serial port found — build will run without upload.")
    print("Connect the board or pass --port COMx")
    return None


def print_plan(target_id: str) -> int:
    plan = deploy_plan(target_id)
    if "error" in plan and plan.get("approach") == "unknown":
        print(f"ERROR: {plan['error']}")
        return 1
    print(f"MCU:      {plan['id']} — {plan['name']}")
    print(f"Approach: {plan['approach']}")
    if plan.get("alias_summary"):
        print(f"Alias:    {plan['alias_summary']}")
    if plan.get("notes"):
        print(f"Notes:    {plan['notes']}")
    print("\nSteps:")
    for i, step in enumerate(plan.get("steps", []), 1):
        print(f"  {i}. {step}")
    if plan.get("flash_command"):
        print(f"\nFlash:    {plan['flash_command']}")
    if plan.get("stub_files"):
        print("\nStub files:")
        for f in plan["stub_files"]:
            print(f"  - {f}")
    if plan.get("compile_defines"):
        print("\nDefines:", " ".join(plan["compile_defines"]))
    if plan.get("doc"):
        print(f"\nDoc:      {plan['doc']}")
    return 0


def run_pio_build(
    target_id: str,
    port: str | None,
    upload: bool,
    auto_port: bool,
    bridge: bool,
    node_id: int,
) -> int:
    plan = deploy_plan(target_id)
    approach = plan.get("approach")
    if approach not in ("direct_pio", "flash_alias"):
        print(f"{target_id} uses approach '{approach}' — cannot run PlatformIO.")
        print("Use --plan-only to see instructions.")
        return 1

    spec = resolve_build(target_id)
    if not spec:
        print(f"No PIO build resolved for {target_id}")
        return 1

    pio = find_pio()
    if not pio:
        print("PlatformIO not found. Install: pip install platformio")
        return 1

    path = ROOT / spec["path"]
    if not path.exists():
        print(f"Path not found: {path}")
        return 1

    upload_port = None
    if upload:
        upload_port = resolve_upload_port(port, auto_port)
        if not upload_port:
            print("Refusing to upload without a serial port. Use --port COMx or connect the board.")
            return 1

    cmd = [pio, "run", "-e", spec["env"], "-d", str(path)]
    if upload:
        cmd.extend(["-t", "upload", "--upload-port", upload_port])

    label = target_id if approach == "direct_pio" else f"{target_id} (via {spec['builds_as']})"
    print(f"\nBUILD {label}: {' '.join(cmd)}")
    rc = subprocess.call(cmd, env=os.environ.copy())
    if rc != 0:
        return rc

    print(f"\nFlash OK on {upload_port}")
    bridge_cmd = (
        f"python tools/serial-bridge/bridge.py {upload_port} --register "
        f"--node-id {node_id} --name \"{plan['name']}\" --node-type {target_id}"
    )
    print("\nNext — start control plane and serial bridge:")
    print("  python run.py")
    print(f"  {bridge_cmd}")

    if bridge:
        print("\nStarting serial bridge (Ctrl+C to stop)...")
        bridge_path = ROOT / "tools" / "serial-bridge" / "bridge.py"
        return subprocess.call(
            [
                sys.executable, str(bridge_path), upload_port,
                "--register", "--node-id", str(node_id),
                "--name", plan["name"], "--node-type", target_id,
            ],
            cwd=str(ROOT),
        )
    return 0


def list_approaches() -> None:
    summary = deploy_approaches_summary()
    for key, block in summary["approaches"].items():
        print(f"\n=== {key} ({block['count']}) ===")
        print(block["description"])
        for t in block["targets"]:
            fc = t.get("flash_command") or "(see deploy plan)"
            print(f"  {t['id']:16} {t['name'][:40]:40} {fc}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="HERMES MCU deploy — auto-detects serial port, builds, and flashes"
    )
    parser.add_argument("mcu", nargs="?", help="Registry target id (e.g. esp32c6, esp8266)")
    parser.add_argument("--port", "-p", help="Serial port (auto-detected if omitted)")
    parser.add_argument("--list", action="store_true", help="List all targets by deploy approach")
    parser.add_argument("--list-ports", action="store_true", help="List connected serial ports")
    parser.add_argument("--plan-only", action="store_true", help="Print deploy steps without building")
    parser.add_argument("--no-upload", action="store_true", help="Build only, do not flash")
    parser.add_argument(
        "--no-auto-port", action="store_true",
        help="Do not auto-detect port (upload requires --port)",
    )
    parser.add_argument(
        "--bridge", action="store_true",
        help="After flash, start serial-bridge and register the node",
    )
    parser.add_argument("--node-id", type=int, default=1, help="Node id for --bridge (default 1)")
    args = parser.parse_args()

    if args.list_ports:
        print_ports()
        return 0 if list_ports() else 1

    if args.list:
        list_approaches()
        print("\nCLI:", deploy_approaches_summary()["cli"])
        return 0

    if not args.mcu:
        parser.print_help()
        print("\nExamples:")
        print("  python scripts/flash_mcu.py esp8266          # auto-detect port + flash")
        print("  python scripts/flash_mcu.py esp32c3 --bridge # flash then register node")
        print("  python scripts/flash_mcu.py --list-ports")
        return 1

    if args.mcu not in all_targets():
        print(f"Unknown MCU: {args.mcu}")
        print("Run: python scripts/flash_mcu.py --list")
        return 1

    if args.plan_only:
        return print_plan(args.mcu)

    return run_pio_build(
        args.mcu,
        args.port,
        upload=not args.no_upload,
        auto_port=not args.no_auto_port,
        bridge=args.bridge,
        node_id=args.node_id,
    )


if __name__ == "__main__":
    sys.exit(main())
