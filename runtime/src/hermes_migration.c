#include "hermes_migration.h"
#include "hermes_checkpoint_io.h"
#include <string.h>

int hermes_migration_begin(hermes_migration_t *mig, const char *app_id,
                           uint8_t src_node, uint8_t dst_node, hermes_link_t *link)
{
    if (!mig || !app_id || !link) return -1;
    memset(mig, 0, sizeof(*mig));
    strncpy(mig->app_id, app_id, sizeof(mig->app_id) - 1);
    mig->src_node = src_node;
    mig->dst_node = dst_node;
    mig->link = link;
    mig->state = HERMES_MIG_PAUSING;
    return 0;
}

int hermes_migration_step(hermes_migration_t *mig,
                          hermes_state_serialize_fn serialize,
                          hermes_state_restore_fn restore,
                          void *app_ctx)
{
    if (!mig) return -1;
    switch (mig->state) {
    case HERMES_MIG_PAUSING:
        mig->state = HERMES_MIG_CHECKPOINT;
        return 0;
    case HERMES_MIG_CHECKPOINT: {
        uint8_t buf[HERMES_CHECKPOINT_MAX_SIZE];
        size_t written = 0;
        if (serialize && serialize(app_ctx, buf, sizeof(buf), &written) != 0) {
            mig->state = HERMES_MIG_FAILED;
            return -1;
        }
        hermes_checkpoint_create(mig->app_id, buf, written, NULL, 0, &mig->checkpoint);
        hermes_checkpoint_io_save(&mig->checkpoint);
        mig->state = HERMES_MIG_TRANSFER;
        return 0;
    }
    case HERMES_MIG_TRANSFER: {
        hermes_checkpoint_envelope_t env;
        memset(&env, 0, sizeof(env));
        strncpy(env.app_id, mig->app_id, sizeof(env.app_id) - 1);
        env.state_size = mig->checkpoint.state_size;
        env.version = mig->checkpoint.version;
        hermes_link_send(mig->link, HERMES_MSG_CHECKPOINT, mig->dst_node,
                         &env, (uint16_t)sizeof(env));
        mig->state = HERMES_MIG_RESTORE;
        return 0;
    }
    case HERMES_MIG_RESTORE:
        if (hermes_checkpoint_io_load(mig->app_id, &mig->checkpoint) != 0)
            mig->state = HERMES_MIG_FAILED;
        else if (restore)
            restore(app_ctx, mig->checkpoint.data, mig->checkpoint.state_size);
        mig->state = HERMES_MIG_RESUME;
        return 0;
    case HERMES_MIG_RESUME:
        mig->state = HERMES_MIG_DONE;
        return 0;
    default:
        return -2;
    }
}

int hermes_migration_rollback(hermes_migration_t *mig)
{
    if (!mig) return -1;
    mig->state = HERMES_MIG_FAILED;
    return 0;
}
