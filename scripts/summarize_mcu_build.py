#!/usr/bin/env python3
"""Summarize MCU build report — final status per target + workaround coverage."""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))

from mcu_registry import all_targets, load_registry, workaround_for  # noqa: E402

REPORT = ROOT / "build" / "mcu_build_report.txt"
SUMMARY = ROOT / "build" / "mcu_build_summary.txt"


def parse_report() -> dict[str, tuple[str, str]]:
    """target_id -> (status, detail) — PASS/STUB/FAIL wins last write; PASS kept if seen."""
    out: dict[str, tuple[str, str]] = {}
    if not REPORT.exists():
        return out
    for line in REPORT.read_text(encoding="utf-8", errors="replace").splitlines():
        if not line or line.startswith("MCU build report"):
            continue
        parts = line.split("\t", 2)
        if len(parts) < 3:
            continue
        status, name, detail = parts[0], parts[1], parts[2]
        if status == "PASS":
            out[name] = ("PASS", detail)
        elif name not in out or out[name][0] != "PASS":
            out[name] = (status, detail)
    return out


def effective_status(target_id: str, built: dict[str, tuple[str, str]]) -> tuple[str, str]:
    cfg = all_targets().get(target_id, {})
    tier = cfg.get("tier", "")
    if target_id in built and built[target_id][0] == "PASS":
        return "PASS", built[target_id][1]
    wa = workaround_for(target_id)
    if wa:
        use = wa.get("use_target")
        if use and use in built and built[use][0] == "PASS":
            return "WORKAROUND", f"via {use} ({wa.get('summary', '')[:80]})"
        if wa.get("kind") == "coprocessor":
            return "COPROCESSOR", wa.get("summary", "cluster link")
        if wa.get("kind") == "stub_port":
            return "STUB", wa.get("summary", "port generic_node.c")
        if use:
            return "WORKAROUND_PENDING", f"needs {use} build — {wa.get('summary', '')[:60]}"
        return "STUB", wa.get("summary", cfg.get("notes", ""))
    if tier == "alias":
        return "WORKAROUND", cfg.get("notes", "production via flash alias")
    if tier == "port":
        wa = workaround_for(target_id)
        if wa:
            return "PORT", wa.get("summary", "port generic_node.c")
        return "PORT", cfg.get("notes", "port generic_node.c")
    if tier in ("stub", "coprocessor"):
        return tier.upper(), cfg.get("notes", "stub")
    if target_id in built:
        return built[target_id]
    return "UNKNOWN", "not built"


def write_summary() -> dict:
    built = parse_report()
    targets = all_targets()
    rows: list[tuple[str, str, str, str]] = []
    counts = {"PASS": 0, "WORKAROUND": 0, "COPROCESSOR": 0, "STUB": 0, "FAIL": 0, "OTHER": 0}

    for tid in sorted(targets):
        status, detail = effective_status(tid, built)
        if status == "PASS":
            counts["PASS"] += 1
        elif status == "WORKAROUND":
            counts["WORKAROUND"] += 1
        elif status == "COPROCESSOR":
            counts["COPROCESSOR"] += 1
        elif status in ("STUB", "WORKAROUND_PENDING"):
            counts["STUB"] += 1
        elif status == "FAIL":
            counts["FAIL"] += 1
        else:
            counts["OTHER"] += 1
        rows.append((tid, targets[tid].get("name", tid), status, detail))

    lines = [
        "HERMES MCU build summary (effective status per registry target)",
        f"PIO passed: {counts['PASS']}  workaround-covered: {counts['WORKAROUND']}  "
        f"coprocessor: {counts['COPROCESSOR']}  stub: {counts['STUB']}  fail: {counts['FAIL']}",
        "",
        "ID\tSTATUS\tNAME\tDETAIL",
    ]
    for tid, name, status, detail in rows:
        lines.append(f"{tid}\t{status}\t{name}\t{detail.split(chr(10))[0]}")

    SUMMARY.parent.mkdir(exist_ok=True)
    SUMMARY.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return {"counts": counts, "rows": len(rows), "summary_path": str(SUMMARY)}


def main() -> int:
    info = write_summary()
    print(info["summary_path"])
    c = info["counts"]
    print(
        f"PASS {c['PASS']}  WORKAROUND {c['WORKAROUND']}  COPROCESSOR {c['COPROCESSOR']}  "
        f"STUB {c['STUB']}  FAIL {c['FAIL']}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
