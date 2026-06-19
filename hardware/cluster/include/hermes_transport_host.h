#ifndef HERMES_TRANSPORT_HOST_H
#define HERMES_TRANSPORT_HOST_H

#include "hermes_link.h"

int  hermes_transport_host_init(hermes_link_t *link, uint8_t node_id, hermes_node_type_t type);
void hermes_transport_host_reset(void);

#endif /* HERMES_TRANSPORT_HOST_H */
