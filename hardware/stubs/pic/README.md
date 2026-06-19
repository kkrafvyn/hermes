# PIC16 / PIC18 — SDK Port

**Tier:** port (production via generic_node.c)

Port these files to MPLAB X:

- `hardware/stubs/generic_node.c`
- `hardware/stubs/hermes_stub_sensor.c`
- `hardware/stubs/hermes_stub_sensor.h`

Define at compile time:

```c
#define HERMES_STUB_MCU_NAME "pic16"
#define HERMES_STUB_FAMILY "pic"
#define HERMES_NODE_ID 1
```

Reference build: `pio run -e pic32 -d hardware` for protocol parity.

All 37 sensor IDs simulate in `hermes_stub_sensor.c` until real drivers are wired.
