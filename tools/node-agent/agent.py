#!/usr/bin/env python3
"""HERMES node agent — runs on each MCU (or host) and reports to control plane."""

from __future__ import annotations

import argparse
import json
import random
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from link import Frame, MsgType
from runtime import NodeRuntime

API_URL = "http://localhost:8787"

NODE_PROFILES = {
    0: {"name": "STM32H7", "type": "stm32", "cpu_base": 40, "ram_kb": 256, "temp": 38, "power": 420},
    1: {"name": "ESP32-S3", "type": "esp32", "cpu_base": 25, "ram_kb": 180, "temp": 42, "power": 380},
    2: {"name": "nRF52840", "type": "nrf", "cpu_base": 10, "ram_kb": 64, "temp": 35, "power": 95},
}


def api_post(path: str, body: dict) -> dict:
    data = json.dumps(body).encode()
    req = urllib.request.Request(
        f"{API_URL}{path}",
        data=data,
        headers={"Content-Type": "application/json", "X-Hermes-Token": "hermes-dev"},
        method="POST",
    )
    with urllib.request.urlopen(req, timeout=5) as resp:
        return json.loads(resp.read())


def handle_frame(node_id: int, frame_dict: dict, seq_counter: list) -> None:
    f = Frame.from_dict(frame_dict)
    if f.msg_type == MsgType.PING:
        pong = Frame(f.dst, f.src, MsgType.PONG, f.seq, f.payload)
        api_post("/api/link/send", {"frame": pong.to_dict()})
        print(f"[agent:{node_id}] link pong -> node {f.src}")
    elif f.msg_type == MsgType.PONG:
        print(f"[agent:{node_id}] link pong from node {f.src} seq={f.seq}")
    elif f.msg_type == MsgType.HEARTBEAT:
        pass
    else:
        print(f"[agent:{node_id}] link frame type={f.msg_type} from {f.src}")


def maybe_send_heartbeat(node_id: int, seq_counter: list, hub: bool) -> None:
    if not hub:
        return
    dst = (node_id + 1) % 3
    seq_counter[0] += 1
    hb = Frame(node_id, dst, MsgType.HEARTBEAT, seq_counter[0])
    api_post("/api/link/send", {"frame": hb.to_dict()})


def run_agent(node_id: int, interval: float, hub: bool) -> None:
    profile = NODE_PROFILES.get(node_id)
    if not profile:
        print(f"Unknown node id {node_id}")
        sys.exit(1)

    rt = NodeRuntime(node_id=node_id, base_cpu=profile["cpu_base"] * 0.3)
    seq = [0]
    print(f"[agent:{node_id}] {profile['name']} starting (hub={hub})")
    try:
        api_post("/api/nodes/register", {
            "node_id": node_id,
            "name": profile["name"],
            "node_type": profile["type"],
            "agent": True,
        })
    except urllib.error.URLError as e:
        print(f"[agent:{node_id}] Control plane unreachable: {e}")
        sys.exit(1)

    tick = 0
    while True:
        telemetry = {
            "cpu": round(rt.total_cpu() + random.uniform(-2, 2), 1),
            "ram_kb": profile["ram_kb"] + random.randint(-5, 5),
            "temp_c": round(profile["temp"] + random.uniform(-1, 1), 1),
            "power_mw": round(profile["power"] + random.uniform(-15, 15), 1),
            "link_quality": random.randint(85, 100),
        }
        try:
            resp = api_post("/api/nodes/heartbeat", {
                "node_id": node_id,
                "telemetry": telemetry,
            })
            for cmd in resp.get("commands", []):
                handle_command(node_id, rt, cmd)
            for frame in resp.get("frames", []):
                handle_frame(node_id, frame, seq)
            if hub and tick % 5 == node_id:
                maybe_send_heartbeat(node_id, seq, hub)
        except urllib.error.URLError:
            print(f"[agent:{node_id}] heartbeat failed")
        tick += 1
        time.sleep(interval)


def handle_command(node_id: int, rt: NodeRuntime, cmd: dict) -> None:
    action = cmd.get("action", "")
    app = cmd.get("app_name", "")
    cmd_id = cmd.get("cmd_id")
    result = rt.handle(action, app, cmd.get("payload"))
    print(f"[agent:{node_id}] {result}")
    try:
        api_post("/api/nodes/ack", {"node_id": node_id, "cmd_id": cmd_id})
    except urllib.error.URLError:
        pass


def main() -> None:
    global API_URL
    parser = argparse.ArgumentParser(description="HERMES node agent")
    parser.add_argument("node_id", type=int, choices=[0, 1, 2], help="Node ID")
    parser.add_argument("--interval", type=float, default=3.0, help="Heartbeat interval (s)")
    parser.add_argument("--api", default=API_URL, help="Control plane URL")
    parser.add_argument("--hub", action="store_true", help="Enable inter-node link via control plane")
    args = parser.parse_args()
    API_URL = args.api.rstrip("/")
    run_agent(args.node_id, args.interval, args.hub)


if __name__ == "__main__":
    main()
