#include "hermes_node_runtime.h"
#include "hermes_checkpoint_io.h"
#include "hermes_freertos.h"
#include "hermes_migration.h"
#include "hermes_ota.h"
#include "hermes_quota.h"
#include <stdlib.h>
#include <string.h>

struct hermes_node_runtime {
    hermes_runtime_t *runtime;
    hermes_task_manager_t tasks;
    hermes_link_t *link;
    uint8_t node_id;
    char storage_path[128];
};

static int serialize_app(void *ctx, uint8_t *buf, size_t max, size_t *written)
{
    const char *app = (const char *)ctx;
    size_t len = strlen(app);
    if (len > max) return -1;
    memcpy(buf, app, len);
    *written = len;
    return 0;
}

static int restore_app(void *ctx, const uint8_t *buf, size_t len)
{
    (void)ctx;
    (void)buf;
    (void)len;
    return 0;
}

static void on_checkpoint(void *ctx, const hermes_checkpoint_envelope_t *env)
{
    if (!ctx || !env) return;
    hermes_checkpoint_t cp;
    if (hermes_checkpoint_io_load(env->app_id, &cp) == 0)
        hermes_node_runtime_on_checkpoint((hermes_node_runtime_t *)ctx, &cp);
}

int hermes_node_runtime_init(hermes_node_runtime_t **out, hermes_link_t *link,
                             uint8_t node_id, const char *storage_path)
{
    if (!out || !link) return -1;
    hermes_node_runtime_t *nr = calloc(1, sizeof(*nr));
    if (!nr) return -2;
    nr->link = link;
    nr->node_id = node_id;
    if (storage_path)
        strncpy(nr->storage_path, storage_path, sizeof(nr->storage_path) - 1);
    else
        strncpy(nr->storage_path, "data/checkpoints", sizeof(nr->storage_path) - 1);

    hermes_checkpoint_io_init(nr->storage_path);
    hermes_freertos_bind();
    hermes_task_manager_init(&nr->tasks);
    if (hermes_runtime_init(&nr->runtime, link, node_id) != 0) {
        free(nr);
        return -3;
    }
    hermes_link_set_checkpoint_handler(link, nr, on_checkpoint);
    *out = nr;
    return 0;
}

void hermes_node_runtime_deinit(hermes_node_runtime_t *nr)
{
    if (!nr) return;
    hermes_runtime_deinit(nr->runtime);
    free(nr);
}

void hermes_node_runtime_tick(hermes_node_runtime_t *nr)
{
    if (!nr || !nr->runtime) return;
    hermes_runtime_enforce_quotas(nr->runtime, &nr->tasks);
}

int hermes_node_runtime_dispatch(hermes_node_runtime_t *nr, const char *action,
                                 const char *app_name, const uint8_t *binary,
                                 uint32_t binary_size)
{
    if (!nr || !action) return -1;
    const char *app = app_name ? app_name : "";

    if (strcmp(action, "install_app") == 0) {
        return hermes_runtime_install(nr->runtime, app, binary, binary_size);
    }
    if (strcmp(action, "run_app") == 0) {
        uint32_t tid = 0;
        hermes_create_task(&nr->tasks, app, NULL, NULL, 4096, 5, &tid);
        hermes_freertos_start_task(&nr->tasks, tid);
        return hermes_runtime_start(nr->runtime, app);
    }
    if (strcmp(action, "pause_app") == 0) {
        hermes_task_t *t = hermes_get_task(&nr->tasks, 1);
        if (t) hermes_pause_task(&nr->tasks, t->id);
        return hermes_runtime_pause(nr->runtime, app);
    }
    if (strcmp(action, "stop_app") == 0)
        return hermes_runtime_stop(nr->runtime, app);
    if (strcmp(action, "checkpoint_app") == 0) {
        hermes_migration_t mig;
        hermes_migration_begin(&mig, app, nr->node_id, nr->node_id, nr->link);
        while (mig.state != HERMES_MIG_DONE && mig.state != HERMES_MIG_FAILED)
            hermes_migration_step(&mig, serialize_app, restore_app, (void *)app);
        hermes_checkpoint_io_save(&mig.checkpoint);
        return mig.state == HERMES_MIG_DONE ? 0 : -2;
    }
    if (strcmp(action, "restore_app") == 0) {
        hermes_checkpoint_t cp;
        if (hermes_checkpoint_io_load(app, &cp) != 0) return -2;
        restore_app((void *)app, cp.data, cp.state_size);
        return hermes_runtime_start(nr->runtime, app);
    }
    return -3;
}

hermes_runtime_t *hermes_node_runtime_get(hermes_node_runtime_t *nr)
{
    return nr ? nr->runtime : NULL;
}

hermes_task_manager_t *hermes_node_runtime_tasks(hermes_node_runtime_t *nr)
{
    return nr ? &nr->tasks : NULL;
}

void hermes_node_runtime_on_checkpoint(hermes_node_runtime_t *nr,
                                       const hermes_checkpoint_t *cp)
{
    if (!nr || !cp) return;
    hermes_checkpoint_io_save(cp);
    hermes_runtime_start(nr->runtime, cp->app_id);
}
