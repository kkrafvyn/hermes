add_library(hermes_cluster STATIC
    src/hermes_link.c
    src/hermes_power.c
    src/hermes_cluster_mgr.c
    src/hermes_transport_host.c
    src/hermes_router.c
    src/hermes_transport_ble.c
)

target_include_directories(hermes_cluster PUBLIC include)
