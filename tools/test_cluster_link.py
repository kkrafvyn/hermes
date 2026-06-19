#!/usr/bin/env python3
"""Test inter-node cluster link through control plane hub."""

from __future__ import annotations

import json
import subprocess
import sys
import time
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools" / "node-agent"))
from link import Frame, MsgType

API = "http://localhost:8787"
SERVER = ROOT / "tools" / "control-plane" / "server.py"
AGENT = ROOT / "tools" / "node-agent" / "agent.py"


def api(method: str, path: str, body: dict | None = None) -> dict:
    data = json.dumps(body or {}).encode() if body else None
    headers = {"X-Hermes-Token": "hermes-dev"}
    if data:
        headers["Content-Type"] = "application/json"
    req = urllib.request.Request(
        f"{API}{path}", data=data, headers=headers, method=method,
    )
    with urllib.request.urlopen(req, timeout=5) as r:
        return json.loads(r.read())


def wait_api() -> bool:
    for _ in range(30):
        try:
            api("GET", "/api/status")
            return True
        except Exception:
            time.sleep(0.2)
    return False


def main() -> int:
    print("Cluster Link Test")
    print("=" * 40)
    srv = subprocess.Popen([sys.executable, str(SERVER)], cwd=str(ROOT))
    agents = []
    try:
        if not wait_api():
            print("FAIL: control plane")
            return 1

        for nid in (0, 1, 2):
            agents.append(subprocess.Popen(
                [sys.executable, str(AGENT), str(nid), "--hub"],
                cwd=str(ROOT),
            ))
            time.sleep(0.3)

        # Manual ping STM32 -> ESP32
        ping = Frame(0, 1, MsgType.PING, 42)
        api("POST", "/api/link/send", {"frame": ping.to_dict()})
        time.sleep(2)

        status = api("GET", "/api/link/status")
        routed = status["stats"]["frames_routed"]
        pings = status["stats"]["pings"]
        print(f"  frames routed: {routed}")
        print(f"  pings: {pings}")

        ok = routed >= 1 and pings >= 1
        print(f"\n{'PASS' if ok else 'FAIL'}: cluster link hub")
        return 0 if ok else 1
    finally:
        for p in agents:
            p.terminate()
        srv.terminate()
        for p in agents + [srv]:
            p.wait(timeout=3)


if __name__ == "__main__":
    sys.exit(main())
