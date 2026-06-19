# HERMES User Guide — How to Use It on Any Board

This guide teaches you how to run HERMES from zero to a working node on **any of the 60 supported MCU targets**.

**Repository:** https://github.com/SixteenAngels/hermes.git  
**Full reference:** [HERMES_COMPLETE_DOCUMENTATION.md](HERMES_COMPLETE_DOCUMENTATION.md)

---

## Can HERMES be used on all boards?

**Yes.** Every MCU in `hardware/mcu_registry.json` has a production path:

| Approach | Boards | How |
|----------|--------|-----|
| **Direct PIO** | 36 boards | Build & flash unified firmware |
| **Flash alias** | 15 boards | Flash a close sibling chip; register as your real board |
| **SDK port** | 3 boards (PIC16/18, STM8) | Port portable C stub to vendor IDE |
| **Coprocessor** | 6 radio dongles | Zigbee link + separate host node for apps |

There are **0 uncovered boards** in the registry. Sensors (all 37 types) work on every target.

---

## Prerequisites

| Tool | Purpose |
|------|---------|
| Python 3.8+ | Control plane, bridge, scripts |
| PlatformIO | Build firmware for 36+ PIO targets |
| USB serial driver | CH340, CP2102, etc. for your board |
| Optional: ESP-IDF | Full ESP32-S3 gateway stack only |

```bash
pip install -r requirements.txt
pip install platformio pyserial intelhex
```

On Windows, set PlatformIO to project drive (optional):

```powershell
. F:\hermes\scripts\pio_env.ps1
```

---

## Step 1 — Start the control plane

```bash
cd hermes
python run.py
```

Open **http://localhost:8787** in your browser.

- Auth token (dev): `hermes-dev`
- API: `curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/status`

---

## Step 2 — Find your board

### Option A: CLI list (recommended)

```bash
python scripts/flash_mcu.py --list
```

Groups all 60 boards by deploy approach.

### Option B: API

```bash
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/mcus
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/deploy/approaches
```

### Option C: Deploy plan for one board

```bash
python scripts/flash_mcu.py esp32c6 --plan-only
# or
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/mcus/esp32c6/deploy
```

---

## Step 3 — Flash your board

The flash script **auto-detects your serial port** when you omit `--port`. It prefers USB-UART adapters (CH340, CP210x, FTDI, etc.).

```bash
# Auto-detect port and flash
python scripts/flash_mcu.py esp8266

# List connected ports
python scripts/flash_mcu.py --list-ports

# Flash + immediately start serial bridge
python scripts/flash_mcu.py esp8266 --bridge

# Force a specific port
python scripts/flash_mcu.py esp8266 --port COM4
```

Replace `COM4` only when auto-detect picks the wrong port (multiple devices plugged in).

### Path A — Direct PIO (most boards)

Examples:

```bash
# ESP8266 / NodeMCU / D1 Mini (port auto-detected)
python scripts/flash_mcu.py esp8266
python scripts/flash_mcu.py d1_mini

# STM32
python scripts/flash_mcu.py stm32h7 --port COM6
python scripts/flash_mcu.py stm32f4 --port COM6

# Nordic / RP2040 / AVR
python scripts/flash_mcu.py nrf52840 --port COM6
python scripts/flash_mcu.py rp2040 --port COM4
python scripts/flash_mcu.py atmega328p --port COM5

# Tuya / LibreTiny (first build is slow)
python scripts/flash_mcu.py bk7231t --port COM4
python scripts/flash_mcu.py rtl8710 --port COM4
```

Build only (no flash): add `--no-upload`

### Path B — Flash alias (unsupported PIO, but board works)

Flash the **alias** firmware, then register as **your** board type:

