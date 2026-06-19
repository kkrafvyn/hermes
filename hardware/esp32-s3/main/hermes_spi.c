/**
 * SPI slave transport stub — replace with real SPI driver.
 */
#include "hermes_link.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "hermes_spi";

int esp32_spi_send(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    ESP_LOGD(TAG, "spi send %u bytes", (unsigned)len);
    return (int)len;
}

int esp32_spi_recv(uint8_t *data, size_t max_len, uint32_t timeout_ms, void *ctx)
{
    (void)data; (void)max_len; (void)timeout_ms; (void)ctx;
    return -1;
}
