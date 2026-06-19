#ifndef HERMES_TEMPLATES_H
#define HERMES_TEMPLATES_H

#include "hermes_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *id;
    const char *name;
    const char *mcu;
    const char *description;
} hermes_template_info_t;

const hermes_template_info_t *hermes_template_list(size_t *count);
int hermes_template_apply(const char *template_id, hermes_board_gpio_t *out);
void hermes_template_apply_build_default(hermes_board_gpio_t *out);

#ifdef __cplusplus
}
#endif

#endif
