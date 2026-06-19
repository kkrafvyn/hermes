#!/usr/bin/env python3
"""Coprocessor setup helper — Zigbee/radio dongles (cc253x, cc2652, efr32mg*)."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))

from mcu_registry import all_targets, deploy_approach, deploy_plan  # noqa: E402

COPROCESSOR_IDS = [tid for tid in all_targets() if deploy_approach(tid) == "coprocessor"]


def main() -> int:
    parser = argparse.ArgumentParser(description="HERMES coprocessor setup guide")
    parser.add_argument("mcu", nargs="?", help="Coprocessor id (cc2531, cc2652, efr32mg12, …)")
    parser.add_argument("--list", action="store_true", help="List coprocessor targets")
    parser.add_argument("--port", default="COM5", help="Serial port for bridge example")
    args = parser.parse_args()

    if args.list or not args.mcu:
        print("HERMES coprocessors (radio link — not App Studio node firmware):\n")
        for tid in COPROCESSOR_IDS:
            cfg = all_targets()[tid]
            print(f"  {tid:12} {cfg.get('name', '')}")
        print("\nUsage: python scripts/setup_coprocessor.py <mcu_id> [--port COM5]")
        return 0

    if deploy_approach(args.mcu) != "coprocessor":
        print(f"{args.mcu} is not a coprocessor (approach: {deploy_approach(args.mcu)})")
        print("For PIO nodes use: python scripts/flash_mcu.py", args.mcu)
        return 1

    plan = deploy_plan(args.mcu)
    port = args.port
    print(f"=== Coprocessor: {plan['name']} ({plan['id']}) ===\n")
    for i, step in enumerate(plan.get("steps", []), 1):
        s = step.replace("PORT", port)
        print(f"  {i}. {s}")
    print(f"\nDocumentation: {plan.get('doc', 'hardware/stubs/README.md')}")
    print("\nHost node (runs App Studio apps) — pick any direct PIO target:")
    print("  python scripts/flash_mcu.py esp32s3 --port COM4")
    print("  python scripts/flash_mcu.py nrf52840 --port COM6")
    return 0


if __name__ == "__main__":
    sys.exit(main())
