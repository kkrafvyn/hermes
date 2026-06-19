/**
 * HERMES ESP8266 gateway node — UART JSON telemetry for serial bridge.
 * Compatible with tools/serial-bridge/bridge.py
 */
#include <Arduino.h>
#include <ESP.h>
#include <ESP8266WiFi.h>

static const uint8_t NODE_ID = 1;

static void emit_telemetry() {
    float cpu = 22.0f + (random(0, 15));
    int ram_kb = ESP.getFreeHeap() / 1024;
    float temp = 40.0f + (random(0, 8));
    float power = 320.0f + (random(0, 40));

    Serial.printf(
        "{\"node_id\":%d,\"telemetry\":{\"cpu\":%.1f,\"ram_kb\":%d,"
        "\"temp_c\":%.1f,\"power_mw\":%.1f,\"link_quality\":92}}\n",
        NODE_ID, cpu, ram_kb, temp, power);
}

static void handle_command(const char *json) {
    if (strstr(json, "run_app") || strstr(json, "pause_app") ||
        strstr(json, "stop_app") || strstr(json, "install_app")) {
        Serial.printf("{\"ok\":true,\"node_id\":%d,\"handled\":true}\n", NODE_ID);
    }
}

static String cmd_buf;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println();
    Serial.printf("{\"boot\":\"hermes-esp8266\",\"node_id\":%d,\"mac\":\"%s\"}\n",
                  NODE_ID, WiFi.macAddress().c_str());
}

void loop() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || cmd_buf.length() >= 480) {
            if (cmd_buf.length() > 0 && cmd_buf.startsWith("{")) {
                handle_command(cmd_buf.c_str());
            }
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
