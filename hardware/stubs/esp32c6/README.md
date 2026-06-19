# ESP32-C6 — Production Alias

**Tier:** alias (production)  
**Flash:** esp32c3 firmware — same HERMES JSON serial protocol on ESP32-C6 boards.

```bash
pio run -e esp32c3 -d hardware -t upload
```

Register the node as `esp32c6` in the control plane. All 37 sensors work via `hermes_sensor.cpp`.

**Why:** Arduino framework for ESP32-C6 is not supported on PlatformIO espressif32@7.0.1.
