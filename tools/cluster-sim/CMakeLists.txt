add_executable(cluster-sim main.c)
target_link_libraries(cluster-sim PRIVATE hermes_cluster)
target_include_directories(cluster-sim PRIVATE
    ${CMAKE_SOURCE_DIR}/hardware/cluster/include)
