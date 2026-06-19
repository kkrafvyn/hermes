# HERMES SDK Guide

Developer SDK for building, packaging, and deploying HERMES applications outside App Studio.

| Path | Purpose |
|------|---------|
| `sdk/cli/hermes` | CLI entry point |
| `sdk/build.py` | Cross-compile per MCU target |
| `sdk/package.py` | SHA256 + HMAC packaging |
| `sdk/templates/app/` | New app scaffold |
| `sdk/macros.h` | `HERMES_TASK`, `HERMES_STATE`, `HERMES_MONITOR` |

---

## CLI commands

```bash
# Create app from template
python sdk/cli/hermes create myapp
python sdk/cli/hermes create myapp --output examples/myapp

# Build for target
python sdk/cli/hermes build examples/smartfarm -t host
python sdk/cli/hermes build examples/smartfarm -t esp8266
python sdk/cli/hermes build examples/smartfarm all

# Package (manifest + SHA256 + HMAC)
python sdk/cli/hermes package examples/smartfarm -t host

# Deploy to control plane (needs run.py)
python sdk/cli/hermes deploy examples/smartfarm
python sdk/cli/hermes deploy examples/smartfarm -n 1

# Migrate running app
python sdk/cli/hermes migrate smartfarm 0 2
```

CLI uses `X-Hermes-Token: hermes-dev` and `http://localhost:8787` by default.

---

## Application structure

```
myapp/
├── manifest.json       # Required metadata
├── src/app.c           # Application source
├── binaries/           # Packaged output (after package)
├── config/             # Runtime config
├── state/              # Checkpoints
└── resources/          # Static assets
```

### manifest.json

```json
{
  "name": "smartfarm",
  "version": "1.0.0",
  "targets": ["stm32", "esp32", "host"],
  "memory": "128KB",
  "priority": 5,
  "description": "Soil moisture and irrigation"
}
```

---

## C macros (`sdk/macros.h`)

### HERMES_TASK — register an RTOS task

```c
#include "hermes_macros.h"

HERMES_TASK(irrigation_loop, "irrigate", 2048, 5) {
    while (1) {
        check_soil_moisture();
        hermes_task_delay(30000);
    }
}
```

Expands to a static task function + registration metadata for the runtime scheduler.

### HERMES_STATE — migratable application state

```c
HERMES_STATE(float, soil_moisture);
HERMES_STATE(bool, pump_active);
```

State variables are included in checkpoints during migration (`hermes_checkpoint_create()`).

### HERMES_MONITOR — telemetry metric

```c
HERMES_MONITOR("smartfarm", "soil_moisture", soil_moisture);
HERMES_MONITOR("smartfarm", "pump_active", pump_active ? 1.0f : 0.0f);
```

Reports metrics to the runtime monitor layer → control plane heartbeat.

---

## Build targets (`sdk/build.py`)

| Target | Toolchain | Output |
|--------|-----------|--------|
| `host` | gcc | `build/host/myapp` executable |
| `emulator` | gcc | Sim binary |
| `esp8266` | xtensa-lx106-elf-gcc | `.bin` for flash |
| `esp32s3` | xtensa-esp32s3-elf-gcc | `.bin` |
| `esp32c3` | riscv32-esp-elf-gcc | `.bin` |
| `stm32h7` | arm-none-eabi-gcc | `.bin` |
| `stm32f4` | arm-none-eabi-gcc | `.bin` |
| `nrf52840` | arm-none-eabi-gcc | `.bin` |

```bash
python sdk/build.py examples/smartfarm host
python sdk/build.py examples/smartfarm all
```

If a cross-compiler is missing, build stages source for metadata-only packaging.

PlatformIO firmware builds use `scripts/build_all.py` separately — see [BUILD_TARGETS.md](BUILD_TARGETS.md).

---

## Packaging workflow

```bash
python sdk/package.py examples/smartfarm host
```

Produces in `examples/smartfarm/binaries/`:

- `smartfarm.bin` — binary artifact
- `manifest.json` — copied/merged manifest
- `smartfarm.sha256` — integrity digest
- `smartfarm.sig` — HMAC-SHA256 signature (`HERMES_SECRET`)

Deploy verifies signature before writing to `data/flash/<node>/`.

---

## Deploy flow

```
hermes package  →  hermes deploy  →  POST /api/apps/install
                                      →  command queue: install_app
                                      →  node agent executes
```

Or use App Studio (`POST /api/apps/create`) for blueprint-based deploy without CLI.

---

## Migration

```bash
python sdk/cli/hermes migrate smartfarm 0 2
# POST /api/apps/smartfarm/migrate {"dst_node": 2}
```

Runtime sequence: pause → checkpoint → transfer → restore → resume.  
See `runtime/include/hermes_migration.h` for C API.

---

## Emulator integration

Test without hardware:

```bash
python run.py
python tools/emulator/mcu_emulator.py --with-server --node 1 --app examples/smartfarm
python sdk/cli/hermes deploy examples/smartfarm -n 1
```

---

## Related

- [API.md](API.md) — REST endpoints used by CLI
- [SECURITY.md](SECURITY.md) — HMAC signing
- [QUICKSTART.md](QUICKSTART.md) — getting started
- [CONTRIBUTING.md](CONTRIBUTING.md) — add SDK targets
