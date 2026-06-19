#include "hermes_gpio.h"

#include <Arduino.h>
#include <string.h>

#if defined(HERMES_ESP32) || defined(ESP32) || defined(HERMES_STM32) || defined(HERMES_SAMD)
#include <Wire.h>
#endif

static hermes_gpio_entry_t g_active[HERMES_GPIO_MAX];
static uint8_t g_active_count = 0;
static bool g_relay_state[8];
static bool g_button_last[8];
static uint32_t g_last_gpio_loop = 0;

static const char *const FN_NAMES[] = {
    "None", "Relay", "Button", "Input", "Output", "ADC", "PWM",
    "I2C SDA", "I2C SCL", "UART TX", "UART RX", "LED",
    "TFT BL", "TFT DC", "TFT RST", "TFT CS", "TFT MOSI", "TFT SCLK",
    "Modem PWR", "Modem RST", "Modem TX", "Modem RX",
    "Camera PWDN", "Camera RST",
    "SD CLK", "SD CMD", "SD DATA", "SD CD", "User",
};

const char *hermes_gpio_fn_name(uint8_t fn) {
    if (fn == HERMES_FN_USER) return "User";
    if (fn < sizeof(FN_NAMES) / sizeof(FN_NAMES[0])) return FN_NAMES[fn];
    return "Unknown";
}

void hermes_gpio_defaults(hermes_board_gpio_t *out) {
    memset(out, 0, sizeof(*out));
    strncpy(out->template_id, "raw", sizeof(out->template_id) - 1);
    strncpy(out->board_label, "Custom board", sizeof(out->board_label) - 1);
}

static void add_entry(hermes_board_gpio_t *cfg, uint8_t gpio, uint8_t fn, uint8_t flags, uint8_t idx) {
    if (cfg->count >= HERMES_GPIO_MAX) return;
    hermes_gpio_entry_t *e = &cfg->entries[cfg->count++];
    e->gpio = gpio;
    e->fn = fn;
    e->flags = flags;
    e->idx = idx;
}

static int pin_mode_output(uint8_t gpio) {
#if defined(HERMES_ESP8266) || defined(ESP8266)
    if (gpio >= 16) return -1;
#endif
    pinMode(gpio, OUTPUT);
    return 0;
}

static int pin_mode_input(uint8_t gpio, bool pullup) {
#if defined(HERMES_ESP8266) || defined(ESP8266)
    if (gpio >= 16) return -1;
#endif
    pinMode(gpio, pullup ? INPUT_PULLUP : INPUT);
    return 0;
}

int hermes_gpio_apply(const hermes_board_gpio_t *cfg) {
    if (!cfg) return -1;
    g_active_count = cfg->count > HERMES_GPIO_MAX ? HERMES_GPIO_MAX : cfg->count;
    memcpy(g_active, cfg->entries, g_active_count * sizeof(hermes_gpio_entry_t));

    int sda = -1, scl = -1;
    for (uint8_t i = 0; i < g_active_count; i++) {
        const hermes_gpio_entry_t *e = &g_active[i];
        if (e->gpio == HERMES_GPIO_UNUSED) continue;
        switch (e->fn) {
        case HERMES_FN_RELAY:
        case HERMES_FN_OUTPUT:
        case HERMES_FN_LED:
        case HERMES_FN_MODEM_PWR:
        case HERMES_FN_MODEM_RST:
        case HERMES_FN_TFT_BL:
            if (pin_mode_output(e->gpio) == 0)
                digitalWrite(e->gpio, (e->flags & HERMES_GPIO_FLAG_INVERT) ? HIGH : LOW);
            break;
        case HERMES_FN_BUTTON:
        case HERMES_FN_INPUT:
            pin_mode_input(e->gpio, (e->flags & HERMES_GPIO_FLAG_PULLUP) != 0);
            break;
        case HERMES_FN_I2C_SDA:
            sda = e->gpio;
            break;
        case HERMES_FN_I2C_SCL:
            scl = e->gpio;
            break;
        default:
            break;
        }
    }
#if defined(HERMES_ESP32) || defined(ESP32) || defined(HERMES_STM32) || defined(HERMES_SAMD)
    if (sda >= 0 && scl >= 0)
        Wire.begin(sda, scl);
#endif
    Serial.printf("HERMES GPIO: applied template \"%s\" (%u pins)\n", cfg->template_id, cfg->count);
    return 0;
}

