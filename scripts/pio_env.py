"""Set PlatformIO and build caches to F: drive (project root)."""

from __future__ import annotations

import os
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PIO_HOME = ROOT / ".platformio"
TMP_DIR = ROOT / "tmp"
PIP_CACHE = ROOT / ".pip-cache"


def apply() -> dict[str, str]:
    for d in (PIO_HOME, TMP_DIR, PIP_CACHE):
        d.mkdir(parents=True, exist_ok=True)
    env = {
        "PLATFORMIO_CORE_DIR": str(PIO_HOME),
        "PLATFORMIO_HOME_DIR": str(PIO_HOME),
        "TMP": str(TMP_DIR),
        "TEMP": str(TMP_DIR),
        "PIP_CACHE_DIR": str(PIP_CACHE),
    }
    os.environ.update(env)
    return env


def pio_ini_header() -> str:
    p = PIO_HOME.as_posix()
    return f"""[platformio]
core_dir = {p}
packages_dir = {p}/packages
platforms_dir = {p}/platforms
build_cache_dir = {p}/buildcache
"""
