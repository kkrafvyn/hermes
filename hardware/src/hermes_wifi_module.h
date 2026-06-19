#ifndef HERMES_WIFI_MODULE_H
#define HERMES_WIFI_MODULE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** WiFi module types — configurable via HTTP dashboard / EEPROM */
#define HERMES_WIFI_BUILTIN      0  /* ESP8266, ESP32, RTL8710, BK7231 on-chip */
#define HERMES_WIFI_UART_AT      1  /* ESP-01 / generic AT over UART */
#define HERMES_WIFI_REALTEK_UART 2  /* Realtek Ameba module (RTL8710/8720) over UART */
#define HERMES_WIFI_BEKEN_UART   3  /* Beken module over UART */
#define HERMES_WIFI_ESP_COPROC   4  /* ESP8266 co-processor (HTTP on co-proc) */

typedef struct {
    uint8_t module_type;
    char ssid[32];
    char password[64];
    uint32_t uart_baud;
    uint8_t mesh_enabled;
    uint8_t mesh_max_nodes;
    uint16_t mesh_interval_sec;
} hermes_network_config_t;

const char *hermes_wifi_module_name(uint8_t type);
uint8_t hermes_wifi_default_module_type(void);
int hermes_wifi_begin(const hermes_network_config_t *cfg);
bool hermes_wifi_connected(void);
bool hermes_wifi_in_setup_mode(void);
const char *hermes_wifi_setup_ap_name(void);
int hermes_wifi_setup_connect(const char *ssid, const char *password);
void hermes_wifi_loop(void);
const char *hermes_wifi_ip_string(void);

#ifdef __cplusplus
}
#endif

#endif
