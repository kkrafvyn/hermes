# HERMES Build Targets

See **[MCU Support Matrix](MCU_SUPPORT.md)** for all 60 registry targets.

Build verification: `python scripts/summarize_mcu_build.py` → `build/mcu_build_summary.txt`

## Quick commands

```bash
# List all targets
python scripts/build_all.py --list

# Build entire MCU matrix (sequential, ~hours)
python scripts/build_mcu_check.py --timeout 3600
python scripts/summarize_mcu_build.py

# Retry only failed envs
python scripts/build_mcu_check.py --retry-failed

# Build one target + flash
python scripts/build_all.py --firmware --target esp8266 --upload-port COM4

# SDK app for every MCU
python sdk/build.py examples/smartfarm all
```

## Build summary output

After a matrix run, `build/mcu_build_summary.txt` reports effective status per registry ID:

```
PIO passed: 36  workaround-covered: 18  coprocessor: 6  stub: 0  fail: 0
```

| Status | Meaning |
|--------|---------|
| PASS | PlatformIO env compiled successfully |
| WORKAROUND | No direct build — flash alias target instead |
| COPROCESSOR | Zigbee/radio dongle — not HERMES node firmware |
| PORT | Port `hardware/stubs/generic_node.c` to vendor SDK |

## Firmware tiers

| Tier | Count | Build tool |
|------|-------|------------|
| **supported** | 36 | `pio run -e <env> -d <path>` |
| **alias** | 14 | Flash sibling target (see table below) |
| **port** | 3 | MPLAB X / STVD + `generic_node.c` |
| **coprocessor** | 6 | Vendor Z-Stack / Spinel firmware |

Registry: `hardware/mcu_registry.json`  
API: `GET /api/mcus` — each target includes `flash_command`

## Workaround flash aliases

| Register as | Flash env | Command |
|-------------|-----------|---------|
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
| pic16, pic18 | pic32 (reference) | Port stub; `pio run -e pic32 -d hardware` |
| stm8 | stm32f1 (reference) | Port stub; `pio run -e stm32f1 -d hardware` |

Details: `hardware/stubs/README.md`

## Per-target flash one-liners

### Espressif

```bash
pio run -e esp8266 -d hardware/esp8266 -t upload --upload-port COM4
pio run -e d1_mini -d hardware -t upload --upload-port COM4
pio run -e esp32 -d hardware -t upload
pio run -e esp32s3 -d hardware/esp32s3-pio -t upload
pio run -e esp32c3 -d hardware -t upload
pio run -e esp32_cam -d hardware -t upload
```

### STM32 / Nordic / RP2040

```bash
pio run -e stm32h7 -d hardware/stm32h7-pio -t upload
pio run -e stm32f4 -d hardware -t upload
pio run -e nrf52840 -d hardware/nrf52840-pio -t upload
pio run -e rp2040 -d hardware -t upload
```

### AVR / SAMD / MSP430 / PIC / Renesas

```bash
pio run -e atmega328p -d hardware -t upload
pio run -e samd21 -d hardware -t upload
pio run -e msp430 -d hardware -t upload
pio run -e pic32 -d hardware -t upload
pio run -e renesas_ra -d hardware -t upload
```

### Tuya / LibreTiny (long first build)

```bash
pio run -e bk7231t -d hardware -t upload
pio run -e rtl8710 -d hardware -t upload
```

### ESP-IDF gateway (full stack)

```bash
cd hardware/esp32-s3
idf.py set-target esp32s3
idf.py build flash monitor
```

## Shared firmware source

`hardware/src/main.cpp` — serial JSON telemetry, `set_config`, sensor polling.  
Environments in `hardware/platformio_mcu.ini` + per-board trees (`esp8266/`, `esp32s3-pio/`, etc.).

## SDK app targets (cross-compile)

| Target | Toolchain | CPU |
|--------|-----------|-----|
| `host` | gcc | x86_64 (dev/test) |
| `emulator` | gcc | x86_64 (sim) |
| `esp8266` | xtensa-lx106-elf-gcc | ESP8266 |
| `esp32s3` | xtensa-esp32s3-elf-gcc | ESP32-S3 |
| `esp32c3` | riscv32-esp-elf-gcc | ESP32-C3 |
| `stm32h7` | arm-none-eabi-gcc | Cortex-M7 |
| `stm32f4` | arm-none-eabi-gcc | Cortex-M4 |
| `nrf52840` | arm-none-eabi-gcc | Cortex-M4 |

## What each build produces

| Build | Output |
|-------|--------|
| Firmware | `.pio/build/<env>/firmware.bin` — flash to MCU |
| SDK app | `examples/<app>/build/<target>/<app>.bin` |
| Package | `examples/<app>/binaries/` — manifest + SHA256 + HMAC |
| Matrix report | `build/mcu_build_report.txt` + `mcu_build_summary.txt` |

## CI / validation

```bash
python scripts/validate.py
python scripts/summarize_mcu_build.py
```

## Adding a new MCU

See [CONTRIBUTING.md](CONTRIBUTING.md#adding-an-mcu-target).
