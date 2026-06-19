# ESP-IDF project for HERMES ESP32-S3 gateway
cmake_minimum_required(VERSION 3.16)

# Copy hermes cluster headers into components/hermes before building:
#   cp -r ../../cluster/include components/hermes/include
#   cp -r ../../cluster/src components/hermes/src

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(hermes_esp32s3)
