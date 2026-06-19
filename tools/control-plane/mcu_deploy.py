"""MCU deploy API helpers for control plane."""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(ROOT / "scripts"))

from mcu_registry import deploy_approaches_summary, deploy_plan  # noqa: E402


def deploy_plan_detail(target_id: str) -> dict:
    return deploy_plan(target_id)
