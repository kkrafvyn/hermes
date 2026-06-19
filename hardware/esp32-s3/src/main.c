/**
 * ESP32-S3 cluster node — SPI slave + BLE peripheral.
 * Build with ESP-IDF v5.x.
 */
#include "hermes_link.h"

extern int esp32_spi_send(const uint8_t *data, size_t len, void *ctx);
extern int esp32_spi_recv(uint8_t *data, size_t max_len, uint32_t timeout_ms, void *ctx);

static hermes_link_t g_link;

void hermes_node_init(void)
{
    hermes_link_transport_t transport = {
        .send = esp32_spi_send,
        .recv = esp32_spi_recv,
        .ctx = NULL,
        .transport = HERMES_TRANSPORT_SPI,
    };
    hermes_link_init(&g_link, &transport, 1, HERMES_NODE_ESP32S3);
}

void hermes_node_loop(void)
{
    hermes_frame_t frame;
    if (hermes_link_recv(&g_link, &frame, 10) == 0)
        hermes_link_handle_frame(&g_link, &frame);
    hermes_link_send_heartbeat(&g_link, 2);
}
