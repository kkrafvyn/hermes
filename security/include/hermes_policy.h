#ifndef HERMES_POLICY_H
#define HERMES_POLICY_H

#include "hermes_identity.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HERMES_PERM_INSTALL  = 1,
    HERMES_PERM_START    = 2,
    HERMES_PERM_MIGRATE  = 4,
    HERMES_PERM_ADMIN    = 8,
} hermes_permission_t;

typedef struct {
    char subject[32];
    uint8_t permissions;
    bool app_approved;
    char allowed_apps[4][32];
    uint8_t allowed_app_count;
} hermes_policy_t;

#define HERMES_MAX_POLICIES 8

typedef struct {
    hermes_policy_t policies[HERMES_MAX_POLICIES];
    uint32_t count;
} hermes_policy_engine_t;

int  hermes_policy_init(hermes_policy_engine_t *engine);
int  hermes_policy_add(hermes_policy_engine_t *engine, const char *subject,
                       uint8_t permissions);
int  hermes_policy_check(hermes_policy_engine_t *engine, const char *subject,
                         hermes_permission_t perm);
int  hermes_policy_approve_app(hermes_policy_engine_t *engine, const char *subject,
                               const char *app_id);
int  hermes_verify_app(const hermes_signed_app_t *app, const uint8_t *payload);

#endif /* HERMES_POLICY_H */
