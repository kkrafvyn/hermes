#ifndef HERMES_CHECKPOINT_H
#define HERMES_CHECKPOINT_H

#include <stdint.h>
#include <stddef.h>

#define HERMES_CHECKPOINT_MAX_SIZE 4096
#define HERMES_CHECKPOINT_VERSION  1

typedef struct {
    uint32_t version;
    char app_id[32];
    uint32_t state_size;
    uint32_t config_size;
    uint32_t timestamp_us;
    uint8_t data[HERMES_CHECKPOINT_MAX_SIZE];
} hermes_checkpoint_t;

typedef int (*hermes_state_serialize_fn)(void *app_ctx, uint8_t *buf, size_t max, size_t *written);
typedef int (*hermes_state_restore_fn)(void *app_ctx, const uint8_t *buf, size_t len);

typedef struct {
    char app_id[32];
    uint32_t state_size;
    uint32_t version;
} hermes_checkpoint_envelope_t;

int hermes_checkpoint_create(const char *app_id, const uint8_t *state, size_t state_len,
                             const uint8_t *config, size_t config_len,
                             hermes_checkpoint_t *out);

int hermes_checkpoint_restore(const hermes_checkpoint_t *cp,
                              uint8_t *state_out, size_t state_max,
                              uint8_t *config_out, size_t config_max);

#endif /* HERMES_CHECKPOINT_H */
