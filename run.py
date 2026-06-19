#!/usr/bin/env python3
"""Start HERMES control plane (dashboard + API)."""

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
server = ROOT / "tools" / "control-plane" / "server.py"

if __name__ == "__main__":
    print("Starting HERMES...")
    sys.exit(subprocess.call([sys.executable, str(server)]))
