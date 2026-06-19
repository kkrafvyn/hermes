/**
 * HERMES unified sensor layer — same API on ESP8266, ESP32, STM32, nRF52840.
 * Reads configured sensors and returns normalized values (hardware or simulated).
 */
#ifndef HERMES_SENSOR_H
#define HERMES_SENSOR_H

#include <stdint.h>
#include <stddef.h>

#define HERMES_MAX_SENSORS 16
#define HERMES_SENSOR_ID_LEN 24

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char id[HERMES_SENSOR_ID_LEN];
    float value;
    const char *unit;
    uint8_t valid;
} hermes_sensor_reading_t;

/** Configure active sensors from blueprint config (comma-separated or JSON array). */
void hermes_sensor_configure(const char *const *ids, size_t count);

/** Clear all configured sensors. */
void hermes_sensor_clear(void);

/** Read all configured sensors (updates internal state). */
void hermes_sensor_poll(void);

/** Number of active readings after poll. */
size_t hermes_sensor_count(void);

/** Get reading by index (0 .. count-1). */
const hermes_sensor_reading_t *hermes_sensor_get(size_t index);

/** Append sensor JSON fragment to buf: ,"sensors":[...] */
int hermes_sensor_json(char *buf, size_t max);

#ifdef __cplusplus
}
#endif

#endif /* HERMES_SENSOR_H */
