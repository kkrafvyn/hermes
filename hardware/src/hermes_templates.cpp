#include "hermes_templates.h"

#include <string.h>

static void set_raw(hermes_board_gpio_t *cfg, const char *tid, const char *label) {
    hermes_gpio_defaults(cfg);
    strncpy(cfg->template_id, tid, sizeof(cfg->template_id) - 1);
    strncpy(cfg->board_label, label, sizeof(cfg->board_label) - 1);
}

static void add(hermes_board_gpio_t *c, uint8_t gpio, uint8_t fn, uint8_t flags, uint8_t idx) {
    if (c->count >= HERMES_GPIO_MAX) return;
    c->entries[c->count++] = (hermes_gpio_entry_t){gpio, fn, flags, idx};
}

static void tpl_esp8266_nodemcu(hermes_board_gpio_t *cfg) {
    set_raw(cfg, "esp8266_nodemcu", "ESP8266 NodeMCU");
    add(cfg, 2, HERMES_FN_LED, 0, 0);
    add(cfg, 4, HERMES_FN_RELAY, 0, 0);
    add(cfg, 5, HERMES_FN_BUTTON, HERMES_GPIO_FLAG_PULLUP, 0);
}

static void tpl_esp32_cyd(hermes_board_gpio_t *cfg) {
    set_raw(cfg, "esp32_cyd2432", "ESP32 CYD 2.4 TFT");
    add(cfg, 13, HERMES_FN_TFT_MOSI, 0, 0);
    add(cfg, 14, HERMES_FN_TFT_SCLK, 0, 0);
    add(cfg, 15, HERMES_FN_TFT_CS, 0, 0);
    add(cfg, 2, HERMES_FN_TFT_DC, 0, 0);
    add(cfg, 21, HERMES_FN_TFT_BL, 0, 0);
}

static void tpl_waveshare_sim7670g(hermes_board_gpio_t *cfg) {
    set_raw(cfg, "waveshare_esp32s3_sim7670g", "Waveshare ESP32-S3 SIM7670G");
    add(cfg, 38, HERMES_FN_LED, 0, 0);
    add(cfg, 15, HERMES_FN_I2C_SDA, 0, 0);
    add(cfg, 16, HERMES_FN_I2C_SCL, 0, 0);
    add(cfg, 5, HERMES_FN_SD_CLK, 0, 0);
    add(cfg, 4, HERMES_FN_SD_CMD, 0, 0);
    add(cfg, 6, HERMES_FN_SD_DATA, 0, 0);
    add(cfg, 46, HERMES_FN_SD_CD, 0, 0);
    add(cfg, 17, HERMES_FN_MODEM_TX, 0, 0);
    add(cfg, 18, HERMES_FN_MODEM_RX, 0, 0);
    add(cfg, 45, HERMES_FN_MODEM_PWR, 0, 0);
    add(cfg, 34, HERMES_FN_USER, 0, 0);
    add(cfg, 7, HERMES_FN_USER, 0, 1);
    add(cfg, 8, HERMES_FN_USER, 0, 2);
    add(cfg, 9, HERMES_FN_USER, 0, 3);
    add(cfg, 10, HERMES_FN_USER, 0, 4);
    add(cfg, 11, HERMES_FN_USER, 0, 5);
    add(cfg, 12, HERMES_FN_USER, 0, 6);
    add(cfg, 13, HERMES_FN_USER, 0, 7);
    add(cfg, 14, HERMES_FN_USER, 0, 8);
    add(cfg, 36, HERMES_FN_USER, 0, 9);
    add(cfg, 35, HERMES_FN_USER, 0, 10);
    add(cfg, 37, HERMES_FN_USER, 0, 11);
}

static void tpl_stm32h7_nucleo(hermes_board_gpio_t *cfg) {
    set_raw(cfg, "stm32h7_nucleo", "STM32H743 Nucleo");
    add(cfg, 13, HERMES_FN_LED, 0, 0);  /* LD1 green */
    add(cfg, 14, HERMES_FN_LED, 0, 1);  /* LD2 yellow */
    add(cfg, 15, HERMES_FN_LED, 0, 2);  /* LD3 red */
    add(cfg, 12, HERMES_FN_BUTTON, HERMES_GPIO_FLAG_PULLUP, 0); /* USER btn */
}

static void tpl_nrf52840_feather(hermes_board_gpio_t *cfg) {
    set_raw(cfg, "nrf52840_feather", "Adafruit Feather nRF52840");
    add(cfg, 3, HERMES_FN_LED, 0, 0);   /* red LED */
    add(cfg, 4, HERMES_FN_BUTTON, HERMES_GPIO_FLAG_PULLUP, 0);
    add(cfg, 5, HERMES_FN_I2C_SDA, 0, 0);
    add(cfg, 6, HERMES_FN_I2C_SCL, 0, 0);
}

