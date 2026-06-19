#!/usr/bin/env python3
"""Build-check all PlatformIO MCU targets from mcu_registry.json (F: drive cache)."""

from __future__ import annotations

import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "scripts"))

from pio_env import apply as apply_pio_env  # noqa: E402
from mcu_registry import firmware_build_map, mcu_summary  # noqa: E402

apply_pio_env()

# Disable PIO compile cache during batch builds (reduces Windows SConsign lock errors)
os.environ["PLATFORMIO_BUILD_CACHE_DIR"] = ""

PIO_CANDIDATES = [
    "pio",
    r"C:\Users\Donfrass\AppData\Local\Programs\Python\Python313\Scripts\pio.exe",
]
REPORT = ROOT / "build" / "mcu_build_report.txt"
LOCK = ROOT / "build" / "mcu_build.lock"


def find_pio() -> str | None:
    for p in PIO_CANDIDATES:
        try:
            subprocess.run([p, "--version"], capture_output=True, check=True, timeout=30)
            return p
        except (FileNotFoundError, subprocess.CalledProcessError, subprocess.TimeoutExpired):
            continue
    return None


def load_passed() -> set[str]:
    passed: set[str] = set()
    for path in [REPORT, ROOT / "build" / "mcu_build_live.log"]:
        if not path.exists():
            continue
        for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
            if line.startswith("PASS\t"):
                passed.add(line.split("\t", 2)[1])
            if "  PASS " in line:
                parts = line.strip().split()
                if len(parts) >= 2 and parts[0] == "PASS":
                    passed.add(parts[1].rstrip(":"))
    return passed


def load_failed() -> set[str]:
    passed = load_passed()
    failed: set[str] = set()
    if not REPORT.exists():
        return failed
    for line in REPORT.read_text(encoding="utf-8", errors="replace").splitlines():
        if line.startswith("FAIL\t"):
            name = line.split("\t", 2)[1]
            if name not in passed:
                failed.add(name)
    return failed


def append_report(name: str, success: bool, msg: str) -> None:
    REPORT.parent.mkdir(exist_ok=True)
    with REPORT.open("a", encoding="utf-8") as f:
        f.write(f"{'PASS' if success else 'FAIL'}\t{name}\t{msg.split(chr(10))[0]}\n")


def _kill_process_tree(pid: int) -> None:
    if sys.platform == "win32":
        subprocess.run(
            ["taskkill", "/F", "/T", "/PID", str(pid)],
            capture_output=True,
            timeout=30,
        )
    else:
        import signal

        os.killpg(os.getpgid(pid), signal.SIGTERM)


def _clean_env_build(path: Path, env: str) -> None:
    build_dir = path / ".pio" / "build" / env
    if build_dir.exists():
        shutil.rmtree(build_dir, ignore_errors=True)


def build_one(pio: str, name: str, cfg: dict, timeout_s: int = 1800, clean: bool = False) -> tuple[str, bool, str]:
    if cfg.get("tool") != "platformio" or not cfg.get("env"):
        return name, False, "skip: not platformio"
    path = ROOT / cfg["path"]
    if not path.exists():
        return name, False, f"path missing: {path}"
    env_name = cfg["env"]
    if clean:
        _clean_env_build(path, env_name)
    cmd = [pio, "run", "-e", env_name, "-d", str(path), "-j", "1"]
    t0 = time.time()
    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        env=os.environ.copy(),
    )
    try:
        stdout, stderr = proc.communicate(timeout=timeout_s)
        elapsed = time.time() - t0
        if proc.returncode == 0:
            return name, True, f"OK ({elapsed:.0f}s)"
        tail = (stdout + stderr)[-1200:]
        return name, False, f"exit {proc.returncode} ({elapsed:.0f}s)\n{tail}"
    except subprocess.TimeoutExpired:
        _kill_process_tree(proc.pid)
        proc.wait(timeout=15)
        return name, False, f"timeout after {timeout_s}s"


