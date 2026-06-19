#ifndef HERMES_CLUSTER_MGR_H
#define HERMES_CLUSTER_MGR_H

#include <stdint.h>
#include <stdbool.h>
#include "hermes_link.h"

#define HERMES_MAX_CLUSTER_NODES 8

typedef enum {
    HERMES_CLUSTER_HEALTHY  = 0,
    HERMES_CLUSTER_DEGRADED = 1,
    HERMES_CLUSTER_OFFLINE  = 2,
} hermes_cluster_health_t;

typedef struct {
    uint8_t node_id;
    hermes_node_type_t type;
    bool online;
    uint32_t last_heartbeat_us;
    float cpu_percent;
    uint32_t ram_used_kb;
} hermes_cluster_node_t;

typedef struct {
    hermes_cluster_node_t nodes[HERMES_MAX_CLUSTER_NODES];
    uint8_t node_count;
    uint8_t leader_id;
    hermes_cluster_health_t health;
    hermes_link_t *link;
} hermes_cluster_mgr_t;

int  hermes_cluster_init(hermes_cluster_mgr_t *mgr, hermes_link_t *link);
int  hermes_cluster_register(hermes_cluster_mgr_t *mgr, uint8_t node_id,
                             hermes_node_type_t type);
int  hermes_cluster_heartbeat(hermes_cluster_mgr_t *mgr, uint8_t node_id);
int  hermes_cluster_check_health(hermes_cluster_mgr_t *mgr);
int  hermes_cluster_select_leader(hermes_cluster_mgr_t *mgr);
int  hermes_cluster_isolate_node(hermes_cluster_mgr_t *mgr, uint8_t node_id);
hermes_cluster_node_t *hermes_cluster_find_node(hermes_cluster_mgr_t *mgr, uint8_t node_id);

#endif /* HERMES_CLUSTER_MGR_H */
