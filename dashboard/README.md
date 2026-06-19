# HERMES Dashboard

Web control plane for the HERMES cluster.

## Features

- Node status (STM32, ESP32, nRF)
- **App Studio** — create apps from blueprints (Smart Farm, Smart Home, Sensor Net, Telemetry)
- **Configure** running apps via UI forms (no code edits)
- Application deploy / pause / migrate / stop
- Cluster metrics (load, power, temp)
- Live log stream

## Run locally

```bash
cd dashboard
python -m http.server 8080
```

Open http://localhost:8080

## Phase 8 integration (planned)

Connect `app.js` to a HERMES REST/WebSocket API served by the runtime on the ESP32 gateway node.
