/**
 * HERMES universal node firmware — serial JSON + optional on-board HTTP server.
 */
#include <Arduino.h>
#include "hermes_sensor.h"
#include "hermes_mcu.h"
#include "hermes_print.h"
#include "hermes_node.h"
#include "hermes_board_store.h"
#include "hermes_gpio.h"
#include "hermes_templates.h"

#if defined(HERMES_MQTT)
#include "hermes_mqtt.h"
#endif
#if defined(HERMES_OTA)
#include "hermes_ota.h"
#endif
#if (defined(HERMES_ESP8266) || defined(ESP8266) || defined(HERMES_ESP32))
#include "hermes_console.h"
#endif

#if defined(HERMES_TFT_CYD2432)
#include "hermes_tft.h"
#include "hermes_wifi_module.h"
#endif

#if defined(HERMES_ESP32)
#include <WiFi.h>
#elif defined(HERMES_ESP8266) || defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(HERMES_NRF) && defined(ADAFRUIT_FEATHER_NRF52840)
#include <Adafruit_TinyUSB.h>
#endif

#if defined(HERMES_HTTP_SERVER) && (defined(HERMES_ESP8266) || defined(HERMES_ESP32) || defined(HERMES_REALTEK) || defined(HERMES_BEKEN))
#include "hermes_http.h"
#include "hermes_mesh.h"
#endif

#ifndef HERMES_NODE_ID
#define HERMES_NODE_ID 0
#endif

static const char *active_sensor_ids[HERMES_MAX_SENSORS];
static char active_sensor_id_bufs[HERMES_MAX_SENSORS][HERMES_SENSOR_ID_LEN];
static size_t active_sensor_count = 0;

static void parse_sensors_from_json(const char *json) {
    hermes_sensor_clear();
    active_sensor_count = 0;
    const char *p = strstr(json, "\"sensors\"");
    if (!p) return;
    p = strchr(p, '[');
    if (!p) return;
    p++;
    while (*p && *p != ']' && active_sensor_count < HERMES_MAX_SENSORS) {
        const char *q1 = strchr(p, '"');
        if (!q1) break;
        q1++;
        const char *q2 = strchr(q1, '"');
        if (!q2) break;
        size_t len = (size_t)(q2 - q1);
        if (len >= HERMES_SENSOR_ID_LEN) len = HERMES_SENSOR_ID_LEN - 1;
        memcpy(active_sensor_id_bufs[active_sensor_count], q1, len);
        active_sensor_id_bufs[active_sensor_count][len] = '\0';
        active_sensor_ids[active_sensor_count] = active_sensor_id_bufs[active_sensor_count];
        active_sensor_count++;
        p = q2 + 1;
        const char *comma = strchr(p, ',');
        const char *end = strchr(p, ']');
        if (!comma || (end && end < comma)) break;
        p = comma + 1;
    }
    if (active_sensor_count > 0)
        hermes_sensor_configure(active_sensor_ids, active_sensor_count);
}

void hermes_node_apply_config_json(const char *json) {
    parse_sensors_from_json(json);
}

void hermes_node_build_telemetry_json(char *buf, size_t max) {
    hermes_sensor_poll();
    float cpu = 22.0f + random(0, 15);
    int ram_kb = 64;
#if defined(HERMES_ESP32)
    ram_kb = ESP.getFreeHeap() / 1024;
#elif defined(HERMES_ESP8266) || defined(ESP8266)
    ram_kb = ESP.getFreeHeap() / 1024;
#elif defined(HERMES_STM32)
    ram_kb = 128;
#elif defined(HERMES_NRF)
    ram_kb = 32;
#elif defined(HERMES_RP2040)
    ram_kb = 48;
#elif defined(HERMES_AVR)
    ram_kb = 2;
#elif defined(HERMES_SAMD)
    ram_kb = 24;
#endif
    float temp = 40.0f + random(0, 8);
    float power = 320.0f + random(0, 40);
    char sensor_part[512];
    sensor_part[0] = '\0';
    hermes_sensor_json_all(sensor_part, sizeof(sensor_part));
    snprintf(
        buf, max,
        "{\"node_id\":%d,\"mcu\":\"%s\",\"family\":\"%s\",\"telemetry\":{\"cpu\":%.1f,\"ram_kb\":%d,"
        "\"temp_c\":%.1f,\"power_mw\":%.1f,\"link_quality\":92%s}}",
        HERMES_NODE_ID, HERMES_MCU_NAME, HERMES_MCU_FAMILY, cpu, ram_kb, temp, power, sensor_part);
}

