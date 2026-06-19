#include "hermes_sdk.h"
#include <stdint.h>

typedef struct {
    uint32_t packets_sent;
    float bandwidth_kbps;
} telemetry_state_t;

static telemetry_state_t g_state;
HERMES_STATE(telemetry_state_t, g_state)

static void telemetry_loop(void *arg)
{
    (void)arg;
    g_state.packets_sent++;
    HERMES_MONITOR("telemetry", "packets", g_state.packets_sent);
}

HERMES_TASK(telemetry_loop, "telemetry", 2048, 4)

void app_init(void) { g_state.packets_sent = 0; g_state.bandwidth_kbps = 0; }
