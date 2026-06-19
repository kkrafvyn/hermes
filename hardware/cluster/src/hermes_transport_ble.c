#include "hermes_transport_ble.h"
#include <string.h>

static hermes_ble_hal_t g_ble_hal;

static int ble_send(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    return g_ble_hal.send ? g_ble_hal.send(data, len, g_ble_hal.ctx) : -1;
}

static int ble_recv(uint8_t *data, size_t max_len, uint32_t timeout_ms, void *ctx)
{
    (void)ctx;
    return g_ble_hal.recv ? g_ble_hal.recv(data, max_len, timeout_ms, g_ble_hal.ctx) : -1;
}

int hermes_transport_ble_init(hermes_link_t *link, const hermes_ble_hal_t *hal,
                              uint8_t node_id)
{
    if (!link || !hal) return -1;
    g_ble_hal = *hal;
    if (hal->adv_start) hal->adv_start(hal->ctx);
    hermes_link_transport_t t = {
        .send = ble_send,
        .recv = ble_recv,
        .ctx = NULL,
        .transport = HERMES_TRANSPORT_BLE,
    };
    return hermes_link_init(link, &t, node_id, HERMES_NODE_NRF52840);
}
