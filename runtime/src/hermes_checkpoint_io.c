#include "hermes_checkpoint_io.h"
#include <stdio.h>
#include <string.h>

#ifdef HERMES_USE_FREERTOS
#include "esp_log.h"
static const char *TAG = "cp_io";
#endif

static char g_storage[128] = "data/checkpoints";

int hermes_checkpoint_io_init(const char *storage_path)
{
    if (storage_path)
        strncpy(g_storage, storage_path, sizeof(g_storage) - 1);
    return 0;
}

static void checkpoint_path(const char *app_id, char *out, size_t max)
{
    snprintf(out, max, "%s/%s.cp", g_storage, app_id);
}

int hermes_checkpoint_io_save(const hermes_checkpoint_t *cp)
{
    if (!cp) return -1;
    char path[192];
    checkpoint_path(cp->app_id, path, sizeof(path));
    FILE *f = fopen(path, "wb");
    if (!f) return -2;
    size_t n = fwrite(cp, 1, sizeof(*cp), f);
    fclose(f);
#ifdef HERMES_USE_FREERTOS
    ESP_LOGI(TAG, "saved checkpoint %s (%zu bytes)", cp->app_id, n);
#endif
    return n == sizeof(*cp) ? 0 : -3;
}

int hermes_checkpoint_io_load(const char *app_id, hermes_checkpoint_t *out)
{
    if (!app_id || !out) return -1;
    char path[192];
    checkpoint_path(app_id, path, sizeof(path));
    FILE *f = fopen(path, "rb");
    if (!f) return -2;
    size_t n = fread(out, 1, sizeof(*out), f);
    fclose(f);
    return n == sizeof(*out) ? 0 : -3;
}

int hermes_checkpoint_io_delete(const char *app_id)
{
    if (!app_id) return -1;
    char path[192];
    checkpoint_path(app_id, path, sizeof(path));
    remove(path);
    return 0;
}
