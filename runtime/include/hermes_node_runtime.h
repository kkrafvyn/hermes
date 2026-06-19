#ifndef HERMES_NODE_RUNTIME_H
#define HERMES_NODE_RUNTIME_H

#include "hermes_link.h"
#include "hermes_runtime.h"
#include "hermes_task.h"
#include "hermes_checkpoint.h"

typedef struct hermes_node_runtime hermes_node_runtime_t;

typedef int (*hermes_node_cmd_fn)(hermes_node_runtime_t *nr, const char *action,
                                  const char *app_name, const void *payload,
                                  size_t payload_len);

int hermes_node_runtime_init(hermes_node_runtime_t **out, hermes_link_t *link,
                             uint8_t node_id, const char *storage_path);
void hermes_node_runtime_deinit(hermes_node_runtime_t *nr);
void hermes_node_runtime_tick(hermes_node_runtime_t *nr);
int hermes_node_runtime_dispatch(hermes_node_runtime_t *nr, const char *action,
                                 const char *app_name, const uint8_t *binary,
                                 uint32_t binary_size);
hermes_runtime_t *hermes_node_runtime_get(hermes_node_runtime_t *nr);
hermes_task_manager_t *hermes_node_runtime_tasks(hermes_node_runtime_t *nr);
void hermes_node_runtime_on_checkpoint(hermes_node_runtime_t *nr,
                                       const hermes_checkpoint_t *cp);

#endif
