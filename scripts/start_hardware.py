#!/usr/bin/env python3
"""Start control plane and serial bridges for connected HERMES boards."""

from __future__ import annotations

import argparse
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))

from serial_ports import list_ports, print_ports  # noqa: E402


def main() -> int:
    parser = argparse.ArgumentParser(description="Start HERMES with hardware serial bridges")
    parser.add_argument("--esp8266-port", help="ESP8266 serial port (node 0)")
    parser.add_argument("--esp32-port", help="ESP32 WROOM serial port (node 1)")
    parser.add_argument("--no-server", action="store_true", help="Only start bridges (server already running)")
    args = parser.parse_args()

    ports = list_ports()
    mcu_ports = [p for p in ports if p["score"] > 0]
    if not mcu_ports:
        print_ports()
        print("\nNo USB serial boards detected.")
        print("Connect ESP8266 and/or ESP32 via USB, then rerun.")
        return 1

    esp8266 = args.esp8266_port
    esp32 = args.esp32_port
    if not esp8266 and not esp32:
        if len(mcu_ports) >= 2:
            esp8266, esp32 = mcu_ports[0]["device"], mcu_ports[1]["device"]
        elif len(mcu_ports) == 1:
            esp8266 = mcu_ports[0]["device"]
            print(f"One board on {esp8266} — assign with --esp8266-port / --esp32-port if needed.")

    procs: list[subprocess.Popen] = []
    if not args.no_server:
        server = ROOT / "tools" / "control-plane" / "server.py"
        print("Starting control plane on http://localhost:8787/")
        procs.append(subprocess.Popen([sys.executable, str(server)], cwd=str(ROOT)))
        time.sleep(1.5)

    bridge = ROOT / "tools" / "serial-bridge" / "bridge.py"
    if esp8266:
        cmd = [
            sys.executable, str(bridge), esp8266,
            "--register", "--node-id", "0",
            "--name", "ESP8266", "--node-type", "esp8266",
        ]
        print("Bridge ESP8266 (node 0):", " ".join(cmd))
        procs.append(subprocess.Popen(cmd, cwd=str(ROOT)))
    if esp32:
        cmd = [
            sys.executable, str(bridge), esp32,
            "--register", "--node-id", "1",
            "--name", "ESP32 WROOM TFT", "--node-type", "esp32",
        ]
        print("Bridge ESP32 (node 1):", " ".join(cmd))
        procs.append(subprocess.Popen(cmd, cwd=str(ROOT)))

    if not esp8266 and not esp32:
        return 1

    print("\nDashboard: http://localhost:8787/")
    print("On-board HTTP: ESP8266 and ESP32 each serve their own UI when on WiFi.")
    print("Press Ctrl+C to stop.")
    try:
        for p in procs:
            p.wait()
    except KeyboardInterrupt:
        for p in procs:
            p.terminate()
    return 0


if __name__ == "__main__":
    sys.exit(main())