static void emit_telemetry() {
    char payload[768];
    hermes_node_build_telemetry_json(payload, sizeof(payload));
    HERMES_PRINTF("%s\n", payload);
#if defined(HERMES_MQTT)
    hermes_mqtt_publish_state(payload);
#endif
}

static hermes_board_gpio_t g_serial_gpio;

static void handle_board_gpio_json(const char *json) {
    if (hermes_gpio_from_json(json, &g_serial_gpio) == 0 && g_serial_gpio.count > 0) {
        hermes_board_store_save_gpio(&g_serial_gpio);
        hermes_gpio_apply(&g_serial_gpio);
        HERMES_PRINTF("{\"ok\":true,\"node_id\":%d,\"board_template\":\"%s\",\"pins\":%u}\n",
                      HERMES_NODE_ID, g_serial_gpio.template_id, g_serial_gpio.count);
        return;
    }
    char tid[32] = "";
    const char *q = strstr(json, "\"template_id\"");
    if (q) {
        q = strchr(q + 13, '"');
        if (q) {
            q++;
            const char *e = strchr(q, '"');
            if (e && (size_t)(e - q) < sizeof(tid)) {
                memcpy(tid, q, (size_t)(e - q));
                tid[e - q] = '\0';
            }
        }
    }
    if (tid[0] && hermes_template_apply(tid, &g_serial_gpio) == 0) {
        hermes_board_store_save_gpio(&g_serial_gpio);
        hermes_gpio_apply(&g_serial_gpio);
        HERMES_PRINTF("{\"ok\":true,\"node_id\":%d,\"board_template\":\"%s\"}\n",
                      HERMES_NODE_ID, g_serial_gpio.template_id);
    }
}

static void handle_command(const char *json) {
    if (strstr(json, "board_preset") || strstr(json, "\"action\":\"board_preset\"")) {
        char tid[32] = "";
        const char *src = json;
        const char *payload = strstr(json, "\"payload\"");
        if (payload) src = payload;
        const char *q = strstr(src, "\"template_id\"");
        if (q) {
            q = strchr(q + 13, '"');
            if (q) {
                q++;
                const char *e = strchr(q, '"');
                if (e && (size_t)(e - q) < sizeof(tid)) {
                    memcpy(tid, q, (size_t)(e - q));
                    tid[e - q] = '\0';
                }
            }
        }
        if (tid[0] && hermes_template_apply(tid, &g_serial_gpio) == 0) {
            hermes_board_store_save_gpio(&g_serial_gpio);
            hermes_gpio_apply(&g_serial_gpio);
            HERMES_PRINTF("{\"ok\":true,\"node_id\":%d,\"board_template\":\"%s\"}\n",
                          HERMES_NODE_ID, g_serial_gpio.template_id);
        }
        return;
    }
    if (strstr(json, "board_gpio") || strstr(json, "\"action\":\"board_gpio\"") ||
        strstr(json, "set_board_gpio")) {
        handle_board_gpio_json(json);
        return;
    }
    if (strstr(json, "get_board_gpio")) {
        char buf[1200];
        hermes_board_store_load_gpio(&g_serial_gpio);
        hermes_gpio_to_json(&g_serial_gpio, buf, sizeof(buf));
        HERMES_PRINTF("%s\n", buf);
        return;
    }
    if (strstr(json, "set_config") || strstr(json, "\"config\"")) {
        hermes_node_apply_config_json(json);
        HERMES_PRINTF("{\"ok\":true,\"node_id\":%d,\"sensors\":%u}\n",
                      HERMES_NODE_ID, (unsigned)hermes_sensor_count());
        return;
    }
    if (strstr(json, "run_app") || strstr(json, "pause_app") ||
        strstr(json, "stop_app") || strstr(json, "install_app")) {
        hermes_node_apply_config_json(json);
        HERMES_PRINTF("{\"ok\":true,\"node_id\":%d}\n", HERMES_NODE_ID);
    }
}

