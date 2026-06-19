#ifndef HERMES_CONSOLE_H
#define HERMES_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

void hermes_console_handle_line(const char *line);
void hermes_console_poll(void);

#ifdef __cplusplus
}
#endif

#endif
