#include "hermes_ipc.h"
#include <string.h>
#include <stdlib.h>

int hermes_ipc_init(hermes_ipc_t *ipc)
{
    if (!ipc) return -1;
    memset(ipc, 0, sizeof(*ipc));
    return 0;
}

int hermes_ipc_create_channel(hermes_ipc_t *ipc, const char *name,
                              size_t msg_size, uint32_t *out_id)
{
    if (!ipc || !name || ipc->channel_count >= HERMES_IPC_MAX_CHANNELS) return -1;
    hermes_ipc_channel_t *ch = &ipc->channels[ipc->channel_count];
    ch->id = ipc->channel_count + 1;
    strncpy(ch->name, name, sizeof(ch->name) - 1);
    ch->msg_size = msg_size;
    ch->buffer = calloc(HERMES_IPC_QUEUE_DEPTH, msg_size);
    if (!ch->buffer) return -2;
    if (out_id) *out_id = ch->id;
    ipc->channel_count++;
    return 0;
}

int hermes_ipc_send(hermes_ipc_t *ipc, uint32_t channel_id,
                    const void *msg, uint32_t timeout_ms)
{
    (void)timeout_ms;
    if (!ipc || !msg) return -1;
    for (uint32_t i = 0; i < ipc->channel_count; i++) {
        hermes_ipc_channel_t *ch = &ipc->channels[i];
        if (ch->id != channel_id) continue;
        if (ch->count >= HERMES_IPC_QUEUE_DEPTH) return -2;
        uint8_t *slot = ch->buffer + (size_t)ch->tail * ch->msg_size;
        memcpy(slot, msg, ch->msg_size);
        ch->tail = (ch->tail + 1) % HERMES_IPC_QUEUE_DEPTH;
        ch->count++;
        return 0;
    }
    return -1;
}

int hermes_ipc_recv(hermes_ipc_t *ipc, uint32_t channel_id,
                    void *msg, uint32_t timeout_ms)
{
    (void)timeout_ms;
    if (!ipc || !msg) return -1;
    for (uint32_t i = 0; i < ipc->channel_count; i++) {
        hermes_ipc_channel_t *ch = &ipc->channels[i];
        if (ch->id != channel_id) continue;
        if (ch->count == 0) return -2;
        uint8_t *slot = ch->buffer + (size_t)ch->head * ch->msg_size;
        memcpy(msg, slot, ch->msg_size);
        ch->head = (ch->head + 1) % HERMES_IPC_QUEUE_DEPTH;
        ch->count--;
        return 0;
    }
    return -1;
}
