# HERMES vs Tasmota — GPIO templates & custom boards

HERMES now supports **Tasmota-style board templates**: assign GPIO roles over HTTP without recompiling for every custom pinout.

## Configure by MCU type

| MCU family | Flash raw build | Configure pins |
|------------|-----------------|----------------|
| ESP8266/32/S2/S3/C3 | `flash_mcu.py esp32_raw` etc. | HTTP **/board** after WiFi |
| Beken / Realtek (Tuya) | `flash_mcu.py bk7231t_raw` | HTTP **/board** |
| STM32 / AVR / SAMD | `flash_mcu.py stm32h7_raw` etc. | Serial JSON or PC API |
| nRF52840 / RP2040 | `flash_mcu.py nrf52840_raw` | Serial JSON or PC API |

### Serial GPIO (STM32, nRF, RP2040, AVR)

```json
{"cmd":"board_preset","template_id":"stm32h7_nucleo"}
{"template_id":"custom","board_label":"My board","pins":[{"gpio":13,"fn":11,"flags":0,"idx":0}]}
{"cmd":"get_board_gpio"}
```

### Serial console (ESP8266/ESP32)

```
Status
GPIO
Power 1
Restart
```

### PC dashboard API (via serial bridge)

`POST /api/nodes/{id}/board/gpio` with `{"template_id":"stm32_raw"}` or full pin map.


1. Flash a **raw** or **preset** build:
   ```bash
   python scripts/flash_mcu.py esp32s3_raw --port COMx
   python scripts/flash_mcu.py esp32s3_sim7670g --port COMx
   python scripts/flash_mcu.py esp8266 --port COMx
   ```
2. Join WiFi via setup AP (`HERMES-XXXX` → http://192.168.4.1/).
3. Open **http://\<board-ip\>/board** — pick a preset or add GPIO rows.
4. Save — stored in EEPROM, applied on boot.

Presets: `hardware/board_templates.json` and `hermes_templates.cpp`.

## Waveshare ESP32-S3-SIM7670G-4G

| Feature | Preset mapping |
|---------|----------------|
| RGB LED (WS2812) | GPIO38 → LED |
| Battery gauge (I2C) | GPIO15 SDA, GPIO16 SCL |
| TF card | GPIO5/4/6/46 |
| SIM7670G UART | GPIO17 TX, GPIO18 RX |
| OV camera | GPIO7–14, 34–37 as User (edit per V1/V2) |

Flash: `python scripts/flash_mcu.py esp32s3_sim7670g --port COMx`

## Feature comparison

| Capability | Tasmota | HERMES today |
|------------|---------|--------------|
| WiFi setup AP | Yes | Yes |
| GPIO templates + web | Yes | Yes (`/board`) |
| Runtime pin change | Yes | Yes (EEPROM) |
| Tasmota GPIO[] import | Yes | Yes (`/api/board/tasmota`) |
| MQTT (cmnd/state/tele) | Yes | Yes (ESP8266/ESP32) |
| Serial console | Yes | Yes (`Status`, `GPIO`, `Power`) |
| Rules / scripting | Yes | Blueprint apps |
| OTA | Yes | Yes (`hermes-{node}.local`) |
| 100+ device drivers | Yes | App Studio sensors |
| Multi-MCU cluster | No | **Yes** |
| 4G SIM7670G | Add-ons | Pin preset (driver TBD) |
| Camera / SD | Partial | Pin preset (driver TBD) |

## API

- `GET /api/board/templates` — presets + function list
- `GET/POST /api/board/gpio` — read/write pin map
- `POST /api/board/preset` — apply `template_id`
- `POST /api/board/tasmota` — import Tasmota `GPIO[]` template JSON
- `GET/POST /api/mqtt/config` — MQTT broker settings
- `GET /board` — web UI

## Still missing vs Tasmota

WS2812 driver, SIM7670 PPP/AT, camera pipeline, SD logging, rules engine, full Tasmota template string (module + flags).
