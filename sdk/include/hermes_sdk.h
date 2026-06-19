#ifndef HERMES_SDK_H
#define HERMES_SDK_H

#include <stdint.h>
#include <stddef.h>

/* Lightweight pack/emulator build — macros only, no runtime linkage */
#if defined(HERMES_PACK_BUILD) || defined(HERMES_EMULATOR)

#define HERMES_CHECKPOINT_MAX_SIZE 4096

#define HERMES_TASK(fn, name, stack, prio) /* hermes task: name */

#define HERMES_STATE(type, var) \
    _Static_assert(sizeof(type) <= HERMES_CHECKPOINT_MAX_SIZE, "state too large");

#define HERMES_MONITOR(app, metric, value) \
    do { (void)(app); (void)(metric); (void)(value); } while(0)

#else

#include "hermes_task.h"
#include "hermes_runtime.h"
#include "hermes_app.h"
#include "hermes_migration.h"

#define HERMES_TASK(fn, name, stack, prio) \
    static const char _hermes_task_##fn##_name[] = name; \
    static const uint32_t _hermes_task_##fn##_stack = (stack); \
    static const uint8_t _hermes_task_##fn##_prio = (prio); \
    static void _hermes_task_##fn##_register(hermes_task_manager_t *mgr) { \
        hermes_create_task(mgr, _hermes_task_##fn##_name, fn, NULL, \
                           _hermes_task_##fn##_stack, _hermes_task_##fn##_prio, NULL); \
    }

#define HERMES_STATE(type, var) \
    _Static_assert(sizeof(type) <= HERMES_CHECKPOINT_MAX_SIZE, "state too large");

#define HERMES_MONITOR(app, metric, value) \
    do { (void)(app); (void)(metric); (void)(value); } while(0)

#endif

#endif /* HERMES_SDK_H */
