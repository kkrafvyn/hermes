#ifndef HERMES_POWER_H
#define HERMES_POWER_H

#include <stdint.h>

typedef struct {
    float voltage_v;
    float current_ma;
    float power_mw;
    uint32_t sample_ts_us;
} hermes_power_sample_t;

typedef struct {
    int (*read)(hermes_power_sample_t *sample, void *ctx);
    void *ctx;
} hermes_power_monitor_t;

int hermes_power_init(hermes_power_monitor_t *mon);
int hermes_power_sample(hermes_power_monitor_t *mon, hermes_power_sample_t *out);

#endif /* HERMES_POWER_H */
