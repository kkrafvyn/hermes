/**
 * Host HAL stubs for STM32 — routes SPI through hermes_transport_host bus.
 * Link with this file for PC simulation; omit on real STM32Cube builds.
 */
#include "hermes_link.h"
#include "hermes_transport_host.h"
#include <string.h>

static hermes_link_t *g_bus_link;

void stm32_hal_host_bind(hermes_link_t *link)
{
    g_bus_link = link;
}

int stm32_spi_send(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    if (!g_bus_link) return -1;
    return g_bus_link->transport.send(data, len, g_bus_link->transport.ctx);
}

int stm32_spi_recv(uint8_t *data, size_t max_len, uint32_t timeout_ms, void *ctx)
{
    (void)ctx;
    if (!g_bus_link) return -1;
    return g_bus_link->transport.recv(data, max_len, timeout_ms, g_bus_link->transport.ctx);
}
