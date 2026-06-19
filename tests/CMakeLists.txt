add_executable(test_platform test_platform.c)
target_link_libraries(test_platform PRIVATE hermes_security)
add_test(NAME platform COMMAND test_platform)
