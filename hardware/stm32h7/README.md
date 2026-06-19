# STM32H7 Cluster Node

Target: STM32H743 or compatible H7 series.

## Role

- AI inference workloads
- SPI master to ESP32-S3
- High-performance compute node

## Integration

1. Add `hermes/cluster` sources to your STM32CubeIDE project
2. Implement HAL hooks in `src/main.c`:
   - `stm32_spi_send()` / `stm32_spi_recv()`
   - `stm32_get_time_us()`
3. Call `hermes_node_init()` from `main()`, `hermes_node_loop()` from RTOS task

## Transport

SPI + DMA for inter-node link. UART for debug console.
