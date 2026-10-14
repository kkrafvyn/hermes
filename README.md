# HERMES

**Experimental embedded runtime and control plane for applications across heterogeneous MCU nodes.**

HERMES combines C/C++ runtime components, Python device agents and control-plane services, a browser dashboard, and board-specific firmware configurations. It explores scheduling, isolation, checkpointing and migration across STM32, Espressif and Nordic targets.

## Project status and review path

This is an engineering prototype. A target appearing in the registry or a build configuration is not evidence of physical validation on that board. Host simulation, compilation, flashing, hardware behavior and deployment readiness must be checked separately. Alias or port entries are not interchangeable firmware for different MCUs.

- Review the [architecture](docs/ARCHITECTURE.md) and [public API](docs/API.md).
- Run the [quickstart](docs/QUICKSTART.md) before attempting hardware deployment.
- Inspect [C runtime code](runtime/src), [Python control-plane services](tools/control-plane) and [tests](tests).
- Use the [MCU support notes](docs/MCU_SUPPORT.md) and the actual target SDK; confirm the exact board before flashing.

The commands below are the repository's supplied workflows. A successful local run should be recorded with its revision, environment and test output.

**HERMES** — *Heterogeneous Embedded Runtime Migration & Execution System*

## Documentation

| Document | Description |
|----------|-------------|
| **[User Guide — How to Use on Any Board](docs/USER_GUIDE.md)** | **Start here — setup, serial bridge and App Studio** |
| **[Complete Project Documentation](docs/HERMES_COMPLETE_DOCUMENTATION.md)** | **Submission-ready — MCUs, sensors, build matrix, deployment** |
| [Documentation Index](docs/README.md) | All guides — gaps closed, full index |
| [Full Project Documentation](docs/PROJECT_DOCUMENTATION.md) | Extended technical reference |
| [Project Overview](docs/PROJECT_OVERVIEW.md) | Product definition and software requirements (SRO) |
| [Architecture](docs/ARCHITECTURE.md) | Layer model, build phases, and technical design |
| [API Reference](docs/API.md) | Public API for all layers |
| [Quickstart](docs/QUICKSTART.md) | Build, simulate, deploy |
| [Hardware Bring-Up](docs/HARDWARE_BRINGUP.md) | Flash ESP32, serial bridge, full cluster |
| [Build Targets](docs/BUILD_TARGETS.md) | All ESP / STM32 / nRF build matrix |
| [Sensors](docs/SENSORS.md) | 37 sensors on all 60 MCU targets |
| [MCU Support](docs/MCU_SUPPORT.md) | Target registry — tiers, workarounds and flash commands |
| [Blueprints](docs/BLUEPRINTS.md) | 26 App Studio templates |
| [Security](docs/SECURITY.md) | Auth, HMAC, identity model |
| [Contributing](docs/CONTRIBUTING.md) | Add MCUs, sensors, blueprints |
| [SDK Guide](docs/SDK_GUIDE.md) | CLI, macros, packaging |

## Build

### Python (recommended for development)

```bash
python demo.py                    # control plane + 3 node agents
python tools/cluster-sim/sim.py   # phases 1-6 simulator
python scripts/validate.py        # full validation suite (all tests)
python scripts/build_all.py --list   # all MCU firmware + app targets
python scripts/build_all.py --firmware --target esp8266 --upload-port COM4
```

### C libraries (manual GCC build on Windows)

```bash
.\build-manual.ps1              # builds cluster-sim + test_platform
.\build.ps1 -Sim -Test          # CMake (requires MinGW make in PATH)
```

```bash
cmake -B build -DHERMES_BUILD_SIM=ON -DHERMES_BUILD_TESTS=ON
cmake --build build
./build/tools/cluster-sim/cluster-sim   # Phase 1 cluster test
ctest --test-dir build                # Platform tests
```

## Architecture

```
Applications
      │
      ▼
App Manager          ← install / start / pause / stop / update / rollback
      │
      ▼
HERMES Runtime       ← monitor / schedule / checkpoint / migrate / quota
      │
      ▼
Device OS            ← FreeRTOS + HAL + tasks + memory + IPC
      │
      ▼
Hardware Cluster     ← SPI / BLE / UART / DMA / power monitoring
```

## Repository Layout

| Path | Layer | Phase |
|------|-------|-------|
| `hardware/` | Hardware Cluster | 1 |
| `device-os/` | Device OS | 2 |
| `runtime/` | HERMES Runtime | 3, 5, 6 |
| `platform/` | Application Platform | 4 |
| `sdk/` | Developer SDK | 7 |
| `dashboard/` | Control Plane UI | 8 |
| `security/` | Identity + Policy | 9 |
| `examples/` | Reference apps | 4+ |
| `docs/` | Architecture + API | 10 |

## Roadmap

| Month | Deliverable |
|-------|-------------|
| 1 | 3-node cluster alive — send packets, measure latency |
| 2 | Unified Device OS + core runtime |
| 3 | Multi-app platform + live state migration |
| 4 | SDK (CLI, templates, macros) |
| 5 | Web dashboard |
| 6 | v0.1 Alpha open-source release |

## Quick Start

```bash
# Start control plane (dashboard + API)
python run.py

# Full demo: control plane + 3 live node agents
python demo.py

# Cluster demo with inter-node link hub
python demo_cluster.py

# Run full cluster simulator (phases 1-6)
python tools/cluster-sim/sim.py

# Developer workflow (CLI — optional; use App Studio in dashboard for no-code setup)
python sdk/cli/hermes create myapp
python sdk/cli/hermes build examples/smartfarm -t emulator
python sdk/cli/hermes deploy examples/smartfarm   # needs run.py
python tools/emulator/mcu_emulator.py --with-server

# UI workflow (no code)
# python run.py → http://localhost:8787 → App Studio → pick blueprint → Create & Deploy

# Release
.\scripts\split_repos.ps1          # dry-run repo split
.\scripts\tag_release.ps1          # tag v0.1.0-alpha
```

## Targets

- ESP8266 (serial bridge node — `hardware/esp8266/`)
- ESP32-S3 (gateway — `hardware/esp32-s3/`)
- STM32H7
- nRF52840

## License

MIT — see [LICENSE](LICENSE).