```bash
# ESP32-C6 board → builds esp32c3 firmware
python scripts/flash_mcu.py esp32c6 --port COM4

# ESP32-P4 → esp32s3
python scripts/flash_mcu.py esp32p4 --port COM4

# RTL8720 / XR809 → rtl8710
python scripts/flash_mcu.py rtl8720 --port COM4

# nRF5340 / nRF53 → nrf52840
python scripts/flash_mcu.py nrf5340 --port COM6

# nRF51 → nrf52832
python scripts/flash_mcu.py nrf51 --port COM6

# LPC1768 / Kinetis / Renesas RX
python scripts/flash_mcu.py lpc1768 --port COM6
python scripts/flash_mcu.py kinetis --port COM6
python scripts/flash_mcu.py rx --port COM5

# STM32 variants
python scripts/flash_mcu.py stm32h5 --port COM6
python scripts/flash_mcu.py stm32l4 --port COM6
```

### Path C — SDK port (PIC16, PIC18, STM8)

```bash
python scripts/flash_mcu.py pic16 --plan-only
```

1. Copy from `hardware/stubs/`:
   - `generic_node.c`
   - `hermes_stub_sensor.c`
   - `hermes_stub_sensor.h`
2. Open MPLAB X (PIC) or STVD (STM8).
3. Define at compile time:
   ```c
   #define HERMES_STUB_MCU_NAME "pic16"
   #define HERMES_STUB_FAMILY "pic"
   #define HERMES_NODE_ID 1
   ```
4. Wire `printf` to UART 115200.
5. On UART RX, call `hermes_stub_handle_command(json_line)`.

Host smoke test (GCC):

```bash
cd hardware/stubs/port
make
```

### Path D — Coprocessor (Zigbee dongles)

```bash
python scripts/setup_coprocessor.py cc2531 --port COM5
```

Flash vendor Z-Stack on the dongle. Run App Studio apps on a **host** node (ESP32, nRF52840, etc.).

---

## Step 4 — Connect board to control plane

After flashing, the board emits JSON on serial at **115200 baud**:

```json
{"boot":"hermes-node","mcu":"esp8266","family":"espressif8266","node_id":1,"sensors":true}
```

### Serial bridge (hardware)

Terminal 1 — control plane (already running):

```bash
python run.py
```

Terminal 2 — bridge (use **your** board id as `--node-type`):

```bash
python tools/serial-bridge/bridge.py COM4 --register --node-id 1 --name "My Board" --node-type esp8266
```

For alias boards, `--node-type` must be the **real** board (e.g. `esp32c6`, not `esp32c3`).

### No hardware (simulation)

```bash
python demo.py
# or
python tools/emulator/mcu_emulator.py --with-server --node 1 --sensors soil_moisture,temp_humidity
```

---

## Step 5 — Deploy an application (App Studio)

1. Open http://localhost:8787
2. Go to **App Studio**
3. Pick a blueprint (e.g. **Smart Farm**, **Greenhouse**, **Air Quality**)
4. Fill in the form:
   - Zone name, thresholds
   - **Sensors** — pick from 37 types
   - **Target node** — the node you registered
5. Click **Create & Deploy**
6. Watch telemetry on the dashboard — sensor readings appear in node heartbeat JSON

### Deploy via API

```bash
curl -X POST http://localhost:8787/api/apps/create \
  -H "X-Hermes-Token: hermes-dev" -H "Content-Type: application/json" \
  -d '{
    "blueprint": "smartfarm",
    "name": "field-1",
    "node_id": 1,
    "config": {
      "zone_name": "North",
      "sensors": ["soil_moisture", "temp_humidity"]
    }
  }'
```

---

## Step 6 — Use sensors on any board

All 37 sensors work on all 60 MCUs.

| Layer | File |
|-------|------|
| PIO firmware | `hardware/src/hermes_sensor.cpp` |
| Port/stub nodes | `hardware/stubs/hermes_stub_sensor.c` |
| Catalog | `sensors/catalog.json` |

Sensors are configured when you deploy an app. The control plane sends `set_config` to the node. Telemetry includes:

```json
"sensors": [
  {"id": "soil_moisture", "value": 38.50, "unit": "%"},
  {"id": "temp_humidity", "value": 22.30, "unit": "C"}
]
```

