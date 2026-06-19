/**
 * Unified sensor reads for all HERMES MCU targets (Arduino framework).
 */
#include "hermes_sensor.h"
#include <Arduino.h>
#include <string.h>
#include <stdio.h>

#if defined(HERMES_HTTP_SERVER)
#include "hermes_mesh.h"
#endif

static char s_ids[HERMES_MAX_SENSORS][HERMES_SENSOR_ID_LEN];
static hermes_sensor_reading_t s_readings[HERMES_MAX_SENSORS];
static size_t s_count = 0;

static float simulate_value(const char *id) {
    uint32_t t = millis() / 1000;
    uint32_t h = 0;
    for (const char *p = id; *p; ++p) h = h * 31 + (uint8_t)*p;
    uint32_t bi = (h + t) % 1000;
    float base = (float)bi / 10.0f;

    if (strcmp(id, "temp_humidity") == 0) return 20.0f + (float)(bi % 15);
    if (strcmp(id, "soil_moisture") == 0) return 25.0f + (float)(bi % 50);
    if (strcmp(id, "soil_ec") == 0) return 0.5f + (float)(bi % 30) / 10.0f;
    if (strcmp(id, "soil_npk") == 0) return (float)(bi % 100);
    if (strcmp(id, "leaf_wetness") == 0) return (float)(bi % 80);
    if (strcmp(id, "rain_gauge") == 0) return (t % 3600 < 60) ? (float)(bi % 5) : 0.0f;
    if (strcmp(id, "wind_speed") == 0) return (float)(bi % 60);
    if (strcmp(id, "wind_direction") == 0) return (float)(bi % 360);
    if (strcmp(id, "light_par") == 0) return 200.0f + (float)(bi % 800);
    if (strcmp(id, "lux") == 0) return 100.0f + (float)(bi % 900);
    if (strcmp(id, "co2") == 0) return 400.0f + (float)(bi % 1200);
    if (strcmp(id, "voc") == 0) return (float)(bi % 500);
    if (strcmp(id, "pm25") == 0) return (float)(bi % 80);
    if (strcmp(id, "ammonia") == 0) return (float)(bi % 40);
    if (strcmp(id, "water_level") == 0) return 50.0f + (float)(bi % 150);
    if (strcmp(id, "flow_meter") == 0) return (float)(bi % 80);
    if (strcmp(id, "ph") == 0) return 6.5f + (float)(bi % 15) / 10.0f;
    if (strcmp(id, "orp") == 0) return 200.0f + (float)(bi % 400);
    if (strcmp(id, "motion_pir") == 0) return (bi % 10) > 7 ? 1.0f : 0.0f;
    if (strcmp(id, "door_contact") == 0) return (bi % 5) > 3 ? 1.0f : 0.0f;
    if (strcmp(id, "smoke") == 0) return (float)(bi % 20);
    if (strcmp(id, "co_gas") == 0) return (float)(bi % 30);
    if (strcmp(id, "gas_mq") == 0) return (float)(bi % 15);
    if (strcmp(id, "water_leak") == 0) return (bi % 20) > 17 ? 1.0f : 0.0f;
    if (strcmp(id, "current_clamp") == 0) return (float)(bi % 32);
    if (strcmp(id, "voltage") == 0) return 110.0f + (float)(bi % 130);
    if (strcmp(id, "vibration") == 0) return (float)(bi % 30) / 10.0f;
    if (strcmp(id, "accelerometer") == 0) return (float)(bi % 20) / 10.0f;
    if (strcmp(id, "pressure") == 0) return 95.0f + (float)(bi % 30);
    if (strcmp(id, "ultrasonic") == 0) return 20.0f + (float)(bi % 200);
    if (strcmp(id, "weight_loadcell") == 0) return (float)(bi % 500);
    if (strcmp(id, "gps") == 0) return base;
    if (strcmp(id, "ble_rssi") == 0) return -90.0f + (float)(bi % 50);
    if (strcmp(id, "rfid") == 0) return (float)(bi % 3);
    if (strcmp(id, "shock_tilt") == 0) return (float)(bi % 45);
    if (strcmp(id, "ds18b20") == 0) return 18.0f + (float)(bi % 12);
    if (strcmp(id, "modbus_meter") == 0) return (float)(bi % 1000);
    return base;
}

static const char *sensor_unit(const char *id) {
    if (strcmp(id, "temp_humidity") == 0) return "C";
    if (strcmp(id, "soil_moisture") == 0) return "%";
    if (strcmp(id, "ph") == 0) return "pH";
    if (strcmp(id, "co2") == 0) return "ppm";
    if (strcmp(id, "pm25") == 0) return "ug/m3";
    if (strcmp(id, "ble_rssi") == 0) return "dBm";
    if (strcmp(id, "current_clamp") == 0) return "A";
    if (strcmp(id, "voltage") == 0) return "V";
    if (strcmp(id, "flow_meter") == 0) return "L/min";
    if (strcmp(id, "pressure") == 0) return "kPa";
    return "";
}

