# HERMES MCU Production Aliases, Ports & Coprocessors

Targets that do not have a direct PlatformIO build use one of three **production** paths:

| Tier | Meaning |
|------|---------|
| **supported** | Unified PIO firmware in `hardware/src/` |
| **alias** | Flash a sibling target; register as this MCU in the cluster |
| **port** | Port `generic_node.c` + `hermes_stub_sensor.c` to vendor SDK |
| **coprocessor** | Zigbee/radio dongle — cluster link, not a full App Studio node |

Registry: `hardware/mcu_registry.json`  
API: `GET /api/mcus`, `GET /api/mcus/<id>/deploy`, `GET /api/deploy/approaches`  
CLI: `python scripts/flash_mcu.py --list` | `python scripts/flash_mcu.py <id> --port COM4`  
Build summary: `python scripts/summarize_mcu_build.py` → `build/mcu_build_summary.txt`

## Flash aliases (production firmware from sibling target)

| Target | Flash instead | Command |
|--------|---------------|---------|
| esp32c6 | esp32c3 | `pio run -e esp32c3 -d hardware -t upload` |
| esp32p4 | esp32s3 | `pio run -e esp32s3 -d hardware/esp32s3-pio -t upload` |
| rtl8720 / xr809 | rtl8710 | `pio run -e rtl8710 -d hardware -t upload` |
| lpc1768 | stm32f4 | `pio run -e stm32f4 -d hardware -t upload` |
| nrf5340 / nrf53 | nrf52840 | `pio run -e nrf52840 -d hardware/nrf52840-pio -t upload` |
| nrf51 | nrf52832 | `pio run -e nrf52832 -d hardware -t upload` |
| kinetis | imxrt | `pio run -e imxrt -d hardware -t upload` |
| rx | renesas_ra | `pio run -e renesas_ra -d hardware -t upload` |
| efr32 (Windows) | nrf52840 | `pio run -e nrf52840 -d hardware/nrf52840-pio -t upload` |
| stm32h5 / stm32u5 | stm32h7 | `pio run -e stm32h7 -d hardware/stm32h7-pio -t upload` |
| stm32l4 / stm32wb | stm32f4 | `pio run -e stm32f4 -d hardware -t upload` |

All alias targets support the full **37-sensor** catalog via the same firmware (`hermes_sensor.cpp`) or stub layer.

## SDK port (generic_node.c)

For **stm8**, **pic16**, **pic18**:

1. Copy `generic_node.c`, `hermes_stub_sensor.c`, `hermes_stub_sensor.h`
2. Define `HERMES_STUB_MCU_NAME`, `HERMES_STUB_FAMILY`, `HERMES_NODE_ID`
3. Wire `printf` to UART and call `hermes_stub_handle_command()` on RX
4. Reference **pic32** or **stm32f1** PIO builds for protocol parity

## Coprocessors (cluster link)

| MCU | Role | Path |
|-----|------|------|
| EFR32MG12/MG21 | Zigbee NCP | `efr32/` |
| CC2530/CC2531 | Zigbee coordinator | `ti_cc/` |
| CC2652 / CC26xx | TI Z-Stack coordinator | `ti_cc/` |

Connect via UART/SPI serial-bridge; forward cluster frames to the control plane.

## Sensor support on all MCUs

- **PIO firmware**: `hardware/src/hermes_sensor.cpp` — simulation for all 37 sensors; optional A0 GPIO/ADC on boards with analog pin
- **Stub/port nodes**: `hermes_stub_sensor.c` — same sensor IDs and JSON telemetry format
- **API**: every sensor lists all registry MCU IDs (`sensor_mcu_ids: "__all__"`)
