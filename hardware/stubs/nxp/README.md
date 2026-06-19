# NXP LPC1768 / Kinetis — Production Alias

**Tier:** alias (production)

| Target | Flash instead |
|--------|---------------|
| lpc1768 | stm32f4 |
| kinetis | imxrt (Teensy 4.x) |

```bash
pio run -e stm32f4 -d hardware -t upload
pio run -e imxrt -d hardware -t upload
```

LPC1768 has no Arduino PIO support (mbed-only). Kinetis uses imxrt as the closest Cortex-M HERMES build.

All 37 sensors available on the flashed firmware.
