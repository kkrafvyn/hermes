/**
 * Smartfarm example application for HERMES platform.
 */
#include "hermes_sdk.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    float soil_moisture;
    uint32_t irrigation_cycles;
    bool pump_active;
} smartfarm_state_t;

static smartfarm_state_t g_state;

HERMES_STATE(smartfarm_state_t, g_state)

static void sensor_loop(void *arg)
{
    (void)arg;
    HERMES_MONITOR("smartfarm", "soil_moisture", g_state.soil_moisture);
}

HERMES_TASK(sensor_loop, "sensor", 4096, 5)

void smartfarm_init(void)
{
    g_state.soil_moisture = 0.0f;
    g_state.irrigation_cycles = 0;
    g_state.pump_active = false;
}

int smartfarm_serialize(uint8_t *buf, size_t max, size_t *written)
{
    if (max < sizeof(g_state)) return -1;
    memcpy(buf, &g_state, sizeof(g_state));
    *written = sizeof(g_state);
    return 0;
}

int smartfarm_restore(const uint8_t *buf, size_t len)
{
    if (len < sizeof(g_state)) return -1;
    memcpy(&g_state, buf, sizeof(g_state));
    return 0;
}
