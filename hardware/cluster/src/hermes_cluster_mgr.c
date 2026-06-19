#include "hermes_cluster_mgr.h"
#include <string.h>

int hermes_cluster_init(hermes_cluster_mgr_t *mgr, hermes_link_t *link)
{
    if (!mgr || !link) return -1;
    memset(mgr, 0, sizeof(*mgr));
    mgr->link = link;
    mgr->leader_id = link->node_id;
    mgr->health = HERMES_CLUSTER_HEALTHY;
    return 0;
}

int hermes_cluster_register(hermes_cluster_mgr_t *mgr, uint8_t node_id,
                            hermes_node_type_t type)
{
    if (!mgr || mgr->node_count >= HERMES_MAX_CLUSTER_NODES) return -1;
    hermes_cluster_node_t *n = &mgr->nodes[mgr->node_count++];
    n->node_id = node_id;
    n->type = type;
    n->online = true;
    return 0;
}

int hermes_cluster_heartbeat(hermes_cluster_mgr_t *mgr, uint8_t node_id)
{
    hermes_cluster_node_t *n = hermes_cluster_find_node(mgr, node_id);
    if (!n) return -1;
    n->online = true;
    n->last_heartbeat_us = 0;
    return hermes_link_send_heartbeat(mgr->link, node_id);
}

int hermes_cluster_check_health(hermes_cluster_mgr_t *mgr)
{
    if (!mgr) return -1;
    uint8_t online = 0;
    for (uint8_t i = 0; i < mgr->node_count; i++)
        if (mgr->nodes[i].online) online++;
    if (online == 0) mgr->health = HERMES_CLUSTER_OFFLINE;
    else if (online < mgr->node_count) mgr->health = HERMES_CLUSTER_DEGRADED;
    else mgr->health = HERMES_CLUSTER_HEALTHY;
    return (int)mgr->health;
}

int hermes_cluster_select_leader(hermes_cluster_mgr_t *mgr)
{
    if (!mgr || mgr->node_count == 0) return -1;
    uint8_t best = mgr->nodes[0].node_id;
    float lowest = mgr->nodes[0].cpu_percent;
    for (uint8_t i = 1; i < mgr->node_count; i++) {
        if (mgr->nodes[i].online && mgr->nodes[i].cpu_percent < lowest) {
            lowest = mgr->nodes[i].cpu_percent;
            best = mgr->nodes[i].node_id;
        }
    }
    mgr->leader_id = best;
    return 0;
}

int hermes_cluster_isolate_node(hermes_cluster_mgr_t *mgr, uint8_t node_id)
{
    hermes_cluster_node_t *n = hermes_cluster_find_node(mgr, node_id);
    if (!n) return -1;
    n->online = false;
    return hermes_cluster_check_health(mgr);
}

hermes_cluster_node_t *hermes_cluster_find_node(hermes_cluster_mgr_t *mgr, uint8_t node_id)
{
    if (!mgr) return NULL;
    for (uint8_t i = 0; i < mgr->node_count; i++)
        if (mgr->nodes[i].node_id == node_id) return &mgr->nodes[i];
    return NULL;
}
