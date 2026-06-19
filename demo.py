#!/usr/bin/env python3
"""
HERMES full demo — control plane + 3 node agents.
Run: python demo.py
"""

from __future__ import annotations

import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SERVER = ROOT / "tools" / "control-plane" / "server.py"
AGENT = ROOT / "tools" / "node-agent" / "agent.py"
PORT = 8787


def wait_for_api(timeout: float = 15.0) -> bool:
    import urllib.request
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            urllib.request.urlopen(f"http://localhost:{PORT}/api/status", timeout=1)
            return True
        except Exception:
            time.sleep(0.3)
    return False


def main() -> None:
    print("HERMES Demo — starting control plane + 3 node agents")
    print("=" * 50)

    server = subprocess.Popen([sys.executable, str(SERVER)], cwd=str(ROOT))
    if not wait_for_api():
        print("Control plane failed to start")
        server.kill()
        sys.exit(1)
    print(f"Control plane ready: http://localhost:{PORT}/")

    agents = []
    for node_id in (0, 1, 2):
        p = subprocess.Popen([sys.executable, str(AGENT), str(node_id)], cwd=str(ROOT))
        agents.append(p)
        time.sleep(0.2)
    print("Node agents: STM32H7 (0), ESP32-S3 (1), nRF52840 (2)")
    print()
    print("Dashboard: http://localhost:8787/")
    print("Press Ctrl+C to stop")
    print()

    try:
        server.wait()
    except KeyboardInterrupt:
        print("\nShutting down...")
        for p in agents:
            p.terminate()
        server.terminate()
        for p in agents + [server]:
            p.wait(timeout=3)


if __name__ == "__main__":
    main()