static String cmd_buf;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println();
    hermes_board_app_t boot_app;
    hermes_board_store_load(&boot_app);
    hermes_board_gpio_t boot_gpio;
    hermes_board_store_load_gpio(&boot_gpio);
    memcpy(&g_serial_gpio, &boot_gpio, sizeof(g_serial_gpio));
    hermes_gpio_apply(&boot_gpio);
    HERMES_PRINTF("{\"boot\":\"hermes-node\",\"mcu\":\"%s\",\"family\":\"%s\",\"node_id\":%d,\"http\":%s,\"board_template\":\"%s\"}\n",
                  HERMES_MCU_NAME, HERMES_MCU_FAMILY, HERMES_NODE_ID,
#if defined(HERMES_HTTP_SERVER) && (defined(HERMES_ESP8266) || defined(HERMES_ESP32))
                  "true"
#else
                  "false"
#endif
                  , boot_gpio.template_id);
#if defined(HERMES_HTTP_SERVER) && (defined(HERMES_ESP8266) || defined(HERMES_ESP32) || defined(HERMES_REALTEK) || defined(HERMES_BEKEN))
    hermes_http_begin();
#endif
#if defined(HERMES_TFT_CYD2432)
    hermes_tft_begin();
    if (hermes_wifi_in_setup_mode())
        hermes_tft_show_status("WiFi setup", hermes_wifi_setup_ap_name(), "http://192.168.4.1/");
    else
        hermes_tft_show_status("HERMES node", hermes_wifi_ip_string(), HERMES_MCU_NAME);
#endif
}

void loop() {
#if defined(HERMES_HTTP_SERVER) && (defined(HERMES_ESP8266) || defined(HERMES_ESP32) || defined(HERMES_REALTEK) || defined(HERMES_BEKEN))
    hermes_http_loop();
#endif
#if defined(HERMES_OTA)
    hermes_ota_loop();
#endif
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || cmd_buf.length() >= 480) {
            if (cmd_buf.length() > 0) {
                if (cmd_buf.startsWith("{")) {
                    handle_command(cmd_buf.c_str());
#if defined(HERMES_HTTP_SERVER)
                    hermes_mesh_handle_line(cmd_buf.c_str());
#endif
                }
#if (defined(HERMES_ESP8266) || defined(ESP8266) || defined(HERMES_ESP32))
                else {
                    hermes_console_handle_line(cmd_buf.c_str());
                }
#endif
            }
            cmd_buf = "";
        } else if (c != '\r') {
            cmd_buf += c;
        }
    }
    static uint32_t last = 0;
    static uint32_t last_tft = 0;
    if (millis() - last >= 3000) {
        last = millis();
        emit_telemetry();
    }
#if defined(HERMES_TFT_CYD2432)
    if (millis() - last_tft >= 5000) {
        last_tft = millis();
        if (hermes_wifi_in_setup_mode())
            hermes_tft_show_status("WiFi setup", hermes_wifi_setup_ap_name(), "http://192.168.4.1/");
        else if (hermes_wifi_connected())
            hermes_tft_show_status("Connected", hermes_wifi_ip_string(), "node " HERMES_MCU_NAME);
        else
            hermes_tft_show_status("HERMES", "WiFi...", HERMES_MCU_NAME);
    }
#endif
}
