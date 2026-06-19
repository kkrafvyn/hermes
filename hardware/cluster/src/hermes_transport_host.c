/**
 * Host transport — ring-buffer bus for multi-node simulation on PC.
 * Used by hardware/host and cluster-sim C builds.
 */
#include "hermes_link.h"
#include <string.h>

#define HERMES_HOST_MAX_NODES 8
#define HERMES_HOST_QUEUE_DEPTH 32

typedef struct {
    uint8_t data[sizeof(hermes_frame_t)];
    size_t len;
} host_queue_entry_t;

typedef struct {
    host_queue_entry_t entries[HERMES_HOST_QUEUE_DEPTH];
    int head, tail, count;
} host_queue_t;

static host_queue_t g_host_queues[HERMES_HOST_MAX_NODES];

static void host_queue_push(host_queue_t *q, const uint8_t *data, size_t len)
{
    if (q->count >= HERMES_HOST_QUEUE_DEPTH) return;
    memcpy(q->entries[q->tail].data, data, len);
    q->entries[q->tail].len = len;
    q->tail = (q->tail + 1) % HERMES_HOST_QUEUE_DEPTH;
    q->count++;
}

static int host_queue_pop(host_queue_t *q, uint8_t *data, size_t max_len)
{
    if (q->count == 0) return 0;
    size_t len = q->entries[q->head].len;
    if (len > max_len) len = max_len;
    memcpy(data, q->entries[q->head].data, len);
    q->head = (q->head + 1) % HERMES_HOST_QUEUE_DEPTH;
    q->count--;
    return (int)len;
}

static int host_send(const uint8_t *data, size_t len, void *ctx)
{
    (void)ctx;
    hermes_frame_t *frame = (hermes_frame_t *)data;
    uint8_t dst = frame->header.dst_node;
    if (dst >= HERMES_HOST_MAX_NODES) return -1;
    host_queue_push(&g_host_queues[dst], data, len);
    return (int)len;
}

static int host_recv(uint8_t *data, size_t max_len, uint32_t timeout_ms, void *ctx)
{
    (void)timeout_ms;
    uint8_t node_id = *(uint8_t *)ctx;
    return host_queue_pop(&g_host_queues[node_id], data, max_len);
}

int hermes_transport_host_init(hermes_link_t *link, uint8_t node_id, hermes_node_type_t type)
{
    static uint8_t ids[HERMES_HOST_MAX_NODES];
    ids[node_id] = node_id;
    hermes_link_transport_t t = {
        .send = host_send,
        .recv = host_recv,
        .ctx = &ids[node_id],
        .transport = HERMES_TRANSPORT_SPI,
    };
    return hermes_link_init(link, &t, node_id, type);
}

void hermes_transport_host_reset(void)
{
    memset(g_host_queues, 0, sizeof(g_host_queues));
}
