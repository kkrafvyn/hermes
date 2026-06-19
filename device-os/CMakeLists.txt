add_library(hermes_device_os STATIC
    src/hermes_task.c
    src/hermes_memory.c
    src/hermes_ipc.c
    src/hermes_freertos.c
)

target_include_directories(hermes_device_os PUBLIC include)
