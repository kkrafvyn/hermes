idf_component_register(
    SRCS "hermes_gateway.c" "hermes_wifi.c" "hermes_spi.c" "hermes_cmd.c"
    INCLUDE_DIRS "."
    REQUIRES hermes esp_http_client esp_wifi nvs_flash esp_event esp_netif driver
)
