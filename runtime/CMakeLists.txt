add_library(hermes_runtime STATIC
    src/hermes_runtime.c
    src/hermes_monitor.c
    src/hermes_scheduler.c
    src/hermes_checkpoint.c
    src/hermes_checkpoint_io.c
    src/hermes_node_runtime.c
    src/hermes_quota.c
    src/hermes_migration.c
    src/hermes_ota.c
)

target_include_directories(hermes_runtime PUBLIC include)
target_include_directories(hermes_runtime PRIVATE ${CMAKE_SOURCE_DIR}/security/include)
target_link_libraries(hermes_runtime PUBLIC hermes_cluster hermes_device_os)
target_sources(hermes_runtime PRIVATE ${CMAKE_SOURCE_DIR}/security/src/hermes_sha256.c)
