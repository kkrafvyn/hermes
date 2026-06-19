#include "hermes_tft.h"

#if defined(HERMES_TFT_CYD2432) && defined(HERMES_ESP32)

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ESP32-2432S028 / common WROOM + 2.4" ST7789 (CYD-style) default SPI pins
#ifndef HERMES_TFT_MOSI
#define HERMES_TFT_MOSI 13
#endif
#ifndef HERMES_TFT_SCLK
#define HERMES_TFT_SCLK 14
#endif
#ifndef HERMES_TFT_CS
#define HERMES_TFT_CS 15
#endif
#ifndef HERMES_TFT_DC
#define HERMES_TFT_DC 2
#endif
#ifndef HERMES_TFT_RST
#define HERMES_TFT_RST -1
#endif
#ifndef HERMES_TFT_BL
#define HERMES_TFT_BL 21
#endif

static Adafruit_ST7789 tft(HERMES_TFT_CS, HERMES_TFT_DC, HERMES_TFT_MOSI, HERMES_TFT_SCLK, HERMES_TFT_RST);
static bool g_tft_ok = false;

void hermes_tft_begin(void) {
    pinMode(HERMES_TFT_BL, OUTPUT);
    digitalWrite(HERMES_TFT_BL, HIGH);
    tft.init(240, 320);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setCursor(8, 8);
    tft.println("HERMES");
    tft.setTextSize(1);
    tft.println("ESP32 WROOM");
    g_tft_ok = true;
}

void hermes_tft_show_status(const char *line1, const char *line2, const char *line3) {
    if (!g_tft_ok) return;
    tft.fillRect(0, 48, 320, 192, ST77XX_BLACK);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(8, 52);
    tft.println(line1 ? line1 : "");
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(8, 80);
    tft.println(line2 ? line2 : "");
    tft.setCursor(8, 108);
    tft.println(line3 ? line3 : "");
}

#else

void hermes_tft_begin(void) {}
void hermes_tft_show_status(const char *line1, const char *line2, const char *line3) {
    (void)line1;
    (void)line2;
    (void)line3;
}

#endif
