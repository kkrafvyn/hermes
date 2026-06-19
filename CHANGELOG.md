# Changelog

## v0.1.0 — 2026-06-16

### Platform
- Complete 6-layer architecture: hardware cluster, device OS, runtime, platform, SDK, dashboard
- Python control plane with REST API, SSE live updates, persistent state
- Node agents with command execution and cluster link hub
- Integration tests (11/11), cluster link tests, CI workflow

### Hardware
- Inter-node protocol (CRC32 frames, ping/pong, reliable send)
- ESP32-S3 ESP-IDF gateway (WiFi HTTP + UART serial bridge)
- STM32 SPI HAL reference, nRF UART HAL, BLE transport layer
- Gateway router (SPI ↔ UART forwarding)
- Serial bridge tool for USB development

### Runtime
- Application lifecycle, quotas, distributed scheduler
- State migration state machine with persistent checkpoints
- OTA update module with SHA256 verification
- FreeRTOS task binding layer

### Security
- SHA256 implementation
- App signature verification on deploy
- Policy engine with permissions

### SDK
- CLI: create, build, package, deploy, migrate
- Cross-target build script (host, stm32, esp32)
- App packaging with SHA256 manifest signing
- C macros: HERMES_TASK, HERMES_STATE, HERMES_MONITOR

### Examples
- smartfarm, telemetry, sensor-net, testapp

### Documentation
- Full project documentation, hardware bring-up guide, API reference

### Fixes (v0.1.0 final)
- Control plane persistence deadlock fixed
- SDK pack build (`HERMES_PACK_BUILD`) compiles apps without full runtime linkage
- CI updated for all runtime/security modules + `scripts/validate.py`
