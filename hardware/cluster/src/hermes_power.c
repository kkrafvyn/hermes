#include "hermes_power.h"

int hermes_power_init(hermes_power_monitor_t *mon)
{
    return (mon && mon->read) ? 0 : -1;
}

int hermes_power_sample(hermes_power_monitor_t *mon, hermes_power_sample_t *out)
{
    if (!mon || !out || !mon->read) return -1;
    return mon->read(out, mon->ctx);
}
