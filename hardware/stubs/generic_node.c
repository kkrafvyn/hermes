/**
 * HERMES production stub node — portable C for any vendor SDK.
 * Protocol-compatible with hardware/src/main.cpp (boot, telemetry, set_config).
 *
 * Port: wire UART printf/puts, replace stub_millis with your tick source,
 *       call hermes_stub_poll_command() from your UART RX handler.
 */
#include "hermes_stub_sensor.h"
#include <stdio.h>
#include <string.h>

#ifndef HERMES_NODE_ID
#define HERMES_NODE_ID 0
#endif

#ifndef HERMES_STUB_MCU_NAME
#define HERMES_STUB_MCU_NAME "stub"
#endif

#ifndef HERMES_STUB_FAMILY
#define HERMES_STUB_FAMILY "generic"
#endif

static void emit_boot(void) {
    printf("{\"boot\":\"hermes-node\",\"mcu\":\"%s\",\"family\":\"%s\","
           "\"node_id\":%d,\"sensors\":true}\n",
           HERMES_STUB_MCU_NAME, HERMES_STUB_FAMILY, HERMES_NODE_ID);
}

static void emit_telemetry(void) {
    char sensor_part[512];
    sensor_part[0] = '\0';
    hermes_stub_sensor_poll();
    hermes_stub_sensor_json(sensor_part, sizeof(sensor_part));
    printf("{\"node_id\":%d,\"mcu\":\"%s\",\"family\":\"%s\","
           "\"telemetry\":{\"cpu\":15.0,\"ram_kb\":8,\"temp_c\":35.0,"
           "\"power_mw\":120.0,\"link_quality\":90%s}}\n",
           HERMES_NODE_ID, HERMES_STUB_MCU_NAME, HERMES_STUB_FAMILY, sensor_part);
}

void hermes_stub_handle_command(const char *json) {
    if (!json || json[0] != '{') return;
    if (strstr(json, "set_config") || strstr(json, "\"config\""))
        hermes_stub_parse_sensors_json(json);
}

void hermes_stub_run_forever(void) {
    emit_boot();
    for (;;)
        emit_telemetry();
}

int main(void) {
    hermes_stub_run_forever();
    return 0;
}
