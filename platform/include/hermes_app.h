#ifndef HERMES_APP_H
#define HERMES_APP_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HERMES_APP_INSTALLED = 0,
    HERMES_APP_RUNNING   = 1,
    HERMES_APP_PAUSED    = 2,
    HERMES_APP_STOPPED   = 3,
    HERMES_APP_UPDATING  = 4,
} hermes_app_state_t;

typedef struct {
    char name[32];
    char version[16];
    char targets[64];
    uint32_t memory_kb;
    uint8_t priority;
} hermes_app_manifest_t;

typedef struct {
    hermes_app_manifest_t manifest;
    hermes_app_state_t state;
    uint8_t node_id;
    uint32_t install_ts;
    char previous_version[16];
} hermes_app_t;

#define HERMES_MAX_APPS_MGR 16

typedef struct {
    hermes_app_t apps[HERMES_MAX_APPS_MGR];
    uint32_t count;
} hermes_app_manager_t;

int  hermes_app_manager_init(hermes_app_manager_t *mgr);
int  hermes_app_install(hermes_app_manager_t *mgr, const hermes_app_manifest_t *manifest);
int  hermes_app_remove(hermes_app_manager_t *mgr, const char *name);
int  hermes_app_start(hermes_app_manager_t *mgr, const char *name);
int  hermes_app_pause(hermes_app_manager_t *mgr, const char *name);
int  hermes_app_stop(hermes_app_manager_t *mgr, const char *name);
int  hermes_app_update(hermes_app_manager_t *mgr, const char *name,
                       const hermes_app_manifest_t *new_manifest);
int  hermes_app_rollback(hermes_app_manager_t *mgr, const char *name);
int  hermes_app_enable(hermes_app_manager_t *mgr, const char *name, bool enabled);
hermes_app_t *hermes_app_find(hermes_app_manager_t *mgr, const char *name);

int  hermes_app_parse_manifest(const char *json, hermes_app_manifest_t *out);

#endif /* HERMES_APP_H */
