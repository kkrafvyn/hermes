#!/usr/bin/env python3
"""
Serial bridge — forwards ESP32 UART JSON telemetry to HERMES control plane.

Usage:
  python tools/serial-bridge/bridge.py COM3
  python tools/serial-bridge/bridge.py COM3 --register

ESP32 must be flashed with CONFIG_HERMES_USE_SERIAL_BRIDGE=y (default).
"""

from __future__ import annotations

import argparse
import json
import sys
import time
import urllib.request
import urllib.error

API_URL = "http://localhost:8787"


def api_post(path: str, body: dict) -> dict:
    data = json.dumps(body).encode()
    req = urllib.request.Request(
        f"{API_URL}{path}",
        data=data,
        headers={"Content-Type": "application/json", "X-Hermes-Token": "hermes-dev"},
        method="POST",
    )
    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            return json.loads(resp.read())
    except urllib.error.HTTPError as e:
        return json.loads(e.read())


def wait_api(timeout: float = 30) -> bool:
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            req = urllib.request.Request(
                f"{API_URL}/api/status",
                headers={"X-Hermes-Token": "hermes-dev"},
            )
            urllib.request.urlopen(req, timeout=3)
            return True
        except Exception:
            time.sleep(0.5)
    return False


def main() -> None:
    global API_URL
    parser = argparse.ArgumentParser(description="HERMES serial bridge")
    parser.add_argument("port", help="Serial port (e.g. COM3, /dev/ttyUSB0)")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--api", default=API_URL)
    parser.add_argument("--register", action="store_true", help="Register node on start")
    parser.add_argument("--node-id", type=int, default=1)
    parser.add_argument("--name", default="", help="Node display name (e.g. ESP8266)")
    parser.add_argument("--node-type", default="", help="Node type (e.g. esp8266)")
    args = parser.parse_args()

    API_URL = args.api.rstrip("/")

    try:
        import serial
    except ImportError:
        print("Install pyserial: pip install pyserial")
        sys.exit(1)

    if args.register:
        if not wait_api():
            print(f"Control plane not reachable at {API_URL}")
            sys.exit(1)
        names = {0: "STM32H7", 1: "ESP32-S3", 2: "nRF52840"}
        types = {0: "stm32", 1: "esp32", 2: "nrf"}
        api_post("/api/nodes/register", {
            "node_id": args.node_id,
            "name": args.name or names.get(args.node_id, f"node-{args.node_id}"),
            "node_type": args.node_type or types.get(args.node_id, "unknown"),
            "agent": True,
        })
        print(f"Registered node {args.node_id}")

    print(f"Bridge: {args.port} @ {args.baud} -> {API_URL}")
    with serial.Serial(args.port, args.baud, timeout=1) as ser:
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if not line or not line.startswith("{"):
                continue
            try:
                msg = json.loads(line)
            except json.JSONDecodeError:
                continue
            node_id = msg.get("node_id", args.node_id)
            telemetry = msg.get("telemetry", msg)
            try:
                resp = api_post("/api/nodes/heartbeat", {
                    "node_id": node_id,
                    "telemetry": telemetry,
                })
                cmds = resp.get("commands", [])
                for cmd in cmds:
                    print(f"  command -> device: {cmd}")
                    ser.write((json.dumps(cmd) + "\n").encode())
            except urllib.error.URLError as e:
                print(f"  API error: {e}")
            else:
                print(f"  heartbeat node {node_id} OK")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nBridge stopped.")