static float read_hardware(const char *id) {
#if defined(HERMES_STM32) || defined(HERMES_NRF) || defined(HERMES_ESP32) || defined(HERMES_ESP8266) || defined(ESP8266) \
    || defined(HERMES_RP2040) || defined(HERMES_AVR) || defined(HERMES_SAMD) || defined(HERMES_MSP430) \
    || defined(HERMES_EFR32) || defined(HERMES_BEKEN) || defined(HERMES_REALTEK) || defined(HERMES_NXP) \
    || defined(HERMES_RENESAS) || defined(HERMES_PIC)
    if (strcmp(id, "motion_pir") == 0 || strcmp(id, "door_contact") == 0 ||
        strcmp(id, "water_leak") == 0) {
#if defined(A0)
        return digitalRead(A0) == HIGH ? 1.0f : 0.0f;
#else
        return simulate_value(id);
#endif
    }
    if (strcmp(id, "soil_moisture") == 0 || strcmp(id, "smoke") == 0 ||
        strcmp(id, "current_clamp") == 0 || strcmp(id, "ph") == 0) {
#if defined(A0)
        int raw = analogRead(A0);
        return (float)raw / 10.23f;
#else
        return simulate_value(id);
#endif
    }
#endif
    (void)id;
    return simulate_value(id);
}

void hermes_sensor_configure(const char *const *ids, size_t count) {
    s_count = count < HERMES_MAX_SENSORS ? count : HERMES_MAX_SENSORS;
    for (size_t i = 0; i < s_count; ++i) {
        strncpy(s_ids[i], ids[i], HERMES_SENSOR_ID_LEN - 1);
        s_ids[i][HERMES_SENSOR_ID_LEN - 1] = '\0';
    }
}

void hermes_sensor_clear(void) {
    s_count = 0;
    memset(s_ids, 0, sizeof(s_ids));
}

void hermes_sensor_poll(void) {
    for (size_t i = 0; i < s_count; ++i) {
        strncpy(s_readings[i].id, s_ids[i], HERMES_SENSOR_ID_LEN - 1);
        s_readings[i].value = read_hardware(s_ids[i]);
        s_readings[i].unit = sensor_unit(s_ids[i]);
        s_readings[i].valid = 1;
    }
}

size_t hermes_sensor_count(void) { return s_count; }

const hermes_sensor_reading_t *hermes_sensor_get(size_t index) {
    return index < s_count ? &s_readings[index] : NULL;
}

int hermes_sensor_json(char *buf, size_t max) {
    if (!buf || max < 16) return 0;
    int n = snprintf(buf, max, ",\"sensors\":[");
    if (n < 0 || (size_t)n >= max) return 0;
    size_t off = (size_t)n;
    for (size_t i = 0; i < s_count; ++i) {
        int m = snprintf(buf + off, max - off,
            "%s{\"id\":\"%s\",\"value\":%.2f,\"unit\":\"%s\"}",
            i ? "," : "", s_readings[i].id, (double)s_readings[i].value,
            s_readings[i].unit ? s_readings[i].unit : "");
        if (m < 0 || off + (size_t)m >= max) break;
        off += (size_t)m;
    }
    if (off + 2 < max) {
        buf[off++] = ']';
        buf[off] = '\0';
    }
    return (int)off;
}

int hermes_sensor_json_all(char *buf, size_t max) {
    if (!buf || max < 24) return 0;
    int n = snprintf(buf, max, ",\"sensors\":[");
    if (n < 0 || (size_t)n >= max) return 0;
    size_t off = (size_t)n;
    bool first = true;
    for (size_t i = 0; i < s_count; ++i) {
        int m = snprintf(buf + off, max - off,
            "%s{\"id\":\"%s\",\"source\":\"local\",\"value\":%.2f,\"unit\":\"%s\"}",
            first ? "" : ",", s_readings[i].id, (double)s_readings[i].value,
            s_readings[i].unit ? s_readings[i].unit : "");
        if (m < 0 || off + (size_t)m >= max) break;
        off += (size_t)m;
        first = false;
    }
#if defined(HERMES_HTTP_SERVER)
    if (hermes_mesh_enabled()) {
        for (size_t mi = 0; off < max - 48; mi++) {
            const hermes_mesh_node_t *node = hermes_mesh_get_node(mi);
            if (!node) break;
            for (uint8_t r = 0; r < node->reading_count && off < max - 48; r++) {
                int m = snprintf(buf + off, max - off,
                    "%s{\"id\":\"n%u:%s\",\"source\":\"mesh\",\"node_id\":%u,\"value\":%.2f}",
                    first ? "" : ",", node->node_id, node->reading_ids[r], node->node_id,
                    (double)node->readings[r]);
                if (m < 0 || off + (size_t)m >= max) break;
                off += (size_t)m;
                first = false;
            }
        }
    }
#endif
    if (off + 2 < max) {
        buf[off++] = ']';
        buf[off] = '\0';
    }
    return (int)off;
}
