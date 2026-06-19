#!/usr/bin/env python3
"""Build HERMES firmware + SDK apps for all supported MCU targets."""

from __future__ import annotations

import json
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Keep PlatformIO toolchains on F: (project drive)
sys.path.insert(0, str(ROOT / "scripts"))
from pio_env import apply as apply_pio_env  # noqa: E402
from mcu_registry import firmware_build_map, mcu_summary  # noqa: E402

apply_pio_env()

FIRMWARE_TARGETS = firmware_build_map()

# SDK app cross-compile targets — all PIO-supported families
APP_TARGETS = [
    "host", "emulator",
    "esp8266", "esp8285", "esp32", "esp32s2", "esp32s3", "esp32c3", "esp32c6",
    "stm32h7", "stm32f4", "stm32f1", "stm32l4",
    "rp2040", "nrf52840", "nrf52832",
    "atmega328p", "samd21", "bk7231t", "rtl8710",
]


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


def build_firmware(name: str, cfg: dict, pio: str | None, upload: str | None) -> bool:
    path = ROOT / cfg["path"]
    if not path.exists():
        print(f"  SKIP {name}: {path} not found")
        return False

    if cfg["tool"] == "platformio":
        if not pio:
            print(f"  SKIP {name}: PlatformIO not installed")
            return False
        if not cfg.get("env"):
            print(f"  SKIP {name}: no PlatformIO env (stub target)")
            return False
        cmd = [pio, "run", "-e", cfg["env"], "-d", str(path)]
        if upload:
            cmd.extend(["-t", "upload", "--upload-port", upload])
        print(f"  BUILD {name}: {' '.join(cmd)}")
        env = os.environ.copy()
        return subprocess.call(cmd, env=env) == 0

    if cfg["tool"] == "esp-idf":
        if not shutil_which("idf.py"):
            print(f"  SKIP {name}: ESP-IDF not in PATH (use esp32s3-pio instead)")
            return False
        print(f"  BUILD {name}: idf.py in {path}")
        return subprocess.call(["idf.py", "build"], cwd=str(path)) == 0

    return False


def shutil_which(cmd: str) -> bool:
    from shutil import which
    return which(cmd) is not None


def build_apps(app_dir: Path, targets: list[str]) -> dict[str, bool]:
    build_py = ROOT / "sdk" / "build.py"
    results = {}
    for t in targets:
        print(f"  APP {app_dir.name} -> {t}")
        results[t] = subprocess.call([sys.executable, str(build_py), str(app_dir), t]) == 0
    return results


def main() -> int:
    import argparse
    parser = argparse.ArgumentParser(description="Build all HERMES MCU targets")
    parser.add_argument("--firmware", action="store_true", help="Build node firmware")
    parser.add_argument("--apps", action="store_true", help="Build SDK apps for all targets")
    parser.add_argument("--app", default="examples/smartfarm", help="App directory")
    parser.add_argument("--target", help="Single firmware target only")
    parser.add_argument("--upload-port", help="Serial port for upload (e.g. COM4)")
    parser.add_argument("--list", action="store_true", help="List targets")
    args = parser.parse_args()

    if args.list:
        print("Firmware targets (from mcu_registry.json):")
        for k, v in sorted(FIRMWARE_TARGETS.items()):
            tier = v.get("tier", "?")
            env = v.get("env") or "-"
            print(f"  {k:16} {v['tool']:12} {tier:12} env={env}  {v['path']}")
        summary = mcu_summary()
        print(f"\nMCU registry: {summary['target_count']} targets, tiers: {summary['tiers']}")
        print("SDK app targets:", ", ".join(APP_TARGETS))
        return 0

    do_fw = args.firmware or (not args.apps and not args.firmware)
    do_apps = args.apps or (not args.apps and not args.firmware)

    pio = find_pio()
    failed = 0

    if do_fw:
        print("=== Firmware ===")
        targets = {args.target: FIRMWARE_TARGETS[args.target]} if args.target else FIRMWARE_TARGETS
        if args.target and args.target not in FIRMWARE_TARGETS:
            print(f"Unknown target: {args.target}")
            print("Run with --list to see all MCU targets")
            return 1
        for name, cfg in targets.items():
            if not build_firmware(name, cfg, pio, args.upload_port):
                failed += 1

    if do_apps:
        print("=== SDK apps ===")
        app_dir = ROOT / args.app
        for t, ok in build_apps(app_dir, APP_TARGETS).items():
            if not ok:
                failed += 1

    print(f"\n{'ALL OK' if failed == 0 else f'{failed} FAILED'}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
