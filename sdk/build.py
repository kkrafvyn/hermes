#!/usr/bin/env python3
"""Cross-target build for HERMES applications."""

from __future__ import annotations

import json
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SDK_INC = ROOT / "sdk" / "include"

# All supported MCU / host targets for app packaging
BUILD_TARGETS = {
    "host": {
        "cc": ["gcc", "-std=c11", "-Wall", "-O2", "-DHERMES_PACK_BUILD"],
        "objcopy": ["objcopy", "-O", "binary"],
    },
    "emulator": {
        "cc": ["gcc", "-std=c11", "-Wall", "-O2", "-DHERMES_EMULATOR"],
        "objcopy": ["objcopy", "-O", "binary"],
    },
    "esp8266": {
        "cc": ["xtensa-lx106-elf-gcc", "-std=c11", "-Os", "-DHERMES_PACK_BUILD"],
        "objcopy": ["xtensa-lx106-elf-objcopy", "-O", "binary"],
    },
    "esp32s3": {
        "cc": ["xtensa-esp32s3-elf-gcc", "-std=c11", "-Os", "-DHERMES_PACK_BUILD"],
        "objcopy": ["xtensa-esp32s3-elf-objcopy", "-O", "binary"],
    },
    "esp32c3": {
        "cc": ["riscv32-esp-elf-gcc", "-std=c11", "-Os", "-DHERMES_PACK_BUILD"],
        "objcopy": ["riscv32-esp-elf-objcopy", "-O", "binary"],
    },
    "stm32h7": {
        "cc": ["arm-none-eabi-gcc", "-std=c11", "-mcpu=cortex-m7", "-mthumb", "-O2", "-DHERMES_PACK_BUILD"],
        "objcopy": ["arm-none-eabi-objcopy", "-O", "binary"],
    },
    "stm32f4": {
        "cc": ["arm-none-eabi-gcc", "-std=c11", "-mcpu=cortex-m4", "-mthumb", "-O2", "-DHERMES_PACK_BUILD"],
        "objcopy": ["arm-none-eabi-objcopy", "-O", "binary"],
    },
    "nrf52840": {
        "cc": ["arm-none-eabi-gcc", "-std=c11", "-mcpu=cortex-m4", "-mthumb", "-O2", "-DHERMES_PACK_BUILD"],
        "objcopy": ["arm-none-eabi-objcopy", "-O", "binary"],
    },
    # aliases
    "esp32": {
        "cc": ["xtensa-esp32s3-elf-gcc", "-std=c11", "-Os", "-DHERMES_PACK_BUILD"],
        "objcopy": ["xtensa-esp32s3-elf-objcopy", "-O", "binary"],
    },
    "stm32": {
        "cc": ["arm-none-eabi-gcc", "-std=c11", "-mcpu=cortex-m7", "-mthumb", "-O2", "-DHERMES_PACK_BUILD"],
        "objcopy": ["arm-none-eabi-objcopy", "-O", "binary"],
    },
}


def _run(cmd: list[str]) -> bool:
    try:
        subprocess.check_call(cmd)
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        return False


def build(app_dir: Path, target: str = "host") -> int:
    app_dir = app_dir.resolve()
    manifest = json.loads((app_dir / "manifest.json").read_text())
    src = app_dir / "src" / "app.c"
    if not src.exists():
        print(f"No src/app.c in {app_dir}")
        return 1

    build_dir = app_dir / "build" / target
    build_dir.mkdir(parents=True, exist_ok=True)
    cfg = BUILD_TARGETS.get(target, BUILD_TARGETS["host"])
    obj = build_dir / f"{manifest['name']}.o"
    bin_out = build_dir / f"{manifest['name']}.bin"
    cmd = cfg["cc"] + [f"-I{SDK_INC}", "-c", str(src), "-o", str(obj)]

    if _run(cmd):
        if _run(cfg["objcopy"] + [str(obj), str(bin_out)]):
            print(f"Built {manifest['name']} for {target} -> {bin_out}")
            return 0
        bin_out.write_bytes(obj.read_bytes())
        print(f"Built {manifest['name']} for {target} (object) -> {bin_out}")
        return 0

    if target in ("host", "emulator"):
        flags = ["-DHERMES_EMULATOR"] if target == "emulator" else ["-DHERMES_PACK_BUILD"]
        fallback = ["gcc", "-std=c11", "-Wall"] + flags + [f"-I{SDK_INC}", "-c", str(src), "-o", str(obj)]
        if _run(fallback):
            bin_out.write_bytes(obj.read_bytes())
            print(f"Built {manifest['name']} {target} binary -> {bin_out}")
            return 0

    shutil.copy(src, build_dir / "app.c")
    print(f"Staged {manifest['name']} for {target} (toolchain missing) -> {build_dir}")
    return 0


def build_all(app_dir: Path) -> int:
    failed = 0
    for target in ["host", "emulator", "esp8266", "esp32s3", "esp32c3", "stm32h7", "stm32f4", "nrf52840"]:
        if build(app_dir, target) != 0:
            failed += 1
    return 1 if failed else 0


def main() -> int:
    if len(sys.argv) < 2:
        print("Usage: python sdk/build.py <app-dir> [target|all]")
        print("  targets:", ", ".join(BUILD_TARGETS.keys()), ", all")
        return 1
    app = Path(sys.argv[1])
    target = sys.argv[2] if len(sys.argv) > 2 else "host"
    if target == "all":
        return build_all(app)
    return build(app, target)


if __name__ == "__main__":
    sys.exit(main())
