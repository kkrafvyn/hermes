#ifndef HERMES_ROUTER_H
#define HERMES_ROUTER_H

#include "hermes_link.h"

/**
 * Gateway router — ESP32 forwards frames between SPI (STM32) and UART (nRF).
 */
typedef struct {
    hermes_link_t *spi_link;   /* to STM32, node 0 */
    hermes_link_t *uart_link;  /* to nRF, node 2 */
    uint8_t gateway_id;
} hermes_router_t;

int hermes_router_init(hermes_router_t *router, hermes_link_t *spi,
                       hermes_link_t *uart, uint8_t gateway_id);
int hermes_router_poll(hermes_router_t *router, uint32_t timeout_ms);

#endif
