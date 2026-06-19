# TI CC253x / CC26xx — Coprocessors

**Tier:** coprocessor (cluster link, not full HERMES node firmware)

| MCU | Role |
|-----|------|
| CC2530 / CC2531 | Zigbee coordinator dongle |
| CC2652 / CC26xx | TI Z-Stack 3 coordinator |

Flash vendor Z-Stack firmware and connect to the control plane via UART serial-bridge.

These devices extend mesh/radio reach; App Studio apps run on **supported** or **alias** node targets.

Sensor telemetry originates from PIO/alias nodes, not from the coprocessor directly.
