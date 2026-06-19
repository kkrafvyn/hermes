#include "hermes_scheduler.h"
#include <string.h>

int hermes_scheduler_init(hermes_scheduler_t *sched)
{
    if (!sched) return -1;
    memset(sched, 0, sizeof(*sched));
    return 0;
}

int hermes_scheduler_add(hermes_scheduler_t *sched, const char *app_id,
                         uint8_t preferred_node, uint8_t priority,
                         uint32_t deadline_ms, float cpu_weight)
{
    if (!sched || !app_id || sched->count >= HERMES_MAX_SCHEDULED) return -1;
    hermes_schedule_entry_t *e = &sched->entries[sched->count++];
    strncpy(e->app_id, app_id, sizeof(e->app_id) - 1);
    e->preferred_node = preferred_node;
    e->priority = priority;
    e->deadline_ms = deadline_ms;
    e->cpu_weight = cpu_weight;
    e->placed = false;
    return 0;
}

int hermes_scheduler_place(hermes_scheduler_t *sched,
                           const float node_load[3], uint32_t node_count)
{
    if (!sched || !node_load) return -1;
    for (uint32_t i = 0; i < sched->count; i++) {
        hermes_schedule_entry_t *e = &sched->entries[i];
        uint8_t best = e->preferred_node;
        float best_load = node_load[best < node_count ? best : 0];
        for (uint32_t n = 0; n < node_count; n++) {
            if (node_load[n] < best_load) {
                best_load = node_load[n];
                best = (uint8_t)n;
            }
        }
        e->assigned_node = best;
        e->placed = true;
    }
    return 0;
}

hermes_schedule_entry_t *hermes_scheduler_find(hermes_scheduler_t *sched,
                                               const char *app_id)
{
    if (!sched || !app_id) return NULL;
    for (uint32_t i = 0; i < sched->count; i++)
        if (strcmp(sched->entries[i].app_id, app_id) == 0)
            return &sched->entries[i];
    return NULL;
}
