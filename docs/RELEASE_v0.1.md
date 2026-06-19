# HERMES v0.1 Alpha — Release Notes

## Overview

First alpha release of the HERMES distributed embedded runtime platform. All software layers are implemented and validated in simulation; **60 MCU targets** are registered with **36 PIO builds passing** and **18 production flash aliases**.

## Components

| Component | Path | Status |
|-----------|------|--------|
| Hardware Cluster | `hardware/` | Alpha — 60 MCUs, unified firmware, stubs |
| Device OS | `device-os/` | Alpha — task/memory/IPC + FreeRTOS binding |
| Runtime | `runtime/` | Alpha — scheduler, monitor, migration, OTA |
| Application Platform | `platform/` | Alpha — manifest + lifecycle |
| SDK + CLI | `sdk/` | Alpha — create, build, package, deploy, migrate |
| Dashboard | `dashboard/` | Alpha — REST + WebSocket + SSE + App Studio |
| Security | `security/` | Alpha — SHA256 + HMAC + identity challenge |
| Control Plane | `tools/control-plane/` | Alpha — REST API, 26 blueprints, sensors |
| Sensors | `sensors/` | Alpha — 37 types on all MCUs |
| Examples | `examples/` | Alpha — smartfarm, telemetry, sensor-net |

## MCU build matrix (v0.1)

```
PIO passed: 36  |  workaround-covered: 18  |  coprocessor: 6  |  fail: 0
```

Report: `build/mcu_build_summary.txt` — regenerate with `python scripts/summarize_mcu_build.py`

## Planned split repos (Phase 10)

- `hermes-runtime` — hardware, device-os, runtime, platform, security
- `hermes-sdk` — sdk/
- `hermes-dashboard` — dashboard/
- `hermes-examples` — examples/

## Requirements

- CMake 3.16+, C11 compiler (or `build-manual.ps1` with MSYS2 GCC on Windows)
- Python 3.8+ for control plane, agents, and CLI
- PlatformIO for unified MCU firmware matrix
- Optional: ESP-IDF, STM32CubeIDE, nRF Connect SDK (native HAL integration)

## Validation

```bash
python scripts/validate.py   # integration + link + sim + package
python scripts/summarize_mcu_build.py
.\build-manual.ps1           # C libraries + cluster-sim + test_platform
```

## Known limitations (v0.1) and workarounds

| Limitation | Workaround |
|------------|------------|
| ESP32-C6 no Arduino on PIO 7.0.1 | Flash `esp32c3` alias — [BUILD_TARGETS.md](BUILD_TARGETS.md) |
| ESP32-P4 board support missing | Flash `esp32s3` alias |
| RTL8720 LibreTiny OTA header bug | Flash `rtl8710` alias |
| EFR32 Silabs PIO on Windows | `nrf52840` host + EFR32 UART coprocessor |
| nRF5340 needs Zephyr | Flash `nrf52840` alias |
| PIC16/18, STM8 no PIO | Port `hardware/stubs/generic_node.c` |
| Migration is app-level only | Explicit checkpoint structs, not raw stack |
| Sensors mostly simulated | Wire `read_hardware()` per board; A0 GPIO/ADC where available |
| Dashboard dev token | Set `HERMES_SECRET` / `HERMES_TOKEN` for production |
| Monorepo layout | `scripts/split_repos.ps1` + `tag_release.ps1` |

## Changelog format (v0.2+)

Future releases follow [Keep a Changelog](https://keepachangelog.com/):

```markdown
## [0.2.0] - YYYY-MM-DD
### Added
### Changed
### Fixed
### Deprecated
```

## v0.2 roadmap

| Area | Planned |
|------|---------|
| **Hardware** | Physical 3-node cluster on STM32H7 + ESP32-S3 + nRF52840 |
| **MCU** | Native ESP32-C6/P4 PIO when espressif32 platform updates |
| **Sensors** | Real I2C drivers (BME280, BH1750) behind `hermes_sensor.cpp` |
| **Migration** | Hardware-validated checkpoint/restore on live nodes |
| **Security** | Per-node key provisioning, rotate dev token |
| **Dashboard** | OTA firmware upload from UI |
| **SDK** | `hermes flash` command wrapping `flash_command` from registry |
| **Docs** | IEEE paper alignment notes, video walkthrough |

## Getting started

| Doc | Purpose |
|-----|---------|
| [HERMES_COMPLETE_DOCUMENTATION.md](HERMES_COMPLETE_DOCUMENTATION.md) | Submission master doc |
| [QUICKSTART.md](QUICKSTART.md) | Build, simulate, deploy |
| [HARDWARE_BRINGUP.md](HARDWARE_BRINGUP.md) | Flash ESP8266/ESP32, SPI/UART cluster |
