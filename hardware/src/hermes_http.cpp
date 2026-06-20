#include "hermes_http.h"

#if defined(HERMES_HTTP_SERVER) && (defined(HERMES_ESP8266) || defined(ESP8266) || defined(HERMES_ESP32) || defined(HERMES_REALTEK) || defined(HERMES_BEKEN))

#include "hermes_board_store.h"
#include "hermes_node.h"
#include "hermes_mcu.h"
#include "hermes_wifi_module.h"
#include "hermes_mesh.h"
#include "hermes_gpio.h"
#include "hermes_templates.h"
#include "hermes_mqtt.h"
#include "hermes_ota.h"
#include "hermes_sensor.h"
#include "wifi_config.h"
#include "hermes_blueprints_data.h"

#include <Arduino.h>

#if defined(HERMES_ESP32)
#include <WiFi.h>
#include <WebServer.h>
static WebServer server(80);
#else
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
static ESP8266WebServer server(80);
#endif

#ifndef HERMES_NODE_ID
#define HERMES_NODE_ID 0
#endif

static hermes_board_app_t g_app;
static hermes_network_config_t g_net;
static hermes_board_gpio_t g_gpio;
static hermes_mqtt_config_t g_mqtt;
static uint32_t g_packets = 0;
static bool g_http_ok = false;

static const char *json_str(const char *body, const char *key, char *out, size_t out_len) {
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *p = strstr(body, pattern);
    if (!p) return nullptr;
    p = strchr(p + strlen(pattern), '"');
    if (!p) return nullptr;
    p++;
    const char *end = strchr(p, '"');
    if (!end || (size_t)(end - p) >= out_len) return nullptr;
    memcpy(out, p, (size_t)(end - p));
    out[end - p] = '\0';
    return out;
}

static int json_int(const char *body, const char *key, int fallback) {
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *p = strstr(body, pattern);
    if (!p) return fallback;
    p = strchr(p + strlen(pattern), ':');
    if (!p) return fallback;
    return atoi(p + 1);
}

static void json_key_csv(const char *body, const char *key, char *out, size_t out_len) {
    out[0] = '\0';
    char pattern[24];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *p = strstr(body, pattern);
    if (!p) return;
    p = strchr(p, ':');
    if (!p) return;
    p++;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '"') {
        p++;
        size_t pos = 0;
        while (*p && *p != '"' && pos < out_len - 1) out[pos++] = *p++;
        out[pos] = '\0';
        return;
    }
    if (*p != '[') return;
    p++;
    size_t pos = 0;
    while (*p && *p != ']' && pos < out_len - 1) {
        if (*p == '"') {
            p++;
            while (*p && *p != '"' && pos < out_len - 1) {
                if (pos > 0) out[pos++] = ',';
                while (*p && *p != '"' && pos < out_len - 1) out[pos++] = *p++;
            }
            if (*p == '"') p++;
        } else {
            p++;
        }
    }
    out[pos] = '\0';
}

static const char *state_name(uint8_t state) {
    if (state == HERMES_BOARD_APP_STATE_RUNNING) return "running";
    if (state == HERMES_BOARD_APP_STATE_PAUSED) return "paused";
    return "stopped";
}

static void send_json(int code, const String &body) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(code, "application/json", body);
}

static void handle_status() {
    char telemetry[640];
    hermes_node_build_telemetry_json(telemetry, sizeof(telemetry));
    char node_part[700];
    snprintf(node_part, sizeof(node_part),
             "{\"id\":%d,\"name\":\"%s\",\"node_type\":\"%s\",\"online\":true,\"agent_connected\":true,"
             "\"cpu\":22,\"ram_kb\":%u,\"temp_c\":42,\"power_mw\":320}",
             HERMES_NODE_ID, HERMES_MCU_NAME, HERMES_MCU_NAME, ESP.getFreeHeap() / 1024);

    String body = "{";
    body += "\"board_mode\":true,";
    body += "\"cluster_online\":true,";
    body += "\"agents_connected\":1,";
    body += "\"nodes\":[" + String(node_part) + "],";
    if (g_app.blueprint[0] || g_app.app_name[0]) {
        body += "\"apps\":[{\"name\":\"" + String(g_app.app_name) + "\",";
        body += "\"display_name\":\"" + String(g_app.display_name) + "\",";
        body += "\"blueprint\":\"" + String(g_app.blueprint) + "\",";
        body += "\"node_id\":" + String(HERMES_NODE_ID) + ",";
        body += "\"state\":\"" + String(state_name(g_app.state)) + "\",";
        body += "\"metrics\":{\"packets_sent\":" + String(g_packets) + ",\"load_percent\":22}}],";
        body += "\"metrics\":{\"load_percent\":22,\"power_w\":0.32,\"temp_c\":42,\"app_count\":1},";
    } else {
        body += "\"apps\":[],";
        body += "\"metrics\":{\"load_percent\":22,\"power_w\":0.32,\"temp_c\":42,\"app_count\":0},";
    }
    body += "\"telemetry\":" + String(telemetry) + ",";
    char sens_extra[560];
    sens_extra[0] = '\0';
    hermes_sensor_poll();
    hermes_sensor_json_all(sens_extra, sizeof(sens_extra));
    if (sens_extra[0] == ',') {
        body += String(sens_extra + 1) + ",";
    }
    char gpio_relays[128];
    hermes_gpio_relays_json(gpio_relays, sizeof(gpio_relays));
    body += "\"gpio\":" + String(gpio_relays) + ",";
    char mesh_nodes[512];
    char mesh_readings[640];
    hermes_mesh_json_nodes(mesh_nodes, sizeof(mesh_nodes));
    hermes_mesh_json_readings(mesh_readings, sizeof(mesh_readings));
    body += "\"mesh\":{\"enabled\":" + String(g_net.mesh_enabled ? "true" : "false") + ",";
    body += "\"module\":\"" + String(hermes_wifi_module_name(g_net.module_type)) + "\",";
    body += "\"nodes\":" + String(mesh_nodes) + ",\"readings\":" + String(mesh_readings) + "},";
    body += "\"ip\":\"" + String(hermes_wifi_ip_string()) + "\"";
    body += "}";
    send_json(200, body);
}

static void handle_network_get() {
    String body = "{";
    body += "\"module_type\":" + String(g_net.module_type) + ",";
    body += "\"module\":\"" + String(hermes_wifi_module_name(g_net.module_type)) + "\",";
    body += "\"ssid\":\"" + String(g_net.ssid) + "\",";
    body += "\"uart_baud\":" + String(g_net.uart_baud) + ",";
    body += "\"mesh_enabled\":" + String(g_net.mesh_enabled ? "true" : "false") + ",";
    body += "\"mesh_max_nodes\":" + String(g_net.mesh_max_nodes) + ",";
    body += "\"mesh_interval_sec\":" + String(g_net.mesh_interval_sec) + ",";
    body += "\"connected\":" + String(hermes_wifi_connected() ? "true" : "false") + ",";
    body += "\"ip\":\"" + String(hermes_wifi_ip_string()) + "\"";
    body += "}";
    send_json(200, body);
}

static void handle_network_post() {
    if (!server.hasArg("plain")) {
        send_json(400, "{\"error\":\"missing body\"}");
        return;
    }
    String body = server.arg("plain");
    int mod = json_int(body.c_str(), "module_type", g_net.module_type);
    if (mod >= 0) g_net.module_type = (uint8_t)mod;
    char ssid[32] = "";
    char pass[64] = "";
    if (json_str(body.c_str(), "ssid", ssid, sizeof(ssid))) strncpy(g_net.ssid, ssid, sizeof(g_net.ssid) - 1);
    if (json_str(body.c_str(), "password", pass, sizeof(pass))) strncpy(g_net.password, pass, sizeof(g_net.password) - 1);
    g_net.uart_baud = (uint32_t)json_int(body.c_str(), "uart_baud", (int)g_net.uart_baud);
    g_net.mesh_enabled = json_int(body.c_str(), "mesh_enabled", g_net.mesh_enabled) ? 1 : 0;
    g_net.mesh_max_nodes = (uint8_t)json_int(body.c_str(), "mesh_max_nodes", g_net.mesh_max_nodes);
    g_net.mesh_interval_sec = (uint16_t)json_int(body.c_str(), "mesh_interval_sec", g_net.mesh_interval_sec);
    hermes_board_store_save_network(&g_net);
    if (g_net.module_type == HERMES_WIFI_BUILTIN && g_net.ssid[0]) {
        if (hermes_wifi_setup_connect(g_net.ssid, g_net.password) == 0) {
            send_json(200, "{\"ok\":true,\"reboot\":true,\"ip\":\"" + String(hermes_wifi_ip_string()) + "\"}");
            delay(800);
            ESP.restart();
            return;
        }
    } else {
        hermes_wifi_begin(&g_net);
    }
    hermes_mesh_init(&g_net);
    send_json(200, "{\"ok\":true,\"ip\":\"" + String(hermes_wifi_ip_string()) + "\"}");
}

