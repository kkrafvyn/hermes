#include "hermes_app.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int hermes_app_manager_init(hermes_app_manager_t *mgr)
{
    if (!mgr) return -1;
    memset(mgr, 0, sizeof(*mgr));
    return 0;
}

static hermes_app_t *find_slot(hermes_app_manager_t *mgr, const char *name)
{
    for (uint32_t i = 0; i < mgr->count; i++)
        if (strcmp(mgr->apps[i].manifest.name, name) == 0) return &mgr->apps[i];
    return NULL;
}

int hermes_app_install(hermes_app_manager_t *mgr, const hermes_app_manifest_t *manifest)
{
    if (!mgr || !manifest || mgr->count >= HERMES_MAX_APPS_MGR) return -1;
    if (find_slot(mgr, manifest->name)) return -2;
    hermes_app_t *a = &mgr->apps[mgr->count++];
    a->manifest = *manifest;
    a->state = HERMES_APP_INSTALLED;
    a->install_ts = (uint32_t)time(NULL);
    return 0;
}

int hermes_app_remove(hermes_app_manager_t *mgr, const char *name)
{
    hermes_app_t *a = find_slot(mgr, name);
    if (!a) return -1;
    a->state = HERMES_APP_STOPPED;
    memset(a->manifest.name, 0, sizeof(a->manifest.name));
    return 0;
}

int hermes_app_start(hermes_app_manager_t *mgr, const char *name)
{
    hermes_app_t *a = find_slot(mgr, name);
    if (!a) return -1;
    a->state = HERMES_APP_RUNNING;
    return 0;
}

int hermes_app_pause(hermes_app_manager_t *mgr, const char *name)
{
    hermes_app_t *a = find_slot(mgr, name);
    if (!a || a->state != HERMES_APP_RUNNING) return -1;
    a->state = HERMES_APP_PAUSED;
    return 0;
}

int hermes_app_stop(hermes_app_manager_t *mgr, const char *name)
{
    hermes_app_t *a = find_slot(mgr, name);
    if (!a) return -1;
    a->state = HERMES_APP_STOPPED;
    return 0;
}

int hermes_app_update(hermes_app_manager_t *mgr, const char *name,
                      const hermes_app_manifest_t *new_manifest)
{
    hermes_app_t *a = find_slot(mgr, name);
    if (!a || !new_manifest) return -1;
    strncpy(a->previous_version, a->manifest.version, sizeof(a->previous_version) - 1);
    a->state = HERMES_APP_UPDATING;
    a->manifest = *new_manifest;
    a->state = HERMES_APP_INSTALLED;
    return 0;
}

int hermes_app_rollback(hermes_app_manager_t *mgr, const char *name)
{
    hermes_app_t *a = find_slot(mgr, name);
    if (!a || a->previous_version[0] == '\0') return -1;
    strncpy(a->manifest.version, a->previous_version, sizeof(a->manifest.version) - 1);
    a->previous_version[0] = '\0';
    return 0;
}

int hermes_app_enable(hermes_app_manager_t *mgr, const char *name, bool enabled)
{
    hermes_app_t *a = find_slot(mgr, name);
    if (!a) return -1;
    if (!enabled) a->state = HERMES_APP_STOPPED;
    return 0;
}

hermes_app_t *hermes_app_find(hermes_app_manager_t *mgr, const char *name)
{
    return find_slot(mgr, name);
}

/* Minimal JSON parser for manifest fields */
int hermes_app_parse_manifest(const char *json, hermes_app_manifest_t *out)
{
    if (!json || !out) return -1;
    memset(out, 0, sizeof(*out));
    const char *p;
    if ((p = strstr(json, "\"name\""))) {
        sscanf(p, "\"name\"%*[^\"]\"%31[^\"]\"", out->name);
    }
    if ((p = strstr(json, "\"version\""))) {
        sscanf(p, "\"version\"%*[^\"]\"%15[^\"]\"", out->version);
    }
    if ((p = strstr(json, "\"memory\""))) {
        char mem[16];
        sscanf(p, "\"memory\"%*[^\"]\"%15[^\"]\"", mem);
        if (mem[strlen(mem)-1] == 'K' || mem[strlen(mem)-1] == 'k')
            out->memory_kb = (uint32_t)atoi(mem);
        else
            out->memory_kb = (uint32_t)atoi(mem);
    }
    if ((p = strstr(json, "\"targets\""))) {
        sscanf(p, "\"targets\"%*[^[][%63[^]]]", out->targets);
    }
    return out->name[0] ? 0 : -2;
}
