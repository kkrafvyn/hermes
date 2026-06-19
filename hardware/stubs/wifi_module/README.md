# External WiFi modules (STM32, AVR, PIC + Realtek/Beken/ESP UART)

Boards **without** built-in WiFi use `hermes_wifi_module.cpp` with a module type set in the **HTTP dashboard** or at build time.

## Module types

| Type | ID | Use case |
|------|-----|----------|
| `builtin` | 0 | ESP8266, ESP32, RTL8710, BK7231 (on-chip radio) |
| `uart_at` | 1 | ESP-01 / AT firmware over UART |
| `realtek_uart` | 2 | Realtek Ameba (RTL8710/8720) module over UART |
| `beken_uart` | 3 | Beken BK7231 module over UART |
| `esp_coproc` | 4 | ESP8266 co-processor running HERMES HTTP |

## Build flags

```ini
; STM32 + ESP-01 WiFi co-processor
build_flags = -DHERMES_STM32 -DHERMES_HTTP_SERVER=1 -DHERMES_WIFI_UART_AT_DEFAULT=1

; Realtek LibreTiny (built-in WiFi + HTTP)
build_flags = -DHERMES_REALTEK -DHERMES_HTTP_SERVER=1

; Beken LibreTiny
build_flags = -DHERMES_BEKEN -DHERMES_HTTP_SERVER=1
```

## HTTP configuration

On any board with `HERMES_HTTP_SERVER=1`, open the board URL → **Network**:

- Pick module type (built-in / Realtek UART / Beken UART / AT)
- SSID + password
- UART baud (external modules)
- Enable **sensor mesh** for remote nodes

Settings persist in EEPROM.

## Wiring (external UART module)

| MCU UART | WiFi module |
|----------|-------------|
| TX | RX |
| RX | TX |
| GND | GND |
| 3V3 | VCC |

Default: `Serial1` at 115200 (`HERMES_WIFI_UART_PORT` in `hermes_wifi_module.cpp`).

## Remote sensor mesh

Remote nodes send JSON lines to the gateway UART:

```json
{"mesh":"join","node_id":3,"name":"barn-sensor","sensors":["temp_humidity","motion_pir"]}
{"mesh":"reading","node_id":3,"sensors":[{"id":"temp_humidity","value":23.5}]}
```

Gateway polls with `{"mesh":"poll","seq":1}`. Up to **8** remote nodes per board.

Flash remote nodes with any HERMES MCU target (nRF, STM32, ESP8266) — they join the mesh over UART/BLE/Zigbee per app blueprint `mesh_transport`.
