#ifndef HERMES_OTA_H
#define HERMES_OTA_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    HERMES_OTA_IDLE = 0,
    HERMES_OTA_DOWNLOADING,
    HERMES_OTA_VERIFYING,
    HERMES_OTA_APPLYING,
    HERMES_OTA_DONE,
    HERMES_OTA_FAILED,
} hermes_ota_state_t;

typedef struct {
    char app_id[32];
    char version[16];
    hermes_ota_state_t state;
    uint32_t bytes_received;
    uint32_t bytes_total;
    uint8_t expected_hash[32];
} hermes_ota_t;

int hermes_ota_begin(hermes_ota_t *ota, const char *app_id, const char *version,
                     uint32_t total_size, const uint8_t hash[32]);
int hermes_ota_write(hermes_ota_t *ota, const uint8_t *chunk, size_t len);
int hermes_ota_finalize(hermes_ota_t *ota);
int hermes_ota_apply(hermes_ota_t *ota, const char *app_id);

#endif
