#ifndef HERMES_PRINT_H
#define HERMES_PRINT_H

#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>

inline void hermes_serial_printf(const char *fmt, ...) {
    char buf[320];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    Serial.print(buf);
}
#define HERMES_PRINTF(...) hermes_serial_printf(__VA_ARGS__)

#endif
