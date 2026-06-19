#ifndef HERMES_MIGRATION_H
#define HERMES_MIGRATION_H

#include "hermes_checkpoint.h"
#include "hermes_link.h"

typedef enum {
    HERMES_MIG_IDLE       = 0,
    HERMES_MIG_PAUSING    = 1,
    HERMES_MIG_CHECKPOINT = 2,
    HERMES_MIG_TRANSFER   = 3,
    HERMES_MIG_RESTORE    = 4,
    HERMES_MIG_RESUME     = 5,
    HERMES_MIG_DONE       = 6,
    HERMES_MIG_FAILED     = 7,
} hermes_migration_state_t;

typedef struct {
    char app_id[32];
    uint8_t src_node;
    uint8_t dst_node;
    hermes_migration_state_t state;
    hermes_checkpoint_t checkpoint;
    hermes_link_t *link;
} hermes_migration_t;

int hermes_migration_begin(hermes_migration_t *mig, const char *app_id,
                           uint8_t src_node, uint8_t dst_node, hermes_link_t *link);

int hermes_migration_step(hermes_migration_t *mig,
                          hermes_state_serialize_fn serialize,
                          hermes_state_restore_fn restore,
                          void *app_ctx);

int hermes_migration_rollback(hermes_migration_t *mig);

#endif /* HERMES_MIGRATION_H */
