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
    deploy_approach,
    deploy_approaches_summary,
    deploy_plan,
    resolve_build,
)

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


def run_pio_build(target_id: str, port: str | None, upload: bool) -> int:
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

    cmd = [pio, "run", "-e", spec["env"], "-d", str(path)]
    if upload:
        cmd.append("-t")
        cmd.append("upload")
        if port:
            cmd.extend(["--upload-port", port])

    label = target_id if approach == "direct_pio" else f"{target_id} (via {spec['builds_as']})"
    print(f"BUILD {label}: {' '.join(cmd)}")
    rc = subprocess.call(cmd, env=os.environ.copy())
    if rc != 0:
        return rc

    if upload and port:
        print("\nNext — start control plane and serial bridge:")
        print("  python run.py")
        print(
            f"  python tools/serial-bridge/bridge.py {port} --register "
            f"--node-id 1 --name \"{plan['name']}\" --node-type {target_id}"
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
        description="HERMES MCU deploy — direct PIO, flash alias, port, or coprocessor plan"
    )
    parser.add_argument("mcu", nargs="?", help="Registry target id (e.g. esp32c6, pic16, cc2531)")
    parser.add_argument("--port", "-p", help="Serial port for upload/bridge (e.g. COM4)")
    parser.add_argument("--list", action="store_true", help="List all targets by deploy approach")
    parser.add_argument("--plan-only", action="store_true", help="Print deploy steps without building")
    parser.add_argument("--no-upload", action="store_true", help="Build only, do not flash")
    args = parser.parse_args()

    if args.list:
        list_approaches()
        print("\nCLI:", deploy_approaches_summary()["cli"])
        return 0

    if not args.mcu:
        parser.print_help()
        return 1

    if args.mcu not in all_targets():
        print(f"Unknown MCU: {args.mcu}")
        print("Run: python scripts/flash_mcu.py --list")
        return 1

    if args.plan_only:
        return print_plan(args.mcu)

    return run_pio_build(args.mcu, args.port, upload=not args.no_upload)


if __name__ == "__main__":
    sys.exit(main())
