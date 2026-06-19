#include "hermes_freertos.h"

#include <string.h>



#ifdef HERMES_USE_FREERTOS



static void freertos_trampoline(void *arg)

{

    hermes_task_t *t = (hermes_task_t *)arg;

    if (t && t->entry) {

        t->state = HERMES_TASK_RUNNING;

        t->entry(t->arg);

    }

    vTaskDelete(NULL);

}



int hermes_freertos_start_task(hermes_task_manager_t *mgr, uint32_t task_id)

{

    hermes_task_t *t = hermes_get_task(mgr, task_id);

    if (!t || !t->entry) return -1;

    if (t->handle) return 0;

    BaseType_t ok = xTaskCreate(freertos_trampoline, t->name,

                                t->stack_size / sizeof(StackType_t),

                                t, t->priority, (TaskHandle_t *)&t->handle);

    return ok == pdPASS ? 0 : -2;

}



int hermes_freertos_suspend_task(hermes_task_t *task)

{

    if (!task || !task->handle) return -1;

    vTaskSuspend((TaskHandle_t)task->handle);

    return 0;

}



int hermes_freertos_resume_task(hermes_task_t *task)

{

    if (!task || !task->handle) return -1;

    vTaskResume((TaskHandle_t)task->handle);

    return 0;

}



int hermes_freertos_bind(void)

{

    return 0;

}



#else



int hermes_freertos_start_task(hermes_task_manager_t *mgr, uint32_t task_id)

{

    hermes_task_t *t = hermes_get_task(mgr, task_id);

    if (!t || !t->entry) return -1;

    t->state = HERMES_TASK_RUNNING;

    return 0;

}



int hermes_freertos_suspend_task(hermes_task_t *task)

{

    (void)task;

    return 0;

}



int hermes_freertos_resume_task(hermes_task_t *task)

{

    (void)task;

    return 0;

}



int hermes_freertos_bind(void) { return 0; }



#endif


