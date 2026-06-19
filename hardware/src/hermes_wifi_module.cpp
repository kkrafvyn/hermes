#include "hermes_wifi_module.h"

#include <Arduino.h>
#include <string.h>

#if defined(HERMES_ESP8266) || defined(ESP8266)
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#define HERMES_HAS_BUILTIN_WIFI 1
#define HERMES_HAS_WIFI_SETUP 1
#elif defined(HERMES_ESP32)
#include <WiFi.h>
#include <DNSServer.h>
#define HERMES_HAS_BUILTIN_WIFI 1
#define HERMES_HAS_WIFI_SETUP 1
#elif defined(HERMES_REALTEK) || defined(HERMES_BEKEN)
#include <WiFi.h>
#define HERMES_HAS_BUILTIN_WIFI 1
#endif

#ifndef HERMES_WIFI_UART_PORT
#define HERMES_WIFI_UART_PORT Serial1
#endif

static hermes_network_config_t g_net;
static bool g_connected = false;
static bool g_setup_mode = false;
static char g_ip[16] = "0.0.0.0";
static char g_ap_name[24] = "HERMES-Setup";

#if defined(HERMES_HAS_WIFI_SETUP)
static DNSServer g_dns;
#endif

const char *hermes_wifi_module_name(uint8_t type) {
    switch (type) {
    case HERMES_WIFI_BUILTIN: return "builtin";
    case HERMES_WIFI_UART_AT: return "uart_at";
    case HERMES_WIFI_REALTEK_UART: return "realtek_uart";
    case HERMES_WIFI_BEKEN_UART: return "beken_uart";
    case HERMES_WIFI_ESP_COPROC: return "esp_coproc";
    default: return "unknown";
    }
}

uint8_t hermes_wifi_default_module_type(void) {
#if defined(HERMES_HAS_BUILTIN_WIFI)
    return HERMES_WIFI_BUILTIN;
#elif defined(HERMES_WIFI_UART_AT_DEFAULT)
    return HERMES_WIFI_UART_AT;
#else
    return HERMES_WIFI_UART_AT;
#endif
}

static void uart_at_cmd(const char *cmd, uint32_t timeout_ms = 3000) {
    HERMES_WIFI_UART_PORT.println(cmd);
    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        while (HERMES_WIFI_UART_PORT.available()) {
            Serial.write(HERMES_WIFI_UART_PORT.read());
        }
        delay(10);
    }
}

static bool wifi_uart_at_begin(const hermes_network_config_t *cfg) {
    HERMES_WIFI_UART_PORT.begin(cfg->uart_baud ? cfg->uart_baud : 115200);
    delay(200);
    uart_at_cmd("AT");
    uart_at_cmd("AT+CWMODE=1");
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", cfg->ssid, cfg->password);
    uart_at_cmd(cmd, 15000);
    uart_at_cmd("AT+CIFSR", 2000);
    strncpy(g_ip, "uart-module", sizeof(g_ip) - 1);
    g_connected = cfg->ssid[0] != '\0';
    Serial.printf("HERMES WiFi: UART AT module (%s)\n", g_connected ? "joined" : "not configured");
    return g_connected;
}

static bool wifi_realtek_uart_begin(const hermes_network_config_t *cfg) {
    HERMES_WIFI_UART_PORT.begin(cfg->uart_baud ? cfg->uart_baud : 115200);
    delay(300);
    char cmd[160];
    snprintf(cmd, sizeof(cmd), "{\"cmd\":\"wifi_join\",\"ssid\":\"%s\",\"pass\":\"%s\"}", cfg->ssid, cfg->password);
    HERMES_WIFI_UART_PORT.println(cmd);
    strncpy(g_ip, "realtek-uart", sizeof(g_ip) - 1);
    g_connected = cfg->ssid[0] != '\0';
    Serial.println("HERMES WiFi: Realtek UART module — join command sent");
    return g_connected;
}

static bool wifi_beken_uart_begin(const hermes_network_config_t *cfg) {
    return wifi_realtek_uart_begin(cfg);
}

static bool wifi_builtin_try_sta(const hermes_network_config_t *cfg) {
#if defined(HERMES_HAS_BUILTIN_WIFI)
    if (!cfg->ssid[0]) return false;
    WiFi.mode(WIFI_STA);
    WiFi.begin(cfg->ssid, cfg->password);
    Serial.printf("HERMES WiFi: connecting to %s ...\n", cfg->ssid);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("HERMES WiFi: STA connection failed");
        return false;
    }
    strncpy(g_ip, WiFi.localIP().toString().c_str(), sizeof(g_ip) - 1);
    g_connected = true;
    g_setup_mode = false;
    Serial.printf("HERMES WiFi: connected %s\n", g_ip);
    return true;