int hermes_gpio_find(uint8_t fn, uint8_t idx) {
    for (uint8_t i = 0; i < g_active_count; i++) {
        if (g_active[i].fn == fn && g_active[i].idx == idx)
            return g_active[i].gpio;
    }
    return -1;
}

int hermes_gpio_set_relay(uint8_t idx, bool on) {
    for (uint8_t i = 0; i < g_active_count; i++) {
        const hermes_gpio_entry_t *e = &g_active[i];
        if (e->fn != HERMES_FN_RELAY && e->fn != HERMES_FN_OUTPUT) continue;
        if (e->idx != idx) continue;
        bool level = on;
        if (e->flags & HERMES_GPIO_FLAG_INVERT) level = !level;
        digitalWrite(e->gpio, level ? HIGH : LOW);
        if (idx < 8) g_relay_state[idx] = on;
        return 0;
    }
    return -1;
}

void hermes_gpio_states_json(char *buf, size_t max) {
    if (!buf || max < 32) return;
    size_t pos = 0;
    pos += (size_t)snprintf(buf + pos, max - pos, "{\"POWER\":");
    bool first = true;
    for (uint8_t r = 0; r < 8; r++) {
        int gpio = hermes_gpio_find(HERMES_FN_RELAY, r);
        if (gpio < 0) continue;
        if (!first) buf[pos++] = ',';
        first = false;
        pos += (size_t)snprintf(buf + pos, max - pos, "\"POWER%u\":\"%s\"",
            r + 1, g_relay_state[r] ? "ON" : "OFF");
    }
    if (first) pos += (size_t)snprintf(buf + pos, max - pos, "\"POWER\":\"OFF\"");
    for (uint8_t b = 0; b < 8 && pos < max - 24; b++) {
        int gpio = hermes_gpio_find(HERMES_FN_BUTTON, b);
        if (gpio < 0) continue;
        int v = digitalRead(gpio);
        pos += (size_t)snprintf(buf + pos, max - pos, ",\"BUTTON%u\":%d", b + 1, v == LOW ? 1 : 0);
    }
    snprintf(buf + pos, max - pos, "}");
}

void hermes_gpio_loop(void) {
    if (millis() - g_last_gpio_loop < 50) return;
    g_last_gpio_loop = millis();
    for (uint8_t i = 0; i < g_active_count; i++) {
        const hermes_gpio_entry_t *e = &g_active[i];
        if (e->fn != HERMES_FN_BUTTON) continue;
        if (e->idx >= 8) continue;
        bool pressed = digitalRead(e->gpio) == LOW;
        if (pressed != g_button_last[e->idx]) {
            g_button_last[e->idx] = pressed;
            if (pressed) {
                Serial.printf("{\"button\":%u,\"gpio\":%u}\n", e->idx + 1, e->gpio);
            }
        }
    }
}

/* Tasmota template GPIO[] values (subset) */
static uint8_t tasmota_fn_decode(int code) {
    if (code == 0) return HERMES_FN_NONE;
    if (code >= 224 && code < 240) return HERMES_FN_RELAY;
    if (code >= 32 && code < 48) return HERMES_FN_BUTTON;
    if (code >= 192 && code < 208) return HERMES_FN_PWM;
    if (code == 1 || code == 17) return HERMES_FN_INPUT;
    return HERMES_FN_USER;
}

