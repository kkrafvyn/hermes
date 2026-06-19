# EFR32 — Production Alias + Coprocessor

**Tier:** alias (gateway) / coprocessor (MG12, MG21)  
**Flash host:** nrf52840 on Windows (Silabs PIO unavailable).

```bash
pio run -e nrf52840 -d hardware/nrf52840-pio -t upload
```

**Architecture:** nrf52840 runs HERMES node + App Studio apps; EFR32 runs Zigbee NCP firmware and links via UART (Spinel).

Sensor data flows through the host node. See `hardware/stubs/efr32/` for coprocessor integration notes.
