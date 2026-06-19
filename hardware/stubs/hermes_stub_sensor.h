/**
 * Portable sensor layer for HERMES stub nodes (generic_node.c).
 * Same 37 sensor IDs as sensors/catalog.json — simulation until HAL is wired.
 */
#ifndef HERMES_STUB_SENSOR_H
#define HERMES_STUB_SENSOR_H

#include <stddef.h>

#define HERMES_STUB_MAX_SENSORS 16
#define HERMES_STUB_SENSOR_ID_LEN 32

typedef struct {
    char id[HERMES_STUB_SENSOR_ID_LEN];
    float value;
    const char *unit;
    int valid;
} hermes_stub_reading_t;

void hermes_stub_sensor_clear(void);
void hermes_stub_sensor_configure(const char *const *ids, size_t count);
void hermes_stub_sensor_poll(void);
size_t hermes_stub_sensor_count(void);
int hermes_stub_sensor_json(char *buf, size_t max);
int hermes_stub_parse_sensors_json(const char *json);

#endif
