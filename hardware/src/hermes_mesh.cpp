#include "hermes_mesh.h"

#include <Arduino.h>
#include <string.h>

static hermes_mesh_node_t g_nodes[HERMES_MESH_MAX_NODES];
static uint8_t g_mesh_enabled = 0;
static uint8_t g_mesh_max = HERMES_MESH_MAX_NODES;
static uint16_t g_mesh_interval_sec = 120;
static uint32_t g_last_mesh_poll = 0;
static uint32_t g_mesh_seq = 0;
static char g_remote_filter[96];

static hermes_mesh_node_t *find_node(uint8_t node_id) {
    for (size_t i = 0; i < HERMES_MESH_MAX_NODES; i++) {
        if (g_nodes[i].node_id == node_id)
            return &g_nodes[i];
    }
    return nullptr;
}

static hermes_mesh_node_t *alloc_node(uint8_t node_id) {
    hermes_mesh_node_t *existing = find_node(node_id);
    if (existing) return existing;
    for (size_t i = 0; i < HERMES_MESH_MAX_NODES; i++) {
        if (g_nodes[i].node_id == 0 && g_nodes[i].name[0] == '\0') {
            g_nodes[i].node_id = node_id;
            return &g_nodes[i];
        }
    }
    return nullptr;
}

static void parse_mesh_join(const char *json) {
    int node_id = 0;
    const char *p = strstr(json, "\"node_id\"");
    if (p) node_id = atoi(strchr(p, ':') + 1);
    if (node_id <= 0) return;
    hermes_mesh_node_t *n = alloc_node((uint8_t)node_id);
    if (!n) return;
    const char *name_p = strstr(json, "\"name\"");
    if (name_p) {
        const char *q = strchr(name_p + 6, '"');
        if (q) {
            q++;
            const char *end = strchr(q, '"');
            if (end) {
                size_t len = (size_t)(end - q);
                if (len >= sizeof(n->name)) len = sizeof(n->name) - 1;
                memcpy(n->name, q, len);
                n->name[len] = '\0';
            }
        }
    }
    const char *sens = strstr(json, "\"sensors\"");
    if (sens) {
        const char *br = strchr(sens, '[');
        if (br) {
            size_t pos = 0;
            br++;
            n->sensors[0] = '\0';
            while (*br && *br != ']' && pos < sizeof(n->sensors) - 1) {
                if (*br == '"') {
                    br++;
                    while (*br && *br != '"' && pos < sizeof(n->sensors) - 2) {
                        if (pos > 0) n->sensors[pos++] = ',';
                        n->sensors[pos++] = *br++;
                    }
                    if (*br == '"') br++;
                } else {
                    br++;
                }
            }
            n->sensors[pos] = '\0';
        }
    }
    n->online = 1;
    n->last_seen_ms = millis();
    Serial.printf("HERMES mesh: joined node %u (%s)\n", node_id, n->name);
}

static int remote_type_allowed(const char *sensor_id) {
    if (!g_remote_filter[0]) return 1;
    const char *p = g_remote_filter;
    while (*p) {
        while (*p == ' ' || *p == ',') p++;
        if (!*p) break;
        size_t i = 0;
        while (p[i] && p[i] != ',' && i < 24) i++;
        if (i > 0 && strncmp(p, sensor_id, i) == 0 && strlen(sensor_id) == i)
            return 1;
        p += i;
    }
    return 0;
}

static void parse_mesh_reading(const char *json) {
    int node_id = 0;
    const char *p = strstr(json, "\"node_id\"");
    if (p) node_id = atoi(strchr(p, ':') + 1);
    if (node_id <= 0) return;
    hermes_mesh_node_t *n = alloc_node((uint8_t)node_id);
    if (!n) return;
    n->online = 1;
    n->last_seen_ms = millis();
    n->reading_count = 0;
    const char *sens = strstr(json, "\"sensors\"");
    if (!sens) return;
    const char *br = strchr(sens, '[');
    if (!br) return;
    br++;
    while (*br && *br != ']' && n->reading_count < HERMES_MESH_MAX_SENSORS) {
        const char *id_p = strstr(br, "\"id\"");
        const char *val_p = strstr(br, "\"value\"");
        if (!id_p || id_p > strchr(br, ']')) break;
        const char *q = strchr(id_p + 4, '"');
        if (!q) break;
        q++;
        const char *end = strchr(q, '"');
        if (!end) break;
        size_t len = (size_t)(end - q);
        if (len >= sizeof(n->reading_ids[0])) len = sizeof(n->reading_ids[0]) - 1;
        memcpy(n->reading_ids[n->reading_count], q, len);
        n->reading_ids[n->reading_count][len] = '\0';
        float val = 0.0f;
        if (val_p && val_p < strchr(br, ']')) {
            val = (float)atof(strchr(val_p, ':') + 1);
        }
        if (!remote_type_allowed(n->reading_ids[n->reading_count])) {
            br = end + 1;
            continue;
        }
        n->readings[n->reading_count] = val;
        n->reading_count++;
        br = end + 1;
    }
}

