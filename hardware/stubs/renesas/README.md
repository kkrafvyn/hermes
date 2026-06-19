# Renesas RX — Production Alias

**Tier:** alias (production)  
**Flash:** renesas_ra (Arduino UNO R4 / EK-RA6M3 family).

```bash
pio run -e renesas_ra -d hardware -t upload
```

Register as `rx`. Unified `hermes_sensor.cpp` + `hermes_print.h` on RA builds.
