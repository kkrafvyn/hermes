/**
 * ESP32-S3 HERMES gateway — WiFi + HTTP or UART serial bridge.
 */
#include "hermes_link.h"
#include "hermes_wifi.h"
#include "hermes_cmd.h"
#include "hermes_node_port.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "hermes_gw";

extern int esp32_spi_send(const uint8_t *data, size_t len, void *ctx);
extern int esp32_spi_recv(uint8_t *data, size_t max_len, uint32_t timeout_ms, void *ctx);

static hermes_link_t g_link;

static void emit_serial_telemetry(float cpu, int ram_kb, float temp_c, float power_mw)
{
    printf("{\"node_id\":%d,\"telemetry\":{\"cpu\":%.1f,\"ram_kb\":%d,"
           "\"temp_c\":%.1f,\"power_mw\":%.1f,\"link_quality\":95}}\n",
           CONFIG_HERMES_NODE_ID, cpu, ram_kb, temp_c, power_mw);
    fflush(stdout);
}

static int post_http_heartbeat(float cpu, int ram_kb, float temp_c, float power_mw)
{
    char url[160];
    char body[280];
    snprintf(url, sizeof(url), "%s/api/nodes/heartbeat", CONFIG_HERMES_API_URL);
    snprintf(body, sizeof(body),
        "{\"node_id\":%d,\"telemetry\":{\"cpu\":%.1f,\"ram_kb\":%d,"
        "\"temp_c\":%.1f,\"power_mw\":%.1f,\"link_quality\":95}}",
        CONFIG_HERMES_NODE_ID, cpu, ram_kb, temp_c, power_mw);

    esp_http_client_config_t cfg = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) return -1;
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, body, strlen(body));
    esp_err_t err = esp_http_client_perform(client);
    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);
    return (err == ESP_OK && status == 200) ? 0 : -1;
}

static int register_node(void)
{
    if (CONFIG_HERMES_USE_SERIAL_BRIDGE) return 0;
    char url[160];
    char body[128];
    snprintf(url, sizeof(url), "%s/api/nodes/register", CONFIG_HERMES_API_URL);
    snprintf(body, sizeof(body),
        "{\"node_id\":%d,\"name\":\"ESP32-S3\",\"node_type\":\"esp32\",\"agent\":true}",
        CONFIG_HERMES_NODE_ID);

    esp_http_client_config_t cfg = { .url = url, .method = HTTP_METHOD_POST, .timeout_ms = 5000 };
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, body, strlen(body));
    esp_err_t err = esp_http_client_perform(client);
    esp_http_client_cleanup(client);
    return err == ESP_OK ? 0 : -1;
}

void hermes_node_init(void)
{
    hermes_link_transport_t transport = {
        .send = esp32_spi_send,
        .recv = esp32_spi_recv,
        .ctx = NULL,
        .transport = HERMES_TRANSPORT_SPI,
    };
    hermes_link_init(&g_link, &transport, CONFIG_HERMES_NODE_ID, HERMES_NODE_ESP32S3);
    hermes_node_port_init(&g_link, CONFIG_HERMES_NODE_ID, "/spiffs/checkpoints");
    ESP_LOGI(TAG, "node %d ready (runtime+FreeRTOS)", CONFIG_HERMES_NODE_ID);
}

void hermes_node_loop(void)
{
    hermes_frame_t frame;
    if (hermes_link_recv(&g_link, &frame, 10) == 0)
        hermes_link_handle_frame(&g_link, &frame);
    hermes_link_send_heartbeat(&g_link, 2);
}

static void gateway_task(void *arg)
{
    (void)arg;
    hermes_node_init();

    if (!CONFIG_HERMES_USE_SERIAL_BRIDGE) {
        if (hermes_wifi_init() != ESP_OK) {
            ESP_LOGE(TAG, "WiFi failed — falling back to serial bridge");
        } else {
            register_node();
        }
    }

    while (1) {
        hermes_node_loop();
        hermes_node_port_loop();
        hermes_cmd_poll();
        float cpu = 28.0f + (esp_random() % 10);
        int ram = 180;
        float temp = 42.0f;
        float power = 380.0f;

        if (CONFIG_HERMES_USE_SERIAL_BRIDGE) {
            emit_serial_telemetry(cpu, ram, temp, power);
        } else if (post_http_heartbeat(cpu, ram, temp, power) != 0) {
            ESP_LOGW(TAG, "HTTP heartbeat failed");
            emit_serial_telemetry(cpu, ram, temp, power);
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
    ESP_LOGI(TAG, "HERMES ESP32-S3 gateway");
    ESP_LOGI(TAG, "API: %s", CONFIG_HERMES_API_URL);
    hermes_cmd_init();
    xTaskCreate(gateway_task, "hermes_gw", 8192, NULL, 5, NULL);
}
