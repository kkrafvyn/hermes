# HERMES App Blueprints

**26** UI-configurable apps for App Studio (no code required).  
**37** sensors supported on all MCU targets — see [docs/SENSORS.md](../docs/SENSORS.md).

## Agriculture & environment

| Blueprint | Use case |
|-----------|----------|
| [smartfarm](smartfarm.json) | Soil moisture, irrigation |
| [greenhouse](greenhouse.json) | Humidity, ventilation, climate |
| [aquaponics](aquaponics.json) | pH, water temp, auto feeding |
| [irrigation-scheduler](irrigation-scheduler.json) | Multi-zone schedules, rain skip |
| [livestock-barn](livestock-barn.json) | Barn climate, ammonia, ventilation |
| [pest-trap](pest-trap.json) | Field trap monitoring |
| [smart-gate](smart-gate.json) | Canal/gate flow control |
| [weather-station](weather-station.json) | Outdoor temp, wind, rain |

## Smart home & building

| Blueprint | Use case |
|-----------|----------|
| [smarthome](smarthome.json) | Climate, lighting, HVAC |
| [smart-lighting](smart-lighting.json) | Brightness, schedules, sunset |
| [air-quality](air-quality.json) | CO₂, VOC, PM2.5 |
| [leak-detector](leak-detector.json) | Water leak + auto shutoff |
| [pool-spa](pool-spa.json) | pH, ORP, pump control |
| [fire-smoke](fire-smoke.json) | Smoke, CO, gas alarms |
| [elder-care](elder-care.json) | Room comfort, motion alerts |

## Energy & mobility

| Blueprint | Use case |
|-----------|----------|
| [energy-monitor](energy-monitor.json) | Power usage, peak alerts |
| [solar-battery](solar-battery.json) | Solar output, battery SOC |
| [ev-charger](ev-charger.json) | EV charging, load management |
| [asset-tracker](asset-tracker.json) | BLE/GPS asset tracking |
| [parking-gate](parking-gate.json) | Parking occupancy, barriers |

## Industrial & logistics

| Blueprint | Use case |
|-----------|----------|
| [warehouse](warehouse.json) | Cold storage, zones, doors |
| [cold-chain](cold-chain.json) | Temperature compliance |
| [machine-monitor](machine-monitor.json) | Vibration, predictive maintenance |
| [sensor-net](sensor-net.json) | Multi-sensor mesh |
| [telemetry](telemetry.json) | Cluster/network telemetry |
| [security](security.json) | Motion, doors, alarms |

## Extend

1. Add `blueprints/<id>.json` with `sensor_list` field
2. Add handlers in `tools/control-plane/blueprint_handlers.py`
3. Add sensor to `sensors/catalog.json` + `hermes_sensor.cpp`
