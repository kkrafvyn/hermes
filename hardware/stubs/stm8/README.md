# STM8 — SDK Port

**Tier:** port (production via generic_node.c)

Port `generic_node.c` + `hermes_stub_sensor.c` via STVD or STM8Cube.

Reference UART node: `pio run -e stm32f1 -d hardware`

Workaround flash (8-bit UART test): stm32f1 build.

All sensor IDs from App Studio `set_config` are parsed and reported in telemetry JSON.