static void handle_wifi_scan() {
    int n = WiFi.scanNetworks();
    String body = "{\"networks\":[";
    for (int i = 0; i < n; i++) {
        if (i) body += ",";
        body += "{\"ssid\":\"" + WiFi.SSID(i) + "\",";
        body += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
#if defined(HERMES_ESP32)
        const bool secure = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
#else
        const bool secure = WiFi.encryptionType(i) != AUTH_OPEN;
#endif
        body += "\"secure\":" + String(secure ? "true" : "false") + "}";
    }
    body += "]}";
    send_json(200, body);
}

static void handle_wifi_setup() {
    if (!server.hasArg("plain")) {
        send_json(400, "{\"error\":\"missing body\"}");
        return;
    }
    String body = server.arg("plain");
    char ssid[32] = "";
    char pass[64] = "";
    if (!json_str(body.c_str(), "ssid", ssid, sizeof(ssid)) || !ssid[0]) {
        send_json(400, "{\"error\":\"ssid required\"}");
        return;
    }
    json_str(body.c_str(), "password", pass, sizeof(pass));
    strncpy(g_net.ssid, ssid, sizeof(g_net.ssid) - 1);
    strncpy(g_net.password, pass, sizeof(g_net.password) - 1);
    g_net.module_type = HERMES_WIFI_BUILTIN;
    hermes_board_store_save_network(&g_net);
    if (hermes_wifi_setup_connect(g_net.ssid, g_net.password) != 0) {
        send_json(400, "{\"error\":\"could not join network — check SSID and password\"}");
        return;
    }
    send_json(200, "{\"ok\":true,\"reboot\":true,\"ip\":\"" + String(hermes_wifi_ip_string()) + "\"}");
    delay(800);
    ESP.restart();
}

static void handle_captive_redirect() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
}

static void handle_board_template_list() {
    size_t n = 0;
    const hermes_template_info_t *list = hermes_template_list(&n);
    String body = "{\"templates\":[";
    for (size_t i = 0; i < n; i++) {
        if (i) body += ",";
        body += "{\"id\":\"" + String(list[i].id) + "\",";
        body += "\"name\":\"" + String(list[i].name) + "\",";
        body += "\"mcu\":\"" + String(list[i].mcu) + "\",";
        body += "\"description\":\"" + String(list[i].description) + "\"}";
    }
    body += "],\"functions\":[";
    for (uint8_t fn = 0; fn <= 27; fn++) {
        if (fn) body += ",";
        body += "{\"id\":" + String(fn) + ",\"name\":\"" + String(hermes_gpio_fn_name(fn)) + "\"}";
    }
    body += ",{\"id\":99,\"name\":\"User\"}]}";
    send_json(200, body);
}

static void handle_board_gpio_get() {
    char buf[1400];
    hermes_board_store_load_gpio(&g_gpio);
    hermes_gpio_to_json(&g_gpio, buf, sizeof(buf));
    send_json(200, String(buf));
}

static void handle_board_gpio_post() {
    if (!server.hasArg("plain")) {
        send_json(400, "{\"error\":\"missing body\"}");
        return;
    }
    String body = server.arg("plain");
    hermes_gpio_from_json(body.c_str(), &g_gpio);
    hermes_board_store_save_gpio(&g_gpio);
    send_json(200, "{\"ok\":true}");
}

static void handle_board_preset_post() {
    if (!server.hasArg("plain")) {
        send_json(400, "{\"error\":\"missing body\"}");
        return;
    }
    char tid[32] = "";
    json_str(server.arg("plain").c_str(), "template_id", tid, sizeof(tid));
    if (!tid[0]) {
        send_json(400, "{\"error\":\"template_id required\"}");
        return;
    }
    if (hermes_template_apply(tid, &g_gpio) != 0) {
        send_json(404, "{\"error\":\"unknown template\"}");
        return;
    }
    hermes_board_store_save_gpio(&g_gpio);
    char buf[1400];
    hermes_gpio_to_json(&g_gpio, buf, sizeof(buf));
    send_json(200, String(buf));
}

static void handle_board_tasmota_post() {
    if (!server.hasArg("plain")) {
        send_json(400, "{\"error\":\"missing body\"}");
        return;
    }
    if (hermes_gpio_import_tasmota(server.arg("plain").c_str(), &g_gpio) != 0) {
        send_json(400, "{\"error\":\"invalid Tasmota GPIO template\"}");
        return;
    }
    hermes_board_store_save_gpio(&g_gpio);
    char buf[1400];
    hermes_gpio_to_json(&g_gpio, buf, sizeof(buf));
    send_json(200, String(buf));
}

static char g_last_mqtt_tele[96] = "";
static char g_rules_json[320] = "[]";

static void handle_mqtt_get() {
    hermes_board_store_load_mqtt(&g_mqtt);
    String body = "{";
    body += "\"enabled\":" + String(g_mqtt.enabled ? "true" : "false") + ",";
    body += "\"host\":\"" + String(g_mqtt.host) + "\",";
    body += "\"port\":" + String(g_mqtt.port) + ",";
    body += "\"user\":\"" + String(g_mqtt.user) + "\",";
    body += "\"prefix\":\"" + String(g_mqtt.prefix) + "\",";
    body += "\"connected\":" + String(hermes_mqtt_connected() ? "true" : "false") + ",";
    body += "\"last_tele\":\"" + String(g_last_mqtt_tele) + "\"";
    body += "}";
    send_json(200, body);
}

static void handle_mqtt_post() {
    if (!server.hasArg("plain")) {
        send_json(400, "{\"error\":\"missing body\"}");
        return;
    }
    String body = server.arg("plain");
    hermes_board_store_load_mqtt(&g_mqtt);
    g_mqtt.enabled = json_int(body.c_str(), "enabled", g_mqtt.enabled) ? 1 : 0;
    g_mqtt.port = (uint16_t)json_int(body.c_str(), "port", g_mqtt.port ? g_mqtt.port : 1883);
    char host[48] = "", user[24] = "", pass[32] = "", prefix[16] = "";
    if (json_str(body.c_str(), "host", host, sizeof(host))) strncpy(g_mqtt.host, host, sizeof(g_mqtt.host) - 1);
    if (json_str(body.c_str(), "user", user, sizeof(user))) strncpy(g_mqtt.user, user, sizeof(g_mqtt.user) - 1);
    if (json_str(body.c_str(), "password", pass, sizeof(pass))) strncpy(g_mqtt.pass, pass, sizeof(g_mqtt.pass) - 1);
    if (json_str(body.c_str(), "prefix", prefix, sizeof(prefix))) strncpy(g_mqtt.prefix, prefix, sizeof(g_mqtt.prefix) - 1);
    hermes_board_store_save_mqtt(&g_mqtt);
    send_json(200, "{\"ok\":true,\"connected\":" + String(hermes_mqtt_connected() ? "true" : "false") + "}");
}

static void handle_mqtt_test() {
#if defined(HERMES_MQTT)
    if (!hermes_mqtt_connected()) {
        send_json(400, "{\"error\":\"MQTT not connected\"}");
        return;
    }
    const char *msg = "{\"test\":true,\"source\":\"hermes-ui\"}";
    hermes_mqtt_publish_state(msg);
    strncpy(g_last_mqtt_tele, msg, sizeof(g_last_mqtt_tele) - 1);
    send_json(200, "{\"ok\":true}");
#else
    send_json(400, "{\"error\":\"MQTT not enabled\"}");
#endif
}

static void handle_gpio_relay_post() {
    if (!server.hasArg("plain")) {
        send_json(400, "{\"error\":\"missing body\"}");
        return;
    }
    String body = server.arg("plain");
    int idx = json_int(body.c_str(), "idx", -1);
    int on = json_int(body.c_str(), "on", 0);
    if (idx < 0 || hermes_gpio_set_relay((uint8_t)idx, on != 0) != 0) {
        send_json(400, "{\"error\":\"relay not found\"}");
        return;
    }
#if defined(HERMES_MQTT)
    hermes_mqtt_publish_gpio();
#endif
    send_json(200, "{\"ok\":true,\"idx\":" + String(idx) + ",\"on\":" + String(on ? "true" : "false") + "}");
}

static void handle_rules_get() {
    send_json(200, String("{\"rules\":") + String(g_rules_json) + "}");
}

static void handle_rules_post() {
    if (!server.hasArg("plain")) {
        send_json(400, "{\"error\":\"missing body\"}");
        return;
    }
    String body = server.arg("plain");
    const char *rules = strstr(body.c_str(), "\"rules\"");
    if (rules) {
        const char *start = strchr(rules, '[');
        const char *end = start ? strrchr(start, ']') : NULL;
        if (start && end && end > start && (size_t)(end - start + 1) < sizeof(g_rules_json)) {
            size_t len = (size_t)(end - start + 1);
            memcpy(g_rules_json, start, len);
            g_rules_json[len] = '\0';
        }
    }
    send_json(200, "{\"ok\":true}");
}

static void handle_ota_status() {
    String msg = "{\"message\":\"OTA ready — use Arduino OTA or upload URL\",\"host\":\"hermes-";
    msg += String(HERMES_NODE_ID);
    msg += ".local\"}";
    send_json(200, msg);
}

