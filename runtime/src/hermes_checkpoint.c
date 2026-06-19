#include "hermes_checkpoint.h"
#include <string.h>

int hermes_checkpoint_create(const char *app_id, const uint8_t *state, size_t state_len,
                             const uint8_t *config, size_t config_len,
                             hermes_checkpoint_t *out)
{
    if (!app_id || !out) return -1;
    if (state_len + config_len > HERMES_CHECKPOINT_MAX_SIZE) return -2;
    memset(out, 0, sizeof(*out));
    out->version = HERMES_CHECKPOINT_VERSION;
    strncpy(out->app_id, app_id, sizeof(out->app_id) - 1);
    out->state_size = (uint32_t)state_len;
    out->config_size = (uint32_t)config_len;
    if (state && state_len) memcpy(out->data, state, state_len);
    if (config && config_len) memcpy(out->data + state_len, config, config_len);
    return 0;
}

int hermes_checkpoint_restore(const hermes_checkpoint_t *cp,
                              uint8_t *state_out, size_t state_max,
                              uint8_t *config_out, size_t config_max)
{
    if (!cp) return -1;
    if (state_out && cp->state_size <= state_max)
        memcpy(state_out, cp->data, cp->state_size);
    if (config_out && cp->config_size <= config_max)
        memcpy(config_out, cp->data + cp->state_size, cp->config_size);
    return 0;
}
