#ifndef HERMES_MEMORY_H
#define HERMES_MEMORY_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t total_bytes;
    uint32_t used_bytes;
    uint32_t peak_bytes;
    uint32_t app_budget_bytes;
} hermes_memory_stats_t;

typedef struct {
    hermes_memory_stats_t stats;
    void *heap_start;
    size_t heap_size;
} hermes_memory_manager_t;

int  hermes_memory_init(hermes_memory_manager_t *mgr, void *heap, size_t size);
void *hermes_malloc(hermes_memory_manager_t *mgr, size_t size);
void  hermes_free(hermes_memory_manager_t *mgr, void *ptr);
int   hermes_memory_stats(hermes_memory_manager_t *mgr, hermes_memory_stats_t *out);
int   hermes_memory_set_budget(hermes_memory_manager_t *mgr, uint32_t budget);

#endif /* HERMES_MEMORY_H */