List sensors:

```bash
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/sensors
```

---

## Step 7 — Developer CLI (optional)

```bash
python sdk/cli/hermes create myapp
python sdk/cli/hermes build examples/smartfarm -t host
python sdk/cli/hermes deploy examples/smartfarm
python sdk/cli/hermes migrate smartfarm 0 2
```

See [SDK_GUIDE.md](SDK_GUIDE.md).

---

## Quick reference — common boards

| Your board | Command |
|------------|---------|
| ESP8266 / NodeMCU / D1 Mini | `python scripts/flash_mcu.py esp8266 --port COM4` |
| ESP32 (classic) | `python scripts/flash_mcu.py esp32 --port COM4` |
| ESP32-S3 | `python scripts/flash_mcu.py esp32s3 --port COM4` |
| ESP32-C3 | `python scripts/flash_mcu.py esp32c3 --port COM4` |
| ESP32-C6 | `python scripts/flash_mcu.py esp32c6 --port COM4` |
| STM32H7 Nucleo | `python scripts/flash_mcu.py stm32h7 --port COM6` |
| STM32F4 / Blue Pill | `python scripts/flash_mcu.py stm32f4 --port COM6` |
| nRF52840 Feather | `python scripts/flash_mcu.py nrf52840 --port COM6` |
| Raspberry Pi Pico | `python scripts/flash_mcu.py rp2040 --port COM4` |
| Arduino Uno (ATmega328P) | `python scripts/flash_mcu.py atmega328p --port COM5` |
| Tuya BK7231 | `python scripts/flash_mcu.py bk7231t --port COM4` |
| RTL8710 / RTL8720 | `python scripts/flash_mcu.py rtl8710 --port COM4` |

After every flash:

```bash
python tools/serial-bridge/bridge.py COM4 --register --node-type <your_mcu_id>
```

---

## Troubleshooting

| Problem | Fix |
|---------|-----|
| `pio` not found | `pip install platformio` |
| Upload fails / wrong port | Check Device Manager; use `--upload-port COMx` |
| 401 on API | Add header `X-Hermes-Token: hermes-dev` |
| Node not on dashboard | Run serial-bridge with `--register` |
| ESP32 build errors | `pip install intelhex`; reinstall espressif32 framework |
| esp32c6 won't build directly | Normal — use `flash_mcu.py esp32c6` (alias to c3) |
| No hardware | `python demo.py` or emulator with `--with-server` |
| Which approach for my board? | `python scripts/flash_mcu.py <id> --plan-only` |

---

## Validate everything works

```bash
python scripts/validate.py
python scripts/flash_mcu.py --list
python scripts/summarize_mcu_build.py
```

---

## Learn more

| Document | Content |
|----------|---------|
| [QUICKSTART.md](QUICKSTART.md) | Fast setup |
| [HARDWARE_BRINGUP.md](HARDWARE_BRINGUP.md) | Wiring, SPI/UART cluster |
| [MCU_SUPPORT.md](MCU_SUPPORT.md) | All 60 boards |
| [SENSORS.md](SENSORS.md) | 37 sensors |
| [BLUEPRINTS.md](BLUEPRINTS.md) | 26 apps |
| [API.md](API.md) | REST + WebSocket |
| [BUILD_TARGETS.md](BUILD_TARGETS.md) | Build matrix |
| [HERMES_COMPLETE_DOCUMENTATION.md](HERMES_COMPLETE_DOCUMENTATION.md) | Full submission doc |

---

## Summary

1. `python run.py`
2. `python scripts/flash_mcu.py <your_board> --port COMx`
3. `python tools/serial-bridge/bridge.py COMx --register --node-type <your_board>`
4. App Studio → blueprint → sensors → **Create & Deploy**

HERMES works on **all 60 registered boards** using direct PIO, flash alias, SDK port, or coprocessor + host — whichever fits your chip.
