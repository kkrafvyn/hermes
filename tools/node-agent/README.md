# HERMES Node Agent

Runs on each MCU (or host during development) and reports live telemetry to the control plane.

## Usage

```bash
# Start control plane first
python run.py

# Start agents (one per terminal, or use demo.py)
python tools/node-agent/agent.py 0   # STM32H7
python tools/node-agent/agent.py 1   # ESP32-S3
python tools/node-agent/agent.py 2   # nRF52840
```

## One-command demo

```bash
python demo.py
```

Starts control plane + all 3 agents. Open http://localhost:8787/

## Options

```
--interval 3.0          Heartbeat interval (seconds)
--api http://host:8787  Control plane URL
```

## On real hardware

The ESP32 gateway (`hardware/esp32-s3/main/hermes_gateway.c`) posts the same heartbeat JSON over HTTP. STM32 and nRF agents will use UART/SPI bridge to the ESP32 gateway in production.
