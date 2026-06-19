#include "hermes_monitor.h"
#include <string.h>

int hermes_monitor_init(hermes_monitor_t *mon, uint32_t interval_ms)
{
    if (!mon) return -1;
    memset(mon, 0, sizeof(*mon));
    mon->interval_ms = interval_ms;
    return 0;
}

int hermes_monitor_sample(hermes_monitor_t *mon, const hermes_resource_snapshot_t *snap)
{
    if (!mon || !snap) return -1;
    mon->current = *snap;
    mon->sample_count++;
    if (snap->cpu_percent > mon->peak.cpu_percent) mon->peak.cpu_percent = snap->cpu_percent;
    if (snap->ram_used > mon->peak.ram_used) mon->peak.ram_used = snap->ram_used;
    if (snap->power_mw > mon->peak.power_mw) mon->peak.power_mw = snap->power_mw;
    if (snap->temp_c > mon->peak.temp_c) mon->peak.temp_c = snap->temp_c;
    return 0;
}

int hermes_monitor_get(hermes_monitor_t *mon, hermes_resource_snapshot_t *out)
{
    if (!mon || !out) return -1;
    *out = mon->current;
    return 0;
}
