#include "hermes_node_port.h"
#include "hermes_node_runtime.h"

static hermes_node_runtime_t *g_nr;

void hermes_node_port_init(hermes_link_t *link, uint8_t node_id, const char *storage)
{
    hermes_node_runtime_init(&g_nr, link, node_id, storage);
}

void hermes_node_port_loop(void)
{
    if (g_nr) hermes_node_runtime_tick(g_nr);
}

int hermes_node_port_cmd(const char *action, const char *app_name,
                         const uint8_t *binary, uint32_t binary_size)
{
    if (!g_nr) return -1;
    return hermes_node_runtime_dispatch(g_nr, action, app_name, binary, binary_size);
}
