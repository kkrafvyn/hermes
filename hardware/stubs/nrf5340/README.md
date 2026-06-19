# nRF5340 / nRF53 — Production Alias

**Tier:** alias (production)  
**Flash:** nrf52840 HERMES node firmware.

```bash
pio run -e nrf52840 -d hardware/nrf52840-pio -t upload
```

Use for Nordic dual-core BLE applications until a Zephyr/nRF Connect SDK port is added.

All sensors supported on nrf52840 build; register cluster node as `nrf5340` or `nrf53`.
