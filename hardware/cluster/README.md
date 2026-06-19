# Phase 1 — Hardware Cluster

3-node heterogeneous cluster: **STM32H7 · ESP32-S3 · nRF52840**

## Transports

| Transport | Use case |
|-----------|----------|
| SPI | High-bandwidth inter-MCU (STM32 ↔ ESP32) |
| BLE | Wireless sensor mesh (nRF ↔ ESP32) |
| UART | Debug + fallback link |
| DMA | Zero-copy SPI transfers |

## Build (per target)

```bash
# STM32H7 (STM32CubeIDE / CMake + arm-none-eabi-gcc)
cd hardware/stm32h7 && cmake -B build && cmake --build build

# ESP32-S3 (ESP-IDF)
cd hardware/esp32-s3 && idf.py build

# nRF52840 (Zephyr / nRF Connect SDK)
cd hardware/nrf52840 && west build -b nrf52840dk_nrf52840
```

## Host Simulator

### Python (no compiler required)

```bash
python tools/cluster-sim/sim.py
```

Runs phases 1-6 end-to-end against the control-plane cluster manager.

### C (requires CMake + GCC)

```bash
cmake -B build -DHERMES_BUILD_SIM=ON
cmake --build build
./build/tools/cluster-sim/cluster-sim
```

Windows: `.\build.ps1 -Sim -Test`

**Success criteria:** 3 nodes exchange heartbeats, ping/pong, latency measured.

## Protocol

See `include/hermes_link.h` — all nodes share a common frame format with CRC32.
