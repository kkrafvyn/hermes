cmake_minimum_required(VERSION 3.16)
project(hermes VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(HERMES_BUILD_SIM "Build host cluster simulator" ON)
option(HERMES_BUILD_TESTS "Build unit tests" ON)

add_subdirectory(hardware/cluster)
add_subdirectory(device-os)
add_subdirectory(runtime)
add_subdirectory(platform)
add_subdirectory(security)

if(HERMES_BUILD_SIM)
    add_subdirectory(tools/cluster-sim)
endif()

if(HERMES_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
