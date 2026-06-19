#ifndef HERMES_IPC_H
#define HERMES_IPC_H

#include <stdint.h>
#include <stddef.h>

#define HERMES_IPC_MAX_CHANNELS 8
#define HERMES_IPC_QUEUE_DEPTH  16

typedef struct {
    uint32_t id;
    char name[24];
    uint8_t *buffer;
    size_t msg_size;
    int head, tail, count;
} hermes_ipc_channel_t;

typedef struct {
    hermes_ipc_channel_t channels[HERMES_IPC_MAX_CHANNELS];
    uint32_t channel_count;
} hermes_ipc_t;

int  hermes_ipc_init(hermes_ipc_t *ipc);
int  hermes_ipc_create_channel(hermes_ipc_t *ipc, const char *name,
                               size_t msg_size, uint32_t *out_id);
int  hermes_ipc_send(hermes_ipc_t *ipc, uint32_t channel_id,
                     const void *msg, uint32_t timeout_ms);
int  hermes_ipc_recv(hermes_ipc_t *ipc, uint32_t channel_id,
                     void *msg, uint32_t timeout_ms);

#endif /* HERMES_IPC_H */
