/**
 * STM32H7 SPI+DMA HAL reference implementation.
 * Replace SPI handle with your CubeMX hspi1 instance.
 */
#include "hermes_link.h"
#include <string.h>

static hermes_link_t *g_spi_link;

void stm32_spi_hal_bind(hermes_link_t *link)
{
    g_spi_link = link;
}

int stm32_spi_send(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    if (!g_spi_link) return -1;
    /* Production: HAL_SPI_Transmit_DMA(&hspi1, (uint8_t*)data, len); */
    return g_spi_link->transport.send(data, len, g_spi_link->transport.ctx);
}

int stm32_spi_recv(uint8_t *data, size_t max_len, uint32_t timeout_ms, void *ctx)
{
    (void)ctx;
    if (!g_spi_link) return -1;
    /* Production: HAL_SPI_Receive(&hspi1, data, max_len, timeout_ms); */
    return g_spi_link->transport.recv(data, max_len, timeout_ms, g_spi_link->transport.ctx);
}

uint32_t stm32_get_time_us(void)
{
    /* Production: return DWT->CYCCNT / (SystemCoreClock / 1000000); */
    return 0;
}
