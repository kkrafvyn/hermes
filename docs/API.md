# HERMES API Reference (v0.1 Alpha)

## Layer 1 — Hardware Cluster (`hardware/cluster/`)

| Function | Description |
|----------|-------------|
| `hermes_link_init()` | Initialize node link |
| `hermes_link_send()` | Send framed message |
| `hermes_link_recv()` | Receive framed message |
| `hermes_link_send_heartbeat()` | Send heartbeat |
| `hermes_link_ping()` | Measure round-trip latency |
| `hermes_power_init()` | Bind power monitor HAL |
| `hermes_power_sample()` | Read voltage/current/power |

## Layer 2 — Device OS (`device-os/`)

| Function | Description |
|----------|-------------|
| `hermes_create_task()` | Create RTOS task |
| `hermes_pause_task()` | Pause task |
| `hermes_resume_task()` | Resume task |
| `hermes_destroy_task()` | Destroy task |
| `hermes_malloc()` / `hermes_free()` | Managed heap |
| `hermes_ipc_send()` / `hermes_ipc_recv()` | Inter-process messaging |

## Layer 3 — Runtime (`runtime/`)

| Function | Description |
|----------|-------------|
| `hermes_runtime_install()` | Install app on node |
| `hermes_runtime_start()` | Start app |
| `hermes_runtime_pause()` | Pause app |
| `hermes_runtime_stop()` | Stop app |
| `hermes_runtime_move()` | Move app to another node |
| `hermes_scheduler_place()` | Distributed placement |
| `hermes_migration_step()` | State machine migration step |
| `hermes_checkpoint_create()` | Create checkpoint |
| `hermes_quota_set()` | Set resource quota |

## Layer 4 — Application Platform (`platform/`)

| Function | Description |
|----------|-------------|
| `hermes_app_install()` | Install from manifest |
| `hermes_app_start()` / `pause()` / `stop()` | Lifecycle |
| `hermes_app_update()` / `rollback()` | Version management |
| `hermes_app_parse_manifest()` | Parse manifest.json |

## Layer 5 — SDK (`sdk/`)

```c
HERMES_TASK(fn, name, stack, priority)
HERMES_STATE(type, var)
HERMES_MONITOR(app, metric, value)
```

```bash
hermes create <name>
hermes build <path>
hermes deploy <path> [-n node]
hermes migrate <app> <src> <dst>
```

See [SDK_GUIDE.md](SDK_GUIDE.md) for full CLI and packaging workflow.

## Layer 6 — Security (`security/`)

| Function | Description |
|----------|-------------|
| `hermes_identity_init()` | Node identity |
| `hermes_identity_authenticate()` | Challenge-response auth |
| `hermes_policy_check()` | Permission check |
| `hermes_verify_app()` | Signed app verification |

Flow: **Install → Verify → Run** — see [SECURITY.md](SECURITY.md).

---

## REST API (Control Plane)

Base URL: `http://localhost:8787`  
Auth: `X-Hermes-Token: hermes-dev` (or `Authorization: Bearer <token>`)

Public (no auth): `GET /api/status`, `POST /api/auth/login`

### Auth

```bash
curl -X POST http://localhost:8787/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin"}'
# → {"token":"<hmac-hex>"}
```

Set `HERMES_TOKEN` or `HERMES_SECRET` env vars for production-style tokens.

### Cluster status

```bash
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/status
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/nodes
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/apps
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/logs
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/analytics
```

### MCU & sensor catalogs

```bash
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/mcus
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/sensors

# Deploy approach per target (direct_pio | flash_alias | sdk_port | coprocessor)
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/deploy/approaches

# Step-by-step deploy plan for one MCU
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/mcus/esp32c6/deploy
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/mcus/pic16/deploy
```

`/api/mcus` returns all 60 targets with `approach`, `workaround`, `flash_command`, and `deploy_url`.

### Blueprints (App Studio)

```bash
# List all 26 blueprints
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/blueprints

# Blueprint detail (fields, metrics, defaults)
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/blueprints/smartfarm
```

### Node management

