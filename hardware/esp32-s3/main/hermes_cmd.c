/**
 * ESP32 UART command handler — dispatches to HERMES node runtime.
 */
#include "hermes_cmd.h"
#include "hermes_node_port.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "sdkconfig.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "hermes_cmd";
#define CMD_BUF 512
static char s_cmd_buf[CMD_BUF];
static size_t s_cmd_len;

static const char *json_get(const char *json, const char *key, char *out, size_t max)
{
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *p = strstr(json, pattern);
    if (!p) return NULL;
    p = strchr(p + strlen(pattern), '"');
    if (!p) return NULL;
    p++;
    const char *end = strchr(p, '"');
    if (!end || (size_t)(end - p) >= max) return NULL;
    memcpy(out, p, (size_t)(end - p));
    out[end - p] = '\0';
    return out;
}

void hermes_cmd_init(void)
{
    s_cmd_len = 0;
    ESP_LOGI(TAG, "command handler ready (runtime dispatch)");
}

static void execute_cmd(const char *json)
{
    char action[32] = {0}, app[32] = {0};
    json_get(json, "action", action, sizeof(action));
    json_get(json, "app_name", app, sizeof(app));
    if (action[0] == '\0') return;
    int rc = hermes_node_port_cmd(action, app[0] ? app : NULL, NULL, 0);
    ESP_LOGI(TAG, "execute: %s %s -> %d", action, app, rc);
}

void hermes_cmd_poll(void)
{
    uint8_t byte;
    while (uart_read_bytes(UART_NUM_0, &byte, 1, 0) > 0) {
        if (byte == '\n' || s_cmd_len >= CMD_BUF - 1) {
            s_cmd_buf[s_cmd_len] = '\0';
            if (s_cmd_len > 0 && s_cmd_buf[0] == '{')
                execute_cmd(s_cmd_buf);
            s_cmd_len = 0;
        } else {
            s_cmd_buf[s_cmd_len++] = (char)byte;
        }
    }
}
