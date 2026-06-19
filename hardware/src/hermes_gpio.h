#ifndef HERMES_GPIO_H
#define HERMES_GPIO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HERMES_GPIO_MAX 32
#define HERMES_GPIO_UNUSED 255

/** Pin roles — assign via HTTP /board (Tasmota-style templates) */
#define HERMES_FN_NONE        0
#define HERMES_FN_RELAY       1
#define HERMES_FN_BUTTON      2
#define HERMES_FN_INPUT       3
#define HERMES_FN_OUTPUT      4
#define HERMES_FN_ADC         5
#define HERMES_FN_PWM         6
#define HERMES_FN_I2C_SDA     7
#define HERMES_FN_I2C_SCL     8
#define HERMES_FN_UART_TX     9
#define HERMES_FN_UART_RX    10
#define HERMES_FN_LED        11
#define HERMES_FN_TFT_BL     12
#define HERMES_FN_TFT_DC     13
#define HERMES_FN_TFT_RST    14
#define HERMES_FN_TFT_CS     15
#define HERMES_FN_TFT_MOSI   16
#define HERMES_FN_TFT_SCLK   17
#define HERMES_FN_MODEM_PWR  18
#define HERMES_FN_MODEM_RST  19
#define HERMES_FN_MODEM_TX   20
#define HERMES_FN_MODEM_RX   21
#define HERMES_FN_CAM_PWDN   22
#define HERMES_FN_CAM_RST    23
#define HERMES_FN_SD_CLK     24
#define HERMES_FN_SD_CMD     25
#define HERMES_FN_SD_DATA    26
#define HERMES_FN_SD_CD      27
#define HERMES_FN_USER       99

#define HERMES_GPIO_FLAG_INVERT 0x01
#define HERMES_GPIO_FLAG_PULLUP 0x02

typedef struct {
    uint8_t gpio;
    uint8_t fn;
    uint8_t flags;
    uint8_t idx;
} hermes_gpio_entry_t;

typedef struct {
    char template_id[24];
    char board_label[32];
    uint8_t count;
    uint8_t reserved;
    hermes_gpio_entry_t entries[HERMES_GPIO_MAX];
} hermes_board_gpio_t;

const char *hermes_gpio_fn_name(uint8_t fn);
void hermes_gpio_defaults(hermes_board_gpio_t *out);
int hermes_gpio_apply(const hermes_board_gpio_t *cfg);
void hermes_gpio_loop(void);
int hermes_gpio_find(uint8_t fn, uint8_t idx);
int hermes_gpio_set_relay(uint8_t idx, bool on);
void hermes_gpio_states_json(char *buf, size_t max);
int hermes_gpio_import_tasmota(const char *json, hermes_board_gpio_t *out);
void hermes_gpio_to_json(const hermes_board_gpio_t *cfg, char *buf, size_t max);
int hermes_gpio_from_json(const char *json, hermes_board_gpio_t *out);

#ifdef __cplusplus
}
#endif

#endif
