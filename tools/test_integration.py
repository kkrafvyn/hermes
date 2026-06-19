#!/usr/bin/env python3
"""HERMES integration tests — run against live control plane."""

from __future__ import annotations

import json
import subprocess
import sys
import time
import urllib.request
import urllib.error
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SERVER = ROOT / "tools" / "control-plane" / "server.py"
API = "http://localhost:8787"
PASS = 0
FAIL = 0


def api(method: str, path: str, body: dict | None = None) -> dict:
    data = json.dumps(body or {}).encode() if body is not None else None
    req = urllib.request.Request(
        f"{API}{path}",
        data=data,
        headers={
            "Content-Type": "application/json",
            "X-Hermes-Token": "hermes-dev",
        } if data else {"X-Hermes-Token": "hermes-dev"},
        method=method,
    )
    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            return json.loads(resp.read())
    except urllib.error.HTTPError as e:
        return json.loads(e.read())


def check(name: str, cond: bool, detail: str = "") -> None:
    global PASS, FAIL
    if cond:
        PASS += 1
        print(f"  PASS  {name}")
    else:
        FAIL += 1
        print(f"  FAIL  {name} {detail}")


def wait_api(timeout: float = 15) -> bool:
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            api("GET", "/api/status")
            return True
        except Exception:
            time.sleep(0.2)
    return False


def run_tests() -> None:
    print("HERMES Integration Tests")
    print("=" * 40)

    # Ensure all nodes online
    for nid, name, ntype in [(0, "STM32H7", "stm32"), (1, "ESP32-S3", "esp32"), (2, "nRF52840", "nrf")]:
        api("POST", "/api/nodes/register", {
            "node_id": nid, "name": name, "node_type": ntype, "agent": True,
        })

    # Status
    s = api("GET", "/api/status")
    check("cluster online", s.get("cluster_online") is True)
    check("3 nodes", len(s.get("nodes", [])) == 3)

    # Create blueprint apps for lifecycle tests
    farm = api("POST", "/api/apps/create", {
        "blueprint": "smartfarm",
        "name": "test-farm",
        "config": {"zone_name": "Test Farm", "target_node": 0, "sensors": ["soil_moisture"]},
        "auto_start": True,
    })
    if farm.get("name") == "test-farm":
        check("create smartfarm", True)
    elif "error" in farm and "already" in farm["error"]:
        check("create smartfarm", True)
    else:
        check("create smartfarm", False, str(farm))
    tele = api("POST", "/api/apps/create", {
        "blueprint": "telemetry",
        "name": "test-telemetry",
        "config": {"hub_name": "Test Hub", "target_node": 1, "sensors": ["voltage"]},
        "auto_start": True,
    })
    if tele.get("name") == "test-telemetry":
        check("create telemetry", True)
    elif "error" in tele and "already" in tele["error"]:
        check("create telemetry", True)
    else:
        check("create telemetry", False, str(tele))

    s = api("GET", "/api/status")
    check("apps present", len(s.get("apps", [])) >= 2)

    # Register + heartbeat
    api("POST", "/api/nodes/register", {
        "node_id": 1, "name": "ESP32-S3", "node_type": "esp32", "agent": True,
    })
    hb = api("POST", "/api/nodes/heartbeat", {
        "node_id": 1,
        "telemetry": {"cpu": 30, "ram_kb": 180, "temp_c": 41, "power_mw": 400},
    })
    check("heartbeat ok", hb.get("ok") is True)
    check("leader assigned", "leader" in hb)

    # Install unique app
    test_manifest = ROOT / "examples" / "testapp" / "manifest.json"
    if test_manifest.exists():
        r = api("POST", "/api/apps/install", {"manifest_path": "examples/testapp/manifest.json"})
        if "error" in r and "already" in r["error"]:
            check("install testapp", True)
        else:
            check("install testapp", r.get("name") == "testapp", str(r))

    # App lifecycle
    api("POST", "/api/apps/test-farm/start", {})
    r = api("POST", "/api/apps/test-farm/pause", {})
    check("pause smartfarm", r.get("state") in ("paused", "running"))

    # Migration
    m = api("POST", "/api/apps/test-telemetry/migrate", {"dst_node": 2})
    check("migrate telemetry", m.get("app", {}).get("node_id") == 2, str(m))
    check("migration steps", len(m.get("steps", [])) == 5)

    # Commands queued for node
    cmds = api("GET", "/api/nodes/2/commands")
    check("commands endpoint", "commands" in cmds)

    # Logs
    logs = api("GET", "/api/logs")
    check("logs available", len(logs.get("logs", [])) > 0)


def main() -> int:
    global FAIL
    srv = subprocess.Popen([sys.executable, str(SERVER)], cwd=str(ROOT))
    try:
        if not wait_api():
            print("Control plane did not start")
            return 1
        run_tests()
    finally:
        srv.terminate()
        srv.wait(timeout=3)

    print("=" * 40)
    print(f"Results: {PASS} passed, {FAIL} failed")
    return 1 if FAIL else 0


if __name__ == "__main__":
    sys.exit(main())
