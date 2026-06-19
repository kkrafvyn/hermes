#include "hermes_ota.h"

#include "hermes_sha256.h"

#include <stdio.h>

#include <string.h>



static uint8_t g_ota_buf[4096];

static size_t g_ota_len;



int hermes_ota_begin(hermes_ota_t *ota, const char *app_id, const char *version,

                     uint32_t total_size, const uint8_t hash[32])

{

    if (!ota || !app_id) return -1;

    memset(ota, 0, sizeof(*ota));

    strncpy(ota->app_id, app_id, sizeof(ota->app_id) - 1);

    if (version) strncpy(ota->version, version, sizeof(ota->version) - 1);

    ota->bytes_total = total_size;

    ota->state = HERMES_OTA_DOWNLOADING;

    g_ota_len = 0;

    if (hash) memcpy(ota->expected_hash, hash, 32);

    return 0;

}



int hermes_ota_write(hermes_ota_t *ota, const uint8_t *chunk, size_t len)

{

    if (!ota || !chunk) return -1;

    if (g_ota_len + len > sizeof(g_ota_buf)) return -2;

    memcpy(g_ota_buf + g_ota_len, chunk, len);

    g_ota_len += len;

    ota->bytes_received += (uint32_t)len;

    return 0;

}



int hermes_ota_finalize(hermes_ota_t *ota)

{

    if (!ota) return -1;

    ota->state = HERMES_OTA_VERIFYING;

    uint8_t digest[32];

    hermes_sha256(g_ota_buf, g_ota_len, digest);

    if (memcmp(digest, ota->expected_hash, 32) != 0) {

        ota->state = HERMES_OTA_FAILED;

        return -2;

    }

    ota->state = HERMES_OTA_APPLYING;

    return 0;

}



int hermes_ota_apply(hermes_ota_t *ota, const char *app_id)

{

    if (!ota || !app_id) return -1;

    char path[128];

    snprintf(path, sizeof(path), "data/flash/%s.bin", app_id);

#ifdef HERMES_USE_FREERTOS

    (void)path;

    ota->state = HERMES_OTA_DONE;

    return 0;

#else

    FILE *f = fopen(path, "wb");

    if (!f) return -2;

    size_t n = fwrite(g_ota_buf, 1, g_ota_len, f);

    fclose(f);

    ota->state = (n == g_ota_len) ? HERMES_OTA_DONE : HERMES_OTA_FAILED;

    return ota->state == HERMES_OTA_DONE ? 0 : -3;

#endif

}


