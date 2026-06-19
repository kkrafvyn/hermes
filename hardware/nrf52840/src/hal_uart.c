/**
 * nRF52840 UART HAL — reference for Zephyr uart_driver_api.
 */
#include "hermes_link.h"

static hermes_link_t *g_uart_link;

void nrf_uart_hal_bind(hermes_link_t *link)
{
    g_uart_link = link;
}

int nrf_uart_send(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    if (!g_uart_link) return -1;
    return g_uart_link->transport.send(data, len, g_uart_link->transport.ctx);
}

int nrf_uart_recv(uint8_t *data, size_t max_len, uint32_t timeout_ms, void *ctx)
{
    (void)ctx;
    if (!g_uart_link) return -1;
    return g_uart_link->transport.recv(data, max_len, timeout_ms, g_uart_link->transport.ctx);
}
