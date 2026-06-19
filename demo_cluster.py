#!/usr/bin/env python3
"""
HERMES cluster demo — control plane + 3 hub-enabled agents + link test.
"""

from __future__ import annotations

import subprocess
import sys
import time
import urllib.request
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SERVER = ROOT / "tools" / "control-plane" / "server.py"
AGENT = ROOT / "tools" / "node-agent" / "agent.py"
PORT = 8787


def wait_api(timeout: float = 15.0) -> bool:
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            urllib.request.urlopen(f"http://localhost:{PORT}/api/status", timeout=1)
            return True
        except Exception:
            time.sleep(0.3)
    return False


def main() -> None:
    print("HERMES Cluster Demo (with inter-node link hub)")
    print("=" * 50)

    server = subprocess.Popen([sys.executable, str(SERVER)], cwd=str(ROOT))
    if not wait_api():
        print("Control plane failed to start")
        server.kill()
        sys.exit(1)

    agents = []
    for node_id in (0, 1, 2):
        p = subprocess.Popen(
            [sys.executable, str(AGENT), str(node_id), "--hub"],
            cwd=str(ROOT),
        )
        agents.append(p)
        time.sleep(0.2)

    print(f"Dashboard: http://localhost:{PORT}/")
    print("Agents running with cluster link hub enabled")
    print("Press Ctrl+C to stop\n")

    try:
        while True:
            time.sleep(5)
            s = json.loads(urllib.request.urlopen(
                f"http://localhost:{PORT}/api/status").read())
            link = s.get("link", {}).get("stats", {})
            print(f"  link: {link.get('frames_routed', 0)} frames, "
                  f"{link.get('pings', 0)} pings, "
                  f"{s.get('agents_connected', 0)} agents")
    except KeyboardInterrupt:
        print("\nShutting down...")
        for p in agents:
            p.terminate()
        server.terminate()
        for p in agents + [server]:
            p.wait(timeout=3)


if __name__ == "__main__":
    main()
