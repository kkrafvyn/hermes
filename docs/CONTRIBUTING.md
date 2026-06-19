# Contributing to HERMES

Thank you for extending HERMES. This guide covers the most common contributions: MCUs, sensors, blueprints, and node agents.

## Prerequisites

```bash
python scripts/validate.py
python run.py   # control plane for API/blueprint tests
```

---

## Adding an MCU target

### 1. Registry entry

Add to `hardware/mcu_registry.json` → `targets`:

```json
"my_mcu": {
  "name": "My MCU Board",
  "family": "vendor",
  "tier": "supported",
  "tool": "platformio",
  "path": "hardware",
  "env": "my_mcu",
  "boards": ["My Board v1"]
}
```

Tiers: `supported` | `alias` | `port` | `coprocessor`

If PIO cannot build, add a `workarounds` entry with `use_target`, `env`, `kind` (`flash_alias` | `stub_port` | `coprocessor`).

### 2. PlatformIO environment

Add to `hardware/platformio_mcu.ini`:

```ini
[env:my_mcu]
platform = ...
board = ...
framework = arduino
build_flags =
  -DHERMES_MY_MCU=1
  -DHERMES_NODE_ID=1
lib_ignore = ...
```

### 3. Family macro

Add branch in `hardware/src/hermes_mcu.h`:

```c
#if defined(HERMES_MY_MCU)
  #define HERMES_FAMILY "vendor"
#endif
```

### 4. Build & verify

```bash
pio run -e my_mcu -d hardware
python scripts/build_mcu_check.py --target my_mcu
python scripts/summarize_mcu_build.py
```

### 5. Documentation

Update `docs/MCU_SUPPORT.md` and run `GET /api/mcus` to confirm `flash_command` appears.

For SDK-port-only targets, add `hardware/stubs/my_mcu/README.md` and port `generic_node.c`.

---

## Adding a sensor

1. **`sensors/catalog.json`** — add `{id, name, bus, unit, description}`
2. **`hardware/src/hermes_sensor.cpp`** — `simulate_value()` + `sensor_unit()`
3. **`hardware/stubs/hermes_stub_sensor.c`** — mirror simulation for port targets
4. **`read_hardware()`** (optional) — real driver behind `#if defined(A0)` or I2C bus
5. **Blueprint defaults** — add to relevant `blueprints/*.json` `sensor_list` defaults
6. **Test** — `curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/sensors`

Sensor IDs are automatically available on all 60 MCUs via `sensor_mcu_ids: "__all__"`.

---

## Adding a blueprint

1. Create `blueprints/<id>.json` — see [BLUEPRINTS.md](BLUEPRINTS.md) schema
2. Add `_init_<id>` and `_tick_<id>` in `tools/control-plane/blueprint_handlers.py`
3. Register in `INIT` and `TICK` dicts
4. Run `python tools/test_blueprints.py`
5. Verify in dashboard App Studio

Use `type: sensor_list` for sensor multi-select; values are pushed to node `set_config`.

---

## Adding a node agent

Python agents live in `tools/node-agent/` (and `demo_cluster.py` for examples).

Minimum loop:

```python
# Register
POST /api/nodes/register  {"node_id": 1, "name": "...", "agent": true}

# Heartbeat loop
while True:
    POST /api/nodes/heartbeat  {"node_id": 1, "telemetry": {...}}
    # Execute commands from response
    time.sleep(2)
```

For serial firmware, use `tools/serial-bridge/bridge.py` to forward UART JSON to the API.

---

## Code style

- Match existing naming and file layout in the directory you edit
- Minimize scope — one feature per change
- C firmware: no `Serial.printf` on AVR — use `hermes_print.h`
- Python: type hints where the module already uses them

## Tests

```bash
python scripts/validate.py
python tools/test_integration.py
python tools/test_blueprints.py
python tools/test_cluster_link.py
```

## Documentation

Update the relevant doc when you add user-facing features:

| Change | Doc |
|--------|-----|
| MCU | `MCU_SUPPORT.md`, `BUILD_TARGETS.md` |
| Sensor | `SENSORS.md` |
| Blueprint | `BLUEPRINTS.md` |
| API endpoint | `API.md` |
| Security | `SECURITY.md` |

## Pull requests

1. Describe what tier/build status your MCU change achieves
2. Include `summarize_mcu_build.py` output if you touched firmware matrix
3. Run `validate.py` before submitting

## License

Contributions are MIT-licensed — see [LICENSE](../LICENSE).