#else
    (void)cfg;
    return false;
#endif
}

#if defined(HERMES_HAS_WIFI_SETUP)
static void wifi_build_ap_name(void) {
    uint8_t mac[6] = {0};
    WiFi.macAddress(mac);
    snprintf(g_ap_name, sizeof(g_ap_name), "HERMES-%02X%02X%02X", mac[3], mac[4], mac[5]);
}

static bool wifi_start_setup_ap(void) {
    wifi_build_ap_name();
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    if (!WiFi.softAP(g_ap_name)) {
        Serial.println("HERMES WiFi: failed to start setup AP");
        return false;
    }
    g_dns.start(53, "*", WiFi.softAPIP());
    strncpy(g_ip, WiFi.softAPIP().toString().c_str(), sizeof(g_ip) - 1);
    g_connected = false;
    g_setup_mode = true;
    Serial.printf("HERMES WiFi: setup AP \"%s\" — open http://%s/\n", g_ap_name, g_ip);
    Serial.println("HERMES WiFi: connect your phone to this network to configure WiFi");
    return true;
}
#endif

static bool wifi_builtin_begin(const hermes_network_config_t *cfg) {
#if defined(HERMES_HAS_BUILTIN_WIFI)
    if (wifi_builtin_try_sta(cfg)) return true;
#if defined(HERMES_HAS_WIFI_SETUP)
    return wifi_start_setup_ap();
#else
    if (!cfg->ssid[0])
        Serial.println("HERMES WiFi: SSID not set — configure via HTTP /api/network/config");
    return false;
#endif
#else
    (void)cfg;
    Serial.println("HERMES WiFi: no built-in radio — set module type to uart_at or realtek_uart");
    return false;
#endif
}

int hermes_wifi_begin(const hermes_network_config_t *cfg) {
    if (!cfg) return -1;
    memcpy(&g_net, cfg, sizeof(g_net));
    g_connected = false;
    strncpy(g_ip, "0.0.0.0", sizeof(g_ip));

    switch (cfg->module_type) {
    case HERMES_WIFI_UART_AT:
        return wifi_uart_at_begin(cfg) ? 0 : -1;
    case HERMES_WIFI_REALTEK_UART:
        return wifi_realtek_uart_begin(cfg) ? 0 : -1;
    case HERMES_WIFI_BEKEN_UART:
        return wifi_beken_uart_begin(cfg) ? 0 : -1;
    case HERMES_WIFI_ESP_COPROC:
        return wifi_uart_at_begin(cfg) ? 0 : -1;
    case HERMES_WIFI_BUILTIN:
    default:
        return wifi_builtin_begin(cfg) ? 0 : -1;
    }
}

bool hermes_wifi_connected(void) {
#if defined(HERMES_HAS_BUILTIN_WIFI)
    if (g_net.module_type == HERMES_WIFI_BUILTIN)
        return WiFi.status() == WL_CONNECTED;
#endif
    return g_connected;
}

bool hermes_wifi_in_setup_mode(void) {
    return g_setup_mode;
}

const char *hermes_wifi_setup_ap_name(void) {
    return g_ap_name;
}

int hermes_wifi_setup_connect(const char *ssid, const char *password) {
    if (!ssid || !ssid[0]) return -1;
    strncpy(g_net.ssid, ssid, sizeof(g_net.ssid) - 1);
    g_net.ssid[sizeof(g_net.ssid) - 1] = '\0';
    if (password)
        strncpy(g_net.password, password, sizeof(g_net.password) - 1);
    else
        g_net.password[0] = '\0';
    g_net.password[sizeof(g_net.password) - 1] = '\0';

#if defined(HERMES_HAS_WIFI_SETUP)
    g_dns.stop();
#endif
    WiFi.softAPdisconnect(true);

    if (!wifi_builtin_try_sta(&g_net)) {
#if defined(HERMES_HAS_WIFI_SETUP)
        wifi_start_setup_ap();
#endif
        return -1;
    }
    return 0;
}

void hermes_wifi_loop(void) {
#if defined(HERMES_HAS_WIFI_SETUP)
    if (g_setup_mode)
        g_dns.processNextRequest();
#endif
#if defined(HERMES_HAS_BUILTIN_WIFI)
    if (g_net.module_type == HERMES_WIFI_BUILTIN && g_connected)
        strncpy(g_ip, WiFi.localIP().toString().c_str(), sizeof(g_ip) - 1);
#endif
}

const char *hermes_wifi_ip_string(void) {
    return g_ip;
}
