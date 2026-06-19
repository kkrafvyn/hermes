# Hardware Bring-Up Guide

Step-by-step path from simulation to physical cluster.

## Stage 1 — Simulation (no hardware)

```bash
python demo.py
python tools/test_integration.py
python tools/cluster-sim/sim.py
.\build-manual.ps1
```

## Stage 2 — ESP8266 via serial bridge (COM4 / CH9102)

HERMES includes a lightweight ESP8266 node firmware for boards without ESP32-S3.

### 1. Detect device

```bash
python -m esptool --port COM4 chip-id
# Chip type: ESP8266EX
```

### 2. Flash (PlatformIO)

```bash
cd hardware/esp8266
pio run -e esp8266 -t upload --upload-port COM4
```

Or one-shot:

```powershell
.\scripts\run_esp8266.ps1 -Port COM4
```

### 3. Run control plane + bridge

```bash
python run.py
python tools/serial-bridge/bridge.py COM4 --register --node-id 1 --name ESP8266 --node-type esp8266
```

Live telemetry appears on the dashboard (node 1, ~50 KB free heap typical for ESP8266).

## Stage 3 — ESP32-S3 via serial bridge

Best first hardware step. No WiFi configuration required.

### 1. Install ESP-IDF v5.x

https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/

### 2. Configure (optional)

```bash
cd hardware/esp32-s3
idf.py menuconfig
# HERMES Gateway -> set node ID, keep "serial bridge" enabled
```

Default `sdkconfig.defaults` uses serial bridge mode.

### 3. Build and flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p COM3 flash monitor
```

### 4. Start control plane + serial bridge

```bash
# Terminal 1
python run.py

# Terminal 2
pip install pyserial
python tools/serial-bridge/bridge.py COM3 --register --node-id 1
```

ESP32 telemetry appears live on the dashboard. Commands are forwarded back over UART.

## Stage 3 — ESP32-S3 via WiFi (direct HTTP)

```bash
idf.py menuconfig
# Disable "Emit JSON telemetry on UART"
# Set WiFi SSID/password
# Set HERMES_API_URL to http://<your-pc-lan-ip>:8787
```

```bash
python run.py   # on your PC
idf.py flash monitor
```

## Stage 4 — STM32H7 + nRF52840

| Node | Link to ESP32 | HAL file | Reference pins (Nucleo / Feather) |
|------|---------------|----------|-----------------------------------|
| STM32H7 | SPI master | `hardware/stm32h7/src/hal_spi.c` | SPI1: PA5 SCK, PA6 MISO, PA7 MOSI, PA4 CS |
| nRF52840 | UART | `hardware/nrf52840/src/hal_uart.c` | UART0: P0.08 TX → ESP RX, P0.06 RX ← ESP TX |

### STM32H7 SPI wiring (to ESP32-S3 gateway)

| Signal | STM32H7 (Nucleo-H743) | ESP32-S3 (gateway) |
|--------|----------------------|-------------------|
| SCK | PA5 (SPI1_SCK) | GPIO18 (SPI2_SCK) |
| MISO | PA6 (SPI1_MISO) | GPIO19 (SPI2_MISO) |
| MOSI | PA7 (SPI1_MOSI) | GPIO23 (SPI2_MOSI) |
| CS | PA4 (SPI1_NSS) | GPIO5 (SPI2_CS) |
| GND | GND | GND |

In CubeMX: enable SPI1 master, DMA TX/RX, 8 MHz initial clock. Bind `hspi1` in `hal_spi.c`.

### nRF52840 UART wiring (to ESP32-S3 gateway)

| Signal | nRF52840 (Feather) | ESP32-S3 |
|--------|-------------------|----------|
| TX | D0 / P0.08 | GPIO44 (RX) |
| RX | D1 / P0.06 | GPIO43 (TX) |
| GND | GND | GND |

Baud: **115200**. Zephyr: `uart0` device in `hal_uart.c`. Adafruit Feather uses USB-serial for debug; use UART1 for cluster link if D0/D1 are occupied.

### Power monitoring (INA219)

Wire INA219 on I2C for `hermes_power_sample()` telemetry (`power_mw` in heartbeat):

| INA219 pin | Connect to |
|------------|------------|
| VCC | 3.3 V |
| GND | GND |
| SDA | MCU SDA (STM32: PB7, ESP32: GPIO21, nRF: P0.26) |
| SCL | MCU SCL (STM32: PB6, ESP32: GPIO22, nRF: P0.27) |
| V+ / V− | High-side shunt on 5 V or battery rail |

Implement `read` callback in `hermes_power_monitor_t` (`hardware/cluster/include/hermes_power.h`) to return `voltage_v`, `current_ma`, `power_mw`.

### Bring-up steps

1. Add HERMES cluster sources to your MCU project
2. Implement HAL hooks (`stm32_spi_send`, `nrf_uart_send`, etc.)
3. ESP32 gateway relays cluster frames via `hermes_router.c`
4. Flash PIO firmware or integrate native SDK tree (`hardware/stm32h7/`, `hardware/nrf52840/`)

## Stage 5 — Full cluster

```
PC (control plane + link hub)
  |
  WiFi or UART
  |
ESP32-S3 (gateway, node 1)  ← hermes_router.c forwards SPI <-> UART
  |-- SPI --> STM32H7 (node 0)   hal_spi.c
  |-- UART --> nRF52840 (node 2) hal_uart.c
```

### Software-only full cluster (no hardware)

```bash
python demo_cluster.py
python tools/test_cluster_link.py
```

Agents use `--hub` to route inter-node frames through the control plane.
On hardware, the ESP32 gateway router replaces the software hub.

Deploy apps from dashboard or CLI:

```bash
python sdk/cli/hermes deploy examples/smartfarm
python sdk/cli/hermes migrate smartfarm 0 2
```
