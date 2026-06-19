#ifndef HERMES_LINK_H
#define HERMES_LINK_H

#include "hermes_checkpoint.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define HERMES_LINK_MAGIC     0x484D  /* "HM" */
#define HERMES_LINK_VERSION   1
#define HERMES_LINK_MAX_PAYLOAD 512

typedef enum {
    HERMES_TRANSPORT_SPI  = 0,
    HERMES_TRANSPORT_BLE  = 1,
    HERMES_TRANSPORT_UART = 2,
} hermes_transport_t;

typedef enum {
    HERMES_MSG_HEARTBEAT    = 0x01,
    HERMES_MSG_PING         = 0x02,
    HERMES_MSG_PONG         = 0x03,
    HERMES_MSG_RESOURCE     = 0x10,
    HERMES_MSG_APP_CONTROL  = 0x20,
    HERMES_MSG_CHECKPOINT   = 0x30,
    HERMES_MSG_ACK          = 0xFE,
    HERMES_MSG_NACK         = 0xFF,
} hermes_msg_type_t;

typedef enum {
    HERMES_NODE_STM32H7   = 0,
    HERMES_NODE_ESP32S3   = 1,
    HERMES_NODE_NRF52840  = 2,
    HERMES_NODE_UNKNOWN   = 0xFF,
} hermes_node_type_t;

typedef struct {
    uint16_t magic;
    uint8_t  version;
    uint8_t  type;
    uint8_t  src_node;
    uint8_t  dst_node;
    uint16_t seq;
    uint16_t payload_len;
} hermes_frame_header_t;

typedef struct {
    hermes_frame_header_t header;
    uint8_t payload[HERMES_LINK_MAX_PAYLOAD];
    uint32_t crc32;
} hermes_frame_t;

typedef struct {
    uint32_t timestamp_us;
    uint8_t  node_id;
    uint8_t  node_type;
    uint8_t  link_quality;
    uint8_t  reserved;
} hermes_heartbeat_t;

typedef struct {
    uint32_t send_ts_us;
    uint32_t recv_ts_us;
    uint16_t seq;
    uint16_t rtt_us;
} hermes_latency_t;

typedef struct {
    int (*send)(const uint8_t *data, size_t len, void *ctx);
    int (*recv)(uint8_t *data, size_t max_len, uint32_t timeout_ms, void *ctx);
    void *ctx;
    hermes_transport_t transport;
} hermes_link_transport_t;

typedef struct {
    hermes_link_transport_t transport;
    uint8_t node_id;
    hermes_node_type_t node_type;
    uint16_t tx_seq;
    uint16_t rx_seq;
    hermes_latency_t last_latency;
    void *checkpoint_ctx;
    void (*checkpoint_handler)(void *ctx, const hermes_checkpoint_envelope_t *env);
} hermes_link_t;

uint32_t hermes_link_crc32(const uint8_t *data, size_t len);

int hermes_link_init(hermes_link_t *link, const hermes_link_transport_t *transport,
                     uint8_t node_id, hermes_node_type_t node_type);

int hermes_link_send(hermes_link_t *link, hermes_msg_type_t type,
                     uint8_t dst_node, const void *payload, uint16_t payload_len);

int hermes_link_recv(hermes_link_t *link, hermes_frame_t *frame, uint32_t timeout_ms);

int hermes_link_send_heartbeat(hermes_link_t *link, uint8_t dst_node);

int hermes_link_ping(hermes_link_t *link, uint8_t dst_node, hermes_latency_t *out);

int hermes_link_handle_frame(hermes_link_t *link, const hermes_frame_t *frame);

int hermes_link_send_reliable(hermes_link_t *link, hermes_msg_type_t type,
                              uint8_t dst_node, const void *payload,
                              uint16_t payload_len, uint8_t max_retries);

void hermes_link_set_checkpoint_handler(hermes_link_t *link, void *ctx,
    void (*handler)(void *ctx, const hermes_checkpoint_envelope_t *env));

#endif /* HERMES_LINK_H */
