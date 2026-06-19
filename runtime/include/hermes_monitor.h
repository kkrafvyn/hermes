#ifndef HERMES_MONITOR_H
#define HERMES_MONITOR_H

#include "hermes_runtime.h"

typedef struct {
    hermes_resource_snapshot_t current;
    hermes_resource_snapshot_t peak;
    uint32_t sample_count;
    uint32_t interval_ms;
} hermes_monitor_t;

int  hermes_monitor_init(hermes_monitor_t *mon, uint32_t interval_ms);
int  hermes_monitor_sample(hermes_monitor_t *mon, const hermes_resource_snapshot_t *snap);
int  hermes_monitor_get(hermes_monitor_t *mon, hermes_resource_snapshot_t *out);

#endif /* HERMES_MONITOR_H */
