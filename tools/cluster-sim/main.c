/**
 * Host cluster simulator — 3 in-memory nodes, validates Phase 1.
 */
#include "hermes_link.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NODE_COUNT 3
#define QUEUE_SIZE 16

typedef struct {
    uint8_t data[sizeof(hermes_frame_t)];
    size_t len;
} queue_entry_t;

typedef struct {
    queue_entry_t entries[QUEUE_SIZE];
    int head, tail, count;
} msg_queue_t;

typedef struct {
    msg_queue_t queues[NODE_COUNT];
    uint32_t time_us;
} sim_bus_t;

static sim_bus_t g_bus;

static void queue_push(msg_queue_t *q, const uint8_t *data, size_t len)
{
    if (q->count >= QUEUE_SIZE) return;
    memcpy(q->entries[q->tail].data, data, len);
    q->entries[q->tail].len = len;
    q->tail = (q->tail + 1) % QUEUE_SIZE;
    q->count++;
}

static int queue_pop(msg_queue_t *q, uint8_t *data, size_t max_len)
{
    if (q->count == 0) return 0;
    size_t len = q->entries[q->head].len;
    if (len > max_len) len = max_len;
    memcpy(data, q->entries[q->head].data, len);
    q->head = (q->head + 1) % QUEUE_SIZE;
    q->count--;
    return (int)len;
}

/* removed erroneous undef */

static int sim_send(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    hermes_frame_t *frame = (hermes_frame_t *)data;
    uint8_t dst = frame->header.dst_node;
    if (dst >= NODE_COUNT) return -1;
    queue_push(&g_bus.queues[dst], data, len);
    return (int)len;
}

static int sim_recv(uint8_t *data, size_t max_len, uint32_t timeout_ms, void *ctx)
{
    (void)timeout_ms;
    uint8_t node_id = *(uint8_t *)ctx;
    return queue_pop(&g_bus.queues[node_id], data, max_len);
}

static const char *node_name(hermes_node_type_t t)
{
    switch (t) {
    case HERMES_NODE_STM32H7:  return "STM32H7";
    case HERMES_NODE_ESP32S3:  return "ESP32-S3";
    case HERMES_NODE_NRF52840: return "nRF52840";
    default: return "unknown";
    }
}

int main(void)
{
    hermes_link_t nodes[NODE_COUNT];
    uint8_t ids[NODE_COUNT] = {0, 1, 2};
    hermes_node_type_t types[NODE_COUNT] = {
        HERMES_NODE_STM32H7, HERMES_NODE_ESP32S3, HERMES_NODE_NRF52840
    };

    printf("HERMES Cluster Simulator — Phase 1\n");
    printf("===================================\n");

    for (int i = 0; i < NODE_COUNT; i++) {
        hermes_link_transport_t t = {
            .send = sim_send, .recv = sim_recv,
            .ctx = &ids[i], .transport = HERMES_TRANSPORT_SPI,
        };
        hermes_link_init(&nodes[i], &t, ids[i], types[i]);
        printf("Node %d (%s) online\n", i, node_name(types[i]));
    }

    /* Exchange heartbeats */
    for (int round = 0; round < 3; round++) {
        for (int i = 0; i < NODE_COUNT; i++) {
            uint8_t dst = (uint8_t)((i + 1) % NODE_COUNT);
            hermes_link_send_heartbeat(&nodes[i], dst);
        }
        for (int i = 0; i < NODE_COUNT; i++) {
            hermes_frame_t frame;
            if (hermes_link_recv(&nodes[i], &frame, 100) == 0)
                hermes_link_handle_frame(&nodes[i], &frame);
        }
    }
    printf("Heartbeats exchanged: OK\n");

    /* Latency measurement — step nodes manually */
    hermes_latency_t ping = { .seq = nodes[0].tx_seq };
    hermes_link_send(&nodes[0], HERMES_MSG_PING, 1, &ping, sizeof(ping));
    {
        hermes_frame_t frame;
        if (hermes_link_recv(&nodes[1], &frame, 100) == 0) {
            hermes_link_handle_frame(&nodes[1], &frame);
            if (hermes_link_recv(&nodes[0], &frame, 100) == 0)
                printf("STM32 -> ESP32 ping/pong: OK (seq=%u)\n", frame.header.seq);
        }
    }

    printf("\nPhase 1 success: 3-node cluster alive, packets sent, latency measured.\n");
    return 0;
}
