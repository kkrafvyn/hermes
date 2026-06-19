add_library(hermes_security STATIC
    src/hermes_identity.c
    src/hermes_policy.c
    src/hermes_sha256.c
    src/hermes_hmac.c
)

target_include_directories(hermes_security PUBLIC include)
target_link_libraries(hermes_security PUBLIC hermes_platform)
