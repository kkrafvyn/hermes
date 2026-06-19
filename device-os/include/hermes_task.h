#ifndef HERMES_TASK_H
#define HERMES_TASK_H

#include <stdint.h>
#include <stddef.h>

typedef void (*hermes_task_fn_t)(void *arg);

typedef enum {
    HERMES_TASK_READY   = 0,
    HERMES_TASK_RUNNING = 1,
    HERMES_TASK_PAUSED  = 2,
    HERMES_TASK_STOPPED = 3,
} hermes_task_state_t;

typedef struct {
    uint32_t id;
    char name[32];
    hermes_task_fn_t entry;
    void *arg;
    uint32_t stack_size;
    uint8_t priority;
    hermes_task_state_t state;
    void *handle;  /* FreeRTOS task handle or host pthread */
} hermes_task_t;

#define HERMES_MAX_TASKS 16

typedef struct {
    hermes_task_t tasks[HERMES_MAX_TASKS];
    uint32_t count;
} hermes_task_manager_t;

int  hermes_task_manager_init(hermes_task_manager_t *mgr);

int  hermes_create_task(hermes_task_manager_t *mgr, const char *name,
                        hermes_task_fn_t entry, void *arg,
                        uint32_t stack_size, uint8_t priority,
                        uint32_t *out_id);

int  hermes_pause_task(hermes_task_manager_t *mgr, uint32_t task_id);
int  hermes_resume_task(hermes_task_manager_t *mgr, uint32_t task_id);
int  hermes_destroy_task(hermes_task_manager_t *mgr, uint32_t task_id);

hermes_task_t *hermes_get_task(hermes_task_manager_t *mgr, uint32_t task_id);

#endif /* HERMES_TASK_H */
