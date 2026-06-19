#include "hermes_mqtt.h"
#include "hermes_gpio.h"
#include "hermes_wifi_module.h"

#include <Arduino.h>
#include <string.h>

#if (defined(HERMES_ESP8266) || defined(ESP8266) || defined(HERMES_ESP32)) && defined(HERMES_MQTT)

#if defined(HERMES_ESP32)
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <PubSubClient.h>

#ifndef HERMES_NODE_ID
#define HERMES_NODE_ID 0
#endif

static WiFiClient g_wifi_client;
static PubSubClient g_mqtt(g_wifi_client);
static hermes_mqtt_config_t g_cfg;
static char g_topic_cmnd[64];
static char g_topic_state[64];
static char g_topic_tele[64];
static bool g_ready = false;

static void mqtt_callback(char *topic, byte *payload, unsigned int len) {
    char msg[32];
    if (len >= sizeof(msg)) len = sizeof(msg) - 1;
    memcpy(msg, payload, len);
    msg[len] = '\0';
    const char *pfx = g_cfg.prefix[0] ? g_cfg.prefix : "hermes";
    char expect[48];
    snprintf(expect, sizeof(expect), "%s/node%d/cmnd/POWER", pfx, HERMES_NODE_ID);
    if (strcmp(topic, expect) == 0) {
        bool on = (strcmp(msg, "1") == 0 || strcasecmp(msg, "ON") == 0);
        hermes_mqtt_set_relay(0, on);
        hermes_mqtt_publish_gpio();
        return;
    }
    snprintf(expect, sizeof(expect), "%s/node%d/cmnd/POWER", pfx, HERMES_NODE_ID);
    for (uint8_t i = 1; i < 8; i++) {
        snprintf(expect, sizeof(expect), "%s/node%d/cmnd/POWER%u", pfx, HERMES_NODE_ID, i + 1);
        if (strcmp(topic, expect) == 0) {
            bool on = (strcmp(msg, "1") == 0 || strcasecmp(msg, "ON") == 0);
            hermes_mqtt_set_relay(i, on);
            hermes_mqtt_publish_gpio();
            return;
        }
    }
}

static void build_topics(void) {
    const char *pfx = g_cfg.prefix[0] ? g_cfg.prefix : "hermes";
    snprintf(g_topic_cmnd, sizeof(g_topic_cmnd), "%s/node%d/cmnd/#", pfx, HERMES_NODE_ID);
    snprintf(g_topic_state, sizeof(g_topic_state), "%s/node%d/state", pfx, HERMES_NODE_ID);
    snprintf(g_topic_tele, sizeof(g_topic_tele), "%s/node%d/tele", pfx, HERMES_NODE_ID);
}

void hermes_mqtt_defaults(hermes_mqtt_config_t *out) {
    memset(out, 0, sizeof(*out));
    strncpy(out->prefix, "hermes", sizeof(out->prefix) - 1);
    out->port = 1883;
}

void hermes_mqtt_begin(const hermes_mqtt_config_t *cfg) {
    if (!cfg || !cfg->enabled || !cfg->host[0]) {
        g_ready = false;
        return;
    }
    memcpy(&g_cfg, cfg, sizeof(g_cfg));
    build_topics();
    g_mqtt.setServer(g_cfg.host, g_cfg.port ? g_cfg.port : 1883);
    g_mqtt.setCallback(mqtt_callback);
    g_mqtt.setBufferSize(512);
    g_ready = true;
    Serial.printf("HERMES MQTT: broker %s:%u prefix=%s\n", g_cfg.host, g_cfg.port, g_cfg.prefix);
}

bool hermes_mqtt_connected(void) {
    return g_ready && g_mqtt.connected();
}

static void mqtt_reconnect(void) {
    if (!g_ready || !hermes_wifi_connected() || hermes_wifi_in_setup_mode()) return;
    if (g_mqtt.connected()) return;
    char client_id[24];
    snprintf(client_id, sizeof(client_id), "hermes-%d", HERMES_NODE_ID);
    bool ok = false;
    if (g_cfg.user[0])
        ok = g_mqtt.connect(client_id, g_cfg.user, g_cfg.pass);
    else
        ok = g_mqtt.connect(client_id);
    if (ok) {
        g_mqtt.subscribe(g_topic_cmnd);
        g_mqtt.publish(g_topic_state, "Online", true);
        hermes_mqtt_publish_gpio();
        Serial.println("HERMES MQTT: connected");
    }
}

void hermes_mqtt_loop(void) {
    if (!g_ready) return;
    if (!g_mqtt.connected()) mqtt_reconnect();
    else g_mqtt.loop();
}

void hermes_mqtt_publish_state(const char *json) {
    if (!hermes_mqtt_connected() || !json) return;
    g_mqtt.publish(g_topic_tele, json);
}

void hermes_mqtt_publish_gpio(void) {
    if (!hermes_mqtt_connected()) return;
    char buf[384];
    hermes_gpio_states_json(buf, sizeof(buf));
    g_mqtt.publish(g_topic_state, buf);
}

int hermes_mqtt_set_relay(uint8_t idx, bool on) {
    return hermes_gpio_set_relay(idx, on);
}

#else

void hermes_mqtt_defaults(hermes_mqtt_config_t *out) {
    memset(out, 0, sizeof(*out));
}
void hermes_mqtt_begin(const hermes_mqtt_config_t *cfg) { (void)cfg; }
void hermes_mqtt_loop(void) {}
bool hermes_mqtt_connected(void) { return false; }
void hermes_mqtt_publish_state(const char *json) { (void)json; }
void hermes_mqtt_publish_gpio(void) {}
int hermes_mqtt_set_relay(uint8_t idx, bool on) { (void)idx; (void)on; return -1; }

#endif
