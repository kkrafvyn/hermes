/**
 * HERMES MCU family detection — shared by main.cpp and hermes_sensor.cpp
 */
#ifndef HERMES_MCU_H
#define HERMES_MCU_H

#if defined(HERMES_ESP8266) || defined(ESP8266)
#define HERMES_MCU_FAMILY "espressif8266"
#define HERMES_MCU_NAME "esp8266"
#elif defined(HERMES_ESP32) || defined(ESP32)
#define HERMES_MCU_FAMILY "espressif32"
#define HERMES_MCU_NAME "esp32"
#elif defined(HERMES_STM32)
#define HERMES_MCU_FAMILY "stm32"
#define HERMES_MCU_NAME "stm32"
#elif defined(HERMES_NRF)
#define HERMES_MCU_FAMILY "nordic"
#define HERMES_MCU_NAME "nrf52"
#elif defined(HERMES_RP2040)
#define HERMES_MCU_FAMILY "rp2040"
#define HERMES_MCU_NAME "rp2040"
#elif defined(HERMES_AVR)
#define HERMES_MCU_FAMILY "avr"
#define HERMES_MCU_NAME "avr"
#elif defined(HERMES_SAMD)
#define HERMES_MCU_FAMILY "samd"
#define HERMES_MCU_NAME "samd"
#elif defined(HERMES_MSP430)
#define HERMES_MCU_FAMILY "msp430"
#define HERMES_MCU_NAME "msp430"
#elif defined(HERMES_EFR32)
#define HERMES_MCU_FAMILY "efr32"
#define HERMES_MCU_NAME "efr32"
#elif defined(HERMES_BEKEN)
#define HERMES_MCU_FAMILY "beken"
#define HERMES_MCU_NAME "bk7231"
#elif defined(HERMES_REALTEK)
#define HERMES_MCU_FAMILY "realtek"
#define HERMES_MCU_NAME "rtl87xx"
#elif defined(HERMES_NXP) || defined(HERMES_IMXRT)
#define HERMES_MCU_FAMILY "nxp"
#define HERMES_MCU_NAME "nxp"
#elif defined(HERMES_RENESAS)
#define HERMES_MCU_FAMILY "renesas"
#define HERMES_MCU_NAME "renesas_ra"
#elif defined(HERMES_PIC)
#define HERMES_MCU_FAMILY "pic"
#define HERMES_MCU_NAME "pic32"
#else
#define HERMES_MCU_FAMILY "generic"
#define HERMES_MCU_NAME "mcu"
#endif

#endif
