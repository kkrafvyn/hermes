#include "hermes_runtime.h"
#include "hermes_monitor.h"
#include "hermes_scheduler.h"
#include "hermes_quota.h"
#include "hermes_ota.h"
#include "hermes_sha256.h"
#include "hermes_task.h"
#include <stdlib.h>
#include <string.h>

#define HERMES_MAX_APPS 8

struct hermes_runtime {
    hermes_link_t *link;
    uint8_t node_id;
    hermes_monitor_t monitor;
    hermes_scheduler_t scheduler;
    hermes_quota_manager_t quotas;
    hermes_runtime_app_t apps[HERMES_MAX_APPS];
    uint32_t app_count;
    hermes_resource_snapshot_t resources;
};

int hermes_runtime_init(hermes_runtime_t **out, hermes_link_t *link, uint8_t node_id)
{
    if (!out || !link) return -1;
    hermes_runtime_t *rt = calloc(1, sizeof(*rt));
    if (!rt) return -2;
    rt->link = link;
    rt->node_id = node_id;
    hermes_monitor_init(&rt->monitor, 1000);
    hermes_scheduler_init(&rt->scheduler);
    hermes_quota_init(&rt->quotas);
    rt->resources.ram_total = 512 * 1024;
    *out = rt;
    return 0;
}

void hermes_runtime_deinit(hermes_runtime_t *rt)
{
    free(rt);
}

static hermes_runtime_app_t *find_app(hermes_runtime_t *rt, const char *app_id)
{
    for (uint32_t i = 0; i < rt->app_count; i++)
        if (strcmp(rt->apps[i].app_id, app_id) == 0) return &rt->apps[i];
    return NULL;
}

int hermes_runtime_install(hermes_runtime_t *rt, const char *app_id,
                           const uint8_t *binary, uint32_t size)
{
    if (!rt || !app_id || rt->app_count >= HERMES_MAX_APPS) return -1;
    if (find_app(rt, app_id)) return -2;

    if (binary && size > 0) {
        hermes_ota_t ota;
        uint8_t hash[32] = {0};
        hermes_sha256(binary, size, hash);
        if (hermes_ota_begin(&ota, app_id, "1.0.0", size, hash) != 0) return -3;
        if (hermes_ota_write(&ota, binary, size) != 0) return -4;
        if (hermes_ota_finalize(&ota) != 0) return -5;
        if (hermes_ota_apply(&ota, app_id) != 0) return -6;
    }

    hermes_runtime_app_t *a = &rt->apps[rt->app_count++];
    strncpy(a->app_id, app_id, sizeof(a->app_id) - 1);
    a->node_id = rt->node_id;
    a->running = false;
    a->memory_kb = size > 0 ? (size / 1024) + 1 : 64;
    a->cpu_percent = 0;
    hermes_quota_set(&rt->quotas, app_id, 30.0f, a->memory_kb);
    return 0;
}

int hermes_runtime_start(hermes_runtime_t *rt, const char *app_id)
{
    hermes_runtime_app_t *a = find_app(rt, app_id);
    if (!a) return -1;
    a->running = true;
    a->cpu_percent = 10.0f;
    return 0;
}

int hermes_runtime_pause(hermes_runtime_t *rt, const char *app_id)
{
    hermes_runtime_app_t *a = find_app(rt, app_id);
    if (!a || !a->running) return -1;
    a->running = false;
    return 0;
}

int hermes_runtime_stop(hermes_runtime_t *rt, const char *app_id)
{
    hermes_runtime_app_t *a = find_app(rt, app_id);
    if (!a) return -1;
    a->running = false;
    return 0;
}

int hermes_runtime_move(hermes_runtime_t *rt, const char *app_id, uint8_t dst_node)
{
    hermes_runtime_app_t *a = find_app(rt, app_id);
    if (!a) return -1;
    a->node_id = dst_node;
    return 0;
}

int hermes_runtime_snapshot(hermes_runtime_t *rt, hermes_resource_snapshot_t *out)
{
    if (!rt || !out) return -1;
    *out = rt->resources;
    hermes_monitor_sample(&rt->monitor, out);
    return 0;
}

int hermes_runtime_list_apps(hermes_runtime_t *rt, hermes_runtime_app_t *apps,
                             uint32_t max, uint32_t *count)
{
    if (!rt || !apps || !count) return -1;
    uint32_t n = rt->app_count < max ? rt->app_count : max;
    memcpy(apps, rt->apps, n * sizeof(hermes_runtime_app_t));
    *count = n;
    return 0;
}

void hermes_runtime_enforce_quotas(hermes_runtime_t *rt, hermes_task_manager_t *tasks)
{
    if (!rt) return;
    for (uint32_t i = 0; i < rt->app_count; i++) {
        hermes_runtime_app_t *a = &rt->apps[i];
        if (!a->running) continue;
        bool throttle = false;
        if (hermes_quota_enforce(&rt->quotas, a->app_id, a->cpu_percent,
                                 a->memory_kb, &throttle) != 0 && throttle && tasks) {
            hermes_task_t *t = hermes_get_task(tasks, i + 1);
            if (t && t->state == HERMES_TASK_RUNNING)
                hermes_pause_task(tasks, t->id);
            a->running = false;
        }
    }
}
