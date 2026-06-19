#include "hermes_link.h"
#include "hermes_task.h"
#include "hermes_memory.h"
#include "hermes_app.h"
#include "hermes_quota.h"
#include "hermes_migration.h"
#include <assert.h>
#include <string.h>

static void dummy_task(void *arg) { (void)arg; }

static int stub_send(const uint8_t *d, size_t l, void *c) { (void)d;(void)l;(void)c; return (int)l; }
static int stub_recv(uint8_t *d, size_t m, uint32_t t, void *c) { (void)d;(void)m;(void)t;(void)c; return 0; }

int main(void)
{
    /* Phase 2: Device OS */
    hermes_task_manager_t tm;
    assert(hermes_task_manager_init(&tm) == 0);
    uint32_t tid;
    assert(hermes_create_task(&tm, "test", dummy_task, NULL, 1024, 5, &tid) == 0);
    tm.tasks[0].state = HERMES_TASK_RUNNING;
    assert(hermes_pause_task(&tm, tid) == 0);
    assert(hermes_resume_task(&tm, tid) == 0);
    assert(hermes_destroy_task(&tm, tid) == 0);

    uint8_t heap[4096];
    hermes_memory_manager_t mm;
    assert(hermes_memory_init(&mm, heap, sizeof(heap)) == 0);
    void *p = hermes_malloc(&mm, 128);
    assert(p != NULL);
    hermes_free(&mm, p);

    /* Phase 4: Application Platform */
    hermes_app_manager_t am;
    assert(hermes_app_manager_init(&am) == 0);
    hermes_app_manifest_t manifest;
    assert(hermes_app_parse_manifest(
        "{\"name\":\"test\",\"version\":\"1.0\",\"memory\":\"64KB\"}", &manifest) == 0);
    assert(hermes_app_install(&am, &manifest) == 0);
    assert(hermes_app_start(&am, "test") == 0);

    /* Phase 5: Quotas */
    hermes_quota_manager_t qm;
    assert(hermes_quota_init(&qm) == 0);
    assert(hermes_quota_set(&qm, "test", 40.0f, 64) == 0);
    assert(hermes_quota_check_cpu(&qm, "test", 30.0f) == 0);

    /* Phase 6: Migration state machine */
    hermes_migration_t mig;
    hermes_link_t link;
    hermes_link_transport_t transport = { .send = stub_send, .recv = stub_recv, .ctx = NULL };
    hermes_link_init(&link, &transport, 0, HERMES_NODE_STM32H7);
    assert(hermes_migration_begin(&mig, "test", 0, 1, &link) == 0);
    while (mig.state != HERMES_MIG_DONE && mig.state != HERMES_MIG_FAILED)
        hermes_migration_step(&mig, NULL, NULL, NULL);
    assert(mig.state == HERMES_MIG_DONE);

    return 0;
}
