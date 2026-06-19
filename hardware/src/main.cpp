/**
 * HERMES universal node firmware — serial JSON telemetry + commands + sensors.
 * Builds for ESP8266, ESP32/S3/C3, STM32H7/F4, nRF52840 via PlatformIO.
 */
#include <Arduino.h>
#include "hermes_sensor.h"
#include "hermes_mcu.h"
#include "hermes_print.h"

#if defined(HERMES_ESP32)
#include <WiFi.h>
#elif defined(HERMES_ESP8266) || defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(HERMES_NRF) && defined(ADAFRUIT_FEATHER_NRF52840)
#include <Adafruit_TinyUSB.h>
#endif

#ifndef HERMES_NODE_ID
#define HERMES_NODE_ID 1
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

static void emit_telemetry() {
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
    hermes_sensor_json(sensor_part, sizeof(sensor_part));
    HERMES_PRINTF(
        "{\"node_id\":%d,\"mcu\":\"%s\",\"family\":\"%s\",\"telemetry\":{\"cpu\":%.1f,\"ram_kb\":%d,"
        "\"temp_c\":%.1f,\"power_mw\":%.1f,\"link_quality\":92%s}}\n",
        HERMES_NODE_ID, HERMES_MCU_NAME, HERMES_MCU_FAMILY, cpu, ram_kb, temp, power, sensor_part);
}

static void handle_command(const char *json) {
    if (strstr(json, "set_config") || strstr(json, "\"config\"")) {
        parse_sensors_from_json(json);
        HERMES_PRINTF("{\"ok\":true,\"node_id\":%d,\"sensors\":%u}\n",
                      HERMES_NODE_ID, (unsigned)hermes_sensor_count());
        return;
    }
    if (strstr(json, "run_app") || strstr(json, "pause_app") ||
        strstr(json, "stop_app") || strstr(json, "install_app")) {
        parse_sensors_from_json(json);
        HERMES_PRINTF("{\"ok\":true,\"node_id\":%d}\n", HERMES_NODE_ID);
    }
}

static String cmd_buf;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println();
    HERMES_PRINTF("{\"boot\":\"hermes-node\",\"mcu\":\"%s\",\"family\":\"%s\",\"node_id\":%d,\"sensors\":true}\n",
                  HERMES_MCU_NAME, HERMES_MCU_FAMILY, HERMES_NODE_ID);
}

void loop() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || cmd_buf.length() >= 480) {
            if (cmd_buf.length() > 0 && cmd_buf.startsWith("{"))
                handle_command(cmd_buf.c_str());
            cmd_buf = "";
        } else {
            cmd_buf += c;
        }
    }
    static uint32_t last = 0;
    if (millis() - last >= 3000) {
        last = millis();
        emit_telemetry();
    }
}
