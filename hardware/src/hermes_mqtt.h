#ifndef HERMES_MQTT_H
#define HERMES_MQTT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t enabled;
    uint16_t port;
    char host[48];
    char user[24];
    char pass[32];
    char prefix[16];
} hermes_mqtt_config_t;

void hermes_mqtt_defaults(hermes_mqtt_config_t *out);
void hermes_mqtt_begin(const hermes_mqtt_config_t *cfg);
void hermes_mqtt_loop(void);
bool hermes_mqtt_connected(void);
void hermes_mqtt_publish_state(const char *json);
void hermes_mqtt_publish_gpio(void);
int hermes_mqtt_set_relay(uint8_t idx, bool on);

#ifdef __cplusplus
}
#endif

#endif
