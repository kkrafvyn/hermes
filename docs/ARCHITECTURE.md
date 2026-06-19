# HERMES Architecture

## Vision

HERMES = open runtime platform that can host **many applications** across **multiple MCUs**.

Design principle: build as a **distributed OS**, not as a one-off migration tool.

## Layer Model

```
HERMES
│
├── Layer 1 → Hardware Cluster
├── Layer 2 → Device OS
├── Layer 3 → HERMES Runtime
├── Layer 4 → Application Platform
├── Layer 5 → Developer SDK
└── Layer 6 → Dashboard + Deployment
```

## Phase Map

### Phase 1 — Hardware Cluster (Weeks 1–2)

**Goal:** Heterogeneous hardware talking.

- Targets: STM32H7, ESP32-S3, nRF52840
- Transports: SPI, BLE, UART, DMA, power monitoring
- **Success:** 3-node cluster alive; send packets; measure latency

### Phase 2 — Device OS (Weeks 2–4)

**Goal:** Standardize MCU execution.

- FreeRTOS, HAL, task manager, memory manager, IPC
- API: `create_task()`, `pause_task()`, `resume_task()`, `destroy_task()`
- **Output:** Unified execution API

### Phase 3 — HERMES Runtime (Weeks 4–8)

**Goal:** Core distributed runtime.

- Monitor, scheduler, migration, checkpoint, resource manager
- Tracks: CPU, RAM, power, temp, deadlines
- API: `runtime.install()`, `runtime.start()`, `runtime.move()`, `runtime.stop()`
- **Output:** Tasks migrate safely

### Phase 4 — Application Platform (Weeks 8–10)

**Goal:** Support many applications.

```
app/
├── manifest.json
├── binaries/
├── config/
├── state/
└── resources/
```

Capabilities: install, start, pause, stop, update, rollback.

### Phase 5 — Multi-App Scheduler (Weeks 10–12)

**Goal:** Resource sharing across apps.

- Application isolation, quota manager, priority, resource policies
- Example quotas: App A 40%, App B 30%, App C 30%
- Rules govern CPU, memory, migration

### Phase 6 — State Migration (Weeks 12–14)

**Goal:** Move applications live.

```
Pause → Checkpoint → Transfer → Restore → Resume
```

Transfer: state, metadata, config.

**Never transfer:** raw stack, registers.

### Phase 7 — SDK (Weeks 14–16)

CLI: `hermes create|build|deploy|migrate`

C macros: `HERMES_TASK()`, `HERMES_STATE()`, `HERMES_MONITOR()`

### Phase 8 — Dashboard (Weeks 16–18)

Web control plane: node status, deploy, logs, metrics, migration.

### Phase 9 — Security (Weeks 18–20)

Identity, signed apps, policy engine, recovery.

Flow: install → verify → run.

### Phase 10 — Open Source Release (Weeks 20–24)

Repos: `hermes-runtime`, `hermes-sdk`, `hermes-dashboard`, `hermes-examples`

Release: **v0.1 Alpha**

## Migration Model

State migration is **application-level**, not process-level:

| Transferred | Not transferred |
|-------------|-----------------|
| Application state | Raw stack |
| Metadata | CPU registers |
| Config | |

This matches embedded constraints: no VM, no full process snapshot — only explicit, serializable state.

## Inter-Node Protocol

All cluster nodes speak a common frame format (`hardware/cluster/include/hermes_link.h`):

- Heartbeat + latency measurement (Phase 1)
- Resource reports (Phase 3)
- Checkpoint envelopes (Phase 6)
- App control messages (Phase 4)

## Implementation Status (v0.1 Alpha)

| Layer | Status | Location |
|-------|--------|----------|
| Hardware cluster protocol | Done | `hardware/cluster/` |
| Host C simulator | Done | `build/manual/cluster-sim.exe` |
| Device OS API | Done | `device-os/` + FreeRTOS on ESP32 |
| Runtime + migration | Done | `runtime/` + `hermes_node_runtime.c` on MCUs |
| Application platform | Done | `platform/` + binary deploy pipeline |
| Control plane API | Done | `tools/control-plane/` |
| Node agents | Done | `tools/node-agent/` + flash + checkpoints |
| Dashboard (live) | Done | REST + WebSocket + auth + analytics |
| ESP32-S3 gateway | Ready to flash | `hardware/esp32-s3/` (full runtime stack) |
| Cluster link hub | Done | `POST /api/link/send`, agent `--hub` |
| MCU emulator | Done | `tools/emulator/mcu_emulator.py` |
| STM32 / nRF HAL | Reference | `hardware/stm32h7/`, `hardware/nrf52840/` |
| Security | Done | SHA256 + HMAC + identity challenge |
| BLE transport | Done | `hermes_transport_ble.c` |
| OTA + binary flash | Done | `hermes_ota.c` + `data/flash/` |
| Checkpoint I/O | Done | `hermes_checkpoint_io.c` + link envelope |
| Quota enforcement | Done | runtime tick + node agents |
| Persistence | Done | `data/` state + checkpoints + analytics |
| Open-source split | Ready | `scripts/split_repos.ps1` + `tag_release.ps1` |
| CI | Done | `.github/workflows/ci.yml` |

### Development entry points

```bash
python demo.py                      # full stack
python demo_cluster.py              # with inter-node link hub
python scripts/validate.py            # all test suites
python tools/emulator/mcu_emulator.py --with-server
python tools/test_integration.py    # API tests
python tools/test_cluster_link.py   # cluster link hub
python tools/cluster-sim/sim.py     # phases 1-6
.\build-manual.ps1                  # C libraries
.\scripts\tag_release.ps1           # v0.1.0-alpha tag
```

Hardware: [HARDWARE_BRINGUP.md](HARDWARE_BRINGUP.md)

## Related Documents

| Document | Purpose |
|----------|---------|
| [PROJECT_DOCUMENTATION.md](PROJECT_DOCUMENTATION.md) | **Full project documentation** |
| [PROJECT_OVERVIEW.md](PROJECT_OVERVIEW.md) | Product definition — what HERMES does (SRO) |
| [API.md](API.md) | Public API reference |
| [QUICKSTART.md](QUICKSTART.md) | Build and run guide |
| [HARDWARE_BRINGUP.md](HARDWARE_BRINGUP.md) | Simulation to physical cluster guide |
| [../README.md](../README.md) | Repository entry point |
