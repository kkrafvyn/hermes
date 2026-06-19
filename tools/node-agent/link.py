"""HERMES inter-node link protocol (Python mirror of hermes_link.h)."""

from __future__ import annotations

import struct
import zlib
from dataclasses import dataclass
from enum import IntEnum

HERMES_MAGIC = 0x484D


class MsgType(IntEnum):
    HEARTBEAT = 0x01
    PING = 0x02
    PONG = 0x03
    RESOURCE = 0x10
    APP_CONTROL = 0x20
    CHECKPOINT = 0x30


@dataclass
class Frame:
    src: int
    dst: int
    msg_type: int
    seq: int
    payload: bytes = b""

    def pack(self) -> bytes:
        header = struct.pack(
            "<HBBBBHH",
            HERMES_MAGIC, 1, self.msg_type,
            self.src, self.dst, self.seq, len(self.payload),
        )
        body = header + self.payload
        crc = zlib.crc32(body) & 0xFFFFFFFF
        return body + struct.pack("<I", crc)

    @classmethod
    def unpack(cls, data: bytes) -> Frame:
        magic, ver, mtype, src, dst, seq, plen = struct.unpack("<HBBBBHH", data[:10])
        if magic != HERMES_MAGIC:
            raise ValueError("bad magic")
        payload = data[10:10 + plen]
        return cls(src, dst, mtype, seq, payload)

    def to_dict(self) -> dict:
        return {
            "src": self.src,
            "dst": self.dst,
            "type": self.msg_type,
            "seq": self.seq,
            "payload_b64": self.payload.hex() if self.payload else "",
        }

    @classmethod
    def from_dict(cls, d: dict) -> Frame:
        payload = bytes.fromhex(d.get("payload_b64", "")) if d.get("payload_b64") else b""
        return cls(d["src"], d["dst"], d["type"], d["seq"], payload)
