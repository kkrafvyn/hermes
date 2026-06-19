#include "hermes_policy.h"
#include "hermes_identity.h"
#include "hermes_sha256.h"
#include <string.h>

int hermes_policy_init(hermes_policy_engine_t *engine)
{
    if (!engine) return -1;
    memset(engine, 0, sizeof(*engine));
    return 0;
}

int hermes_policy_add(hermes_policy_engine_t *engine, const char *subject,
                      uint8_t permissions)
{
    if (!engine || !subject || engine->count >= HERMES_MAX_POLICIES) return -1;
    hermes_policy_t *p = &engine->policies[engine->count++];
    strncpy(p->subject, subject, sizeof(p->subject) - 1);
    p->permissions = permissions;
    return 0;
}

int hermes_policy_check(hermes_policy_engine_t *engine, const char *subject,
                        hermes_permission_t perm)
{
    if (!engine || !subject) return -1;
    for (uint32_t i = 0; i < engine->count; i++) {
        if (strcmp(engine->policies[i].subject, subject) == 0)
            return (engine->policies[i].permissions & perm) ? 0 : -2;
    }
    return -1;
}

int hermes_policy_approve_app(hermes_policy_engine_t *engine, const char *subject,
                              const char *app_id)
{
    if (!engine || !subject || !app_id) return -1;
    for (uint32_t i = 0; i < engine->count; i++) {
        hermes_policy_t *p = &engine->policies[i];
        if (strcmp(p->subject, subject) != 0) continue;
        if (p->allowed_app_count >= 4) return -2;
        strncpy(p->allowed_apps[p->allowed_app_count++], app_id, 31);
        p->app_approved = true;
        return 0;
    }
    return -1;
}

int hermes_verify_app(const hermes_signed_app_t *app, const uint8_t *payload)
{
    if (!app || !payload) return -1;
    uint8_t digest[HERMES_SHA256_DIGEST_SIZE];
    hermes_sha256(payload, app->payload_size, digest);
    return memcmp(digest, app->signature, HERMES_SHA256_DIGEST_SIZE) == 0 ? 0 : -2;
}
