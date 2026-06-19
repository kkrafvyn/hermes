/**
 * nRF52840 cluster node — UART + HERMES runtime port.
 */
#include "hermes_link.h"
#include "hermes_node_port.h"

extern int nrf_uart_send(const uint8_t *data, size_t len, void *ctx);
extern int nrf_uart_recv(uint8_t *data, size_t max_len, uint32_t timeout_ms, void *ctx);

static hermes_link_t g_link;
static uint32_t g_tick;

void hermes_node_init(void)
{
    hermes_link_transport_t transport = {
        .send = nrf_uart_send,
        .recv = nrf_uart_recv,
        .ctx = NULL,
        .transport = HERMES_TRANSPORT_UART,
    };
    hermes_link_init(&g_link, &transport, 2, HERMES_NODE_NRF52840);
    hermes_node_port_init(&g_link, 2, "data/checkpoints");
}

void hermes_node_loop(void)
{
    hermes_frame_t frame;
    if (hermes_link_recv(&g_link, &frame, 10) == 0)
        hermes_link_handle_frame(&g_link, &frame);
    hermes_link_send_heartbeat(&g_link, 1);
    if (++g_tick % 100 == 0)
        hermes_node_port_loop();
}
