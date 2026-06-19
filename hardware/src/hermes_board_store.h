#ifndef HERMES_BOARD_STORE_H
#define HERMES_BOARD_STORE_H

#include "hermes_wifi_module.h"
#include "hermes_gpio.h"
#include "hermes_mqtt.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HERMES_BOARD_APP_STATE_STOPPED 0
#define HERMES_BOARD_APP_STATE_RUNNING 1
#define HERMES_BOARD_APP_STATE_PAUSED 2

typedef struct {
    char blueprint[24];
    char app_name[28];
    char display_name[32];
    char label_key[16];
    char label_value[32];
    int sample_rate_hz;
    uint8_t alert_on_high_load;
    uint8_t state;
    char sensors[200];
    char remote_sensors[128];
    uint8_t mesh_enabled;
    uint8_t mesh_max_nodes;
    uint16_t mesh_interval_sec;
} hermes_board_app_t;

void hermes_board_store_load(hermes_board_app_t *out);
void hermes_board_store_save(const hermes_board_app_t *app);
void hermes_board_store_apply_sensors(const hermes_board_app_t *app);
void hermes_board_store_load_network(hermes_network_config_t *out);
void hermes_board_store_save_network(const hermes_network_config_t *net);
void hermes_board_store_load_gpio(hermes_board_gpio_t *out);
void hermes_board_store_save_gpio(const hermes_board_gpio_t *gpio);
void hermes_board_store_load_mqtt(hermes_mqtt_config_t *out);
void hermes_board_store_save_mqtt(const hermes_mqtt_config_t *mqtt);

#ifdef __cplusplus
}
#endif

#endif
