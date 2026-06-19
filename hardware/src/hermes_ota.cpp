#include "hermes_ota.h"
#include "hermes_wifi_module.h"

#include <Arduino.h>

#if (defined(HERMES_ESP8266) || defined(ESP8266) || defined(HERMES_ESP32)) && defined(HERMES_OTA)

#include <ArduinoOTA.h>

#ifndef HERMES_NODE_ID
#define HERMES_NODE_ID 0
#endif

static bool g_ota = false;

void hermes_ota_begin(void) {
    if (hermes_wifi_in_setup_mode()) return;
    char hostname[20];
    snprintf(hostname, sizeof(hostname), "hermes-%d", HERMES_NODE_ID);
    ArduinoOTA.setHostname(hostname);
    ArduinoOTA.onStart([]() { Serial.println("HERMES OTA: start"); });
    ArduinoOTA.onEnd([]() { Serial.println("HERMES OTA: end"); });
    ArduinoOTA.onError([](ota_error_t e) { Serial.printf("HERMES OTA error %u\n", e); });
    ArduinoOTA.begin();
    g_ota = true;
    Serial.printf("HERMES OTA: ready (%s.local)\n", hostname);
}

void hermes_ota_loop(void) {
    if (g_ota && hermes_wifi_connected()) ArduinoOTA.handle();
}

#else

void hermes_ota_begin(void) {}
void hermes_ota_loop(void) {}

#endif
