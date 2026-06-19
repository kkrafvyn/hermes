#ifndef HERMES_TRANSPORT_BLE_H
#define HERMES_TRANSPORT_BLE_H

#include "hermes_link.h"

typedef struct {
    int (*adv_start)(void *ctx);
    int (*connect)(uint8_t node_id, void *ctx);
    int (*send)(const uint8_t *data, size_t len, void *ctx);
    int (*recv)(uint8_t *data, size_t max_len, uint32_t timeout_ms, void *ctx);
    void *ctx;
} hermes_ble_hal_t;

int hermes_transport_ble_init(hermes_link_t *link, const hermes_ble_hal_t *hal,
                             uint8_t node_id);

#endif
