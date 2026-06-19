#!/usr/bin/env python3
"""Test blueprint App Studio API."""

from __future__ import annotations

import json
import sys
import threading
import time
import urllib.error
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools" / "control-plane"))

PORT = 8791
API = f"http://localhost:{PORT}"
TOKEN = "hermes-dev"


def api(method: str, path: str, body: dict | None = None) -> dict:
    data = json.dumps(body or {}).encode() if body is not None else None
    req = urllib.request.Request(
        f"{API}{path}",
        data=data,
        headers={"Content-Type": "application/json", "X-Hermes-Token": TOKEN},
        method=method,
    )
    with urllib.request.urlopen(req, timeout=30) as resp:
        return json.loads(resp.read())


def run_server() -> None:
    import server as srv  # noqa: E402
    srv.PORT = PORT
    httpd = srv.HTTPServer(("127.0.0.1", PORT), srv.Handler)
    httpd.serve_forever()


def main() -> int:
    failed = 0

    def check(name: str, ok: bool, detail: str = "") -> None:
        nonlocal failed
        if ok:
            print(f"  OK  {name}")
        else:
            print(f"  FAIL {name}: {detail}")
            failed += 1

    t = threading.Thread(target=run_server, daemon=True)
    t.start()
    time.sleep(2.5)

    try:
        bps = api("GET", "/api/blueprints")
        check("list blueprints", len(bps.get("blueprints", [])) >= 51)

        detail = api("GET", "/api/blueprints/smartfarm")
        check("smartfarm schema", "fields" in detail and len(detail["fields"]) >= 4)

        created = api("POST", "/api/apps/create", {
            "blueprint": "smarthome",
            "name": f"test-home-{int(time.time())}",
            "config": {
                "home_name": "Test Home",
                "target_temp_c": 21,
                "lights_on": True,
                "auto_climate": True,
                "check_interval_sec": 60,
                "target_node": 1,
            },
        })
        check("create smarthome", created.get("blueprint") == "smarthome", str(created))

        cfg = api("GET", f"/api/apps/{created['name']}/config")
        check("get config", cfg.get("config", {}).get("home_name") == "Test Home", str(cfg))

        updated = api("POST", f"/api/apps/{created['name']}/config", {
            "config": {**cfg["config"], "target_temp_c": 24},
        })
        check("update config", updated.get("config", {}).get("target_temp_c") == 24, str(updated))

        status = api("GET", "/api/status")
        app_names = [a["name"] for a in status.get("apps", [])]
        check("app in status", created["name"] in app_names, str(app_names))

    except urllib.error.URLError as e:
        print(f"  FAIL server: {e}")
        failed += 1

    print(f"\n{'ALL PASS' if failed == 0 else f'{failed} FAILED'}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
