# HERMES Documentation

## Primary references

| Document | Audience | Purpose |
|----------|----------|---------|
| **[USER_GUIDE.md](USER_GUIDE.md)** | Everyone | **How to use HERMES on any board — step-by-step** |
| **[HERMES_COMPLETE_DOCUMENTATION.md](HERMES_COMPLETE_DOCUMENTATION.md)** | Submission, reviewers | **Master doc — architecture, 60 MCUs, 37 sensors, 26 blueprints** |
| [PROJECT_DOCUMENTATION.md](PROJECT_DOCUMENTATION.md) | Engineers | Extended 18-section technical reference |
| [PROJECT_OVERVIEW.md](PROJECT_OVERVIEW.md) | Product, stakeholders | SRO — 12 capability areas, mission |

## Getting started

| Document | Purpose |
|----------|---------|
| [QUICKSTART.md](QUICKSTART.md) | Build, simulate, App Studio, emulator, troubleshooting |
| [HARDWARE_BRINGUP.md](HARDWARE_BRINGUP.md) | ESP8266/ESP32 flash, SPI/UART pins, INA219, full cluster |
| [BUILD_TARGETS.md](BUILD_TARGETS.md) | MCU matrix, workarounds, flash one-liners, CI output |

## Platform reference

| Document | Purpose |
|----------|---------|
| [MCU_SUPPORT.md](MCU_SUPPORT.md) | 60 MCU matrix — tiers, families, flash aliases |
| [SENSORS.md](SENSORS.md) | 37 sensors by bus, firmware, App Studio, telemetry JSON |
| [BLUEPRINTS.md](BLUEPRINTS.md) | 26 App Studio blueprints, schema, handlers |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Layer model, build phases, implementation status |
| [API.md](API.md) | C API + REST curl examples + WebSocket + SSE |

## Developer guides

| Document | Purpose |
|----------|---------|
| [SDK_GUIDE.md](SDK_GUIDE.md) | CLI, macros, build/package/deploy workflow |
| [SECURITY.md](SECURITY.md) | Auth, HMAC, identity, dev vs production |
| [CONTRIBUTING.md](CONTRIBUTING.md) | Add MCUs, sensors, blueprints, agents |

## Release

| Document | Purpose |
|----------|---------|
| [RELEASE_v0.1.md](RELEASE_v0.1.md) | Alpha notes, limitations, v0.2 roadmap |

## Recommended read order

**Submission deadline:** [HERMES_COMPLETE_DOCUMENTATION.md](HERMES_COMPLETE_DOCUMENTATION.md) first.

**New developer:** QUICKSTART → HARDWARE_BRINGUP → API → SDK_GUIDE

**Hardware engineer:** MCU_SUPPORT → BUILD_TARGETS → SENSORS → HARDWARE_BRINGUP

**App Studio user:** BLUEPRINTS → QUICKSTART (App Studio section)
