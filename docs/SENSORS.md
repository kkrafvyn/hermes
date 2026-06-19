# HERMES Sensor Support

Unified sensor catalog for **all 60 MCU targets** in `hardware/mcu_registry.json`.

| Item | Value |
|------|-------|
| Sensor types | **37** |
| MCU coverage | **60** (registry `sensor_mcu_ids: "__all__"`) |
| Catalog file | `sensors/catalog.json` |
| API | `GET /api/sensors` |
| Firmware (PIO) | `hardware/src/hermes_sensor.cpp` |
| Firmware (port) | `hardware/stubs/hermes_stub_sensor.c` |

---

## Catalog by bus

| Bus | Sensor IDs |
|-----|------------|
| **I2C** | temp_humidity, co2, voc, pm25, light_par, lux, vibration, accelerometer, pressure, shock_tilt |
| **ADC** | soil_moisture, soil_ec, leaf_wetness, ammonia, ph, orp, smoke, co_gas, gas_mq, current_clamp, voltage, weight_loadcell |
| **GPIO** | motion_pir, door_contact, water_leak, ultrasonic |
| **Pulse** | rain_gauge, wind_speed, flow_meter |
| **UART** | soil_npk, gps, modbus_meter |
| **BLE** | ble_rssi |
| **SPI** | rfid |
| **1-Wire** | ds18b20 |

### API

```bash
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/sensors
```

Response includes `mcu_targets` (all 60 IDs) on every sensor entry.

---

## App Studio flow

```
Dashboard → App Studio → blueprint form → Sensors multi-select
       ↓
POST /api/apps/create  { config: { sensors: [...] } }
       ↓
data/apps/<name>/config.json
       ↓
Command queue → set_config → node firmware
       ↓
hermes_sensor_configure() → poll → telemetry JSON
```

1. User selects sensors in blueprint **Sensors** field (`type: sensor_list`)
2. Config saved to `data/apps/{name}/config.json`
3. Control plane queues `set_config` for target node
4. `main.cpp` parses JSON sensor array
5. `hermes_sensor_poll()` runs each heartbeat interval
6. Telemetry includes `"sensors":[{id,value,unit},…]`

---

## Firmware integration

### PIO nodes (`hardware/src/`)

| File | Role |
|------|------|
| `hermes_sensor.h` | C API: configure, poll, JSON emit |
| `hermes_sensor.cpp` | `simulate_value()` for all 37 IDs; `read_hardware()` for GPIO/ADC on `A0` when defined |
| `main.cpp` | Parses `set_config`, calls sensor poll, embeds in telemetry |

**Simulation:** All sensors return deterministic pseudo-random values keyed by sensor ID + time — works on every MCU without wiring.

**Hardware reads (optional):** When `A0` exists, GPIO sensors (`motion_pir`, `door_contact`, `water_leak`) and ADC sensors (`soil_moisture`, `smoke`, `current_clamp`, `ph`) read the analog pin; otherwise fall back to simulation.

### Port / stub nodes (`hardware/stubs/`)

| File | Role |
|------|------|
| `generic_node.c` | Boot + telemetry loop, `hermes_stub_handle_command()` |
| `hermes_stub_sensor.c` | Same 37 IDs and JSON format as PIO firmware |
| `hermes_stub_sensor.h` | Portable API (no Arduino) |

Used for PIC16/18, STM8, and any vendor SDK port.

---

## Telemetry JSON format

```json
{
  "node_id": 1,
  "mcu": "esp8266",
  "family": "espressif8266",
  "telemetry": {
    "cpu": 28.0,
    "ram_kb": 42,
    "temp_c": 44.0,
    "power_mw": 350.0,
    "link_quality": 92,
    "sensors": [
      {"id": "soil_moisture", "value": 38.50, "unit": "%"},
      {"id": "temp_humidity", "value": 22.30, "unit": "C"},
      {"id": "co2", "value": 612.00, "unit": "ppm"}
    ]
  }
}
```

Sensor JSON fragment from `hermes_sensor_json()`:

```json
,"sensors":[{"id":"soil_moisture","value":38.50,"unit":"%"}]
```

---

## set_config command

```json
{
  "cmd": "set_config",
  "config": {
    "sensors": ["soil_moisture", "temp_humidity", "rain_gauge"]
  }
}
```

Firmware calls `hermes_sensor_configure(ids, count)` and begins polling on next telemetry cycle.

---

## Build & test

```bash
pio run -e esp8266 -d hardware/esp8266
pio run -e stm32h7 -d hardware/stm32h7-pio
pio run -e nrf52840 -d hardware/nrf52840-pio
pio run -e atmega328p -d hardware
pio run -e renesas_ra -d hardware
```

Emulator without hardware:

```bash
python tools/emulator/mcu_emulator.py --with-server --node 1 --sensors soil_moisture,co2
```

---

## Adding a sensor

1. Add entry to `sensors/catalog.json` (`id`, `name`, `bus`, `unit`, `description`)
2. Add `simulate_value()` + `sensor_unit()` in `hermes_sensor.cpp`
3. Mirror in `hermes_stub_sensor.c` for port targets
4. Optionally wire driver in `read_hardware()`
5. Add to blueprint `sensor_list` defaults in `blueprints/*.json`
6. Validate: `curl …/api/sensors` lists new ID with all MCU targets

See [CONTRIBUTING.md](CONTRIBUTING.md#adding-a-sensor).

---

## Related

- [BLUEPRINTS.md](BLUEPRINTS.md) — sensor_list in blueprints
- [MCU_SUPPORT.md](MCU_SUPPORT.md) — per-family firmware paths
- [API.md](API.md) — REST `/api/sensors`
