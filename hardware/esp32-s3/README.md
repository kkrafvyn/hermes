# HERMES ESP32-S3 Gateway

ESP-IDF project — cluster gateway with WiFi HTTP or UART serial bridge.

## Quick flash (serial bridge mode — recommended first)

```bash
cd hardware/esp32-s3
idf.py set-target esp32s3
idf.py build
idf.py -p COM3 flash monitor
```

Then on your PC:

```bash
python run.py
python tools/serial-bridge/bridge.py COM3 --register --node-id 1
```

See [docs/HARDWARE_BRINGUP.md](../../docs/HARDWARE_BRINGUP.md) for full guide.

## Role

- SPI slave to STM32H7
- BLE to nRF52840
- HTTP client posts heartbeats to HERMES control plane
- Serves as cluster leader candidate (lowest CPU)

## Setup

```bash
cd hardware/esp32-s3
idf.py set-target esp32s3
idf.py menuconfig   # set WiFi SSID/PASSWORD, HERMES_API_URL
idf.py build flash monitor
```

## Configuration (menuconfig)

| Option | Default |
|--------|---------|
| `HERMES_API_URL` | `http://192.168.1.100:8787` |
| `HERMES_NODE_ID` | `1` |
| WiFi SSID/PASS | your network |

## Files

| File | Purpose |
|------|---------|
| `main/hermes_gateway.c` | Node loop + HTTP heartbeat |
| `main/hermes_spi.c` | SPI slave transport |
| `components/hermes/` | Shared HERMES cluster library |

## Without hardware

Use the Python node agent instead:

```bash
python tools/node-agent/agent.py 1
```
