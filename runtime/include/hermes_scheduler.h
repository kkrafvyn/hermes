#ifndef HERMES_SCHEDULER_H
#define HERMES_SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>

#define HERMES_MAX_SCHEDULED 16

typedef struct {
    char app_id[32];
    uint8_t preferred_node;
    uint8_t assigned_node;
    uint8_t priority;
    uint32_t deadline_ms;
    float cpu_weight;
    bool placed;
} hermes_schedule_entry_t;

typedef struct {
    hermes_schedule_entry_t entries[HERMES_MAX_SCHEDULED];
    uint32_t count;
} hermes_scheduler_t;

int  hermes_scheduler_init(hermes_scheduler_t *sched);
int  hermes_scheduler_add(hermes_scheduler_t *sched, const char *app_id,
                          uint8_t preferred_node, uint8_t priority,
                          uint32_t deadline_ms, float cpu_weight);
int  hermes_scheduler_place(hermes_scheduler_t *sched,
                            const float node_load[3], uint32_t node_count);
hermes_schedule_entry_t *hermes_scheduler_find(hermes_scheduler_t *sched,
                                               const char *app_id);

#endif /* HERMES_SCHEDULER_H */
