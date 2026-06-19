#include "hermes_memory.h"
#include <string.h>
#include <stdlib.h>

typedef struct block_header {
    size_t size;
    struct block_header *next;
} block_header_t;

typedef struct {
    block_header_t *free_list;
} heap_state_t;

static heap_state_t g_heap;

int hermes_memory_init(hermes_memory_manager_t *mgr, void *heap, size_t size)
{
    if (!mgr || !heap || size < 64) return -1;
    memset(mgr, 0, sizeof(*mgr));
    mgr->heap_start = heap;
    mgr->heap_size = size;
    mgr->stats.total_bytes = (uint32_t)size;
    g_heap.free_list = (block_header_t *)heap;
    g_heap.free_list->size = size - sizeof(block_header_t);
    g_heap.free_list->next = NULL;
    return 0;
}

void *hermes_malloc(hermes_memory_manager_t *mgr, size_t size)
{
    if (!mgr || size == 0) return NULL;
    size = (size + 7) & ~7u;
    block_header_t *prev = NULL;
    block_header_t *cur = g_heap.free_list;
    while (cur) {
        if (cur->size >= size) {
            void *ptr = (uint8_t *)cur + sizeof(block_header_t);
            if (cur->size >= size + sizeof(block_header_t) + 8) {
                block_header_t *split = (block_header_t *)((uint8_t *)cur + sizeof(block_header_t) + size);
                split->size = cur->size - size - sizeof(block_header_t);
                split->next = cur->next;
                cur->size = size;
                cur->next = split;
            }
            if (prev) prev->next = cur->next;
            else g_heap.free_list = cur->next;
            mgr->stats.used_bytes += (uint32_t)(size + sizeof(block_header_t));
            if (mgr->stats.used_bytes > mgr->stats.peak_bytes)
                mgr->stats.peak_bytes = mgr->stats.used_bytes;
            return ptr;
        }
        prev = cur;
        cur = cur->next;
    }
    return NULL;
}

void hermes_free(hermes_memory_manager_t *mgr, void *ptr)
{
    if (!mgr || !ptr) return;
    block_header_t *block = (block_header_t *)((uint8_t *)ptr - sizeof(block_header_t));
    block->next = g_heap.free_list;
    g_heap.free_list = block;
    mgr->stats.used_bytes -= (uint32_t)(block->size + sizeof(block_header_t));
}

int hermes_memory_stats(hermes_memory_manager_t *mgr, hermes_memory_stats_t *out)
{
    if (!mgr || !out) return -1;
    *out = mgr->stats;
    return 0;
}

int hermes_memory_set_budget(hermes_memory_manager_t *mgr, uint32_t budget)
{
    if (!mgr) return -1;
    mgr->stats.app_budget_bytes = budget;
    return 0;
}
