#!/usr/bin/env python3
"""Refresh IDE IntelliSense: compile_flags.txt + sanitized compile_commands.json."""

from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ESP8266 = ROOT / "hardware" / "esp8266"
SRC = ROOT / "hardware" / "src"

# Xtensa / PlatformIO flags that host clangd cannot parse.
_STRIP_PATTERNS = [
    r"-mlongcalls\b",
    r"-mlong-calls\b",
    r"-mtext-section-literals\b",
    r"-free\b",
    r"-fipa-pta\b",
    r"-falign-functions=\S+",
    r"-fno-rtti\b",
    r"-fno-exceptions\b",
    r"-march=\S+",
    r"-mtarget-align\b",
    r"-Werror=\S+",
    r"-U__STRICT_ANSI__\b",
]


def sanitize_command(cmd: str) -> str:
    """Strip Xtensa-only flags; use clang++ so IDE parsing works on Windows."""
    out = re.sub(
        r"[^\s\"]+xtensa-lx106-elf-g\+\+\.exe",
        "clang++",
        cmd,
        flags=re.IGNORECASE,
    )
    for pat in _STRIP_PATTERNS:
        out = re.sub(pat, "", out)
    out = re.sub(r"\s+", " ", out).strip()
    return out


def sanitize_compile_db(path: Path) -> None:
    data = json.loads(path.read_text(encoding="utf-8"))
    for entry in data:
        if "command" in entry:
            entry["command"] = sanitize_command(entry["command"])
        elif "arguments" in entry:
            entry["arguments"] = [
                a for a in entry["arguments"]
                if not any(re.fullmatch(p.lstrip("-").replace("\\b", ""), a) for p in _STRIP_PATTERNS)
                and a not in ("-mlongcalls", "-mtext-section-literals", "-free", "-fipa-pta")
            ]
    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


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
    sanitize_compile_db(src_db)

    # Optional copies for tools that read compile_commands (sanitized, no Xtensa flags).
    for dest in (ROOT / "compile_commands.json", SRC / "compile_commands.json"):
        dest.write_text(src_db.read_text(encoding="utf-8"), encoding="utf-8")
        print(f"Sanitized copy: {dest}")

    print(f"OK: {src_db}")
    print("clangd uses hardware/src/compile_flags.txt (see .clangd CompilationDatabase: none)")
    print("Reload the editor: Ctrl+Shift+P -> Developer: Reload Window")
    return 0


if __name__ == "__main__":
    sys.exit(main())
