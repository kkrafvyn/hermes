#ifndef HERMES_MESH_H
#define HERMES_MESH_H

#include "hermes_wifi_module.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HERMES_MESH_MAX_NODES 8
#define HERMES_MESH_MAX_SENSORS 6

typedef struct {
    uint8_t node_id;
    char name[24];
    char sensors[96];
    float readings[HERMES_MESH_MAX_SENSORS];
    char reading_ids[HERMES_MESH_MAX_SENSORS][24];
    uint8_t reading_count;
    uint32_t last_seen_ms;
    uint8_t online;
} hermes_mesh_node_t;

void hermes_mesh_init(const hermes_network_config_t *net);
void hermes_mesh_loop(void);
void hermes_mesh_handle_line(const char *json);
size_t hermes_mesh_node_count(void);
const hermes_mesh_node_t *hermes_mesh_get_node(size_t index);
int hermes_mesh_json_nodes(char *buf, size_t max);
int hermes_mesh_json_readings(char *buf, size_t max);
void hermes_mesh_apply_app_config(const char *local_csv, const char *remote_csv,
    uint8_t max_nodes, uint16_t interval_sec);
uint8_t hermes_mesh_enabled(void);

#ifdef __cplusplus
}
#endif

#endif