```bash
# Register node
curl -X POST http://localhost:8787/api/nodes/register \
  -H "X-Hermes-Token: hermes-dev" -H "Content-Type: application/json" \
  -d '{"node_id":1,"name":"ESP8266","node_type":"esp8266","agent":true}'

# Heartbeat + receive pending commands
curl -X POST http://localhost:8787/api/nodes/heartbeat \
  -H "X-Hermes-Token: hermes-dev" -H "Content-Type: application/json" \
  -d '{"node_id":1,"telemetry":{"cpu":28,"ram_kb":42,"temp_c":44,"power_mw":350}}'

# Ack command
curl -X POST http://localhost:8787/api/nodes/ack \
  -H "X-Hermes-Token: hermes-dev" -H "Content-Type: application/json" \
  -d '{"node_id":1,"cmd_id":5}'

# Pending commands for node
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/nodes/1/commands
```

### Application lifecycle

```bash
# Create from App Studio blueprint
curl -X POST http://localhost:8787/api/apps/create \
  -H "X-Hermes-Token: hermes-dev" -H "Content-Type: application/json" \
  -d '{
    "blueprint":"smartfarm",
    "name":"field-1",
    "node_id":1,
    "auto_start":true,
    "config":{"zone_name":"North","moisture_threshold":30,"sensors":["soil_moisture","temp_humidity"]}
  }'

# Install from manifest path
curl -X POST http://localhost:8787/api/apps/install \
  -H "X-Hermes-Token: hermes-dev" -H "Content-Type: application/json" \
  -d '{"manifest_path":"examples/smartfarm/manifest.json"}'

curl -X POST http://localhost:8787/api/apps/field-1/start  -H "X-Hermes-Token: hermes-dev"
curl -X POST http://localhost:8787/api/apps/field-1/pause  -H "X-Hermes-Token: hermes-dev"
curl -X POST http://localhost:8787/api/apps/field-1/stop   -H "X-Hermes-Token: hermes-dev"
curl -X POST http://localhost:8787/api/apps/field-1/migrate -H "X-Hermes-Token: hermes-dev" \
  -H "Content-Type: application/json" -d '{"dst_node":2}'

# Live config update (pushes set_config to node)
curl -X POST http://localhost:8787/api/apps/field-1/config \
  -H "X-Hermes-Token: hermes-dev" -H "Content-Type: application/json" \
  -d '{"config":{"moisture_threshold":25,"sensors":["soil_moisture","rain_gauge"]}}'

curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/apps/field-1/config
```

### Cluster link hub

```bash
curl -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/link/status

curl -X POST http://localhost:8787/api/link/send \
  -H "X-Hermes-Token: hermes-dev" -H "Content-Type: application/json" \
  -d '{"frame":{"src":0,"dst":1,"type":2,"seq":42,"payload":"ping"}}'
```

---

## WebSocket

Real-time dashboard updates:

```
ws://localhost:8787/ws
```

Connect from browser or `websocat`:

```bash
websocat ws://localhost:8787/ws
```

The server pushes cluster state changes; dashboard `app.js` subscribes on load.

---

## Server-Sent Events (SSE)

Stream live events (logs, app state changes):

```bash
curl -N -H "X-Hermes-Token: hermes-dev" http://localhost:8787/api/events
```

Response is `text/event-stream`. Useful for headless monitoring without WebSocket.

---

## Firmware serial protocol (nodes)

Nodes emit newline-delimited JSON on UART:

```json
{"boot":"hermes-node","mcu":"esp8266","family":"espressif8266","node_id":1,"sensors":true}
{"node_id":1,"telemetry":{"cpu":15.0,"ram_kb":48,"sensors":[{"id":"soil_moisture","value":38.5,"unit":"%"}]}}
```

Inbound commands (from serial-bridge or control plane):

```json
{"cmd":"set_config","config":{"sensors":["soil_moisture","temp_humidity"]}}
```

---

## Related docs

| Doc | Content |
|-----|---------|
| [BLUEPRINTS.md](BLUEPRINTS.md) | 26 App Studio templates |
| [SENSORS.md](SENSORS.md) | 37-sensor catalog |
| [MCU_SUPPORT.md](MCU_SUPPORT.md) | 60 MCU matrix |
| [SECURITY.md](SECURITY.md) | Auth, HMAC, identity |
