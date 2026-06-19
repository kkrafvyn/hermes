set(HERMES_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)
set(HERMES_CLUSTER ${HERMES_ROOT}/hardware/cluster)
set(HERMES_DEVICE_OS ${HERMES_ROOT}/device-os)
set(HERMES_RUNTIME ${HERMES_ROOT}/runtime)
set(HERMES_SECURITY ${HERMES_ROOT}/security)
set(HERMES_COMMON ${HERMES_ROOT}/hardware/common)

idf_component_register(
    SRCS
        "${HERMES_CLUSTER}/src/hermes_link.c"
        "${HERMES_CLUSTER}/src/hermes_power.c"
        "${HERMES_CLUSTER}/src/hermes_cluster_mgr.c"
        "${HERMES_CLUSTER}/src/hermes_router.c"
        "${HERMES_DEVICE_OS}/src/hermes_task.c"
        "${HERMES_DEVICE_OS}/src/hermes_memory.c"
        "${HERMES_DEVICE_OS}/src/hermes_ipc.c"
        "${HERMES_DEVICE_OS}/src/hermes_freertos.c"
        "${HERMES_RUNTIME}/src/hermes_runtime.c"
        "${HERMES_RUNTIME}/src/hermes_monitor.c"
        "${HERMES_RUNTIME}/src/hermes_scheduler.c"
        "${HERMES_RUNTIME}/src/hermes_checkpoint.c"
        "${HERMES_RUNTIME}/src/hermes_checkpoint_io.c"
        "${HERMES_RUNTIME}/src/hermes_node_runtime.c"
        "${HERMES_RUNTIME}/src/hermes_quota.c"
        "${HERMES_RUNTIME}/src/hermes_migration.c"
        "${HERMES_RUNTIME}/src/hermes_ota.c"
        "${HERMES_SECURITY}/src/hermes_sha256.c"
        "${HERMES_COMMON}/hermes_node_port.c"
    INCLUDE_DIRS
        "${HERMES_CLUSTER}/include"
        "${HERMES_DEVICE_OS}/include"
        "${HERMES_RUNTIME}/include"
        "${HERMES_SECURITY}/include"
        "${HERMES_COMMON}"
    REQUIRES esp_timer
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC HERMES_USE_FREERTOS)
