# HERMES — Full Project Documentation

> **Version:** 0.1.0-alpha  
> **HERMES** — *Heterogeneous Embedded Runtime Migration & Execution System*

This is the complete reference for the HERMES distributed embedded runtime platform. For shorter guides, see the [documentation index](README.md).

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Vision and Mission](#2-vision-and-mission)
3. [Core Concepts](#3-core-concepts)
4. [System Architecture](#4-system-architecture)
5. [Repository Structure](#5-repository-structure)
6. [Build Phases and Roadmap](#6-build-phases-and-roadmap)
7. [Inter-Node Protocol](#7-inter-node-protocol)
8. [Layer-by-Layer Technical Reference](#8-layer-by-layer-technical-reference)
9. [Control Plane and Dashboard](#9-control-plane-and-dashboard)
10. [Application Model](#10-application-model)
11. [Migration and State Management](#11-migration-and-state-management)
12. [Developer Workflow](#12-developer-workflow)
13. [Hardware Targets and Bring-Up](#13-hardware-targets-and-bring-up)
14. [Testing and CI](#14-testing-and-ci)
15. [Implementation Status](#15-implementation-status)
16. [Example Deployment](#16-example-deployment)
17. [Known Limitations](#17-known-limitations)
18. [Related Documents](#18-related-documents)

---

## 1. Introduction

HERMES is an **open runtime platform** for hosting **many applications** across **heterogeneous microcontroller clusters**. It is designed as a **tiny distributed operating system for microcontrollers** — not a one-off task migration tool.

### What problem does HERMES solve?

Traditional embedded systems follow:

```
1 MCU → 1 Firmware → 1 Purpose
```

HERMES enables:

```
Multiple Applications
        ↓
  Shared Runtime
        ↓
   Multiple MCUs
        ↓
 Dynamic Execution
```

### Supported hardware (v0.1)

| Node ID | MCU | Role |
|---------|-----|------|
| 0 | STM32H7 | High-performance compute (AI inference) |
| 1 | ESP32-S3 | Gateway, dashboard, telemetry |
| 2 | nRF52840 | Sensor network, low-power monitoring |

### Project type

Distributed Embedded Runtime Platform

---

## 2. Vision and Mission

### Mission

Create a software platform that allows **multiple applications to run, move, scale, and recover across multiple heterogeneous microcontrollers as one unified embedded system.**

### Design principle

Build as a **distributed OS**, not as a migration utility. Migration is one capability among many: scheduling, isolation, quotas, monitoring, and application lifecycle management.

### Product capabilities (SRO summary)

| # | Capability | Description |
|---|------------|-------------|
| 1 | Application Management | Install, start, pause, stop, update, rollback apps |
| 2 | Runtime Execution | CPU/RAM allocation, priority, concurrent execution |
| 3 | Distributed Scheduling | Node selection, load balancing, deadline scheduling |
| 4 | Task Migration | Pause, checkpoint, transfer, restore, resume |
| 5 | Resource Management | CPU/memory/storage quotas per app |
| 6 | Monitoring & Telemetry | Per-node and per-app metrics |
| 7 | Cluster Management | Registration, health, leader election, recovery |
| 8 | State Management | Checkpoints, serialization, versioning |
| 9 | Communication Platform | SPI, BLE, UART inter-node messaging |
| 10 | Security Framework | Identity, signed apps, policy engine |
| 11 | Developer Platform | SDK, CLI, templates, packaging |
| 12 | Dashboard & Operations | Web UI for deploy, monitor, migrate |

---

## 3. Core Concepts

### Node

A physical or simulated MCU in the cluster. Each node has an ID (0–2 in v0.1), a type, and reports telemetry.

### Application

A packaged unit with a `manifest.json`, source/binary, config, state, and resources. Multiple apps run concurrently across the cluster.

### Control plane

The Python HTTP server (`tools/control-plane/`) that unifies cluster operations: node health, app lifecycle, migration, command queues, and inter-node frame routing.

### Node agent

Software running on each node (Python for development, C/firmware for production) that registers with the control plane, sends heartbeats, executes commands, and optionally participates in the cluster link hub.

### Gateway

The ESP32-S3 node acts as the cluster gateway, routing frames between STM32 (SPI) and nRF (UART) via `hermes_router.c`.

### Checkpoint

Application-level serialized state. HERMES never transfers raw stack or CPU registers — only explicit, serializable state, metadata, and config.

---

## 4. System Architecture

### Layer model

```
Applications          (examples/, sdk/templates/)
      │
      ▼
App Manager           (platform/)     install / start / pause / stop / rollback
      │
      ▼
HERMES Runtime        (runtime/)      monitor / schedule / checkpoint / migrate / quota
      │
      ▼
Device OS             (device-os/)    tasks / memory / IPC
      │
      ▼
Hardware Cluster      (hardware/)     SPI / BLE / UART / power monitoring
```

### Full stack diagram (development)

```
┌─────────────────────────────────────────────────────────┐
│  Browser Dashboard          http://localhost:8787/      │
└────────────────────────────┬────────────────────────────┘
                             │ HTTP REST
┌────────────────────────────▼────────────────────────────┐
│  Control Plane (Python)                                 │
│  - Cluster manager    - App lifecycle                   │
│  - Command queues     - Link hub (frame routing)        │
│  - Leader election    - Migration orchestration         │
└────────────┬───────────────────────┬────────────────────┘
             │ heartbeat/API         │ serial bridge (optional)
┌────────────▼──────────┐   ┌────────▼──────────────────┐
│  Node Agents (0,1,2)  │   │  ESP32-S3 Gateway (C)     │
│  Python or firmware   │   │  WiFi HTTP or UART JSON   │
└────────────┬──────────┘   └────────┬──────────────────┘
             │                       │ SPI / UART
             │              ┌────────┴────────┐
             │              ▼                 ▼
             │         STM32H7 (0)      nRF52840 (2)
             └──── cluster link hub (software) or hermes_router (hardware)
```

### Data flows

**Deploy flow:**
```
CLI/Dashboard → POST /api/apps/install → pick node → queue install_app command → agent executes
```

**Migration flow:**
```
CLI/Dashboard → POST /api/apps/{name}/migrate → pause → checkpoint → transfer → restore → resume
```

**Telemetry flow:**
```
Agent/Firmware → POST /api/nodes/heartbeat → dashboard updates → leader re-elected
```

---

## 5. Repository Structure

```
hermes/
├── hardware/                 # Layer 1 — cluster protocol + MCU targets
│   ├── cluster/              # hermes_link, router, cluster_mgr, transport
│   ├── stm32h7/              # STM32 node + SPI HAL
│   ├── esp32-s3/             # ESP-IDF gateway project
│   └── nrf52840/             # nRF node + UART HAL
├── device-os/                # Layer 2 — task, memory, IPC
├── runtime/                  # Layer 3 — monitor, scheduler, migration, quota
├── platform/                 # Layer 4 — app manager, manifest parser
├── security/                 # Layer 9 — identity, policy (alpha stubs)
├── sdk/                      # Layer 5 — CLI, templates, C macros
├── dashboard/                # Layer 6 — web control plane UI
├── examples/                 # Reference applications (smartfarm, testapp)
├── tools/
│   ├── control-plane/        # Python API server + cluster manager
│   ├── node-agent/           # Per-node agent + link protocol + runtime sim
│   ├── serial-bridge/        # ESP32 UART → control plane
│   ├── cluster-sim/          # Phase 1-6 Python + C simulators
│   ├── test_integration.py   # API integration tests
│   └── test_cluster_link.py  # Inter-node link hub tests
├── tests/                    # C platform tests
├── docs/                     # Documentation
├── demo.py                   # Control plane + 3 agents
├── demo_cluster.py           # Demo with link hub enabled
├── run.py                    # Start control plane only
├── build-manual.ps1          # Windows GCC build script
├── build.ps1                 # CMake build script
├── CMakeLists.txt            # Root CMake project
├── requirements.txt          # Python deps (pyserial)
└── VERSION                   # 0.1.0-alpha
```

---

## 6. Build Phases and Roadmap

| Phase | Weeks | Goal | Key deliverables |
|-------|-------|------|------------------|
| 1 | 1–2 | Hardware cluster talking | SPI/BLE/UART, 3-node alive, latency |
| 2 | 2–4 | Device OS | `create_task`, memory, IPC |
| 3 | 4–8 | Core runtime | install/start/move/stop, monitor |
| 4 | 8–10 | Application platform | manifest, lifecycle, multi-app |
| 5 | 10–12 | Multi-app scheduler | quotas 40/30/30, isolation |
| 6 | 12–14 | State migration | pause/checkpoint/transfer/restore |
| 7 | 14–16 | SDK | CLI, templates, C macros |
| 8 | 16–18 | Dashboard | node status, deploy, migrate |
| 9 | 18–20 | Security | identity, signed apps, policy |
| 10 | 20–24 | Open source | split repos, v0.1 Alpha release |

### Monthly milestones

| Month | Deliverable |
|-------|-------------|
| 1 | 3-node cluster alive |
| 2 | Unified Device OS + core runtime |
| 3 | Multi-app platform + live migration |
| 4 | SDK (CLI, templates, macros) |
| 5 | Web dashboard |
| 6 | v0.1 Alpha open-source release |

---

## 7. Inter-Node Protocol

All nodes share a common frame format defined in `hardware/cluster/include/hermes_link.h`.

### Frame structure

```
┌─────────────── header (10 bytes) ───────────────┐
│ magic(2) │ ver(1) │ type(1) │ src(1) │ dst(1) │
│ seq(2) │ payload_len(2) │
├──────────────── payload (0–512 bytes) ──────────┤
└──────────────── crc32 (4 bytes) ────────────────┘
```

- **Magic:** `0x484D` ("HM")
- **Version:** 1
- **CRC32:** IEEE polynomial over header + payload

### Message types

| Type | Value | Purpose |
|------|-------|---------|
| `HERMES_MSG_HEARTBEAT` | 0x01 | Node liveness |
| `HERMES_MSG_PING` | 0x02 | Latency measurement |
| `HERMES_MSG_PONG` | 0x03 | Ping response |
| `HERMES_MSG_RESOURCE` | 0x10 | Resource report |
| `HERMES_MSG_APP_CONTROL` | 0x20 | App commands |
| `HERMES_MSG_CHECKPOINT` | 0x30 | State transfer envelope |
| `HERMES_MSG_ACK` | 0xFE | Acknowledgment |
| `HERMES_MSG_NACK` | 0xFF | Negative acknowledgment |

### Transports

| Transport | Enum | Typical link |
|-----------|------|--------------|
| SPI | 0 | STM32 ↔ ESP32 |
| BLE | 1 | nRF ↔ ESP32 |
| UART | 2 | nRF ↔ ESP32, debug |

### Gateway routing

`hermes_router.c` on ESP32 forwards frames:
- SPI (STM32) → UART (nRF) when `dst == 2`
- UART (nRF) → SPI (STM32) when `dst == 0`

### Software link hub

During development, agents with `--hub` route frames through the control plane (`POST /api/link/send`) instead of physical links.

---

## 8. Layer-by-Layer Technical Reference

### Layer 1 — Hardware Cluster (`hardware/cluster/`)

| Module | File | Purpose |
|--------|------|---------|
| Link protocol | `hermes_link.c` | Frame send/recv, CRC, ping/pong |
| Cluster manager | `hermes_cluster_mgr.c` | Registration, health, leader |
| Router | `hermes_router.c` | Gateway frame forwarding |
| Power monitor | `hermes_power.c` | Voltage/current/power sampling |
| Host transport | `hermes_transport_host.c` | PC simulation bus |

**Key C APIs:**

```c
hermes_link_init(link, transport, node_id, node_type);
hermes_link_send(link, type, dst_node, payload, len);
hermes_link_recv(link, frame, timeout_ms);
hermes_link_send_heartbeat(link, dst_node);
hermes_link_ping(link, dst_node, &latency);
hermes_cluster_register(mgr, node_id, type);
hermes_router_poll(router, timeout_ms);
```

### Layer 2 — Device OS (`device-os/`)

Unified execution API over FreeRTOS (or host):

```c
hermes_create_task(mgr, name, entry, arg, stack_size, priority, &id);
hermes_pause_task(mgr, task_id);
hermes_resume_task(mgr, task_id);
hermes_destroy_task(mgr, task_id);
hermes_malloc(mgr, size);
hermes_free(mgr, ptr);
hermes_ipc_send(ipc, channel_id, msg, timeout);
hermes_ipc_recv(ipc, channel_id, msg, timeout);
```

### Layer 3 — HERMES Runtime (`runtime/`)

| Module | Purpose |
|--------|---------|
| `hermes_runtime.c` | Per-node app install/start/pause/stop/move |
| `hermes_monitor.c` | CPU/RAM/power/temp sampling |
| `hermes_scheduler.c` | Distributed placement by load |
| `hermes_quota.c` | Per-app CPU/memory quotas |
| `hermes_checkpoint.c` | State snapshot create/restore |
| `hermes_migration.c` | Pause→checkpoint→transfer→restore state machine |

```c
hermes_runtime_install(rt, app_id, binary, size);
hermes_runtime_start(rt, app_id);
hermes_runtime_move(rt, app_id, dst_node);
hermes_scheduler_place(sched, node_load, count);
hermes_migration_step(mig, serialize, restore, ctx);
hermes_quota_set(qm, app_id, cpu_percent, memory_kb);
```

### Layer 4 — Application Platform (`platform/`)

```c
hermes_app_install(mgr, &manifest);
hermes_app_start(mgr, name);
hermes_app_pause(mgr, name);
hermes_app_stop(mgr, name);
hermes_app_update(mgr, name, &new_manifest);
hermes_app_rollback(mgr, name);
hermes_app_parse_manifest(json_string, &out);
```

### Layer 9 — Security (`security/`)

```c
hermes_identity_init(id, node_id, node_type);
hermes_policy_check(engine, subject, HERMES_PERM_INSTALL);
hermes_verify_app(signed_app, payload);
```

Flow: **Install → Verify → Run**

---

## 9. Control Plane and Dashboard

### Starting the control plane

```bash
python run.py
# Dashboard + API: http://localhost:8787/
```

### REST API reference

#### Cluster status

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/status` | Full cluster state (nodes, apps, metrics, logs, link) |
| GET | `/api/nodes` | Node list |
| GET | `/api/apps` | Application list |
| GET | `/api/logs` | Recent log entries |
| GET | `/api/link/status` | Inter-node frame routing statistics |

#### Node management

| Method | Path | Body | Description |
|--------|------|------|-------------|
| POST | `/api/nodes/register` | `{"node_id":1,"name":"ESP32-S3","node_type":"esp32","agent":true}` | Register node |
| POST | `/api/nodes/heartbeat` | `{"node_id":1,"telemetry":{"cpu":28,"ram_kb":180,"temp_c":42,"power_mw":380}}` | Heartbeat + receive commands/frames |
| POST | `/api/nodes/ack` | `{"node_id":1,"cmd_id":5}` | Acknowledge command |
| GET | `/api/nodes/{id}/commands` | — | Pending commands for node |

#### Application lifecycle

| Method | Path | Description |
|--------|------|-------------|
| POST | `/api/apps/install` | `{"manifest_path":"examples/smartfarm/manifest.json"}` |
| POST | `/api/apps/{name}/start` | Start app |
| POST | `/api/apps/{name}/pause` | Pause or resume app |
| POST | `/api/apps/{name}/stop` | Stop app |
| POST | `/api/apps/{name}/migrate` | `{"dst_node":2}` — live migration |

#### Cluster link hub

| Method | Path | Body | Description |
|--------|------|------|-------------|
| POST | `/api/link/send` | `{"frame":{"src":0,"dst":1,"type":2,"seq":42}}` | Route inter-node frame |
| GET | `/api/link/status` | — | `frames_routed`, `pings`, pending queues |

### Control plane features

- **Leader election** — lowest CPU online node
- **Health checks** — node offline after 30s without heartbeat
- **Work redistribution** — running apps moved when node fails
- **Command queues** — per-node: `install_app`, `run_app`, `pause_app`, `stop_app`, `checkpoint_app`, `restore_app`
- **Quota tracking** — default 40/30/30 CPU split across apps

### Dashboard

Static web UI in `dashboard/` served by the control plane. Polls `/api/status` every 2 seconds.

Features:
- Node cards (CPU, RAM, temp, power, online status)
- Application table (deploy, pause, migrate, stop)
- Cluster metrics (load, power, temp, app count)
- Live log stream

---

## 10. Application Model

### Package layout

```
myapp/
├── manifest.json       # Required — app metadata
├── src/app.c           # Application source
├── binaries/           # Built artifacts
├── config/             # Runtime configuration
├── state/              # Checkpoint storage
└── resources/          # Static assets
```

### Manifest format

```json
{
  "name": "smartfarm",
  "version": "1.0.0",
  "targets": ["stm32", "esp32"],
  "memory": "128KB",
  "priority": 5,
  "description": "Soil moisture, irrigation control, telemetry"
}
```

### Lifecycle states

| State | Description |
|-------|-------------|
| `installed` | Deployed but not running |
| `running` | Actively executing |
| `paused` | Suspended, state retained |
| `stopped` | Halted |
| `updating` | Version change in progress |

---

## 11. Migration and State Management

### Migration model

HERMES uses **application-level** migration, not process-level snapshots.

| Transferred | Not transferred |
|-------------|-----------------|
| Application state (explicit structs) | Raw stack |
| Metadata (app ID, version) | CPU registers |
| Configuration | Thread contexts |

### Migration state machine

```
PAUSING → CHECKPOINT → TRANSFER → RESTORE → RESUME → DONE
                                              ↓ (on failure)
                                           FAILED → ROLLBACK
```

### C migration API

```c
hermes_migration_begin(&mig, app_id, src_node, dst_node, link);
while (mig.state != HERMES_MIG_DONE)
    hermes_migration_step(&mig, serialize_fn, restore_fn, app_ctx);
hermes_migration_rollback(&mig);  // on failure
```

### Checkpoint format

```c
typedef struct {
    uint32_t version;
    char app_id[32];
    uint32_t state_size;
    uint32_t config_size;
    uint8_t data[4096];
} hermes_checkpoint_t;
```

---

## 12. Developer Workflow

### Prerequisites

- Python 3.8+
- Optional: GCC (MSYS2 MinGW on Windows), CMake 3.16+
- Optional: ESP-IDF, STM32CubeIDE, nRF Connect SDK (hardware)

### Quick start commands

```bash
# Full software stack
python demo.py

# Cluster with inter-node link hub
python demo_cluster.py

# Control plane only
python run.py

# Phase 1-6 simulator
python tools/cluster-sim/sim.py

# C libraries (Windows)
.\build-manual.ps1
```

### CLI reference

```bash
python sdk/cli/hermes create myapp
python sdk/cli/hermes build examples/smartfarm
python sdk/cli/hermes deploy examples/smartfarm      # requires run.py
python sdk/cli/hermes migrate smartfarm 0 2          # requires run.py
```

### SDK C macros

```c
#include "hermes_sdk.h"

HERMES_TASK(sensor_loop, "sensor", 4096, 5)
HERMES_STATE(smartfarm_state_t, g_state)
HERMES_MONITOR("smartfarm", "soil_moisture", value);
```

### Node agent

```bash
python tools/node-agent/agent.py 0          # STM32H7
python tools/node-agent/agent.py 1 --hub     # ESP32 with link hub
python tools/node-agent/agent.py 2          # nRF52840
```

Options: `--interval`, `--api`, `--hub`

---

## 13. Hardware Targets and Bring-Up

### Stage 1 — Simulation (no hardware)

```bash
python demo.py
python tools/test_integration.py
python tools/test_cluster_link.py
python tools/cluster-sim/sim.py
.\build-manual.ps1
```

### Stage 2 — ESP32 via serial bridge (recommended first hardware step)

1. Flash ESP32 with `hardware/esp32-s3/` (serial bridge mode is default)
2. Start control plane: `python run.py`
3. Bridge UART: `python tools/serial-bridge/bridge.py COM3 --register --node-id 1`

### Stage 3 — ESP32 via WiFi

Configure in `idf.py menuconfig`:
- WiFi SSID/password
- `HERMES_API_URL` = `http://<pc-lan-ip>:8787`
- Disable serial bridge mode

### Stage 4 — STM32 + nRF

| Node | HAL file | Transport to ESP32 |
|------|----------|-------------------|
| STM32H7 | `hal_spi.c` | SPI master |
| nRF52840 | `hal_uart.c` | UART |

### Stage 5 — Full hardware cluster

```
PC (control plane)
  │ WiFi or UART
  ▼
ESP32-S3 (gateway) ── hermes_router.c
  ├── SPI  → STM32H7
  └── UART → nRF52840
```

See [HARDWARE_BRINGUP.md](HARDWARE_BRINGUP.md) for step-by-step instructions.

---

## 14. Testing and CI

### Test suites

| Test | Command | Coverage |
|------|---------|----------|
| Integration | `python tools/test_integration.py` | API: status, heartbeat, install, migrate, commands |
| Cluster link | `python tools/test_cluster_link.py` | Inter-node frame routing hub |
| Phase 1-6 sim | `python tools/cluster-sim/sim.py` | End-to-end phase validation |
| C platform | `.\build-manual.ps1` | cluster-sim + test_platform |
| CMake tests | `ctest --test-dir build` | C unit tests |

### CI pipeline

`.github/workflows/ci.yml` runs on push/PR:
- Integration tests
- Cluster link tests
- Python simulator
- GCC C build + cluster-sim + test_platform

### Verified results (local)

- Integration: **11/11 passed**
- Cluster link: **PASS** (3 frames routed, 1 ping)
- C build: **cluster-sim + test_platform exit 0**

---

## 15. Implementation Status

| Component | Status | Location |
|-----------|--------|----------|
| Hardware cluster protocol | **Done** | `hardware/cluster/` |
| Host C simulator | **Done** | `build/manual/cluster-sim.exe` |
| Device OS API | **Done** | `device-os/` |
| Runtime + migration | **Done** | `runtime/` |
| Application platform | **Done** | `platform/` |
| Control plane API | **Done** | `tools/control-plane/` |
| Node agents | **Done** | `tools/node-agent/` |
| Dashboard (live API) | **Done** | `dashboard/` |
| Cluster link hub | **Done** | `/api/link/send` |
| Serial bridge | **Done** | `tools/serial-bridge/` |
| ESP32-S3 gateway | **Ready to flash** | `hardware/esp32-s3/` |
| STM32 / nRF HAL | **Reference** | `hardware/stm32h7/`, `hardware/nrf52840/` |
| Security | **Done** | SHA256 + policy + verify on deploy |
| BLE transport | **Done** | `hermes_transport_ble.c` |
| OTA updates | **Done** | `hermes_ota.c` |
| Persistence | **Done** | `data/cluster_state.json`, checkpoints |
| SSE dashboard | **Done** | `/api/events` |
| CI | **Done** | `.github/workflows/ci.yml` |

---

## 16. Example Deployment

### Example cluster layout

```
HERMES Cluster

STM32 (node 0)
├── smartfarm (AI inference)

ESP32 (node 1)
├── telemetry
├── dashboard gateway

nRF (node 2)
├── sensor-net
```

### Deploy smartfarm

```bash
# Terminal 1
python run.py

# Terminal 2
python sdk/cli/hermes deploy examples/smartfarm
python sdk/cli/hermes migrate smartfarm 0 2
```

### Default apps (pre-loaded in control plane)

| App | Version | Node | State |
|-----|---------|------|-------|
| smartfarm | 1.0.0 | 0 | running |
| telemetry | 0.2.1 | 1 | running |
| sensor-net | 0.1.0 | 2 | paused |

---

## 17. Known Limitations

v0.1 Alpha limitations:

- **Migration** — application-level checkpoints only; no raw stack/register transfer
- **MCU flash** — deploy packages metadata; real binary load requires target toolchains
- **STM32/nRF** — HAL reference code; board-specific wiring and drivers required
- **BLE mesh** — transport layer implemented; full BLE stack not integrated on hardware
- **CMake on Windows** — use `build-manual.ps1` if MinGW make is not in PATH
- **Dashboard** — no authentication; minimal deploy UI

### Planned v0.2 improvements

- Full HAL integration and on-device app loading for STM32 and nRF
- Dashboard auth and richer deploy workflow
- BLE mesh on nRF52840 hardware
- Split into separate GitHub repos (runtime, sdk, dashboard, examples)

---

## 18. Related Documents

| Document | Purpose |
|----------|---------|
| [README.md](../README.md) | Repository entry point |
| [PROJECT_OVERVIEW.md](PROJECT_OVERVIEW.md) | Product definition (SRO) |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Layer model and phase map |
| [API.md](API.md) | Concise C API reference |
| [QUICKSTART.md](QUICKSTART.md) | Build and run guide |
| [HARDWARE_BRINGUP.md](HARDWARE_BRINGUP.md) | Simulation to physical cluster |
| [RELEASE_v0.1.md](RELEASE_v0.1.md) | Alpha release notes |

---

*HERMES v0.1.0-alpha — Heterogeneous Embedded Runtime Migration & Execution System*
