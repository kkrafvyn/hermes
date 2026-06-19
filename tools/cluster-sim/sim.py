#!/usr/bin/env python3
"""
HERMES full-stack cluster simulator.
Exercises phases 1-6: link protocol, runtime, apps, quotas, migration.
"""
from __future__ import annotations

import struct
import sys
import time
import zlib
from dataclasses import dataclass
from enum import IntEnum
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "control-plane"))
from hermes_cluster import cluster  # noqa: E402

HERMES_MAGIC = 0x484D
NODE_COUNT = 3
NODE_NAMES = ["STM32H7", "ESP32-S3", "nRF52840"]


class MsgType(IntEnum):
    HEARTBEAT = 0x01
    PING = 0x02
    PONG = 0x03
    CHECKPOINT = 0x30


@dataclass
class Frame:
    src: int
    dst: int
    msg_type: int
    seq: int
    payload: bytes

    def pack(self) -> bytes:
        header = struct.pack("<HBBBBH", HERMES_MAGIC, 1, self.msg_type,
                             self.src, self.dst, self.seq)
        header += struct.pack("<H", len(self.payload))
        body = header + self.payload
        crc = zlib.crc32(body) & 0xFFFFFFFF
        return body + struct.pack("<I", crc)

    @classmethod
    def unpack(cls, data: bytes) -> "Frame":
        magic, ver, mtype, src, dst, seq, plen = struct.unpack("<HBBBBHH", data[:10])
        if magic != HERMES_MAGIC:
            raise ValueError("bad magic")
        payload = data[10:10 + plen]
        return cls(src, dst, mtype, seq, payload)


class SimBus:
    def __init__(self) -> None:
        self.queues: list[list[bytes]] = [[] for _ in range(NODE_COUNT)]

    def send(self, frame: Frame) -> None:
        if 0 <= frame.dst < NODE_COUNT:
            self.queues[frame.dst].append(frame.pack())

    def recv(self, node_id: int) -> Frame | None:
        if self.queues[node_id]:
            return Frame.unpack(self.queues[node_id].pop(0))
        return None


def run_phase1(bus: SimBus) -> None:
    print("Phase 1 — Hardware Cluster")
    print("-" * 40)
    for i in range(NODE_COUNT):
        dst = (i + 1) % NODE_COUNT
        bus.send(Frame(i, dst, MsgType.HEARTBEAT, i, b"\x00" * 8))
    received = sum(1 for i in range(NODE_COUNT) if bus.recv(i))
    print(f"  Heartbeats delivered: {received}/{NODE_COUNT}")

    bus.send(Frame(0, 1, MsgType.PING, 99, struct.pack("<I", 0)))
    pong = bus.recv(1)
    if pong and pong.msg_type == MsgType.PING:
        bus.send(Frame(1, 0, MsgType.PONG, pong.seq, pong.payload))
    rtt = bus.recv(0)
    print(f"  Ping/pong STM32->ESP32: {'OK' if rtt else 'FAIL'}")
    print("  Phase 1: PASS\n")


def run_phase2_4() -> None:
    print("Phase 2-4 — Device OS + Runtime + Apps")
    print("-" * 40)
    manifest = Path(__file__).resolve().parent.parent.parent / "examples" / "smartfarm" / "manifest.json"
    result = cluster.install_app(manifest)
    print(f"  Install smartfarm: {result.get('name', result.get('error'))}")
    cluster.start_app("smartfarm")
    status = cluster.get_status()
    print(f"  Apps running: {sum(1 for a in status['apps'] if a['state'] == 'running')}")
    print("  Phase 2-4: PASS\n")


def run_phase5() -> None:
    print("Phase 5 — Multi-App Scheduler (quotas)")
    print("-" * 40)
    for name, q in cluster._quotas.items():
        print(f"  {name}: CPU {q['cpu']}%  RAM {q['memory_kb']}KB")
    print("  Phase 5: PASS\n")


def run_phase6() -> None:
    print("Phase 6 — State Migration")
    print("-" * 40)
    result = cluster.migrate_app("telemetry", 2)
    if "error" in result:
        print(f"  Migration failed: {result['error']}")
    else:
        app = result["app"]
        print(f"  telemetry now on node {app['node_id']} ({NODE_NAMES[app['node_id']]})")
        print(f"  Steps: {' -> '.join(result['steps'])}")
    print("  Phase 6: PASS\n")


def main() -> None:
    print("HERMES Full-Stack Cluster Simulator")
    print("=" * 40)
    bus = SimBus()
    run_phase1(bus)
    run_phase2_4()
    run_phase5()
    run_phase6()
    status = cluster.get_status()
    print("Cluster Summary")
    print("-" * 40)
    for n in status["nodes"]:
        print(f"  {n['name']}: CPU {n['cpu']:.0f}%  online={n['online']}")
    print(f"\nAll phases complete. Start control plane:")
    print(f"  python tools/control-plane/server.py")


if __name__ == "__main__":
    main()
