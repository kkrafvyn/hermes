#!/usr/bin/env python3
"""Serial port discovery for HERMES flash and bridge scripts."""

from __future__ import annotations

# USB-UART adapters and common MCU board descriptors (lowercase match)
_PREFERRED = (
    "ch340", "ch910", "cp210", "ftdi", "usb serial", "usb-serial",
    "silicon labs", "jlink", "stlink", "st-link", "usb-enhanced-serial",
    "wch", "uart", "serial port", "usb serial device",
)
_DEPRIORITIZE = ("bluetooth", "com0com", "virtual", "management", "amt", "sol ")


def list_ports() -> list[dict]:
    """Return connected serial ports with metadata."""
    try:
        from serial.tools import list_ports
    except ImportError:
        return []
    out: list[dict] = []
    for p in list_ports.comports():
        desc = p.description or ""
        mfr = p.manufacturer or ""
        hwid = p.hwid or ""
        blob = f"{desc} {mfr} {hwid}".lower()
        score = sum(2 for k in _PREFERRED if k in blob)
        score -= sum(3 for k in _DEPRIORITIZE if k in blob)
        out.append({
            "device": p.device,
            "description": desc.strip() or "(no description)",
            "manufacturer": mfr.strip(),
            "score": score,
        })
    out.sort(key=lambda x: (-x["score"], x["device"]))
    return out


def detect_port(preferred: str | None = None) -> str | None:
    """
    Pick best upload port. If preferred is set and present, use it.
    Returns None when no ports found.
    """
    ports = list_ports()
    if not ports:
        return None
    if preferred:
        for p in ports:
            if p["device"].lower() == preferred.lower():
                return p["device"]
        print(f"Warning: {preferred} not found — auto-selecting from detected ports.")
    if len(ports) == 1:
        return ports[0]["device"]
    return ports[0]["device"]


def print_ports(selected: str | None = None) -> None:
    ports = list_ports()
    if not ports:
        print("No serial ports found.")
        print("  - Connect your board via USB")
        print("  - Install the USB-UART driver (CH340, CP210x, etc.)")
        print("  - pip install pyserial")
        return
    print("Serial ports:")
    for p in ports:
        mark = "  <-- selected" if selected and p["device"] == selected else ""
        extra = f" ({p['manufacturer']})" if p["manufacturer"] else ""
        print(f"  {p['device']}: {p['description']}{extra}{mark}")
