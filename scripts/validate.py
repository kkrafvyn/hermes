#!/usr/bin/env python3
"""Run all HERMES validation suites."""

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

TESTS = [
    ("Integration", [sys.executable, str(ROOT / "tools" / "test_integration.py")]),
    ("Blueprints", [sys.executable, str(ROOT / "tools" / "test_blueprints.py")]),
    ("Cluster link", [sys.executable, str(ROOT / "tools" / "test_cluster_link.py")]),
    ("Phase sim", [sys.executable, str(ROOT / "tools" / "cluster-sim" / "sim.py")]),
    ("Package smartfarm", [sys.executable, str(ROOT / "sdk" / "package.py"),
                          str(ROOT / "examples" / "smartfarm"), "host"]),
]

def main() -> int:
    print("HERMES Complete Validation")
    print("=" * 40)
    failed = 0
    for name, cmd in TESTS:
        print(f"\n--- {name} ---")
        r = subprocess.call(cmd, cwd=str(ROOT))
        if r != 0:
            print(f"FAIL: {name}")
            failed += 1
        else:
            print(f"PASS: {name}")
    print("\n" + "=" * 40)
    print(f"{'ALL PASS' if failed == 0 else f'{failed} FAILED'}")
    return 1 if failed else 0

if __name__ == "__main__":
    sys.exit(main())
