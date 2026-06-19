#include "hermes_sdk.h"
#include <stdint.h>

typedef struct {
    uint8_t sensor_count;
    uint16_t readings[8];
} sensornet_state_t;

static sensornet_state_t g_state;
HERMES_STATE(sensornet_state_t, g_state)

static void sensor_loop(void *arg)
{
    (void)arg;
    HERMES_MONITOR("sensor-net", "sensors", g_state.sensor_count);
}

HERMES_TASK(sensor_loop, "sensors", 2048, 3)

void app_init(void) { g_state.sensor_count = 0; }