void hermes_mesh_init(const hermes_network_config_t *net) {
    memset(g_nodes, 0, sizeof(g_nodes));
    g_mesh_enabled = net && net->mesh_enabled;
    g_mesh_max = net && net->mesh_max_nodes ? net->mesh_max_nodes : HERMES_MESH_MAX_NODES;
    g_mesh_interval_sec = net && net->mesh_interval_sec ? net->mesh_interval_sec : 120;
}

void hermes_mesh_apply_app_config(const char *local_csv, const char *remote_csv,
    uint8_t max_nodes, uint16_t interval_sec) {
    if (max_nodes) g_mesh_max = max_nodes;
    if (interval_sec) g_mesh_interval_sec = interval_sec;
    (void)local_csv;
    g_remote_filter[0] = '\0';
    if (remote_csv && remote_csv[0]) {
        strncpy(g_remote_filter, remote_csv, sizeof(g_remote_filter) - 1);
    }
    g_mesh_enabled = 1;
}

uint8_t hermes_mesh_enabled(void) {
    return g_mesh_enabled;
}

void hermes_mesh_handle_line(const char *json) {
    if (!strstr(json, "\"mesh\"")) return;
    if (strstr(json, "\"join\"")) {
        parse_mesh_join(json);
        return;
    }
    if (strstr(json, "\"reading\"") || strstr(json, "\"read\"")) {
        parse_mesh_reading(json);
    }
}

void hermes_mesh_loop(void) {
    if (!g_mesh_enabled) return;
    uint32_t now = millis();
    for (size_t i = 0; i < HERMES_MESH_MAX_NODES; i++) {
        if (g_nodes[i].node_id && now - g_nodes[i].last_seen_ms > (uint32_t)g_mesh_interval_sec * 3000U) {
            g_nodes[i].online = 0;
        }
    }
    if (now - g_last_mesh_poll < (uint32_t)g_mesh_interval_sec * 1000U) return;
    g_last_mesh_poll = now;
    g_mesh_seq++;
    Serial.printf("{\"mesh\":\"poll\",\"seq\":%lu,\"max_nodes\":%u}\n",
                  (unsigned long)g_mesh_seq, g_mesh_max);
}

size_t hermes_mesh_node_count(void) {
    size_t n = 0;
    for (size_t i = 0; i < HERMES_MESH_MAX_NODES; i++)
        if (g_nodes[i].node_id) n++;
    return n;
}

const hermes_mesh_node_t *hermes_mesh_get_node(size_t index) {
    size_t seen = 0;
    for (size_t i = 0; i < HERMES_MESH_MAX_NODES; i++) {
        if (!g_nodes[i].node_id) continue;
        if (seen == index) return &g_nodes[i];
        seen++;
    }
    return nullptr;
}

int hermes_mesh_json_nodes(char *buf, size_t max) {
    size_t pos = 0;
    pos += (size_t)snprintf(buf + pos, max - pos, "[");
    bool first = true;
    for (size_t i = 0; i < HERMES_MESH_MAX_NODES && pos < max - 4; i++) {
        if (!g_nodes[i].node_id) continue;
        if (!first) pos += (size_t)snprintf(buf + pos, max - pos, ",");
        first = false;
        pos += (size_t)snprintf(buf + pos, max - pos,
            "{\"node_id\":%u,\"name\":\"%s\",\"online\":%s,\"sensors\":\"%s\"}",
            g_nodes[i].node_id, g_nodes[i].name,
            g_nodes[i].online ? "true" : "false", g_nodes[i].sensors);
    }
    snprintf(buf + pos, max - pos, "]");
    return 0;
}

int hermes_mesh_json_readings(char *buf, size_t max) {
    size_t pos = 0;
    pos += (size_t)snprintf(buf + pos, max - pos, "[");
    bool first = true;
    for (size_t i = 0; i < HERMES_MESH_MAX_NODES && pos < max - 8; i++) {
        if (!g_nodes[i].node_id || !g_nodes[i].reading_count) continue;
        if (!first) pos += (size_t)snprintf(buf + pos, max - pos, ",");
        first = false;
        pos += (size_t)snprintf(buf + pos, max - pos,
            "{\"node_id\":%u,\"readings\":[", g_nodes[i].node_id);
        for (uint8_t r = 0; r < g_nodes[i].reading_count && pos < max - 32; r++) {
            if (r) pos += (size_t)snprintf(buf + pos, max - pos, ",");
            pos += (size_t)snprintf(buf + pos, max - pos,
                "{\"id\":\"%s\",\"value\":%.2f}",
                g_nodes[i].reading_ids[r], g_nodes[i].readings[r]);
        }
        pos += (size_t)snprintf(buf + pos, max - pos, "]}");
    }
    snprintf(buf + pos, max - pos, "]");
    return 0;
}
