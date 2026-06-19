/**
 * STM32H7 cluster node — SPI + HERMES runtime port.
 */
#include "hermes_link.h"
#include "hermes_cluster_mgr.h"
#include "hermes_node_port.h"

extern int stm32_spi_send(const uint8_t *data, size_t len, void *ctx);
extern int stm32_spi_recv(uint8_t *data, size_t max_len, uint32_t timeout_ms, void *ctx);

static hermes_link_t g_link;
static hermes_cluster_mgr_t g_cluster;
static uint32_t g_tick;

void hermes_node_init(void)
{
    hermes_link_transport_t transport = {
        .send = stm32_spi_send,
        .recv = stm32_spi_recv,
        .ctx = NULL,
        .transport = HERMES_TRANSPORT_SPI,
    };
    hermes_link_init(&g_link, &transport, 0, HERMES_NODE_STM32H7);
    hermes_cluster_init(&g_cluster, &g_link);
    hermes_cluster_register(&g_cluster, 0, HERMES_NODE_STM32H7);
    hermes_node_port_init(&g_link, 0, "data/checkpoints");
}

void hermes_node_loop(void)
{
    hermes_frame_t frame;
    if (hermes_link_recv(&g_link, &frame, 10) == 0)
        hermes_link_handle_frame(&g_link, &frame);
    hermes_link_send_heartbeat(&g_link, 1);
    hermes_cluster_heartbeat(&g_cluster, 0);
    if (++g_tick % 100 == 0)
        hermes_node_port_loop();
}
