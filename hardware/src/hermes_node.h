#ifndef HERMES_NODE_H
#define HERMES_NODE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void hermes_node_apply_config_json(const char *json);
void hermes_node_build_telemetry_json(char *buf, size_t max);
uint8_t hermes_node_app_state(void);
void hermes_node_set_app_state(uint8_t state);

#ifdef __cplusplus
}
#endif

#endif
