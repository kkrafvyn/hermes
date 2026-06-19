#ifndef HERMES_QUOTA_H
#define HERMES_QUOTA_H

#include <stdint.h>
#include <stdbool.h>

#define HERMES_MAX_QUOTAS 16

typedef struct {
    char app_id[32];
    float cpu_percent;
    uint32_t memory_kb;
    uint32_t storage_kb;
    uint8_t reserved_node;
    bool migration_allowed;
} hermes_quota_t;

typedef struct {
    hermes_quota_t quotas[HERMES_MAX_QUOTAS];
    uint32_t count;
} hermes_quota_manager_t;

int  hermes_quota_init(hermes_quota_manager_t *qm);
int  hermes_quota_set(hermes_quota_manager_t *qm, const char *app_id,
                      float cpu_percent, uint32_t memory_kb);
int  hermes_quota_check_cpu(hermes_quota_manager_t *qm, const char *app_id,
                            float current_cpu);
int  hermes_quota_check_memory(hermes_quota_manager_t *qm, const char *app_id,
                               uint32_t current_kb);
hermes_quota_t *hermes_quota_find(hermes_quota_manager_t *qm, const char *app_id);

int hermes_quota_enforce(hermes_quota_manager_t *qm, const char *app_id,
                         float current_cpu, uint32_t current_kb,
                         bool *throttle_out);

#endif /* HERMES_QUOTA_H */
