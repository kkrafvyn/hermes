# nRF52840 Cluster Node

Target: nRF52840 DK or compatible.

## Role

- Sensor network coordinator
- BLE mesh / central
- Low-power monitoring node

## Integration

1. Add HERMES sources as Zephyr module or lib
2. Implement `nrf_uart_send()` / `nrf_uart_recv()` in `src/main.c`
3. Call `hermes_node_init()` from main thread

## Build

```bash
west build -b nrf52840dk_nrf52840
west flash
```
