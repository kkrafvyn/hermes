#ifndef HERMES_FREERTOS_H
#define HERMES_FREERTOS_H

#include "hermes_task.h"

#ifdef HERMES_USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

int hermes_freertos_start_task(hermes_task_manager_t *mgr, uint32_t task_id);
int hermes_freertos_suspend_task(hermes_task_t *task);
int hermes_freertos_resume_task(hermes_task_t *task);
int hermes_freertos_bind(void);

#endif
