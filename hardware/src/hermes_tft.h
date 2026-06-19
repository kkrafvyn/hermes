#ifndef HERMES_TFT_H
#define HERMES_TFT_H

#ifdef __cplusplus
extern "C" {
#endif

void hermes_tft_begin(void);
void hermes_tft_show_status(const char *line1, const char *line2, const char *line3);

#ifdef __cplusplus
}
#endif

#endif