static const char BOARD_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="en"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>HERMES Board GPIO</title>
<style>
body{font-family:system-ui,sans-serif;background:#0f1117;color:#e1e4ea;margin:0;padding:1rem}
h1{color:#6ee7b7;font-size:1.2rem}a{color:#6ee7b7}
.card{background:#1a1d27;border:1px solid #2a2f3d;border-radius:8px;padding:1rem;margin:.8rem 0}
table{width:100%;border-collapse:collapse;font-size:.8rem}th,td{border-bottom:1px solid #2a2f3d;padding:.35rem;text-align:left}
input,select{padding:.3rem;border-radius:4px;border:1px solid #2a2f3d;background:#0f1117;color:#e1e4ea;width:100%}
button{background:#059669;color:#fff;border:none;border-radius:6px;padding:.5rem .8rem;margin:.3rem .2rem 0 0;cursor:pointer}
.hint{color:#8b92a5;font-size:.8rem}
</style></head><body>
<h1>Board GPIO template</h1>
<p class="hint">Assign pin roles like Tasmota templates. <a href="/">Dashboard</a></p>
<div class="card">
<label>Preset</label>
<select id="preset"></select>
<button id="apply-preset">Apply preset</button>
<label>Board label</label><input id="label">
</div>
<div class="card"><table><thead><tr><th>GPIO</th><th>Function</th><th>Flags</th><th>Idx</th><th></th></tr></thead><tbody id="pins"></tbody></table>
<button id="add-pin">Add pin</button><button id="save">Save to board</button>
<p class="hint">Tasmota import: paste template JSON with GPIO[] array below</p>
<textarea id="tasmota" rows="4" placeholder='{"GPIO":[0,0,224,0,225,...]}'></textarea>
<button id="import-tasmota">Import Tasmota GPIO</button></div>
<script>
let functions=[],pins=[];
async function api(p,o){const r=await fetch(p,{...o,headers:{'Content-Type':'application/json'}});return r.json()}
function fnOpts(sel){return functions.map(f=>`<option value="${f.id}" ${sel==f.id?'selected':''}>${f.name}</option>`).join('')}
function render(){document.getElementById('pins').innerHTML=pins.map((p,i)=>`<tr>
<td><input type="number" min="0" max="48" value="${p.gpio}" data-i="${i}" data-k="gpio"></td>
<td><select data-i="${i}" data-k="fn">${fnOpts(p.fn)}</select></td>
<td><input type="number" min="0" max="3" value="${p.flags}" data-i="${i}" data-k="flags"></td>
<td><input type="number" min="0" max="15" value="${p.idx}" data-i="${i}" data-k="idx"></td>
<td><button data-rm="${i}">×</button></td></tr>`).join('');
 document.querySelectorAll('[data-k]').forEach(el=>el.onchange=()=>{pins[+el.dataset.i][el.dataset.k]=Number(el.value)});
 document.querySelectorAll('[data-rm]').forEach(el=>el.onclick=()=>{pins.splice(+el.dataset.rm,1);render()})}
document.getElementById('add-pin').onclick=()=>{pins.push({gpio:0,fn:0,flags:0,idx:0});render()};
document.getElementById('apply-preset').onclick=async()=>{const j=await api('/api/board/preset',{method:'POST',body:JSON.stringify({template_id:document.getElementById('preset').value})});
 pins=j.pins||[];document.getElementById('label').value=j.board_label||'';render()};
document.getElementById('save').onclick=async()=>{await api('/api/board/gpio',{method:'POST',body:JSON.stringify({template_id:'custom',board_label:document.getElementById('label').value,pins})});alert('Saved')};
document.getElementById('import-tasmota').onclick=async()=>{const t=document.getElementById('tasmota').value;if(!t)return;const j=await api('/api/board/tasmota',{method:'POST',body:t});pins=j.pins||[];document.getElementById('label').value=j.board_label||'';render();alert('Imported')};
(async()=>{const t=await api('/api/board/templates');functions=t.functions||[];
 document.getElementById('preset').innerHTML=(t.templates||[]).map(x=>`<option value="${x.id}">${x.name}</option>`).join('');
 const g=await api('/api/board/gpio');pins=g.pins||[];document.getElementById('label').value=g.board_label||'';render()})();
</script></body></html>)HTML";

static void handle_board_page() {
    server.send_P(200, "text/html", BOARD_HTML);
}

static void handle_mesh_nodes() {
    char mesh_nodes[512];
    hermes_mesh_json_nodes(mesh_nodes, sizeof(mesh_nodes));
    send_json(200, String("{\"nodes\":") + mesh_nodes + "}");
}

static void handle_sensors() {
    send_json(200,
        "{\"sensors\":["
        "{\"id\":\"temp_humidity\",\"name\":\"Temperature & Humidity\"},"
        "{\"id\":\"soil_moisture\",\"name\":\"Soil Moisture\"},"
        "{\"id\":\"motion_pir\",\"name\":\"Motion (PIR)\"},"
        "{\"id\":\"door_contact\",\"name\":\"Door Contact\"},"
        "{\"id\":\"lux\",\"name\":\"Ambient Light\"},"
        "{\"id\":\"co2\",\"name\":\"CO2\"},"
        "{\"id\":\"current_clamp\",\"name\":\"Current Clamp\"},"
        "{\"id\":\"voltage\",\"name\":\"Voltage\"},"
        "{\"id\":\"pressure\",\"name\":\"Pressure\"},"
        "{\"id\":\"flow_meter\",\"name\":\"Flow Meter\"},"
        "{\"id\":\"water_leak\",\"name\":\"Water Leak\"},"
        "{\"id\":\"modbus_meter\",\"name\":\"Modbus Meter\"}"
        "]}");
}

static const char BP_MESH_FIELDS[] =
    ",{\"key\":\"mesh_enabled\",\"label\":\"Enable sensor mesh\",\"type\":\"toggle\",\"default\":false}"
    ",{\"key\":\"mesh_max_nodes\",\"label\":\"Max remote nodes\",\"type\":\"number\",\"min\":1,\"max\":8,\"default\":4}"
    ",{\"key\":\"remote_sensors\",\"label\":\"Remote / mesh sensors\",\"type\":\"sensor_list\"}";

static void handle_blueprints() {
    send_json(200, FPSTR(HERMES_BLUEPRINTS_JSON));
}

static void handle_blueprint_detail() {
    String path = server.uri();
    String id = path.substring(String("/api/blueprints/").length());
    id.trim();
    int idx = hermes_board_blueprint_index(id.c_str());
    if (idx < 0) {
        send_json(404, "{\"error\":\"blueprint not found\"}");
        return;
    }
    char name[32];
    strncpy_P(name, HERMES_BP_NAMES[idx], sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    String body = "{\"id\":\"" + id + "\",\"name\":\"" + String(name) + "\",\"fields\":[";
    body += "{\"key\":\"app_name\",\"label\":\"App name\",\"type\":\"text\",\"default\":\"";
    body += name;
    body += "\"},{\"key\":\"check_interval_sec\",\"label\":\"Check interval (sec)\",\"type\":\"number\",\"min\":10,\"max\":3600,\"default\":120}";
    body += ",{\"key\":\"sensors\",\"label\":\"Local sensors\",\"type\":\"sensor_list\"}";
    body += String(BP_MESH_FIELDS) + "]}";
    send_json(200, body);
}

static void apply_body_to_app(const String &body) {
    char blueprint[24] = "";
    char name[28] = "";
    char label_value[32] = "";
    json_str(body.c_str(), "blueprint", blueprint, sizeof(blueprint));
    json_str(body.c_str(), "name", name, sizeof(name));
    if (blueprint[0]) strncpy(g_app.blueprint, blueprint, sizeof(g_app.blueprint) - 1);
    if (name[0]) strncpy(g_app.app_name, name, sizeof(g_app.app_name) - 1);

    const char *config = strstr(body.c_str(), "\"config\"");
    if (config) {
        json_str(config, "app_name", label_value, sizeof(label_value));
        if (label_value[0]) strncpy(g_app.label_key, "app_name", sizeof(g_app.label_key) - 1);
        if (!label_value[0]) {
            json_str(config, "hub_name", label_value, sizeof(label_value));
            if (label_value[0]) strncpy(g_app.label_key, "hub_name", sizeof(g_app.label_key) - 1);
        }
        if (!label_value[0]) {
            json_str(config, "home_name", label_value, sizeof(label_value));
            if (label_value[0]) strncpy(g_app.label_key, "home_name", sizeof(g_app.label_key) - 1);
        }
        if (!label_value[0]) {
            json_str(config, "network_name", label_value, sizeof(label_value));
            if (label_value[0]) strncpy(g_app.label_key, "network_name", sizeof(g_app.label_key) - 1);
        }
        if (!label_value[0]) {
            json_str(config, "zone_name", label_value, sizeof(label_value));
            if (label_value[0]) strncpy(g_app.label_key, "zone_name", sizeof(g_app.label_key) - 1);
        }
        if (label_value[0]) strncpy(g_app.label_value, label_value, sizeof(g_app.label_value) - 1);
        g_app.sample_rate_hz = json_int(config, "sample_rate_hz", g_app.sample_rate_hz);
        g_app.alert_on_high_load = json_int(config, "alert_on_high_load", g_app.alert_on_high_load) ? 1 : 0;
        g_app.mesh_max_nodes = (uint8_t)json_int(config, "mesh_max_nodes", g_app.mesh_max_nodes);
        g_app.mesh_interval_sec = (uint16_t)json_int(config, "report_interval_sec", g_app.mesh_interval_sec);
        if (!g_app.mesh_interval_sec) g_app.mesh_interval_sec = (uint16_t)json_int(config, "mesh_interval_sec", 120);
        g_app.mesh_enabled = json_int(config, "mesh_enabled", g_app.mesh_enabled) ? 1 : 0;
        json_key_csv(config, "sensors", g_app.sensors, sizeof(g_app.sensors));
        json_key_csv(config, "remote_sensors", g_app.remote_sensors, sizeof(g_app.remote_sensors));
        hermes_node_apply_config_json(config);
        if (g_app.mesh_enabled) {
            g_net.mesh_enabled = 1;
            g_net.mesh_max_nodes = g_app.mesh_max_nodes ? g_app.mesh_max_nodes : 8;
            g_net.mesh_interval_sec = g_app.mesh_interval_sec;
            hermes_board_store_save_network(&g_net);
            hermes_mesh_init(&g_net);
            hermes_mesh_apply_app_config(g_app.sensors, g_app.remote_sensors,
                g_net.mesh_max_nodes, g_net.mesh_interval_sec);
        } else {
            g_net.mesh_enabled = 0;
            hermes_board_store_save_network(&g_net);
            hermes_mesh_init(&g_net);
        }
    }
    if (label_value[0]) strncpy(g_app.display_name, label_value, sizeof(g_app.display_name) - 1);
    else if (name[0]) strncpy(g_app.display_name, name, sizeof(g_app.display_name) - 1);
}

static void handle_app_create() {
    if (!server.hasArg("plain")) {
        send_json(400, "{\"error\":\"missing body\"}");
        return;
    }
    String body = server.arg("plain");
    char blueprint[24] = "";
    json_str(body.c_str(), "blueprint", blueprint, sizeof(blueprint));
    if (!blueprint[0]) {
        send_json(400, "{\"error\":\"blueprint required\"}");
        return;
    }
    apply_body_to_app(body);
    if (!g_app.app_name[0]) {
        strncpy(g_app.app_name, blueprint, sizeof(g_app.app_name) - 1);
    }
    if (!g_app.blueprint[0]) {
        strncpy(g_app.blueprint, blueprint, sizeof(g_app.blueprint) - 1);
    }
    g_app.state = HERMES_BOARD_APP_STATE_RUNNING;
    hermes_board_store_save(&g_app);
    hermes_board_store_apply_sensors(&g_app);
    g_packets = 0;
    String resp = "{\"name\":\"" + String(g_app.app_name) + "\",";
    resp += "\"display_name\":\"" + String(g_app.display_name) + "\",";
    resp += "\"blueprint\":\"" + String(g_app.blueprint) + "\",";
    resp += "\"node_id\":" + String(HERMES_NODE_ID) + ",\"state\":\"running\"}";
    send_json(200, resp);
}

static void handle_app_config_get() {
    String path = server.uri();
    String resp = "{\"app\":\"" + String(g_app.app_name) + "\",";
    resp += "\"blueprint\":\"" + String(g_app.blueprint) + "\",";
    resp += "\"config\":{";
    resp += "\"" + String(g_app.label_key) + "\":\"" + String(g_app.label_value) + "\",";
    resp += "\"sample_rate_hz\":" + String(g_app.sample_rate_hz) + ",";
    resp += "\"alert_on_high_load\":" + String(g_app.alert_on_high_load ? "true" : "false") + ",";
    resp += "\"sensors\":\"" + String(g_app.sensors) + "\"";
    resp += "}}";
    send_json(200, resp);
}

static void handle_app_config_post() {
    if (!server.hasArg("plain")) {
        send_json(400, "{\"error\":\"missing body\"}");
        return;
    }
    String body = server.arg("plain");
    const char *config = strstr(body.c_str(), "\"config\"");
    if (config) apply_body_to_app(String("{\"config\":") + String(config + 8));
    hermes_board_store_save(&g_app);
    send_json(200, "{\"ok\":true}");
}

static const char SETUP_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="en"><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>HERMES WiFi Setup</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}body{font-family:system-ui,sans-serif;background:#0f1117;color:#e1e4ea;padding:1.2rem;max-width:420px;margin:0 auto}
h1{color:#6ee7b7;font-size:1.4rem;margin-bottom:.25rem}.sub{color:#8b92a5;font-size:.85rem;margin-bottom:1.2rem}
.card{background:#1a1d27;border:1px solid #2a2f3d;border-radius:10px;padding:1rem;margin-bottom:1rem}
label{display:block;font-size:.75rem;color:#8b92a5;margin:.7rem 0 .25rem}
input,select{width:100%;padding:.55rem;border-radius:8px;border:1px solid #2a2f3d;background:#0f1117;color:#e1e4ea}
button{width:100%;background:#059669;color:#fff;border:none;border-radius:8px;padding:.65rem;margin-top:.8rem;font-size:1rem;cursor:pointer}
button.secondary{background:#2a2f3d;margin-top:.4rem}
.msg{margin-top:.8rem;font-size:.85rem;color:#6ee7b7}.err{color:#f87171}
</style></head><body>
<h1>HERMES WiFi Setup</h1>
<p class="sub">Connect to your home WiFi. This page saves credentials to the board and reboots.</p>
<div class="card">
<label>WiFi network</label>
<select id="ssid-sel"><option value="">— scan or type below —</option></select>
<label>SSID (network name)</label>
<input id="ssid" placeholder="Your WiFi name" autocomplete="off">
<label>Password</label>
<input id="pass" type="password" placeholder="WiFi password" autocomplete="off">
<button class="secondary" id="scan">Scan networks</button>
<button id="save">Connect</button>
<p id="msg" class="msg"></p>
</div>
<script>
const apName=document.title;
async function scan(){
  document.getElementById('msg').textContent='Scanning...';
  const r=await fetch('/api/wifi/scan');const j=await r.json();
  const sel=document.getElementById('ssid-sel');sel.innerHTML='<option value="">— pick a network —</option>';
  (j.networks||[]).forEach(n=>{const o=document.createElement('option');o.value=n.ssid;o.textContent=n.ssid+(n.secure?' 🔒':'')+' ('+n.rssi+' dBm)';sel.appendChild(o);});
  document.getElementById('msg').textContent=(j.networks||[]).length+' network(s) found';
}
document.getElementById('scan').onclick=scan;
document.getElementById('ssid-sel').onchange=e=>{if(e.target.value)document.getElementById('ssid').value=e.target.value;};
document.getElementById('save').onclick=async()=>{
  const ssid=document.getElementById('ssid').value.trim();
  const password=document.getElementById('pass').value;
  const msg=document.getElementById('msg');
  if(!ssid){msg.className='msg err';msg.textContent='Enter a network name';return;}
  msg.className='msg';msg.textContent='Connecting...';
  const r=await fetch('/api/wifi/setup',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid,password})});
  const j=await r.json();
  if(!r.ok){msg.className='msg err';msg.textContent=j.error||'Failed';return;}
  msg.textContent='Connected! Rebooting to '+j.ip+'...';
};
scan();
</script></body></html>)HTML";

static const char INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="en" data-theme="apple"><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="apple-mobile-web-app-capable" content="yes"><title>HERMES</title>
<style>
:root,[data-theme=apple]{--bg:#f2f2f7;--card:#fff;--border:#d1d1d6;--text:#1c1c1e;--muted:#8e8e93;--accent:#007aff;--btn:#007aff;--ring:rgba(0,122,255,.18);--shadow:0 8px 24px rgba(0,0,0,.06);--ok:#34c759;--err:#ff453a}
[data-theme=oled]{--bg:#000;--card:#0a0a0a;--border:#2a2a2a;--text:#f5f5f7;--muted:#8e8e93;--accent:#0a84ff;--btn:#0a84ff;--ring:rgba(10,132,255,.25);--shadow:none}
[data-theme=midnight]{--bg:#0f1117;--card:#1a1d27;--border:#2a2f3d;--text:#e1e4ea;--muted:#8b92a5;--accent:#6ee7b7;--btn:#059669;--ring:rgba(110,231,183,.2);--shadow:none}
*{box-sizing:border-box;margin:0;padding:0}body{font-family:-apple-system,BlinkMacSystemFont,system-ui,sans-serif;background:var(--bg);color:var(--text);padding:1rem;padding-bottom:calc(4.5rem + env(safe-area-inset-bottom));max-width:720px;margin:0 auto;-webkit-font-smoothing:antialiased}
.top{display:flex;align-items:center;gap:.75rem;margin-bottom:.35rem}.logo{width:38px;height:38px;border-radius:11px;background:linear-gradient(145deg,var(--btn),#0056cc);color:#fff;display:grid;place-items:center;font-weight:800}
h1{font-size:1.35rem;font-weight:700}h2{font-size:1rem;font-weight:700;margin-bottom:.35rem}.sub{color:var(--muted);font-size:.85rem;margin-bottom:1rem}
.seg{display:flex;gap:.25rem;background:var(--card);border:1px solid var(--border);border-radius:14px;padding:.28rem;margin-bottom:1rem;overflow-x:auto}
.seg button{flex:1;min-width:4.2rem;border:none;background:transparent;color:var(--muted);font:inherit;font-size:.74rem;font-weight:600;padding:.5rem .35rem;border-radius:10px;cursor:pointer;white-space:nowrap}
.seg button.on{background:var(--btn);color:#fff}.view{display:none}.view.on{display:block}
.card{background:var(--card);border:1px solid var(--border);border-radius:18px;padding:1rem;margin-bottom:.85rem;box-shadow:var(--shadow)}
.grid{display:grid;grid-template-columns:repeat(2,1fr);gap:.65rem}.grid3{grid-template-columns:repeat(3,1fr)}
.stat{background:var(--bg);border:1px solid var(--border);border-radius:14px;padding:.75rem}.stat .label{font-size:.68rem;color:var(--muted)}.stat .value{font-size:1.05rem;font-weight:700}.online{color:var(--accent)}
.chips{display:flex;flex-wrap:wrap;gap:.35rem;margin:.5rem 0}.chip{border:1px solid var(--border);background:var(--bg);color:inherit;border-radius:999px;padding:.35rem .65rem;font-size:.72rem;font-weight:600;cursor:pointer}
.chip.on{border-color:var(--btn);color:var(--btn);box-shadow:inset 0 0 0 1px var(--btn)}.chip .ico{display:inline-flex;vertical-align:middle;margin-right:.2rem}
.group-title{font-size:.72rem;font-weight:700;color:var(--muted);text-transform:uppercase;letter-spacing:.06em;margin:.25rem 0 .5rem}
.studio{display:grid;grid-template-columns:minmax(140px,1fr) 1.4fr;gap:.65rem}
.bp-rail{max-height:min(52vh,420px);overflow:auto}.bp-search{width:100%;margin-bottom:.4rem;font-size:.82rem;padding:.55rem .65rem;border-radius:11px;border:1px solid var(--border);background:var(--bg);color:var(--text)}
.bp-count{font-size:.72rem;color:var(--muted);margin-bottom:.35rem}
.bp{display:block;width:100%;text-align:left;background:var(--bg);border:1px solid var(--border);color:inherit;border-radius:12px;padding:.65rem;margin-bottom:.35rem;cursor:pointer;position:relative}
.bp.on{border-color:var(--btn);box-shadow:inset 0 0 0 1px var(--btn)}.bp strong{display:block;font-size:.84rem;padding-right:1.5rem}.bp .ico{margin-right:.25rem;display:inline-flex;vertical-align:middle}
.bp-fav{position:absolute;top:.45rem;right:.45rem;border:none;background:transparent;cursor:pointer;padding:.1rem;line-height:0;opacity:.55}
.bp-fav.on{opacity:1;color:#ff9f0a}.svg-icon{width:1em;height:1em;display:inline-block;vertical-align:-.12em}
.field{margin:.55rem 0}.field label{display:block;font-size:.76rem;font-weight:600;color:var(--muted);margin-bottom:.22rem}
input,select,textarea{width:100%;padding:.6rem .7rem;border-radius:11px;border:1px solid var(--border);background:var(--bg);color:var(--text);font:inherit}
input:focus,select:focus{outline:none;border-color:var(--btn);box-shadow:0 0 0 4px var(--ring)}
button.primary{background:var(--btn);color:#fff;border:none;border-radius:999px;padding:.65rem 1rem;cursor:pointer;margin-top:.4rem;font-weight:600;width:100%}
button.primary:disabled{opacity:.45}button.ghost{background:transparent;border:1px solid var(--border);color:var(--text);border-radius:999px;padding:.5rem .85rem;font:inherit;font-size:.8rem;cursor:pointer}
.sensor-grid{display:grid;grid-template-columns:1fr 1fr;gap:.2rem;font-size:.74rem;color:var(--muted);padding:.45rem;border-radius:11px;border:1px solid var(--border);background:var(--bg)}
.hint{color:var(--muted);font-size:.78rem;line-height:1.45}.app-card{padding:.8rem;border-radius:14px;border:1px solid var(--border);background:var(--bg)}.app-card h3{font-size:.98rem;font-weight:700}
.link{color:var(--accent);text-decoration:none;font-weight:600}.theme{float:right;font-size:.72rem;color:var(--muted)}.theme select{padding:.35rem .5rem;border-radius:10px;border:1px solid var(--border);background:var(--card);color:var(--text)}
.relay-row{display:flex;align-items:center;justify-content:space-between;padding:.55rem 0;border-bottom:1px solid var(--border)}.relay-row:last-child{border:none}
.toggle{width:46px;height:28px;border-radius:999px;border:none;background:var(--border);position:relative;cursor:pointer}.toggle.on{background:var(--btn)}.toggle::after{content:'';position:absolute;top:3px;left:3px;width:22px;height:22px;border-radius:50%;background:#fff;transition:.15s}.toggle.on::after{left:21px}
.preview{display:grid;grid-template-columns:repeat(3,1fr);gap:.45rem;margin:.65rem 0}.preview .stat{padding:.55rem}.preview .value{font-size:.95rem}
.bottom-nav{position:fixed;left:0;right:0;bottom:0;display:none;background:var(--card);border-top:1px solid var(--border);padding:.35rem .5rem calc(.35rem + env(safe-area-inset-bottom));z-index:30}
.bottom-nav button{flex:1;border:none;background:transparent;color:var(--muted);font:inherit;font-size:.68rem;padding:.45rem;border-radius:10px}
.bottom-nav button.on{color:var(--btn);font-weight:700}
.overlay{position:fixed;inset:0;background:rgba(0,0,0,.45);display:flex;align-items:flex-end;justify-content:center;z-index:40;padding:1rem}
.sheet{background:var(--card);border:1px solid var(--border);border-radius:22px 22px 18px 18px;padding:1.1rem;width:min(100%,420px);max-height:80vh;overflow:auto}
.sheet h3{font-size:1.05rem;margin-bottom:.65rem}.sheet .row{display:flex;justify-content:space-between;padding:.35rem 0;font-size:.85rem}
.success-pop{position:fixed;inset:0;display:flex;align-items:center;justify-content:center;background:rgba(0,0,0,.35);z-index:50}
.success-box{background:var(--card);border-radius:20px;padding:1.5rem;text-align:center;box-shadow:var(--shadow)}.success-box .check{font-size:2.5rem;color:var(--ok)}
.toast{position:fixed;left:50%;bottom:calc(4.5rem + env(safe-area-inset-bottom));transform:translateX(-50%);padding:.7rem .95rem;border-radius:14px;background:var(--card);border:1px solid var(--border);font-size:.84rem;font-weight:600;box-shadow:var(--shadow);z-index:20}.toast.err{color:var(--err)}.hidden{display:none!important}
.pull-hint{text-align:center;font-size:.72rem;color:var(--muted);margin-top:.25rem}
@media(max-width:560px){.studio{grid-template-columns:1fr}.bottom-nav{display:flex}.seg{display:none}}
</style></head><body>
<div class="top"><div class="logo">H</div><div><h1 id="title">HERMES</h1><div class="sub" id="subtitle">Board server</div></div>
<label class="theme">Theme <select id="theme-sel"><option value="apple">Apple</option><option value="oled">OLED</option><option value="midnight">Midnight</option></select></label></div>
<div class="seg" id="seg-desk"><button class="on" data-tab="home">Home</button><button data-tab="apps">Apps</button><button data-tab="gpio">GPIO</button><button data-tab="connect">Connect</button></div>
<nav class="bottom-nav" id="seg-mob" aria-label="Mobile navigation"></nav>
<div id="view-home" class="view on">
<div class="card"><div class="grid" id="stats"></div><p class="pull-hint">Pull down to refresh</p></div>
<div class="card"><h2>Live sensors</h2><div class="grid grid3" id="sensor-tiles"><p class="hint">Deploy an app with sensors to see readings.</p></div></div>
<div class="card"><h2>Application</h2><div id="appinfo" class="app-card hint">No app deployed</div><div class="chips" id="app-history"></div></div>
<div class="card"><h2>Mesh</h2><pre id="meshinfo" class="hint">No remote nodes</pre></div>
<div class="card"><h2>Quick controls</h2><div id="relay-quick"></div></div>
</div>
<div id="view-apps" class="view">
<div class="card"><h2>App Studio</h2><p class="hint"><span id="bp-total">51</span> blueprints · scroll or search</p>
<div class="chips" id="cat-chips"></div>
<div class="studio"><div class="bp-rail"><input class="bp-search" id="bp-search" placeholder="Search apps…"><div class="bp-count" id="bp-count"></div><div id="bps"></div></div>
<div><div id="studio-title" style="font-weight:700;margin-bottom:.45rem">Select a blueprint</div>
<div class="preview hidden" id="live-preview"></div><div id="fields"></div>
<button class="primary" id="deploy" disabled>Deploy App</button></div></div></div>
</div>
<div id="view-gpio" class="view">
<div class="card"><h2>GPIO &amp; relays</h2><p class="hint">Template editor and Tasmota import on full board page.</p>
<div id="gpio-relays"></div><p style="margin-top:.65rem"><a class="link" href="/board">Open GPIO template editor →</a></p></div>
</div>
<div id="view-connect" class="view">
<div class="card"><p class="group-title">Network</p>
<div class="field"><label>Module</label><select id="wifi-mod"><option value="0">Built-in WiFi</option><option value="1">UART AT</option></select></div>
<div class="field"><label>SSID</label><input id="wifi-ssid"></div>
<div class="field"><label>Password</label><input id="wifi-pass" type="password"></div>
<div class="field"><label><input type="checkbox" id="mesh-on"> Enable sensor mesh</label></div>
<button class="primary" id="save-net">Save Network</button></div>
<div class="card"><p class="group-title">MQTT</p>
<div class="field"><label><input type="checkbox" id="mqtt-on"> Enable MQTT</label></div>
<div class="field"><label>Broker</label><input id="mqtt-host" placeholder="192.168.1.10"></div>
<div class="field"><label>Port</label><input id="mqtt-port" type="number" value="1883"></div>
<div class="field"><label>Topic prefix</label><input id="mqtt-prefix" value="hermes"></div>
<p class="hint" id="mqtt-status">MQTT: —</p><pre class="hint" id="mqtt-tele" style="margin-top:.45rem;white-space:pre-wrap"></pre>
<button class="primary" id="save-mqtt">Save MQTT</button><button class="ghost" id="mqtt-test" style="margin-left:.35rem">Test publish</button></div>
<div class="card"><p class="group-title">Rules</p>
<p class="hint">Simple alerts when a sensor crosses a threshold.</p>
<div id="rules-list"></div>
<button class="ghost" id="add-rule">+ Add rule</button><button class="primary" id="save-rules">Save rules</button></div>
<div class="card"><p class="group-title">OTA</p>
<p class="hint">Upload from Arduino OTA: <code id="ota-host">hermes-0.local</code></p>
<div class="field"><label>Firmware URL (advanced)</label><input id="ota-url" placeholder="http://server/firmware.bin"></div>
<button class="primary" id="ota-check">Check for update</button><p class="hint" id="ota-status"></p></div>
</div>
<div id="deploy-sheet" class="overlay hidden"><div class="sheet"><h3>Confirm deploy</h3><div id="deploy-summary"></div>
<button class="primary" id="deploy-confirm">Deploy</button><button class="ghost" id="deploy-cancel" style="margin-top:.45rem;width:100%">Cancel</button></div></div>
<div id="success-pop" class="success-pop hidden"><div class="success-box"><div class="check">✓</div><h3>Deployed</h3><p class="hint" id="success-msg"></p><button class="primary" id="success-ok">View Home</button></div></div>
<div id="toast" class="toast hidden"></div>
<script>
const API='';
const ICO={all:'<svg class="svg-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="3" y="3" width="7" height="7" rx="1.5"/><rect x="14" y="3" width="7" height="7" rx="1.5"/><rect x="3" y="14" width="7" height="7" rx="1.5"/><rect x="14" y="14" width="7" height="7" rx="1.5"/></svg>',ag:'<svg class="svg-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 22c4-4 8-7.5 8-12a8 8 0 1 0-16 0c0 4.5 4 8 8 12z"/><path d="M12 10v6"/><path d="M9 13h6"/></svg>',home:'<svg class="svg-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M3 10.5 12 3l9 7.5"/><path d="M5 9.5V20a1 1 0 0 0 1 1h4v-6h4v6h4a1 1 0 0 0 1-1V9.5"/></svg>',energy:'<svg class="svg-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M13 2 3 14h8l-1 8 10-12h-8l1-8z"/></svg>',ind:'<svg class="svg-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M2 20h20"/><path d="M4 20V10l4-2v12"/><path d="M10 20V6l4-2v16"/><path d="M16 20v-8l4-1v9"/></svg>',star:'<svg class="svg-icon" viewBox="0 0 24 24" fill="currentColor"><path d="M12 2l3.09 6.26L22 9.27l-5 4.87L18.18 22 12 18.27 5.82 22 7 14.14l-5-4.87 6.91-1.01z"/></svg>',starOutline:'<svg class="svg-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2l3.09 6.26L22 9.27l-5 4.87L18.18 22 12 18.27 5.82 22 7 14.14l-5-4.87 6.91-1.01z"/></svg>'};
function ico(k){return ICO[k]||ICO.all}
const CATS=[{id:'all',l:'All',i:'all'},{id:'ag',l:'Farm',i:'ag',ids:['smartfarm','greenhouse','aquaponics','irrigation-scheduler','livestock-barn','pest-trap','smart-gate','weather-station','hydroponics','compost-monitor','vineyard','bee-hive','poultry-coop','grain-silo','lawn-irrigation']},{id:'home',l:'Home',i:'home',ids:['smarthome','smart-lighting','air-quality','leak-detector','pool-spa','fire-smoke','elder-care','hvac-controller','window-blinds','garage-door','water-meter','gas-safety','wine-cellar','mold-risk']},{id:'energy',l:'Energy',i:'energy',ids:['energy-monitor','solar-battery','ev-charger','asset-tracker','parking-gate','battery-room','diesel-generator','street-lighting']},{id:'ind',l:'Industrial',i:'ind',ids:['warehouse','cold-chain','machine-monitor','sensor-net','telemetry','security','conveyor-monitor','tank-level','pressure-vessel','freezer-monitor','brewery','server-room','lift-elevator','rfid-gate']}];
let blueprints=[],detail=null,sensors=[],status={},bpFilter='',bpCat='all',previewIv=null,pendingDeploy=null,selectedBpId=null;
let touchY0=0;
function catOf(id){for(const c of CATS)if(c.ids&&c.ids.includes(id))return c;return CATS[0]}
function favs(){try{return JSON.parse(localStorage.getItem('hermes_fav')||'[]')}catch(e){return[]}}
function toggleFav(id){let f=favs();f=f.includes(id)?f.filter(x=>x!==id):[id,...f].slice(0,8);localStorage.setItem('hermes_fav',JSON.stringify(f));renderBps()}
function hist(){try{return JSON.parse(localStorage.getItem('hermes_hist')||'[]')}catch(e){return[]}}
function pushHist(id,name){let h=hist().filter(x=>x.id!==id);h.unshift({id,name});localStorage.setItem('hermes_hist',JSON.stringify(h.slice(0,3)))}
function setTab(id){document.querySelectorAll('.seg button,.bottom-nav button').forEach(b=>b.classList.toggle('on',b.dataset.tab===id));
 document.querySelectorAll('.view').forEach(v=>v.classList.toggle('on',v.id==='view-'+id));}
function initNav(){const tabs=['home','apps','gpio','connect'];const labels=['Home','Apps','GPIO','Connect'];
 document.getElementById('seg-desk').querySelectorAll('button').forEach(b=>b.onclick=()=>setTab(b.dataset.tab));
 const mob=document.getElementById('seg-mob');mob.innerHTML=tabs.map((t,i)=>`<button data-tab="${t}">${labels[i]}</button>`).join('');
 mob.querySelectorAll('button').forEach(b=>b.onclick=()=>setTab(b.dataset.tab));}
async function api(p,o){const r=await fetch(API+p,{...o,headers:{'Content-Type':'application/json',...(o&&o.headers||{})}});let j={};try{j=await r.json()}catch(e){j={error:'Invalid response'}};if(!r.ok&&!j.error)j.error='Request failed';return j}
function toast(msg,ok){const t=document.getElementById('toast');t.textContent=msg;t.className='toast'+(ok?'':' err');t.classList.remove('hidden');setTimeout(()=>t.classList.add('hidden'),3000)}
function slug(v){return String(v||'').trim().toLowerCase().replace(/[^a-z0-9]+/g,'-').replace(/^-+|-+$/g,'')}
function pickName(cfg){if(cfg.app_name)return cfg.app_name;for(const k of Object.keys(cfg))if(k.endsWith('_name')&&cfg[k])return cfg[k];return ''}
function sensorSel(val,id){const s=Array.isArray(val)?val.join(','):String(val||'');return s.includes(id)}
function renderCats(){document.getElementById('cat-chips').innerHTML=CATS.map(c=>`<button type="button" class="chip ${bpCat===c.id?'on':''}" data-c="${c.id}"><span class="ico">${ico(c.i)}</span>${c.l}</button>`).join('');
 document.querySelectorAll('#cat-chips .chip').forEach(el=>el.onclick=()=>{bpCat=el.dataset.c;renderCats();renderBps()})}
function filteredBps(){const q=bpFilter.trim().toLowerCase();let list=blueprints;
 if(bpCat!=='all'){const c=CATS.find(x=>x.id===bpCat);if(c&&c.ids)list=list.filter(b=>c.ids.includes(b.id))}
 if(q)list=list.filter(b=>b.name.toLowerCase().includes(q)||b.id.includes(q)||String(b.description||'').toLowerCase().includes(q));
 const fav=favs();list=[...list.filter(b=>fav.includes(b.id)),...list.filter(b=>!fav.includes(b.id))];return list}
function renderBps(){const list=filteredBps();document.getElementById('bp-count').textContent=list.length===blueprints.length?`${blueprints.length} apps`:`${list.length} of ${blueprints.length}`;
 document.getElementById('bp-total').textContent=blueprints.length;
 document.getElementById('bps').innerHTML=list.length?list.map(b=>{const c=catOf(b.id);const f=favs().includes(b.id);
 return `<div class="bp ${selectedBpId===b.id?'on':''}" role="button" tabindex="0" data-id="${b.id}"><button type="button" class="bp-fav ${f?'on':''}" data-fav="${b.id}" aria-label="Favorite">${ico(f?'star':'starOutline')}</button><strong><span class="ico">${ico(c.i)}</span>${b.name}</strong><br><span class="hint">${b.description||b.id}</span></div>`}).join(''):'<p class="hint">No matches</p>';
 document.querySelectorAll('.bp').forEach(el=>{const id=el.dataset.id;el.onclick=e=>{if(e.target.closest('.bp-fav'))return;selectBp(id)};el.onkeydown=e=>{if(e.target.closest('.bp-fav'))return;if(e.key==='Enter'||e.key===' '){e.preventDefault();selectBp(id)}}});
 document.querySelectorAll('.bp-fav').forEach(btn=>btn.onclick=e=>{e.stopPropagation();toggleFav(btn.dataset.fav)})}
function startPreview(){stopPreview();if(!detail||!detail.metrics)return;const el=document.getElementById('live-preview');el.classList.remove('hidden');
 const tick=()=>{el.innerHTML=(detail.metrics||[]).slice(0,6).map(m=>{const v=(Math.random()*100).toFixed(m.unit==='%'?0:1);
 return `<div class="stat"><span class="label">${m.label}</span><span class="value">${v}${m.unit&&m.unit!=='bool'?m.unit:''}</span></div>`}).join('')};
 tick();previewIv=setInterval(tick,1800)}
function stopPreview(){if(previewIv)clearInterval(previewIv);previewIv=null}
function renderBoard(){
 const n=status.nodes&&status.nodes[0];document.getElementById('title').textContent=n?n.name:'HERMES';
 document.getElementById('subtitle').textContent=status.ip?`${status.ip} · ${n?n.node_type:''}`:'Board server';
 document.getElementById('ota-host').textContent='hermes-'+(n?n.id:0)+'.local';
 const t=status.telemetry&&status.telemetry.telemetry||{};
 document.getElementById('stats').innerHTML=`<div class="stat"><span class="label">CPU</span><span class="value">${Math.round(t.cpu||0)}%</span></div>`+
 `<div class="stat"><span class="label">RAM</span><span class="value">${t.ram_kb||0} KB</span></div>`+
 `<div class="stat"><span class="label">Temp</span><span class="value">${Math.round(t.temp_c||0)}°C</span></div>`+
 `<div class="stat"><span class="label">Status</span><span class="value online">Online</span></div>`;
 const sens=(status.telemetry&&status.telemetry.sensors)||status.sensors||[];
 const st=document.getElementById('sensor-tiles');
 st.innerHTML=sens.length?sens.slice(0,9).map(s=>`<div class="stat"><span class="label">${s.id}</span><span class="value">${Number(s.value).toFixed(1)}${s.unit||''}</span></div>`).join(''):'<p class="hint">No sensor readings yet.</p>';
 const a=status.apps&&status.apps[0];const appEl=document.getElementById('appinfo');
 if(a&&a.blueprint){appEl.className='app-card';appEl.innerHTML=`<h3>${a.display_name||a.name}</h3><p class="hint">${a.blueprint} · ${a.state}</p>`}
 else{appEl.className='app-card hint';appEl.textContent='No app deployed'}
 const h=hist();document.getElementById('app-history').innerHTML=h.map(x=>`<button type="button" class="chip" data-h="${x.id}">${x.name}</button>`).join('');
 document.querySelectorAll('#app-history .chip').forEach(el=>el.onclick=()=>{selectBp(el.dataset.h);setTab('apps')});
 const mesh=status.mesh&&status.mesh.nodes||[];
 document.getElementById('meshinfo').textContent=mesh.length?mesh.map(m=>`node ${m.node_id} ${m.name} ${m.online?'online':'offline'} [${m.sensors}]`).join('\n'):'No remote nodes yet';
 renderRelays(status.gpio||[])}
function renderRelays(relays){const q=document.getElementById('relay-quick');const g=document.getElementById('gpio-relays');
 const html=relays.length?relays.map(r=>`<div class="relay-row"><span>Relay ${r.idx}</span><button class="toggle ${r.on?'on':''}" data-idx="${r.idx}" aria-label="Relay ${r.idx}"></button></div>`).join(''):'<p class="hint">Assign relay pins in GPIO template.</p>';
 q.innerHTML=relays.length?html:'';g.innerHTML=html;document.querySelectorAll('.toggle').forEach(btn=>btn.onclick=async()=>{
 const idx=Number(btn.dataset.idx);const on=!btn.classList.contains('on');const r=await api('/api/gpio/relay',{method:'POST',body:JSON.stringify({idx,on})});
 if(r.error)toast(r.error,false);else{btn.classList.toggle('on',on);toast('Relay '+(on?'on':'off'),true)}})}
function fieldHtml(f,val){if(f.type==='toggle')return`<div class="field"><label><input type="checkbox" data-k="${f.key}" ${val?'checked':''}> ${f.label}</label></div>`;
 if(f.type==='sensor_list')return`<div class="field"><label>${f.label}</label><div class="sensor-grid">${sensors.map(s=>`<label><input type="checkbox" data-s="${s.id}" data-k="${f.key}" ${sensorSel(val,s.id)?'checked':''}> ${s.name}</label>`).join('')}</div></div>`;
 const t=f.type==='number'?'number':'text';return`<div class="field"><label>${f.label}</label><input data-k="${f.key}" type="${t}" value="${val??f.default??''}"></div>`;}
async function selectBp(id){selectedBpId=id;detail=await api('/api/blueprints/'+id);if(detail.error){toast(detail.error,false);return}
 document.querySelectorAll('.bp').forEach(b=>b.classList.toggle('on',b.dataset.id===id));
 document.getElementById('studio-title').textContent=detail.name||id;
 document.getElementById('fields').innerHTML=(detail.fields||[]).map(f=>fieldHtml(f,f.default)).join('');
 document.getElementById('deploy').disabled=false;startPreview();setTab('apps')}
function readCfg(){const cfg={};const sensorLists={};
 document.querySelectorAll('[data-k]').forEach(el=>{if(el.dataset.s){if(!sensorLists[el.dataset.k])sensorLists[el.dataset.k]=[];if(el.checked)sensorLists[el.dataset.k].push(el.dataset.s);return;}
 cfg[el.dataset.k]=el.type==='checkbox'?el.checked:(el.type==='number'?Number(el.value):el.value)});
 Object.keys(sensorLists).forEach(k=>{cfg[k]=sensorLists[k].join(',')});return cfg}
function showDeploySheet(cfg){pendingDeploy=cfg;const el=document.getElementById('deploy-summary');
 el.innerHTML=`<div class="row"><span>Blueprint</span><strong>${detail.name}</strong></div>`+
 `<div class="row"><span>Name</span><strong>${pickName(cfg)||detail.id}</strong></div>`+
 `<div class="row"><span>Sensors</span><span>${cfg.sensors||'—'}</span></div>`+
 `<div class="row"><span>Mesh</span><span>${cfg.mesh_enabled?'on':'off'}</span></div>`;
 document.getElementById('deploy-sheet').classList.remove('hidden')}
document.getElementById('deploy').onclick=()=>{if(!detail)return;showDeploySheet(readCfg())};
document.getElementById('deploy-cancel').onclick=()=>{document.getElementById('deploy-sheet').classList.add('hidden');pendingDeploy=null};
document.getElementById('deploy-sheet').onclick=e=>{if(e.target.id==='deploy-sheet'){document.getElementById('deploy-sheet').classList.add('hidden');pendingDeploy=null}};
document.getElementById('success-pop').onclick=e=>{if(e.target.id==='success-pop')document.getElementById('success-pop').classList.add('hidden')};
document.getElementById('deploy-confirm').onclick=async()=>{if(!pendingDeploy||!detail)return;const cfg=pendingDeploy;const name=slug(pickName(cfg)||detail.id)||detail.id;
 document.getElementById('deploy-sheet').classList.add('hidden');const btn=document.getElementById('deploy-confirm');btn.disabled=true;
 try{const r=await api('/api/apps/create',{method:'POST',body:JSON.stringify({blueprint:detail.id,name,config:cfg})});
 if(r.error)toast(r.error,false);else{pushHist(detail.id,r.display_name||r.name);document.getElementById('success-msg').textContent=r.display_name||r.name;
 document.getElementById('success-pop').classList.remove('hidden');}await refresh()}finally{btn.disabled=false;pendingDeploy=null}};
document.getElementById('success-ok').onclick=()=>{document.getElementById('success-pop').classList.add('hidden');setTab('home')};
async function loadNet(){const n=await api('/api/network/config');document.getElementById('wifi-mod').value=n.module_type||0;
 document.getElementById('wifi-ssid').value=n.ssid||'';document.getElementById('mesh-on').checked=!!n.mesh_enabled}
document.getElementById('save-net').onclick=async()=>{const r=await api('/api/network/config',{method:'POST',body:JSON.stringify({
 module_type:Number(document.getElementById('wifi-mod').value),ssid:document.getElementById('wifi-ssid').value,
 password:document.getElementById('wifi-pass').value,mesh_enabled:document.getElementById('mesh-on').checked,mesh_max_nodes:8,mesh_interval_sec:120})});
 if(r.error)toast(r.error,false);else toast('Network saved',true);await refresh()};
async function loadMqtt(){const m=await api('/api/mqtt/config');document.getElementById('mqtt-on').checked=!!m.enabled;
 document.getElementById('mqtt-host').value=m.host||'';document.getElementById('mqtt-port').value=m.port||1883;
 document.getElementById('mqtt-prefix').value=m.prefix||'hermes';document.getElementById('mqtt-status').textContent='MQTT: '+(m.connected?'connected':'disconnected');
 document.getElementById('mqtt-tele').textContent=m.last_tele||''}
document.getElementById('save-mqtt').onclick=async()=>{const r=await api('/api/mqtt/config',{method:'POST',body:JSON.stringify({
 enabled:document.getElementById('mqtt-on').checked,host:document.getElementById('mqtt-host').value,
 port:Number(document.getElementById('mqtt-port').value),prefix:document.getElementById('mqtt-prefix').value})});
 if(r.error)toast(r.error,false);else toast('MQTT saved',true);await loadMqtt()};
document.getElementById('mqtt-test').onclick=async()=>{const r=await api('/api/mqtt/test',{method:'POST',body:'{}'});toast(r.error||'Test sent',!r.error)};
function loadRules(){const rules=JSON.parse(localStorage.getItem('hermes_rules')||'[]');
 document.getElementById('rules-list').innerHTML=rules.map((r,i)=>`<div class="field"><label>Rule ${i+1}</label>
 <select data-r="${i}" data-f="sensor"><option ${r.sensor==='temp_humidity'?'selected':''}>temp_humidity</option><option ${r.sensor==='soil_moisture'?'selected':''}>soil_moisture</option></select>
 <select data-r="${i}" data-f="op"><option value="gt" ${r.op==='gt'?'selected':''}>&gt;</option><option value="lt" ${r.op==='lt'?'selected':''}>&lt;</option></select>
 <input data-r="${i}" data-f="threshold" type="number" value="${r.threshold||30}"></div>`).join('')||'<p class="hint">No rules yet.</p>'}
document.getElementById('add-rule').onclick=()=>{const rules=JSON.parse(localStorage.getItem('hermes_rules')||'[]');rules.push({sensor:'soil_moisture',op:'lt',threshold:30});localStorage.setItem('hermes_rules',JSON.stringify(rules));loadRules()};
document.getElementById('save-rules').onclick=async()=>{const rules=[];document.querySelectorAll('[data-r]').forEach(()=>{});
 const els=document.querySelectorAll('#rules-list [data-r]');const idx=new Set([...els].map(e=>e.dataset.r));
 idx.forEach(i=>{const g=k=>document.querySelector(`[data-r="${i}"][data-f="${k}"]`);rules.push({sensor:g('sensor').value,op:g('op').value,threshold:Number(g('threshold').value),enabled:true})});
 localStorage.setItem('hermes_rules',JSON.stringify(rules));await api('/api/rules',{method:'POST',body:JSON.stringify({rules})});toast('Rules saved',true)};
document.getElementById('ota-check').onclick=async()=>{const url=document.getElementById('ota-url').value;
 const r=await api('/api/ota/status'+(url?'?url='+encodeURIComponent(url):''));document.getElementById('ota-status').textContent=r.message||r.error||'No update'};
async function refresh(){status=await api('/api/status');renderBoard()}
document.body.addEventListener('touchstart',e=>{touchY0=e.touches[0].clientY},{passive:true});
document.body.addEventListener('touchend',async e=>{if(touchY0-e.changedTouches[0].clientY<-70){await refresh();toast('Refreshed',true)}},{passive:true});
(async()=>{initNav();renderCats();
 const ts=document.getElementById('theme-sel');const lt=localStorage.getItem('hermes_theme')||'apple';document.documentElement.dataset.theme=lt;ts.value=lt;
 ts.onchange=()=>{document.documentElement.dataset.theme=ts.value;localStorage.setItem('hermes_theme',ts.value)};
 document.getElementById('bp-search').oninput=e=>{bpFilter=e.target.value;renderBps()};
 sensors=(await api('/api/sensors')).sensors||[];blueprints=(await api('/api/blueprints')).blueprints||[];renderBps();
 await loadNet();await loadMqtt();loadRules();await refresh();setInterval(refresh,3000)})();
</script></body></html>
)HTML";

static void handle_root() {
    if (hermes_wifi_in_setup_mode())
        server.send_P(200, "text/html", SETUP_HTML);
    else
        server.send_P(200, "text/html", INDEX_HTML);
}

static void setup_routes() {
    server.on("/", HTTP_GET, handle_root);
    server.on("/api/status", HTTP_GET, handle_status);
    server.on("/api/sensors", HTTP_GET, handle_sensors);
    server.on("/api/blueprints", HTTP_GET, handle_blueprints);
    server.on("/api/network/config", HTTP_GET, handle_network_get);
    server.on("/api/network/config", HTTP_POST, handle_network_post);
    server.on("/api/wifi/scan", HTTP_GET, handle_wifi_scan);
    server.on("/api/wifi/setup", HTTP_POST, handle_wifi_setup);
    server.on("/generate_204", HTTP_GET, handle_captive_redirect);
    server.on("/hotspot-detect.html", HTTP_GET, handle_captive_redirect);
    server.on("/ncsi.txt", HTTP_GET, handle_captive_redirect);
    server.on("/connecttest.txt", HTTP_GET, handle_captive_redirect);
    server.on("/fwlink", HTTP_GET, handle_captive_redirect);
    server.on("/board", HTTP_GET, handle_board_page);
    server.on("/api/board/templates", HTTP_GET, handle_board_template_list);
    server.on("/api/board/gpio", HTTP_GET, handle_board_gpio_get);
    server.on("/api/board/gpio", HTTP_POST, handle_board_gpio_post);
    server.on("/api/board/preset", HTTP_POST, handle_board_preset_post);
    server.on("/api/board/tasmota", HTTP_POST, handle_board_tasmota_post);
    server.on("/api/mqtt/config", HTTP_GET, handle_mqtt_get);
    server.on("/api/mqtt/config", HTTP_POST, handle_mqtt_post);
    server.on("/api/mqtt/test", HTTP_POST, handle_mqtt_test);
    server.on("/api/gpio/relay", HTTP_POST, handle_gpio_relay_post);
    server.on("/api/rules", HTTP_GET, handle_rules_get);
    server.on("/api/rules", HTTP_POST, handle_rules_post);
    server.on("/api/ota/status", HTTP_GET, handle_ota_status);
    server.on("/api/mesh/nodes", HTTP_GET, handle_mesh_nodes);
    server.on("/api/apps/create", HTTP_POST, handle_app_create);
    server.on("/api/apps/create", HTTP_OPTIONS, []() { server.send(204); });
    server.onNotFound([]() {
        if (hermes_wifi_in_setup_mode()) {
            handle_captive_redirect();
            return;
        }
        String uri = server.uri();
        if (uri.startsWith("/api/blueprints/")) {
            handle_blueprint_detail();
            return;
        }
        if (uri.startsWith("/api/apps/") && uri.endsWith("/config")) {
            if (server.method() == HTTP_GET) handle_app_config_get();
            else if (server.method() == HTTP_POST) handle_app_config_post();
            else server.send(405);
            return;
        }
        server.send(404, "text/plain", "Not found");
    });
}

void hermes_http_begin() {
    hermes_board_store_load(&g_app);
    hermes_board_store_load_network(&g_net);
    hermes_board_store_load_gpio(&g_gpio);
    if (!g_net.ssid[0] && strlen(HERMES_WIFI_SSID) > 0) {
        strncpy(g_net.ssid, HERMES_WIFI_SSID, sizeof(g_net.ssid) - 1);
        strncpy(g_net.password, HERMES_WIFI_PASS, sizeof(g_net.password) - 1);
    }
    if (!g_net.module_type) g_net.module_type = hermes_wifi_default_module_type();
    setup_routes();
    hermes_mesh_init(&g_net);
    if (hermes_wifi_begin(&g_net) != 0) {
        Serial.println("HERMES HTTP: WiFi module failed to start");
        return;
    }
    hermes_gpio_apply(&g_gpio);
    hermes_board_store_load_mqtt(&g_mqtt);
    if (!g_mqtt.prefix[0]) {
        hermes_mqtt_defaults(&g_mqtt);
    }
    hermes_mqtt_begin(&g_mqtt);
    if (!hermes_wifi_in_setup_mode()) {
        hermes_ota_begin();
    }
    g_http_ok = true;
    server.begin();
    if (hermes_wifi_in_setup_mode()) {
        Serial.printf("HERMES WiFi setup: join AP \"%s\" then open http://%s/\n",
                      hermes_wifi_setup_ap_name(), hermes_wifi_ip_string());
    } else {
        Serial.printf("HERMES HTTP server: http://%s/\n", hermes_wifi_ip_string());
    }
}

void hermes_http_loop() {
    if (!g_http_ok) return;
    hermes_wifi_loop();
    hermes_gpio_loop();
    hermes_mqtt_loop();
    hermes_ota_loop();
    hermes_mesh_loop();
    server.handleClient();
    g_packets++;
}

#else

void hermes_http_begin(void) {}
void hermes_http_loop(void) {}

#endif
