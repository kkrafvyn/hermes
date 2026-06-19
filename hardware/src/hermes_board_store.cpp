#include "hermes_board_store.h"
#include "hermes_sensor.h"
#include "hermes_templates.h"
#include "hermes_mqtt.h"

#if defined(HERMES_ESP8266) || defined(ESP8266) || defined(HERMES_ESP32) || defined(ESP32) \
    || defined(HERMES_AVR) || defined(HERMES_STM32) || defined(HERMES_SAMD) \
    || defined(HERMES_BEKEN) || defined(HERMES_REALTEK)
#define HERMES_BOARD_EEPROM 1
#endif

#include <Arduino.h>
#if defined(HERMES_BOARD_EEPROM)
#include <EEPROM.h>
#endif
#include <string.h>

#define STORE_MAGIC 0x484D5242u
#define STORE_VERSION 5
#define EEPROM_SIZE 1664
#define OFF_APP 8
#define OFF_NET 320
#define OFF_MQTT 520
#define OFF_GPIO 640

static hermes_board_app_t g_app;
static hermes_network_config_t g_net;
static hermes_board_gpio_t g_gpio;
static hermes_mqtt_config_t g_mqtt;

static void app_defaults(hermes_board_app_t *a) {
    memset(a, 0, sizeof(*a));
    strncpy(a->blueprint, "telemetry", sizeof(a->blueprint) - 1);
    strncpy(a->app_name, "telemetry-hub", sizeof(a->app_name) - 1);
    strncpy(a->display_name, "Telemetry Hub", sizeof(a->display_name) - 1);
    strncpy(a->label_key, "hub_name", sizeof(a->label_key) - 1);
    strncpy(a->label_value, "Telemetry Hub", sizeof(a->label_value) - 1);
    a->sample_rate_hz = 10;
    a->alert_on_high_load = 1;
    a->state = HERMES_BOARD_APP_STATE_STOPPED;
    a->mesh_max_nodes = 8;
    a->mesh_enabled = 0;
    a->mesh_interval_sec = 120;
    strncpy(a->sensors, "current_clamp,voltage,pressure", sizeof(a->sensors) - 1);
    a->remote_sensors[0] = '\0';
}

static void gpio_defaults(hermes_board_gpio_t *g) {
    hermes_gpio_defaults(g);
    hermes_template_apply_build_default(g);
}

static void net_defaults(hermes_network_config_t *n) {
    memset(n, 0, sizeof(*n));
    n->module_type = hermes_wifi_default_module_type();
    n->uart_baud = 115200;
    n->mesh_enabled = 0;
    n->mesh_max_nodes = 8;
    n->mesh_interval_sec = 120;
}

static void mqtt_defaults(hermes_mqtt_config_t *m) {
    hermes_mqtt_defaults(m);
}

static void persist_all(void) {
#if defined(HERMES_BOARD_EEPROM)
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(0, STORE_MAGIC);
    uint8_t ver = STORE_VERSION;
    EEPROM.put(4, ver);
    EEPROM.put(OFF_APP, g_app);
    EEPROM.put(OFF_NET, g_net);
    EEPROM.put(OFF_MQTT, g_mqtt);
    EEPROM.put(OFF_GPIO, g_gpio);
    EEPROM.commit();
    EEPROM.end();
#endif
}

void hermes_board_store_apply_sensors(const hermes_board_app_t *app) {
    const char *ids[HERMES_MAX_SENSORS];
    char id_bufs[HERMES_MAX_SENSORS][HERMES_SENSOR_ID_LEN];
    size_t count = 0;
    const char *p = app->sensors;
    while (*p && count < HERMES_MAX_SENSORS) {
        while (*p == ' ' || *p == ',') p++;
        if (!*p) break;
        size_t i = 0;
        while (p[i] && p[i] != ',' && i < HERMES_SENSOR_ID_LEN - 1) {
            id_bufs[count][i] = p[i];
            i++;
        }
        id_bufs[count][i] = '\0';
        if (i > 0) {
            ids[count] = id_bufs[count];
            count++;
        }
        p += i;
    }
    if (count > 0)
        hermes_sensor_configure(ids, count);
    else
        hermes_sensor_clear();
}

void hermes_board_store_load(hermes_board_app_t *out) {
    app_defaults(out);
    net_defaults(&g_net);
    gpio_defaults(&g_gpio);
    mqtt_defaults(&g_mqtt);
#if defined(HERMES_BOARD_EEPROM)
    EEPROM.begin(EEPROM_SIZE);
    uint32_t magic = 0;
    uint8_t ver = 0;
    EEPROM.get(0, magic);
    EEPROM.get(4, ver);
    if (magic != STORE_MAGIC) {
        EEPROM.end();
        memcpy(&g_app, out, sizeof(g_app));
        return;
    }
    EEPROM.get(OFF_APP, *out);
    EEPROM.get(OFF_NET, g_net);
    if (ver >= 4) EEPROM.get(OFF_MQTT, g_mqtt);
    if (ver >= 3) EEPROM.get(OFF_GPIO, g_gpio);
    EEPROM.end();
#else
    memcpy(&g_app, out, sizeof(g_app));
#endif
    if (!out->blueprint[0]) app_defaults(out);
    if (ver < 5) {
        out->mesh_enabled = 0;
        out->remote_sensors[0] = '\0';
    }
    if (!g_net.mesh_max_nodes) net_defaults(&g_net);
    if (!g_gpio.template_id[0]) gpio_defaults(&g_gpio);
    memcpy(&g_app, out, sizeof(g_app));
    hermes_board_store_apply_sensors(out);
}

void hermes_board_store_save(const hermes_board_app_t *app) {
    memcpy(&g_app, app, sizeof(g_app));
    persist_all();
    hermes_board_store_apply_sensors(app);
}

void hermes_board_store_load_network(hermes_network_config_t *out) {
    hermes_board_app_t tmp;
    hermes_board_store_load(&tmp);
    memcpy(out, &g_net, sizeof(*out));
}

void hermes_board_store_save_network(const hermes_network_config_t *net) {
    memcpy(&g_net, net, sizeof(g_net));
    persist_all();
}

void hermes_board_store_load_gpio(hermes_board_gpio_t *out) {
    hermes_board_app_t tmp;
    hermes_board_store_load(&tmp);
    memcpy(out, &g_gpio, sizeof(*out));
}

void hermes_board_store_save_gpio(const hermes_board_gpio_t *gpio) {
    memcpy(&g_gpio, gpio, sizeof(g_gpio));
    persist_all();
    hermes_gpio_apply(&g_gpio);
}

void hermes_board_store_load_mqtt(hermes_mqtt_config_t *out) {
    hermes_board_app_t tmp;
    hermes_board_store_load(&tmp);
    memcpy(out, &g_mqtt, sizeof(*out));
}

void hermes_board_store_save_mqtt(const hermes_mqtt_config_t *mqtt) {
    memcpy(&g_mqtt, mqtt, sizeof(g_mqtt));
    persist_all();
    hermes_mqtt_begin(&g_mqtt);
}

uint8_t hermes_node_app_state(void) {
    return g_app.state;
}

void hermes_node_set_app_state(uint8_t state) {
    g_app.state = state;
    hermes_board_store_save(&g_app);
}
