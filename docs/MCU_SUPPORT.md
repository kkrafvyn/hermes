# HERMES MCU Support Matrix

**60 targets** in `hardware/mcu_registry.json` (v2.0.0)  
**API:** `GET /api/mcus` — includes `tier`, `workaround`, `flash_command` per target  
**Build status:** `python scripts/summarize_mcu_build.py` → `build/mcu_build_summary.txt`

## Support tiers

| Tier | Count | Meaning |
|------|-------|---------|
| **supported** | 36 | PlatformIO env + unified `hardware/src/main.cpp` + full sensors |
| **alias** | 14 | Production via flash of a sibling target |
| **port** | 3 | Port `hardware/stubs/generic_node.c` to vendor SDK |
| **coprocessor** | 6 | Zigbee/radio — cluster link, not App Studio node |

> STM32 targets with env issues (h5, u5, l4, wb) have **supported** tier in registry but use **flash aliases** at deploy time — see workarounds table.

## Build matrix summary

```
PIO passed: 36  |  workaround-covered: 18  |  coprocessor: 6  |  fail: 0
```

## Workaround flash aliases

| Target | Flash instead | `pio` command |
|--------|---------------|---------------|
| esp32c6 | esp32c3 | `pio run -e esp32c3 -d hardware -t upload` |
| esp32p4 | esp32s3 | `pio run -e esp32s3 -d hardware/esp32s3-pio -t upload` |
| rtl8720, xr809 | rtl8710 | `pio run -e rtl8710 -d hardware -t upload` |
| lpc1768 | stm32f4 | `pio run -e stm32f4 -d hardware -t upload` |
| nrf5340, nrf53 | nrf52840 | `pio run -e nrf52840 -d hardware/nrf52840-pio -t upload` |
| nrf51 | nrf52832 | `pio run -e nrf52832 -d hardware -t upload` |
| kinetis | imxrt | `pio run -e imxrt -d hardware -t upload` |
| rx | renesas_ra | `pio run -e renesas_ra -d hardware -t upload` |
| efr32 | nrf52840 | `pio run -e nrf52840 -d hardware/nrf52840-pio -t upload` |
| stm32h5, stm32u5 | stm32h7 | `pio run -e stm32h7 -d hardware/stm32h7-pio -t upload` |
| stm32l4, stm32wb | stm32f4 | `pio run -e stm32f4 -d hardware -t upload` |
| pic16, pic18 | pic32 (ref) | Port stub — `pio run -e pic32 -d hardware` |
| stm8 | stm32f1 (ref) | Port stub — `pio run -e stm32f1 -d hardware` |

Stub docs: `hardware/stubs/README.md`

---

## Espressif8266 (9 targets) — ✅ supported

| ID | Boards |
|----|--------|
| esp8266, esp8285, esp01, esp07, esp12e | ESP-01, ESP-07, ESP-12E/F |
| d1_mini, nodemcu | Wemos D1 Mini, NodeMCU, LOLIN |

```bash
pio run -e esp8266 -d hardware/esp8266 -t upload --upload-port COM4
pio run -e d1_mini -d hardware -t upload
```

## Espressif32 (11 targets) — ✅ / alias

| ID | Status | Notes |
|----|--------|-------|
| esp32, esp32_wrover, esp32s2, esp32s3, esp32c3 | ✅ PIO | Unified firmware |
| esp32s3_psram, esp32_cam, esp32_eth | ✅ PIO | PSRAM, camera, Ethernet |
| esp32c6 | alias | → esp32c3 |
| esp32p4 | alias | → esp32s3 |

```bash
pio run -e esp32 -d hardware -t upload
pio run -e esp32s3 -d hardware/esp32s3-pio -t upload
pio run -e esp32c3 -d hardware -t upload
```

## STM32 (11 targets) — ✅ / alias / port

| ID | Status |
|----|--------|
| stm32f0, stm32f1, stm32f4, stm32f7, stm32g0, stm32h7 | ✅ PIO |
| stm32l4, stm32h5, stm32u5, stm32wb | alias (flash f4 or h7) |
| stm8 | port |

```bash
pio run -e stm32h7 -d hardware/stm32h7-pio -t upload
pio run -e stm32f4 -d hardware -t upload
```

## Nordic (5 targets) — ✅ / alias

| ID | Status |
|----|--------|
| nrf52832, nrf52840 | ✅ PIO |
| nrf5340, nrf53, nrf51 | alias |

```bash
pio run -e nrf52840 -d hardware/nrf52840-pio -t upload
```

## Raspberry Pi RP2040 — ✅ supported

```bash
pio run -e rp2040 -d hardware -t upload
```

## AVR / Microchip PIC (6 targets) — ✅ / port

| ID | Status |
|----|--------|
| atmega328p, atmega2560, avr | ✅ PIO |
| pic32 | ✅ PIO |
| pic16, pic18 | port |

```bash
pio run -e atmega328p -d hardware -t upload
pio run -e pic32 -d hardware -t upload
```

## NXP (3 targets) — ✅ / alias

| ID | Status |
|----|--------|
| imxrt | ✅ PIO (Teensy 4.x) |
| lpc1768, kinetis | alias |

## Renesas (2 targets) — ✅ / alias

| ID | Status |
|----|--------|
| ra | ✅ PIO (`renesas_ra` env) |
| rx | alias → ra |

## Silicon Labs / TI (7 targets) — alias / coprocessor

| ID | Tier |
|----|------|
| efr32 | alias → nrf52840 |
| efr32mg12, efr32mg21 | coprocessor |
| cc2530, cc2531, cc2652, cc26xx | coprocessor |

## Beken / Realtek / Tuya (5 targets) — ✅ / alias

| ID | Status | Platform |
|----|--------|----------|
| bk7231t, bk7231n | ✅ PIO | LibreTiny |
| rtl8710 | ✅ PIO | LibreTiny |
| rtl8720, xr809 | alias → rtl8710 | |

```bash
pio run -e bk7231t -d hardware -t upload
pio run -e rtl8710 -d hardware -t upload
```

## MSP430 / SAMD (3 targets) — ✅ supported

```bash
pio run -e msp430 -d hardware -t upload
pio run -e samd21 -d hardware -t upload
pio run -e samd51 -d hardware -t upload
```

---

## Sensors on all MCUs

Registry: `sensor_mcu_ids: "__all__"` — all 60 IDs.

| Firmware path | Sensor layer |
|---------------|--------------|
| PIO builds | `hardware/src/hermes_sensor.cpp` |
| Port/alias stubs | `hardware/stubs/hermes_stub_sensor.c` |

Configured via App Studio → `set_config` → serial telemetry. See [SENSORS.md](SENSORS.md).

## Firmware architecture

```
hardware/src/main.cpp          ← universal node (boot, telemetry, commands)
hardware/src/hermes_sensor.cpp ← 37 sensors
hardware/src/hermes_print.h    ← snprintf shim (AVR, MSP430, RA, …)
hardware/platformio_mcu.ini    ← env matrix
hardware/mcu_registry.json     ← 60 targets + workarounds
```

## Adding an MCU

See [CONTRIBUTING.md](CONTRIBUTING.md#adding-an-mcu-target).

## Related

- [BUILD_TARGETS.md](BUILD_TARGETS.md) — flash one-liners, CI output
- [hardware/stubs/README.md](../hardware/stubs/README.md) — alias/port/coprocessor guide
