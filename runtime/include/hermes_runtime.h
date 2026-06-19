#ifndef HERMES_RUNTIME_H
#define HERMES_RUNTIME_H

#include <stdint.h>
#include <stdbool.h>
#include "hermes_link.h"
#include "hermes_task.h"

typedef struct hermes_runtime hermes_runtime_t;

typedef struct {
    float cpu_percent;
    uint32_t ram_used;
    uint32_t ram_total;
    float power_mw;
    float temp_c;
    uint32_t deadline_ms;
} hermes_resource_snapshot_t;

typedef struct {
    char app_id[32];
    uint8_t node_id;
    uint8_t priority;
    uint32_t memory_kb;
    float cpu_percent;
    bool running;
} hermes_runtime_app_t;

int  hermes_runtime_init(hermes_runtime_t **out, hermes_link_t *link, uint8_t node_id);
void hermes_runtime_deinit(hermes_runtime_t *rt);

int  hermes_runtime_install(hermes_runtime_t *rt, const char *app_id,
                            const uint8_t *binary, uint32_t size);
int  hermes_runtime_start(hermes_runtime_t *rt, const char *app_id);
int  hermes_runtime_pause(hermes_runtime_t *rt, const char *app_id);
int  hermes_runtime_stop(hermes_runtime_t *rt, const char *app_id);
int  hermes_runtime_move(hermes_runtime_t *rt, const char *app_id, uint8_t dst_node);

int  hermes_runtime_snapshot(hermes_runtime_t *rt, hermes_resource_snapshot_t *out);
int  hermes_runtime_list_apps(hermes_runtime_t *rt, hermes_runtime_app_t *apps,
                              uint32_t max, uint32_t *count);
void hermes_runtime_enforce_quotas(hermes_runtime_t *rt, hermes_task_manager_t *tasks);

#endif /* HERMES_RUNTIME_H */
