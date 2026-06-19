#include "hermes_link.h"
#include <string.h>

static uint32_t crc32_table[256];
static int crc32_init_done;

static void init_crc32_table(void)
{
    if (crc32_init_done) return;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++)
            crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320u : crc >> 1;
        crc32_table[i] = crc;
    }
    crc32_init_done = 1;
}

uint32_t hermes_link_crc32(const uint8_t *data, size_t len)
{
    init_crc32_table();
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++)
        crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
    return crc ^ 0xFFFFFFFFu;
}

int hermes_link_init(hermes_link_t *link, const hermes_link_transport_t *transport,
                     uint8_t node_id, hermes_node_type_t node_type)
{
    if (!link || !transport) return -1;
    memset(link, 0, sizeof(*link));
    link->transport = *transport;
    link->node_id = node_id;
    link->node_type = node_type;
    return 0;
}

static int build_frame(hermes_link_t *link, hermes_msg_type_t type, uint8_t dst_node,
                       const void *payload, uint16_t payload_len, hermes_frame_t *frame)
{
    if (payload_len > HERMES_LINK_MAX_PAYLOAD) return -1;
    memset(frame, 0, sizeof(*frame));
    frame->header.magic = HERMES_LINK_MAGIC;
    frame->header.version = HERMES_LINK_VERSION;
    frame->header.type = (uint8_t)type;
    frame->header.src_node = link->node_id;
    frame->header.dst_node = dst_node;
    frame->header.seq = link->tx_seq++;
    frame->header.payload_len = payload_len;
    if (payload && payload_len > 0)
        memcpy(frame->payload, payload, payload_len);
    frame->crc32 = hermes_link_crc32((const uint8_t *)frame,
        sizeof(hermes_frame_header_t) + payload_len);
    return 0;
}

int hermes_link_send(hermes_link_t *link, hermes_msg_type_t type,
                     uint8_t dst_node, const void *payload, uint16_t payload_len)
{
    hermes_frame_t frame;
    if (build_frame(link, type, dst_node, payload, payload_len, &frame) != 0)
        return -1;
    size_t total = sizeof(hermes_frame_header_t) + payload_len + sizeof(uint32_t);
    return link->transport.send((const uint8_t *)&frame, total, link->transport.ctx);
}

int hermes_link_recv(hermes_link_t *link, hermes_frame_t *frame, uint32_t timeout_ms)
{
    if (!link || !frame) return -1;
    size_t need = sizeof(hermes_frame_header_t) + sizeof(uint32_t);
    int n = link->transport.recv((uint8_t *)frame, need + HERMES_LINK_MAX_PAYLOAD,
                                 timeout_ms, link->transport.ctx);
    if (n < (int)need) return -1;
    if (frame->header.magic != HERMES_LINK_MAGIC) return -2;
    if (frame->header.version != HERMES_LINK_VERSION) return -3;
    uint32_t expected = hermes_link_crc32((const uint8_t *)frame,
        sizeof(hermes_frame_header_t) + frame->header.payload_len);
    if (frame->crc32 != expected) return -4;
    link->rx_seq = frame->header.seq;
    return 0;
}

int hermes_link_send_heartbeat(hermes_link_t *link, uint8_t dst_node)
{
    hermes_heartbeat_t hb = {
        .timestamp_us = 0,
        .node_id = link->node_id,
        .node_type = (uint8_t)link->node_type,
        .link_quality = 100,
    };
    return hermes_link_send(link, HERMES_MSG_HEARTBEAT, dst_node, &hb, sizeof(hb));
}

int hermes_link_ping(hermes_link_t *link, uint8_t dst_node, hermes_latency_t *out)
{
    hermes_latency_t ping = { .send_ts_us = 0, .seq = link->tx_seq };
    if (hermes_link_send(link, HERMES_MSG_PING, dst_node, &ping, sizeof(ping)) != 0)
        return -1;
    hermes_frame_t frame;
    if (hermes_link_recv(link, &frame, 1000) != 0) return -2;
    if (frame.header.type != HERMES_MSG_PONG) return -3;
    if (out) {
        memcpy(out, frame.payload, sizeof(hermes_latency_t));
        link->last_latency = *out;
    }
    return 0;
}

int hermes_link_handle_frame(hermes_link_t *link, const hermes_frame_t *frame)
{
    if (!link || !frame) return -1;
    switch (frame->header.type) {
    case HERMES_MSG_PING: {
        hermes_latency_t pong;
        memcpy(&pong, frame->payload, sizeof(pong));
        pong.recv_ts_us = 0;
        pong.rtt_us = 0;
        return hermes_link_send(link, HERMES_MSG_PONG, frame->header.src_node, &pong, sizeof(pong));
    }
    case HERMES_MSG_PONG:
        if (frame->header.payload_len >= sizeof(hermes_latency_t))
            memcpy(&link->last_latency, frame->payload, sizeof(hermes_latency_t));
        return 0;
    case HERMES_MSG_HEARTBEAT:
        return 0;
    case HERMES_MSG_CHECKPOINT:
        if (link->checkpoint_handler &&
            frame->header.payload_len >= sizeof(hermes_checkpoint_envelope_t)) {
            const hermes_checkpoint_envelope_t *env =
                (const hermes_checkpoint_envelope_t *)frame->payload;
            link->checkpoint_handler(link->checkpoint_ctx, env);
            return hermes_link_send(link, HERMES_MSG_ACK, frame->header.src_node, NULL, 0);
        }
        return -3;
    default:
        return 0;
    }
}

void hermes_link_set_checkpoint_handler(hermes_link_t *link, void *ctx,
    void (*handler)(void *ctx, const hermes_checkpoint_envelope_t *env))
{
    if (!link) return;
    link->checkpoint_ctx = ctx;
    link->checkpoint_handler = handler;
}

int hermes_link_send_reliable(hermes_link_t *link, hermes_msg_type_t type,
                              uint8_t dst_node, const void *payload,
                              uint16_t payload_len, uint8_t max_retries)
{
    for (uint8_t attempt = 0; attempt <= max_retries; attempt++) {
        if (hermes_link_send(link, type, dst_node, payload, payload_len) < 0)
            continue;
        hermes_frame_t ack;
        if (hermes_link_recv(link, &ack, 500) == 0 &&
            ack.header.type == HERMES_MSG_ACK)
            return 0;
    }
    return -1;
}