def main() -> int:
    import argparse
    parser = argparse.ArgumentParser(description="Build-check HERMES MCU targets (sequential, F: cache)")
    parser.add_argument("--family", help="Only targets in family")
    parser.add_argument("--exclude-family", action="append", default=[], help="Skip family (repeatable)")
    parser.add_argument("--target", help="Single target id")
    parser.add_argument("--quick", action="store_true", help="Core targets only (12)")
    parser.add_argument("--resume", action="store_true", help="Skip targets already PASS in report")
    parser.add_argument("--retry-failed", action="store_true", help="Only targets that failed and are not yet passed")
    parser.add_argument("--clean", action="store_true", help="Delete .pio/build/<env> before each target")
    parser.add_argument("--timeout", type=int, default=1800, help="Per-target timeout seconds")
    parser.add_argument("--reset-report", action="store_true", help="Clear build report before run")
    args = parser.parse_args()

    if LOCK.exists():
        age = time.time() - LOCK.stat().st_mtime
        if age < 7200:
            print(f"Another build may be running (lock {LOCK}, {age:.0f}s old). Wait or delete lock.")
            return 1
        LOCK.unlink()

    LOCK.write_text(str(os.getpid()))
    try:
        return _run(args)
    finally:
        LOCK.unlink(missing_ok=True)


def _run(args) -> int:
    pio = find_pio()
    if not pio:
        print("PlatformIO not found")
        return 1

    if args.reset_report and REPORT.exists():
        REPORT.unlink()

    passed_before = load_passed() if args.resume else set()
    targets = firmware_build_map()
    quick_set = {
        "esp8266", "esp32", "esp32s3", "esp32c3", "esp32c6", "d1_mini",
        "stm32h7", "stm32f4", "stm32f1", "nrf52840", "rp2040", "atmega328p",
    }

    items = sorted(targets.items())
    if args.target:
        items = [(k, v) for k, v in items if k == args.target]
    elif args.family:
        items = [(k, v) for k, v in items if v.get("family") == args.family]
    elif args.quick:
        items = [(k, v) for k, v in items if k in quick_set]

    if args.exclude_family:
        skip = set(args.exclude_family)
        items = [(k, v) for k, v in items if v.get("family") not in skip]

    if args.retry_failed:
        retry = load_failed()
        items = [(k, v) for k, v in items if k in retry]
    elif args.resume:
        items = [(k, v) for k, v in items if k not in passed_before]

    print(f"HERMES MCU build — {len(items)} targets (pio={pio})")
    print(f"PlatformIO: {os.environ.get('PLATFORMIO_CORE_DIR', '?')}")
    print(f"Resume: skipped {len(passed_before)} already passed")
    print("=" * 60)

    if not REPORT.exists() or args.reset_report:
        REPORT.parent.mkdir(exist_ok=True)
        REPORT.write_text(f"MCU build report {time.strftime('%Y-%m-%d %H:%M:%S')}\n\n")

    ok, fail, skip = 0, 0, len(passed_before)
    for name, cfg in items:
        print(f"\n--- {name} ({cfg.get('family')}) env={cfg.get('env')} ---", flush=True)
        n, success, msg = build_one(pio, name, cfg, args.timeout, clean=args.clean or args.retry_failed)
        append_report(n, success, msg)
        if success:
            ok += 1
            print(f"  PASS {n}: {msg.split(chr(10))[0]}", flush=True)
        else:
            fail += 1
            print(f"  FAIL {n}: {msg[:600]}", flush=True)

    print("\n" + "=" * 60)
    print(f"PASS {ok}  FAIL {fail}  SKIPPED {skip}  BUILT {len(items)}")
    summary = mcu_summary()
    print(f"Registry: {summary['target_count']} targets")
    print(f"Report: {REPORT}")
    all_passed = load_passed()
    print(f"Total passed (cumulative): {len(all_passed)}")
    return 1 if fail else 0


if __name__ == "__main__":
    sys.exit(main())
