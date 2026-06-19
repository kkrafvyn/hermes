#include "hermes_task.h"
#include "hermes_freertos.h"
#include <string.h>

int hermes_task_manager_init(hermes_task_manager_t *mgr)
{
    if (!mgr) return -1;
    memset(mgr, 0, sizeof(*mgr));
    return 0;
}

int hermes_create_task(hermes_task_manager_t *mgr, const char *name,
                       hermes_task_fn_t entry, void *arg,
                       uint32_t stack_size, uint8_t priority,
                       uint32_t *out_id)
{
    if (!mgr || !name || !entry || mgr->count >= HERMES_MAX_TASKS) return -1;
    hermes_task_t *t = &mgr->tasks[mgr->count];
    t->id = mgr->count + 1;
    strncpy(t->name, name, sizeof(t->name) - 1);
    t->entry = entry;
    t->arg = arg;
    t->stack_size = stack_size;
    t->priority = priority;
    t->state = HERMES_TASK_READY;
    t->handle = NULL;
    if (out_id) *out_id = t->id;
    mgr->count++;
#ifdef HERMES_USE_FREERTOS
    hermes_freertos_start_task(mgr, t->id);
#endif
    return 0;
}

int hermes_pause_task(hermes_task_manager_t *mgr, uint32_t task_id)
{
    hermes_task_t *t = hermes_get_task(mgr, task_id);
    if (!t || t->state != HERMES_TASK_RUNNING) return -1;
    t->state = HERMES_TASK_PAUSED;
#ifdef HERMES_USE_FREERTOS
    hermes_freertos_suspend_task(t);
#endif
    return 0;
}

int hermes_resume_task(hermes_task_manager_t *mgr, uint32_t task_id)
{
    hermes_task_t *t = hermes_get_task(mgr, task_id);
    if (!t || t->state != HERMES_TASK_PAUSED) return -1;
    t->state = HERMES_TASK_RUNNING;
#ifdef HERMES_USE_FREERTOS
    hermes_freertos_resume_task(t);
#endif
    return 0;
}

int hermes_destroy_task(hermes_task_manager_t *mgr, uint32_t task_id)
{
    hermes_task_t *t = hermes_get_task(mgr, task_id);
    if (!t) return -1;
    t->state = HERMES_TASK_STOPPED;
    t->entry = NULL;
    return 0;
}

hermes_task_t *hermes_get_task(hermes_task_manager_t *mgr, uint32_t task_id)
{
    if (!mgr) return NULL;
    for (uint32_t i = 0; i < mgr->count; i++)
        if (mgr->tasks[i].id == task_id) return &mgr->tasks[i];
    return NULL;
}
