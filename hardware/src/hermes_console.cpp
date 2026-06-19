#include "hermes_console.h"
#include "hermes_gpio.h"
#include "hermes_mqtt.h"
#include "hermes_wifi_module.h"
#include "hermes_mcu.h"

#include <Arduino.h>
#include <string.h>

#ifndef HERMES_NODE_ID
#define HERMES_NODE_ID 0
#endif

static char g_line[96];
static uint8_t g_pos = 0;

static void reply(const char *s) {
    Serial.println(s);
}

void hermes_console_handle_line(const char *line) {
    if (!line || line[0] == '\0') return;

    if (strcasecmp(line, "Status") == 0 || strcasecmp(line, "status") == 0) {
        char buf[160];
        snprintf(buf, sizeof(buf),
            "{\"Status\":{\"Module\":\"%s\",\"WiFi\":\"%s\",\"IP\":\"%s\",\"MQTT\":%s}}",
            HERMES_MCU_NAME,
            hermes_wifi_connected() ? "Connected" : "Disconnected",
            hermes_wifi_ip_string(),
            hermes_mqtt_connected() ? "true" : "false");
        reply(buf);
    } else if (strcasecmp(line, "GPIO") == 0) {
        char buf[512];
        hermes_gpio_states_json(buf, sizeof(buf));
        reply(buf);
    } else if (strncasecmp(line, "Power", 5) == 0) {
        int idx = 0;
        int val = 0;
        if (strlen(line) > 5) idx = atoi(line + 5) - 1;
        const char *sp = strchr(line, ' ');
        if (sp) val = atoi(sp + 1);
        else if (strlen(line) == 5) val = 1;
        if (hermes_gpio_set_relay((uint8_t)idx, val != 0) == 0) {
            hermes_mqtt_publish_gpio();
            reply("{\"Power\":\"OK\"}");
        } else {
            reply("{\"Power\":\"Error\"}");
        }
    } else if (strcasecmp(line, "Restart") == 0 || strcasecmp(line, "restart") == 0) {
        reply("{\"Restart\":1}");
        delay(200);
        ESP.restart();
    } else {
        reply("Commands: Status, GPIO, Power [n] 0|1, Restart");
    }
}

void hermes_console_poll(void) {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\r') continue;
        if (c == '\n' || g_pos >= sizeof(g_line) - 1) {
            g_line[g_pos] = '\0';
            g_pos = 0;
            if (g_line[0] != '\0')
                hermes_console_handle_line(g_line);
            continue;
        }
        g_line[g_pos++] = c;
    }
}
