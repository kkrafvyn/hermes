#ifndef HERMES_NODE_PORT_H
#define HERMES_NODE_PORT_H

#include "hermes_link.h"

void hermes_node_port_init(hermes_link_t *link, uint8_t node_id, const char *storage);
void hermes_node_port_loop(void);
int hermes_node_port_cmd(const char *action, const char *app_name,
                         const uint8_t *binary, uint32_t binary_size);

#endif
