#!/usr/bin/env python3
"""Refresh compile_commands.json for IDE IntelliSense (Arduino / PlatformIO)."""

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ESP8266 = ROOT / "hardware" / "esp8266"
SRC = ROOT / "hardware" / "src"


def main() -> int:
    if not ESP8266.exists():
        print(f"Missing {ESP8266}")
        return 1
    cmd = ["pio", "run", "-e", "esp8266", "-t", "compiledb"]
    print("Running:", " ".join(cmd))
    rc = subprocess.call(cmd, cwd=str(ESP8266))
    if rc != 0:
        return rc
    src_db = ESP8266 / "compile_commands.json"
    root_db = ROOT / "compile_commands.json"
    shutil.copy2(src_db, root_db)
    shutil.copy2(src_db, SRC / "compile_commands.json")
    print(f"OK: {src_db}")
    print(f"Copied to: {root_db}")
    print(f"Copied to: {SRC / 'compile_commands.json'}")
    print("Reload the editor window (Ctrl+Shift+P -> Developer: Reload Window)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
