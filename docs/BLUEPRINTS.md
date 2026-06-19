# HERMES App Blueprints

**26** no-code application templates for **App Studio** in the dashboard.  
Each blueprint defines form fields, default sensors, metrics, and runtime simulation handlers.

| Item | Path |
|------|------|
| Blueprint JSON | `blueprints/<id>.json` |
| Runtime handlers | `tools/control-plane/blueprint_handlers.py` |
| API list | `GET /api/blueprints` |
| API detail | `GET /api/blueprints/<id>` |
| Create app | `POST /api/apps/create` |

---

## Blueprint list

### Agriculture & environment

| ID | Name | Default sensors |
|----|------|-----------------|
| smartfarm | Smart Farm | soil_moisture, soil_ec, rain_gauge, temp_humidity |
| greenhouse | Greenhouse | temp_humidity, co2, light_par, lux |
| aquaponics | Aquaponics | ph, water_level, temp_humidity, flow_meter |
| irrigation-scheduler | Irrigation Scheduler | soil_moisture, rain_gauge, flow_meter |
| livestock-barn | Livestock Barn | temp_humidity, ammonia, co2, motion_pir |
| pest-trap | Pest Trap | motion_pir, temp_humidity |
| smart-gate | Smart Gate | flow_meter, water_level, ultrasonic |
| weather-station | Weather Station | temp_humidity, wind_speed, wind_direction, rain_gauge, light_par |

### Smart home & building

| ID | Name | Default sensors |
|----|------|-----------------|
| smarthome | Smart Home | temp_humidity, lux, motion_pir, co2 |
| smart-lighting | Smart Lighting | lux, motion_pir |
| air-quality | Air Quality | co2, voc, pm25, temp_humidity |
| leak-detector | Leak Detector | water_leak, flow_meter, water_level |
| pool-spa | Pool & Spa | ph, orp, temp_humidity, flow_meter |
| fire-smoke | Fire & Smoke | smoke, co_gas, gas_mq, temp_humidity |
| elder-care | Elder Care | motion_pir, temp_humidity, door_contact |

### Energy & mobility

| ID | Name | Default sensors |
|----|------|-----------------|
| energy-monitor | Energy Monitor | current_clamp, voltage, modbus_meter |
| solar-battery | Solar & Battery | voltage, current_clamp, light_par |
| ev-charger | EV Charger | current_clamp, voltage, temp_humidity |
| asset-tracker | Asset Tracker | ble_rssi, gps, accelerometer |
| parking-gate | Parking Gate | ultrasonic, motion_pir |

### Industrial & logistics

| ID | Name | Default sensors |
|----|------|-----------------|
| warehouse | Warehouse | temp_humidity, door_contact, motion_pir |
| cold-chain | Cold Chain | temp_humidity, ds18b20, shock_tilt |
| machine-monitor | Machine Monitor | vibration, accelerometer, current_clamp, temp_humidity |
| sensor-net | Sensor Net | (user-selected sensor_list) |
| telemetry | Telemetry | (cluster metrics — no fixed sensors) |
| security | Security | motion_pir, door_contact, smoke |

---

## Blueprint JSON schema

```json
{
  "id": "smartfarm",
  "name": "Smart Farm",
  "description": "Soil moisture monitoring and automated irrigation",
  "version": "1.0.0",
  "memory": "128KB",
  "priority": 5,
  "fields": [
    { "key": "zone_name", "label": "Zone name", "type": "text", "default": "Field 1", "required": true },
    { "key": "moisture_threshold", "label": "Moisture threshold (%)", "type": "number", "min": 0, "max": 100, "default": 30 },
    { "key": "auto_irrigate", "label": "Auto irrigate", "type": "toggle", "default": true },
    { "key": "sensors", "label": "Sensors", "type": "sensor_list", "default": ["soil_moisture", "soil_ec"] },
    { "key": "target_node", "label": "Target node", "type": "node", "default": 1 }
  ],
  "metrics": [
    { "key": "soil_moisture", "label": "Soil moisture", "unit": "%" },
    { "key": "pump_active", "label": "Pump active", "type": "bool" }
  ]
}
```

### Field types

| Type | UI control | Notes |
|------|------------|-------|
| `text` | Text input | |
| `number` | Number input | Optional `min`, `max` |
| `toggle` | Checkbox | Boolean config |
| `sensor_list` | Multi-select | Values from `GET /api/sensors` |
| `node` | Node picker | Cluster node ID (0–N) |

---

## Handler pairs

Each blueprint needs **INIT** and **TICK** handlers in `blueprint_handlers.py`:

```python
def _init_smartfarm(cfg: dict) -> dict:
    return {"soil_moisture": 30.0, "pump_active": False, "irrigation_cycles": 0}

def _tick_smartfarm(cfg: dict, rt: dict) -> dict:
    # Update runtime state from cfg + simulated sensor logic
    return {"soil_moisture": rt["soil_moisture"], "pump_active": rt["pump_active"]}

INIT["smartfarm"] = _init_smartfarm
TICK["smartfarm"] = _tick_smartfarm
```

- **INIT** — called when app starts; returns initial runtime dict
- **TICK** — called each control-plane tick; returns metrics for dashboard

`blueprint_engine.py` wires INIT/TICK into cluster simulation.

---

## Create app via API

```bash
curl -X POST http://localhost:8787/api/apps/create \
  -H "X-Hermes-Token: hermes-dev" -H "Content-Type: application/json" \
  -d '{
    "blueprint": "greenhouse",
    "name": "gh-west",
    "node_id": 1,
    "auto_start": true,
    "config": {
      "zone_name": "West Wing",
      "target_temp_c": 24,
      "sensors": ["temp_humidity", "co2", "light_par"]
    }
  }'
```

Config is validated against blueprint fields. Sensor IDs are validated against `sensors/catalog.json`.

---

## Custom blueprint guide

1. **Create JSON** — `blueprints/my-app.json` with `id`, `fields`, `metrics`
2. **Add handlers** — `_init_my_app` and `_tick_my_app` in `blueprint_handlers.py`
3. **Register** — add to `INIT` and `TICK` dicts (key = blueprint `id`)
4. **Test** — `python tools/test_blueprints.py`
5. **Use** — appears in App Studio after control plane restart

Field with `type: sensor_list` automatically pushes selected sensors to node `set_config`.

---

## Related

- [SENSORS.md](SENSORS.md) — 37-sensor catalog
- [API.md](API.md) — REST blueprint endpoints
- [QUICKSTART.md](QUICKSTART.md) — App Studio walkthrough
- [blueprints/README.md](../blueprints/README.md) — short index
