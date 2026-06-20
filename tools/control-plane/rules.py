#!/usr/bin/env python3
"""Simple alert rules for cluster and board simulation."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
RULES_PATH = ROOT / "data" / "rules.json"

DEFAULT_RULES = [
    {"id": "high-load", "sensor": "load_percent", "op": "gt", "threshold": 80, "action": "alert", "enabled": True},
    {"id": "low-moisture", "sensor": "soil_moisture", "op": "lt", "threshold": 25, "action": "alert", "enabled": True},
]


def load_rules() -> list[dict]:
    if RULES_PATH.is_file():
        try:
            return json.loads(RULES_PATH.read_text(encoding="utf-8"))
        except json.JSONDecodeError:
            pass
    return [dict(r) for r in DEFAULT_RULES]


def save_rules(rules: list[dict]) -> list[dict]:
    RULES_PATH.parent.mkdir(parents=True, exist_ok=True)
    RULES_PATH.write_text(json.dumps(rules, indent=2) + "\n", encoding="utf-8")
    return rules


def evaluate_rules(metrics: dict) -> list[dict]:
    fired = []
    for rule in load_rules():
        if not rule.get("enabled", True):
            continue
        key = rule.get("sensor", "")
        val = metrics.get(key)
        if val is None:
            continue
        try:
            val = float(val)
            th = float(rule.get("threshold", 0))
        except (TypeError, ValueError):
            continue
        op = rule.get("op", "gt")
        hit = (op == "gt" and val > th) or (op == "lt" and val < th) or (op == "eq" and val == th)
        if hit:
            fired.append({**rule, "value": val})
    return fired
