# RTL8720 — Production Alias

**Tier:** alias (production)  
**Flash:** rtl8710 LibreTiny build (same Tuya/WBR Realtek module family).

```bash
pio run -e rtl8710 -d hardware -t upload
```

Register as `rtl8720`. Sensors via `hermes_sensor.cpp` on LibreTiny targets.

**Why:** LibreTiny ambz2 OTA header compile bug on RTL8720CF board definition.
