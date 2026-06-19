#include "hermes_quota.h"
#include <stdbool.h>
#include <string.h>

int hermes_quota_init(hermes_quota_manager_t *qm)
{
    if (!qm) return -1;
    memset(qm, 0, sizeof(*qm));
    return 0;
}

int hermes_quota_set(hermes_quota_manager_t *qm, const char *app_id,
                     float cpu_percent, uint32_t memory_kb)
{
    if (!qm || !app_id) return -1;
    hermes_quota_t *q = hermes_quota_find(qm, app_id);
    if (!q) {
        if (qm->count >= HERMES_MAX_QUOTAS) return -2;
        q = &qm->quotas[qm->count++];
        strncpy(q->app_id, app_id, sizeof(q->app_id) - 1);
        q->migration_allowed = true;
    }
    q->cpu_percent = cpu_percent;
    q->memory_kb = memory_kb;
    return 0;
}

int hermes_quota_check_cpu(hermes_quota_manager_t *qm, const char *app_id,
                           float current_cpu)
{
    hermes_quota_t *q = hermes_quota_find(qm, app_id);
    if (!q) return -1;
    return current_cpu <= q->cpu_percent ? 0 : -2;
}

int hermes_quota_check_memory(hermes_quota_manager_t *qm, const char *app_id,
                              uint32_t current_kb)
{
    hermes_quota_t *q = hermes_quota_find(qm, app_id);
    if (!q) return -1;
    return current_kb <= q->memory_kb ? 0 : -2;
}

hermes_quota_t *hermes_quota_find(hermes_quota_manager_t *qm, const char *app_id)
{
    if (!qm || !app_id) return NULL;
    for (uint32_t i = 0; i < qm->count; i++)
        if (strcmp(qm->quotas[i].app_id, app_id) == 0) return &qm->quotas[i];
    return NULL;
}

int hermes_quota_enforce(hermes_quota_manager_t *qm, const char *app_id,
                         float current_cpu, uint32_t current_kb,
                         bool *throttle_out)
{
    if (throttle_out) *throttle_out = false;
    hermes_quota_t *q = hermes_quota_find(qm, app_id);
    if (!q) return -1;
    if (current_cpu > q->cpu_percent || current_kb > q->memory_kb) {
        if (throttle_out) *throttle_out = true;
        return -2;
    }
    return 0;
}