int hermes_gpio_import_tasmota(const char *json, hermes_board_gpio_t *out) {
    if (!json || !out) return -1;
    const char *gpio_arr = strstr(json, "\"GPIO\"");
    if (!gpio_arr) gpio_arr = strstr(json, "\"gpio\"");
    if (!gpio_arr) return -1;
    gpio_arr = strchr(gpio_arr, '[');
    if (!gpio_arr) return -1;
    gpio_arr++;
    hermes_gpio_defaults(out);
    strncpy(out->template_id, "tasmota_import", sizeof(out->template_id) - 1);
    strncpy(out->board_label, "Tasmota import", sizeof(out->board_label) - 1);
    out->count = 0;
    uint8_t relay_idx = 0, btn_idx = 0;
    for (uint8_t pin = 0; pin < HERMES_GPIO_MAX && *gpio_arr && *gpio_arr != ']'; pin++) {
        while (*gpio_arr == ' ' || *gpio_arr == ',') gpio_arr++;
        if (*gpio_arr == ']') break;
        int code = atoi(gpio_arr);
        uint8_t fn = tasmota_fn_decode(code);
        if (fn != HERMES_FN_NONE && fn != HERMES_FN_USER) {
            hermes_gpio_entry_t *e = &out->entries[out->count++];
            e->gpio = pin;
            e->fn = fn;
            e->flags = (code & 1) ? HERMES_GPIO_FLAG_INVERT : 0;
            if (fn == HERMES_FN_BUTTON) {
                e->flags |= HERMES_GPIO_FLAG_PULLUP;
                e->idx = btn_idx++;
            } else if (fn == HERMES_FN_RELAY) {
                e->idx = relay_idx++;
            }
        }
        while (*gpio_arr && *gpio_arr != ',' && *gpio_arr != ']') gpio_arr++;
    }
    return out->count > 0 ? 0 : -1;
}

static int json_uint(const char *body, const char *key, int fallback) {
    char pat[40];
    snprintf(pat, sizeof(pat), "\"%s\"", key);
    const char *p = strstr(body, pat);
    if (!p) return fallback;
    p = strchr(p + strlen(pat), ':');
    if (!p) return fallback;
    return atoi(p + 1);
}

int hermes_gpio_from_json(const char *json, hermes_board_gpio_t *out) {
    if (!json || !out) return -1;
    hermes_gpio_defaults(out);
    const char *tid = strstr(json, "\"template_id\"");
    if (tid) {
        const char *q = strchr(tid + 13, '"');
        if (q) {
            q++;
            const char *e = strchr(q, '"');
            if (e && (size_t)(e - q) < sizeof(out->template_id)) {
                memcpy(out->template_id, q, (size_t)(e - q));
                out->template_id[e - q] = '\0';
            }
        }
    }
    const char *lbl = strstr(json, "\"board_label\"");
    if (lbl) {
        const char *q = strchr(lbl + 13, '"');
        if (q) {
            q++;
            const char *e = strchr(q, '"');
            if (e && (size_t)(e - q) < sizeof(out->board_label)) {
                memcpy(out->board_label, q, (size_t)(e - q));
                out->board_label[e - q] = '\0';
            }
        }
    }
    const char *pins = strstr(json, "\"pins\"");
    if (!pins) return 0;
    pins = strchr(pins, '[');
    if (!pins) return 0;
    pins++;
    out->count = 0;
    while (*pins && *pins != ']' && out->count < HERMES_GPIO_MAX) {
        const char *obj = strchr(pins, '{');
        if (!obj || obj > strchr(pins, ']')) break;
        hermes_gpio_entry_t *e = &out->entries[out->count];
        e->gpio = (uint8_t)json_uint(obj, "gpio", HERMES_GPIO_UNUSED);
        e->fn = (uint8_t)json_uint(obj, "fn", HERMES_FN_NONE);
        e->flags = (uint8_t)json_uint(obj, "flags", 0);
        e->idx = (uint8_t)json_uint(obj, "idx", 0);
        out->count++;
        pins = strchr(obj, '}');
        if (!pins) break;
        pins++;
    }
    return 0;
}

void hermes_gpio_to_json(const hermes_board_gpio_t *cfg, char *buf, size_t max) {
    if (!cfg || !buf || max < 32) return;
    size_t pos = 0;
    pos += (size_t)snprintf(buf + pos, max - pos,
        "{\"template_id\":\"%s\",\"board_label\":\"%s\",\"pins\":[",
        cfg->template_id, cfg->board_label);
    for (uint8_t i = 0; i < cfg->count && pos < max - 48; i++) {
        const hermes_gpio_entry_t *e = &cfg->entries[i];
        if (i) buf[pos++] = ',';
        pos += (size_t)snprintf(buf + pos, max - pos,
            "{\"gpio\":%u,\"fn\":%u,\"flags\":%u,\"idx\":%u}",
            e->gpio, e->fn, e->flags, e->idx);
    }
    snprintf(buf + pos, max - pos, "]}");
}
