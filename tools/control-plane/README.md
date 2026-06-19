# HERMES Control Plane

Python API server that unifies the cluster for development and dashboard use.

## Start

```bash
python tools/control-plane/server.py
# or
python run.py
```

- Dashboard: http://localhost:8787/
- API status: http://localhost:8787/api/status

## API Endpoints

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/status` | Full cluster state |
| GET | `/api/nodes` | Node list |
| GET | `/api/apps` | Application list |
| GET | `/api/logs` | Recent logs |
| POST | `/api/apps/install` | `{"manifest_path": "examples/smartfarm/manifest.json"}` |
| POST | `/api/apps/{name}/start` | Start app |
| POST | `/api/apps/{name}/pause` | Pause/resume app |
| POST | `/api/apps/{name}/stop` | Stop app |
| POST | `/api/nodes/register` | `{"node_id": 0, "name": "STM32H7", "node_type": "stm32", "agent": true}` |
| POST | `/api/nodes/heartbeat` | `{"node_id": 0, "telemetry": {"cpu": 40, "ram_kb": 256}}` |
| POST | `/api/nodes/ack` | `{"node_id": 0, "cmd_id": 1}` |
| GET | `/api/link/status` | Inter-node frame routing stats |
| POST | `/api/link/send` | `{"frame": {"src":0,"dst":1,"type":2,"seq":1}}` |

## Features

- Node registration and health checks
- Leader selection (lowest CPU)
- Work redistribution on node failure
- Live migration state machine
- Resource quotas per app
