# Zephyr / nRF Connect SDK project for HERMES nRF52840 node

cmake_minimum_required(VERSION 3.20)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(hermes_nrf52840)

target_sources(app PRIVATE
    src/main.c
    src/hal_uart.c
    ${CMAKE_CURRENT_LIST_DIR}/../../cluster/src/hermes_link.c
    ${CMAKE_CURRENT_LIST_DIR}/../../cluster/src/hermes_cluster_mgr.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../../cluster/include
)
