/**
 * Stub sensor reads — portable C (no Arduino). Matches hardware/src/hermes_sensor.cpp.
 */
#include "hermes_stub_sensor.h"
#include <stdio.h>
#include <string.h>

static char s_ids[HERMES_STUB_MAX_SENSORS][HERMES_STUB_SENSOR_ID_LEN];
static hermes_stub_reading_t s_readings[HERMES_STUB_MAX_SENSORS];
static size_t s_count = 0;

static unsigned long stub_millis(void) {
    static unsigned long t;
    return t += 3000;
}

static float simulate_value(const char *id) {
    unsigned long ms = stub_millis();
    unsigned h = 0;
    for (const char *p = id; *p; ++p)
        h = h * 31u + (unsigned char)*p;
    unsigned bi = (h + (unsigned)(ms / 1000)) % 1000u;
    float base = (float)bi / 10.0f;
    unsigned long t = ms / 1000;

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

void hermes_stub_sensor_clear(void) {
    s_count = 0;
    memset(s_ids, 0, sizeof(s_ids));
}

void hermes_stub_sensor_configure(const char *const *ids, size_t count) {
    s_count = count < HERMES_STUB_MAX_SENSORS ? count : HERMES_STUB_MAX_SENSORS;
    for (size_t i = 0; i < s_count; ++i) {
        strncpy(s_ids[i], ids[i], HERMES_STUB_SENSOR_ID_LEN - 1);
        s_ids[i][HERMES_STUB_SENSOR_ID_LEN - 1] = '\0';
    }
}

void hermes_stub_sensor_poll(void) {
    for (size_t i = 0; i < s_count; ++i) {
        strncpy(s_readings[i].id, s_ids[i], HERMES_STUB_SENSOR_ID_LEN - 1);
        s_readings[i].value = simulate_value(s_ids[i]);
        s_readings[i].unit = sensor_unit(s_ids[i]);
        s_readings[i].valid = 1;
    }
}

size_t hermes_stub_sensor_count(void) { return s_count; }

int hermes_stub_sensor_json(char *buf, size_t max) {
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

int hermes_stub_parse_sensors_json(const char *json) {
    const char *p = strstr(json, "\"sensors\"");
    if (!p) return 0;
    p = strchr(p, '[');
    if (!p) return 0;
    p++;
    const char *ids[HERMES_STUB_MAX_SENSORS];
    char id_bufs[HERMES_STUB_MAX_SENSORS][HERMES_STUB_SENSOR_ID_LEN];
    size_t n = 0;
    while (*p && *p != ']' && n < HERMES_STUB_MAX_SENSORS) {
        const char *q1 = strchr(p, '"');
        if (!q1) break;
        q1++;
        const char *q2 = strchr(q1, '"');
        if (!q2) break;
        size_t len = (size_t)(q2 - q1);
        if (len >= HERMES_STUB_SENSOR_ID_LEN) len = HERMES_STUB_SENSOR_ID_LEN - 1;
        memcpy(id_bufs[n], q1, len);
        id_bufs[n][len] = '\0';
        ids[n] = id_bufs[n];
        n++;
        const char *comma = strchr(q2 + 1, ',');
        const char *end = strchr(q2 + 1, ']');
        if (!comma || (end && end < comma)) break;
        p = comma + 1;
    }
    if (n > 0)
        hermes_stub_sensor_configure(ids, n);
    return (int)n;
}