static void tpl_rp2040_pico(hermes_board_gpio_t *cfg) {
    set_raw(cfg, "rp2040_pico", "Raspberry Pi Pico");
    add(cfg, 25, HERMES_FN_LED, 0, 0);  /* onboard LED */
}

static void tpl_avr_uno(hermes_board_gpio_t *cfg) {
    set_raw(cfg, "avr_uno", "Arduino Uno");
    add(cfg, 13, HERMES_FN_LED, 0, 0);
    add(cfg, 2, HERMES_FN_OUTPUT, 0, 0);
    add(cfg, 4, HERMES_FN_RELAY, 0, 0);
}

static const hermes_template_info_t TEMPLATE_LIST[] = {
    {"raw", "Raw / custom", "any", "Blank — assign pins via HTTP /board or serial JSON"},
    {"esp8266_raw", "ESP8266 raw", "esp8266", "Custom ESP8266 pinout"},
    {"esp8266_nodemcu", "ESP8266 NodeMCU", "esp8266", "GPIO2 LED, GPIO4 relay, GPIO5 button"},
    {"esp32_raw", "ESP32-WROOM raw", "esp32", "Custom ESP32 classic"},
    {"esp32_cyd2432", "ESP32 CYD TFT", "esp32", "2.4\" ST7789 SPI"},
    {"esp32c3_raw", "ESP32-C3 raw", "esp32c3", "Custom ESP32-C3"},
    {"esp32s2_raw", "ESP32-S2 raw", "esp32s2", "Custom ESP32-S2"},
    {"esp32s3_raw", "ESP32-S3 raw", "esp32s3", "Custom ESP32-S3"},
    {"waveshare_esp32s3_sim7670g", "Waveshare SIM7670G 4G", "esp32s3", "4G, camera, TF, RGB, battery"},
    {"stm32_raw", "STM32 raw", "stm32", "Custom STM32 — configure via serial or PC dashboard"},
    {"stm32h7_nucleo", "STM32H7 Nucleo", "stm32h7", "Nucleo-H743 LEDs + user button"},
    {"stm32f4_raw", "STM32F4 raw", "stm32f4", "Custom STM32F4"},
    {"stm32f1_raw", "STM32F1 raw", "stm32f1", "Blue Pill / custom F1"},
    {"nrf52840_raw", "nRF52840 raw", "nrf52840", "Custom nRF52840"},
    {"nrf52840_feather", "Feather nRF52840", "nrf52840", "Feather Express defaults"},
    {"nrf52832_raw", "nRF52832 raw", "nrf52832", "Custom nRF52832"},
    {"rp2040_raw", "RP2040 raw", "rp2040", "Custom RP2040 / Pico"},
    {"rp2040_pico", "Raspberry Pi Pico", "rp2040", "GPIO25 onboard LED"},
    {"avr_raw", "AVR raw", "avr", "Custom ATmega"},
    {"avr_uno", "Arduino Uno", "avr", "D13 LED, D2/D4 example"},
    {"samd21_raw", "SAMD21 raw", "samd21", "Custom SAMD21"},
    {"samd51_raw", "SAMD51 raw", "samd51", "Custom SAMD51"},
    {"bk7231_raw", "Beken BK7231 raw", "bk7231", "Tuya CB2S custom — HTTP /board"},
    {"rtl8710_raw", "Realtek RTL8710 raw", "rtl8710", "RTL8710 custom — HTTP /board"},
    {"msp430_raw", "MSP430 raw", "msp430", "Custom MSP430"},
    {"renesas_ra_raw", "Renesas RA raw", "renesas", "Custom Renesas RA"},
    {"pic32_raw", "PIC32 raw", "pic32", "Custom PIC32"},
};

const hermes_template_info_t *hermes_template_list(size_t *count) {
    if (count) *count = sizeof(TEMPLATE_LIST) / sizeof(TEMPLATE_LIST[0]);
    return TEMPLATE_LIST;
}

