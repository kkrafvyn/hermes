#!/usr/bin/env python3
"""MCU node emulator — runs HERMES node runtime in software."""

from __future__ import annotations

import argparse
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
AGENT = ROOT / "tools" / "node-agent" / "agent.py"
BUILD = ROOT / "sdk" / "build.py"
PACKAGE = ROOT / "sdk" / "package.py"
SERVER = ROOT / "tools" / "control-plane" / "server.py"


def main() -> int:
    parser = argparse.ArgumentParser(description="HERMES MCU emulator")
    parser.add_argument("--node", type=int, default=1, choices=[0, 1, 2])
    parser.add_argument("--app", default="examples/smartfarm")
    parser.add_argument("--with-server", action="store_true")
    args = parser.parse_args()

    srv = None
    if args.with_server:
        srv = subprocess.Popen([sys.executable, str(SERVER)], cwd=str(ROOT))
        time.sleep(1.5)

    subprocess.check_call([sys.executable, str(BUILD), str(ROOT / args.app), "emulator"])
    subprocess.check_call([sys.executable, str(PACKAGE), str(ROOT / args.app), "emulator"])

    agent = subprocess.Popen([
        sys.executable, str(AGENT), str(args.node), "--hub",
    ], cwd=str(ROOT))
    print(f"MCU emulator: node {args.node}, app {args.app}")
    print("Press Ctrl+C to stop")
    try:
        agent.wait()
    except KeyboardInterrupt:
        agent.terminate()
        if srv:
            srv.terminate()
    return 0


if __name__ == "__main__":
    sys.exit(main())
