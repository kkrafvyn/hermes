/**
 * HERMES application template.
 */
#include "hermes_sdk.h"

typedef struct {
    uint32_t counter;
} myapp_state_t;

static myapp_state_t g_state;

HERMES_STATE(myapp_state_t, g_state)

static void main_loop(void *arg)
{
    (void)arg;
    g_state.counter++;
    HERMES_MONITOR("myapp", "counter", g_state.counter);
}

HERMES_TASK(main_loop, "main", 2048, 5)

void app_init(void)
{
    g_state.counter = 0;
}