static int apply_id(const char *id, hermes_board_gpio_t *out) {
    if (strcmp(id, "raw") == 0) { set_raw(out, "raw", "Custom / raw"); return 0; }
    if (strcmp(id, "esp8266_raw") == 0) { set_raw(out, "esp8266_raw", "ESP8266 custom"); return 0; }
    if (strcmp(id, "esp32_raw") == 0 || strcmp(id, "esp32_wroom_raw") == 0) {
        set_raw(out, "esp32_raw", "ESP32 custom"); return 0;
    }
    if (strcmp(id, "esp32c3_raw") == 0) { set_raw(out, "esp32c3_raw", "ESP32-C3 custom"); return 0; }
    if (strcmp(id, "esp32s2_raw") == 0) { set_raw(out, "esp32s2_raw", "ESP32-S2 custom"); return 0; }
    if (strcmp(id, "esp32s3_raw") == 0) { set_raw(out, "esp32s3_raw", "ESP32-S3 custom"); return 0; }
    if (strcmp(id, "stm32_raw") == 0) { set_raw(out, "stm32_raw", "STM32 custom"); return 0; }
    if (strcmp(id, "stm32f4_raw") == 0) { set_raw(out, "stm32f4_raw", "STM32F4 custom"); return 0; }
    if (strcmp(id, "stm32f1_raw") == 0) { set_raw(out, "stm32f1_raw", "STM32F1 custom"); return 0; }
    if (strcmp(id, "nrf52840_raw") == 0) { set_raw(out, "nrf52840_raw", "nRF52840 custom"); return 0; }
    if (strcmp(id, "nrf52832_raw") == 0) { set_raw(out, "nrf52832_raw", "nRF52832 custom"); return 0; }
    if (strcmp(id, "rp2040_raw") == 0) { set_raw(out, "rp2040_raw", "RP2040 custom"); return 0; }
    if (strcmp(id, "avr_raw") == 0) { set_raw(out, "avr_raw", "AVR custom"); return 0; }
    if (strcmp(id, "samd21_raw") == 0) { set_raw(out, "samd21_raw", "SAMD21 custom"); return 0; }
    if (strcmp(id, "samd51_raw") == 0) { set_raw(out, "samd51_raw", "SAMD51 custom"); return 0; }
    if (strcmp(id, "bk7231_raw") == 0) { set_raw(out, "bk7231_raw", "Beken custom"); return 0; }
    if (strcmp(id, "rtl8710_raw") == 0) { set_raw(out, "rtl8710_raw", "Realtek custom"); return 0; }
    if (strcmp(id, "msp430_raw") == 0) { set_raw(out, "msp430_raw", "MSP430 custom"); return 0; }
    if (strcmp(id, "renesas_ra_raw") == 0) { set_raw(out, "renesas_ra_raw", "Renesas custom"); return 0; }
    if (strcmp(id, "pic32_raw") == 0) { set_raw(out, "pic32_raw", "PIC32 custom"); return 0; }
    if (strcmp(id, "esp8266_nodemcu") == 0) { tpl_esp8266_nodemcu(out); return 0; }
    if (strcmp(id, "esp32_cyd2432") == 0) { tpl_esp32_cyd(out); return 0; }
    if (strcmp(id, "waveshare_esp32s3_sim7670g") == 0) { tpl_waveshare_sim7670g(out); return 0; }
    if (strcmp(id, "stm32h7_nucleo") == 0) { tpl_stm32h7_nucleo(out); return 0; }
    if (strcmp(id, "nrf52840_feather") == 0) { tpl_nrf52840_feather(out); return 0; }
    if (strcmp(id, "rp2040_pico") == 0) { tpl_rp2040_pico(out); return 0; }
    if (strcmp(id, "avr_uno") == 0) { tpl_avr_uno(out); return 0; }
    return -1;
}

int hermes_template_apply(const char *template_id, hermes_board_gpio_t *out) {
    if (!template_id || !out) return -1;
    return apply_id(template_id, out);
}

void hermes_template_apply_build_default(hermes_board_gpio_t *out) {
#if defined(HERMES_BOARD_WAVESHARE_SIM7670G)
    tpl_waveshare_sim7670g(out);
#elif defined(HERMES_TFT_CYD2432)
    tpl_esp32_cyd(out);
#elif defined(HERMES_BOARD_RAW)
    set_raw(out, "raw", "Custom / raw");
#elif defined(HERMES_STM32) && defined(HERMES_NODE_ID) && (HERMES_NODE_ID == 0)
    tpl_stm32h7_nucleo(out);
#elif defined(HERMES_STM32)
    set_raw(out, "stm32_raw", "STM32 custom");
#elif defined(HERMES_NRF)
    tpl_nrf52840_feather(out);
#elif defined(HERMES_RP2040)
    tpl_rp2040_pico(out);
#elif defined(HERMES_AVR)
    tpl_avr_uno(out);
#elif defined(HERMES_ESP8266) || defined(ESP8266)
    tpl_esp8266_nodemcu(out);
#elif defined(HERMES_BEKEN)
    set_raw(out, "bk7231_raw", "Beken custom");
#elif defined(HERMES_REALTEK)
    set_raw(out, "rtl8710_raw", "Realtek custom");
#elif defined(HERMES_ESP32)
    set_raw(out, "esp32_raw", "ESP32 custom");
#else
    set_raw(out, "raw", "Custom / raw");
#endif
}
