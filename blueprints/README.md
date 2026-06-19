# HERMES App Blueprints

**51** UI-configurable apps for App Studio (no code required).  
**37** sensors supported on all MCU targets — see [docs/SENSORS.md](../docs/SENSORS.md).

ESP8266 boards run **one app at a time** (testing); ESP32/STM32/nRF nodes can run multiple apps per cluster quotas.

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
| [hydroponics](hydroponics.json) | pH, EC, nutrient dosing |
| [compost-monitor](compost-monitor.json) | Compost pile temp and moisture |
| [vineyard](vineyard.json) | Vineyard soil, leaf wetness, frost |
| [bee-hive](bee-hive.json) | Hive weight, temp, swarm alerts |
| [poultry-coop](poultry-coop.json) | Coop climate and ammonia |
| [grain-silo](grain-silo.json) | Grain storage temp, humidity, tilt |
| [lawn-irrigation](lawn-irrigation.json) | Lawn zones with rain skip |

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
| [hvac-controller](hvac-controller.json) | Multi-zone HVAC |
| [window-blinds](window-blinds.json) | Lux/temp automated blinds |
| [garage-door](garage-door.json) | Door state, motion, auto-close |
| [water-meter](water-meter.json) | Domestic flow and leak detection |
| [gas-safety](gas-safety.json) | Gas/CO monitoring, shutoff |
| [wine-cellar](wine-cellar.json) | Cellar temp/humidity for wine |
| [mold-risk](mold-risk.json) | Indoor mold and dew-point risk |

## Energy & mobility

| Blueprint | Use case |
|-----------|----------|
| [energy-monitor](energy-monitor.json) | Power usage, peak alerts |
| [solar-battery](solar-battery.json) | Solar output, battery SOC |
| [ev-charger](ev-charger.json) | EV charging, load management |
| [asset-tracker](asset-tracker.json) | BLE/GPS asset tracking |
| [parking-gate](parking-gate.json) | Parking occupancy, barriers |
| [battery-room](battery-room.json) | Battery bank voltage and safety |
| [diesel-generator](diesel-generator.json) | Backup generator health |
| [street-lighting](street-lighting.json) | Dusk-to-dawn path lighting |

## Industrial & logistics

| Blueprint | Use case |
|-----------|----------|
| [warehouse](warehouse.json) | Cold storage, zones, doors |
| [cold-chain](cold-chain.json) | Temperature compliance |
| [machine-monitor](machine-monitor.json) | Vibration, predictive maintenance |
| [sensor-net](sensor-net.json) | Multi-sensor mesh |
| [telemetry](telemetry.json) | Cluster/network telemetry |
| [security](security.json) | Motion, doors, alarms |
| [conveyor-monitor](conveyor-monitor.json) | Belt conveyor motor health |
| [tank-level](tank-level.json) | Tank/reservoir level alerts |
| [pressure-vessel](pressure-vessel.json) | Pressure vessel monitoring |
| [freezer-monitor](freezer-monitor.json) | Walk-in freezer compliance |
| [brewery](brewery.json) | Fermentation temp, pH, CO₂ |
| [server-room](server-room.json) | Rack temp, humidity, cooling |
| [lift-elevator](lift-elevator.json) | Elevator vibration, door cycles |
| [rfid-gate](rfid-gate.json) | RFID gate throughput and access |

## Extend

1. Add `blueprints/<id>.json` with `sensor_list` field
2. Add handlers in `tools/control-plane/blueprint_handlers.py` (or `blueprint_handlers_ext.py`)
3. Add sensor to `sensors/catalog.json` + `hermes_sensor.cpp`
