# HERMES Quickstart

## Prerequisites

- CMake 3.16+ (optional)
- C compiler (GCC — MSYS2 on Windows)
- Python 3.8+
- Optional: STM32CubeIDE, ESP-IDF, nRF Connect SDK (hardware flash)

## 1. Validate the Platform

```bash
python scripts/validate.py
# Integration (11/11) + cluster link + phase sim + package
```

## 2. Run the Control Plane

```bash
python run.py
# Dashboard: http://localhost:8787
# WebSocket: ws://localhost:8787/ws
# Auth token: hermes-dev (or POST /api/auth/login)
```

## 3. Full Cluster Demo

```bash
# Terminal 1
python run.py

# Terminal 2 — 3 live node agents
python demo_cluster.py
```

## 4. C Libraries (Windows)

```bash
.\build-manual.ps1
# Output: build/manual/cluster-sim.exe, test_platform.exe
```

## 5. Build and Deploy an App

### UI (no code) — App Studio walkthrough

1. Start the control plane: `python run.py`
2. Open http://localhost:8787
3. Click **App Studio** in the sidebar
4. Pick a blueprint (e.g. **Smart Farm**, **Greenhouse**, **Air Quality**)
5. Fill in the form:
   - Zone name, thresholds, toggles
   - **Sensors** multi-select (from 37-sensor catalog)
   - **Target node** (0=STM32, 1=ESP, 2=nRF in default demo)
6. Click **Create & Deploy** — app installs, starts, and pushes `set_config` to the node
7. On the **Apps** tab, click **Configure** on a running app to change settings live
8. Watch sensor telemetry on the node card or in `/api/status`

Blueprint reference: [BLUEPRINTS.md](BLUEPRINTS.md)

### CLI (developers)

```bash
python sdk/cli/hermes build examples/smartfarm -t host
python sdk/cli/hermes package examples/smartfarm -t host
python sdk/cli/hermes deploy examples/smartfarm
python sdk/cli/hermes migrate smartfarm 0 2
```

Packaged binaries land in `examples/smartfarm/binaries/` with SHA256 + HMAC signature.
Flashed copies go to `data/flash/` via node agents.

## 6. MCU Emulator

Simulates a physical node with optional control-plane registration:

```bash
# Standalone — prints JSON telemetry to stdout
python tools/emulator/mcu_emulator.py --node 1

# Register with running control plane + run smartfarm app logic
python tools/emulator/mcu_emulator.py --with-server --node 1 --app examples/smartfarm

# Custom sensors
python tools/emulator/mcu_emulator.py --with-server --node 1 --sensors soil_moisture,temp_humidity,co2
```

Useful when you have no hardware or COM port is busy. Emulator appears on the dashboard like a real agent.

## 7. Cluster Simulator (Phases 1–6)

```bash
python tools/cluster-sim/sim.py
```

## 8. Flash Hardware

See [HARDWARE_BRINGUP.md](HARDWARE_BRINGUP.md).

```bash
# ESP32-S3 gateway
cd hardware/esp32-s3
idf.py build flash monitor

# Or Windows helper
.\scripts\flash_esp32.ps1
```

## 9. Release

```bash
.\scripts\split_repos.ps1          # preview repo split
.\scripts\split_repos.ps1 -Execute # copy to ../hermes-split/
.\scripts\tag_release.ps1          # git tag v0.1.0-alpha
```

## API Auth

All `/api/*` endpoints (except `/api/status` and `/api/auth/login`) require a token:

```bash
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/apps
```

Or set `HERMES_TOKEN` / `HERMES_SECRET` environment variables.

## Troubleshooting

| Issue | Fix |
|-------|-----|
| Integration tests fail (nodes offline) | Delete `data/cluster_state.json` or restart control plane |
| `gcc not found` (Windows) | Install MSYS2, add `C:\msys64\ucrt64\bin` to PATH |
| Deploy 401 | Pass `X-Hermes-Token: hermes-dev` header |
| ESP32 build | Install ESP-IDF v5.x, set `IDF_PATH` |
| PlatformIO SCons lock / dblite error | Run builds sequential: `python scripts/build_mcu_check.py -j 1` |
| ESP32 `intelhex` / `pins_arduino.h` missing | `pip install intelhex`; reinstall `framework-arduinoespressif32` |
| esp32c3 `nosys.specs` missing | Reinstall PlatformIO package `toolchain-riscv32-esp` |
| esp32c6 / esp32p4 / rtl8720 won't build | Use flash alias — see [BUILD_TARGETS.md](BUILD_TARGETS.md) |
| Serial bridge no telemetry | Check COM port, baud 115200, run `python run.py` first |
| App Studio create fails | Ensure blueprint name matches `blueprints/<id>.json` |
| Emulator not on dashboard | Add `--with-server`; control plane must be running |
| MCU matrix stale | `python scripts/summarize_mcu_build.py` → `build/mcu_build_summary.txt` |
