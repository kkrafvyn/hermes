"""Blueprint simulation handlers for all App Studio apps."""

from __future__ import annotations

import random
from typing import Any, Callable

InitFn = Callable[[dict], dict]
TickFn = Callable[[dict, dict], dict[str, Any]]


def _r(a: float, b: float, d: int = 1) -> float:
    return round(random.uniform(a, b), d)


# --- init handlers ---

def _init_smartfarm(cfg: dict) -> dict:
    return {"soil_moisture": _r(20, 45), "pump_active": False, "irrigation_cycles": 0}


def _init_smarthome(cfg: dict) -> dict:
    return {"room_temp_c": _r(18, 26), "lights_active": bool(cfg.get("lights_on")), "hvac_on": False}


def _init_sensor_net(cfg: dict) -> dict:
    return {"active_sensors": int(cfg.get("sensor_count", 4)), "last_reading": _r(0, 1023, 0), "packets_sent": 0}


def _init_telemetry(cfg: dict) -> dict:
    return {"bandwidth_kbps": _r(10, 100), "packets_sent": 0, "load_percent": _r(5, 40)}


def _init_greenhouse(cfg: dict) -> dict:
    return {"air_temp_c": _r(20, 28), "humidity_pct": _r(50, 80), "vents_open": False}


def _init_security(cfg: dict) -> dict:
    return {"motion_events": 0, "doors_open": 0, "alarm_active": bool(cfg.get("alarm_enabled", True))}


def _init_energy(cfg: dict) -> dict:
    return {"power_kw": _r(1, 4, 2), "daily_kwh": _r(10, 50), "alert_active": False}


def _init_aquaponics(cfg: dict) -> dict:
    return {"ph_level": _r(6.4, 7.6, 2), "water_temp_c": _r(20, 26), "feeder_active": False}


def _init_weather(cfg: dict) -> dict:
    return {"outdoor_temp_c": _r(5, 32), "wind_kmh": _r(0, 40), "rain_mm_hr": _r(0, 5)}


def _init_warehouse(cfg: dict) -> dict:
    return {"zone_temp_c": _r(2, 8), "doors_open": 0, "cold_ok": True}


def _init_lighting(cfg: dict) -> dict:
    b = int(cfg.get("default_brightness", 75))
    return {"brightness_pct": b, "fixtures_on": 0, "schedule_active": bool(cfg.get("schedule_on", True))}


def _init_cold_chain(cfg: dict) -> dict:
    return {"current_temp_c": _r(3, 7), "in_range": True, "breach_count": 0}


def _init_livestock(cfg: dict) -> dict:
    return {"barn_temp_c": _r(18, 30), "ammonia_ppm": _r(5, 20), "vents_on": False}


def _init_irrigation_sched(cfg: dict) -> dict:
    return {"zones_active": 0, "soil_moisture": _r(25, 55), "rain_skip_active": False}


def _init_leak(cfg: dict) -> dict:
    return {"leak_detected": False, "valve_closed": False, "active_sensors": int(cfg.get("sensor_count", 2))}


def _init_air_quality(cfg: dict) -> dict:
    return {"co2_ppm": _r(400, 900, 0), "pm25_ug": _r(5, 30), "vent_on": False}


def _init_pool(cfg: dict) -> dict:
    return {"ph_level": _r(7.0, 7.6, 2), "water_temp_c": _r(24, 30), "pump_on": False}


def _init_solar(cfg: dict) -> dict:
    return {"solar_kw": _r(0.5, 4, 2), "battery_pct": _r(40, 90, 0), "grid_import_kw": _r(0, 2, 2)}


def _init_ev(cfg: dict) -> dict:
    return {"charge_current_a": 0.0, "session_kwh": 0.0, "charging": False}


def _init_asset(cfg: dict) -> dict:
    return {"tags_in_range": int(cfg.get("tag_count", 8)), "last_rssi": _r(-80, -40, 0), "geofence_ok": True}


def _init_machine(cfg: dict) -> dict:
    return {"vibration_g": _r(0.1, 0.8, 2), "motor_temp_c": _r(35, 55), "service_due": False}


def _init_pest(cfg: dict) -> dict:
    return {"catches_today": 0, "traps_active": int(cfg.get("trap_count", 12)), "bait_low": False}


def _init_smart_gate(cfg: dict) -> dict:
    return {"flow_lpm": _r(10, 60), "gate_open_pct": 50, "level_cm": _r(80, 120, 0)}


def _init_elder(cfg: dict) -> dict:
    return {"room_temp_c": _r(19, 24), "motion_today": 0, "alert_active": False}


def _init_fire(cfg: dict) -> dict:
    return {"smoke_level": _r(0, 5), "co_ppm": _r(0, 5), "alarm_active": False}


def _init_parking(cfg: dict) -> dict:
    cap = int(cfg.get("capacity", 50))
    occ = random.randint(0, cap)
    return {"occupancy": occ, "barrier_open": False, "spaces_free": cap - occ}


INIT: dict[str, InitFn] = {
    "smartfarm": _init_smartfarm, "smarthome": _init_smarthome, "sensor-net": _init_sensor_net,
    "telemetry": _init_telemetry, "greenhouse": _init_greenhouse, "security": _init_security,
    "energy-monitor": _init_energy, "aquaponics": _init_aquaponics, "weather-station": _init_weather,
    "warehouse": _init_warehouse, "smart-lighting": _init_lighting, "cold-chain": _init_cold_chain,
    "livestock-barn": _init_livestock, "irrigation-scheduler": _init_irrigation_sched,
    "leak-detector": _init_leak, "air-quality": _init_air_quality, "pool-spa": _init_pool,
    "solar-battery": _init_solar, "ev-charger": _init_ev, "asset-tracker": _init_asset,
    "machine-monitor": _init_machine, "pest-trap": _init_pest, "smart-gate": _init_smart_gate,
    "elder-care": _init_elder, "fire-smoke": _init_fire, "parking-gate": _init_parking,
}


# --- tick handlers (abbreviated patterns) ---

def _tick_smartfarm(cfg: dict, rt: dict) -> dict:
    m = max(5, min(95, rt.get("soil_moisture", 30) + random.uniform(-2, 1.5)))
    rt["soil_moisture"] = round(m, 1)
    th = float(cfg.get("moisture_threshold", 30))
    pump = rt.get("pump_active", False)
    if cfg.get("auto_irrigate", True) and m < th and not pump:
        pump = True
        rt["irrigation_cycles"] = rt.get("irrigation_cycles", 0) + 1
    elif pump and m > th + 5:
        pump = False
    rt["pump_active"] = pump
    return {"soil_moisture": rt["soil_moisture"], "pump_active": pump, "irrigation_cycles": rt.get("irrigation_cycles", 0)}


def _tick_smarthome(cfg: dict, rt: dict) -> dict:
    t = round(max(10, min(35, rt.get("room_temp_c", 22) + random.uniform(-0.5, 0.5))), 1)
    rt["room_temp_c"] = t
    target = float(cfg.get("target_temp_c", 22))
    hvac = bool(cfg.get("auto_climate", True)) and abs(t - target) > 0.8
    rt["hvac_on"] = hvac
    lights = bool(cfg.get("lights_on", False))
    rt["lights_active"] = lights
    return {"room_temp_c": t, "lights_active": lights, "hvac_on": hvac}


def _tick_sensor_net(cfg: dict, rt: dict) -> dict:
    c = int(cfg.get("sensor_count", 4))
    rt["active_sensors"] = c
    rt["last_reading"] = _r(0, 1023, 0)
    rt["packets_sent"] = rt.get("packets_sent", 0) + c
    return {"active_sensors": c, "last_reading": rt["last_reading"], "packets_sent": rt["packets_sent"]}


def _tick_telemetry(cfg: dict, rt: dict) -> dict:
    rate = float(cfg.get("sample_rate_hz", 10))
    rt["packets_sent"] = rt.get("packets_sent", 0) + int(rate)
    rt["bandwidth_kbps"] = _r(20, 120)
    rt["load_percent"] = _r(10, 70)
    return {"bandwidth_kbps": rt["bandwidth_kbps"], "packets_sent": rt["packets_sent"], "load_percent": rt["load_percent"]}


def _tick_greenhouse(cfg: dict, rt: dict) -> dict:
    t = round(max(10, min(40, rt.get("air_temp_c", 24) + random.uniform(-0.6, 0.6))), 1)
    h = round(max(30, min(95, rt.get("humidity_pct", 65) + random.uniform(-3, 3))), 1)
    rt["air_temp_c"], rt["humidity_pct"] = t, h
    vents = bool(cfg.get("auto_vent", True)) and (h > float(cfg.get("target_humidity", 65)) + 5 or t > float(cfg.get("target_temp_c", 24)) + 2)
    rt["vents_open"] = vents
    return {"air_temp_c": t, "humidity_pct": h, "vents_open": vents}


def _tick_security(cfg: dict, rt: dict) -> dict:
    if random.random() < float(cfg.get("motion_sensitivity", 70)) / 100 * 0.15:
        rt["motion_events"] = rt.get("motion_events", 0) + 1
    if random.random() < 0.05:
        rt["doors_open"] = random.randint(0, int(cfg.get("entry_points", 4)))
    armed = bool(cfg.get("alarm_enabled", True))
    alarm = armed and (rt.get("motion_events", 0) > 0 or rt.get("doors_open", 0) > 0)
    if cfg.get("night_mode"):
        alarm = alarm or random.random() < 0.08
    rt["alarm_active"] = alarm
    return {"motion_events": rt.get("motion_events", 0), "doors_open": rt.get("doors_open", 0), "alarm_active": alarm}


def _tick_energy(cfg: dict, rt: dict) -> dict:
    p = round(max(0.1, rt.get("power_kw", 2) + random.uniform(-0.3, 0.4)), 2)
    rt["power_kw"] = p
    rt["daily_kwh"] = round(rt.get("daily_kwh", 20) + p * 0.001, 2)
    alert = p > float(cfg.get("power_alert_kw", 5))
    if alert and cfg.get("peak_shaving", True):
        p = round(float(cfg.get("power_alert_kw", 5)) * 0.95, 2)
        rt["power_kw"] = p
    rt["alert_active"] = alert
    return {"power_kw": rt["power_kw"], "daily_kwh": rt["daily_kwh"], "alert_active": alert}


def _tick_aquaponics(cfg: dict, rt: dict) -> dict:
    ph = round(max(4, min(9, rt.get("ph_level", 7) + random.uniform(-0.08, 0.08))), 2)
    rt["ph_level"] = ph
    rt["water_temp_c"] = round(max(15, min(30, rt.get("water_temp_c", 23) + random.uniform(-0.3, 0.3))), 1)
    feeder = bool(cfg.get("auto_feed", True)) and (ph < float(cfg.get("ph_min", 6.5)) or ph > float(cfg.get("ph_max", 7.5)) or random.random() < 0.05)
    rt["feeder_active"] = feeder
    return {"ph_level": ph, "water_temp_c": rt["water_temp_c"], "feeder_active": feeder}


def _tick_weather(cfg: dict, rt: dict) -> dict:
    rt["outdoor_temp_c"] = round(max(-10, min(45, rt.get("outdoor_temp_c", 20) + random.uniform(-0.8, 0.8))), 1)
    rt["wind_kmh"] = round(max(0, rt.get("wind_kmh", 10) + random.uniform(-5, 8)), 1)
    rt["rain_mm_hr"] = round(max(0, random.uniform(0, 12)), 1)
    return {"outdoor_temp_c": rt["outdoor_temp_c"], "wind_kmh": rt["wind_kmh"], "rain_mm_hr": rt["rain_mm_hr"]}


def _tick_warehouse(cfg: dict, rt: dict) -> dict:
    target = float(cfg.get("cold_storage_temp_c", 4))
    temp = round(max(-5, min(15, target + random.uniform(-1.5, 1.5))), 1)
    rt["zone_temp_c"] = temp
    rt["cold_ok"] = abs(temp - target) <= 3
    if random.random() < 0.06:
        rt["doors_open"] = random.randint(0, 2)
    elif rt.get("doors_open", 0) > 0 and random.random() < 0.3:
        rt["doors_open"] = 0
    return {"zone_temp_c": temp, "doors_open": rt.get("doors_open", 0), "cold_ok": rt["cold_ok"]}


def _tick_lighting(cfg: dict, rt: dict) -> dict:
    default = int(cfg.get("default_brightness", 75))
    count = int(cfg.get("lights_count", 8))
    schedule = bool(cfg.get("schedule_on", True))
    on = schedule or bool(cfg.get("sunset_auto", True))
    fixtures = count if on and random.random() > 0.2 else random.randint(0, max(1, count // 2))
    rt["fixtures_on"] = fixtures
    rt["brightness_pct"] = default if fixtures > 0 else 0
    rt["schedule_active"] = schedule
    return {"brightness_pct": rt["brightness_pct"], "fixtures_on": fixtures, "schedule_active": schedule}


def _tick_cold_chain(cfg: dict, rt: dict) -> dict:
    temp = round(max(-30, min(20, rt.get("current_temp_c", 5) + random.uniform(-0.5, 0.5))), 1)
    rt["current_temp_c"] = temp
    ok = float(cfg.get("min_temp_c", 2)) <= temp <= float(cfg.get("max_temp_c", 8))
    rt["in_range"] = ok
    if not ok and cfg.get("auto_notify", True):
        rt["breach_count"] = rt.get("breach_count", 0) + 1
    return {"current_temp_c": temp, "in_range": ok, "breach_count": rt.get("breach_count", 0)}


def _tick_livestock(cfg: dict, rt: dict) -> dict:
    t = round(max(10, min(40, rt.get("barn_temp_c", 24) + random.uniform(-0.5, 0.5))), 1)
    nh3 = round(max(0, rt.get("ammonia_ppm", 10) + random.uniform(-2, 3)), 1)
    rt["barn_temp_c"], rt["ammonia_ppm"] = t, nh3
    vents = bool(cfg.get("auto_vent", True)) and (t > float(cfg.get("max_temp_c", 28)) or nh3 > float(cfg.get("ammonia_alert_ppm", 25)))
    rt["vents_on"] = vents
    return {"barn_temp_c": t, "ammonia_ppm": nh3, "vents_on": vents}


def _tick_irrigation_sched(cfg: dict, rt: dict) -> dict:
    m = round(max(5, min(95, rt.get("soil_moisture", 35) + random.uniform(-2, 1))), 1)
    rt["soil_moisture"] = m
    rain = random.random() < 0.1
    skip = bool(cfg.get("rain_skip", True)) and rain
    rt["rain_skip_active"] = skip
    zones = 0 if skip else random.randint(0, int(cfg.get("zones", 4)))
    rt["zones_active"] = zones
    return {"zones_active": zones, "soil_moisture": m, "rain_skip_active": skip}


def _tick_leak(cfg: dict, rt: dict) -> dict:
    leak = random.random() < 0.04
    rt["leak_detected"] = leak
    if leak and cfg.get("auto_shutoff", True):
        rt["valve_closed"] = True
    rt["active_sensors"] = int(cfg.get("sensor_count", 2))
    return {"leak_detected": leak, "valve_closed": rt.get("valve_closed", False), "active_sensors": rt["active_sensors"]}


def _tick_air_quality(cfg: dict, rt: dict) -> dict:
    co2 = int(max(400, rt.get("co2_ppm", 600) + random.uniform(-30, 40)))
    pm = round(max(0, rt.get("pm25_ug", 15) + random.uniform(-3, 5)), 1)
    rt["co2_ppm"], rt["pm25_ug"] = co2, pm
    vent = bool(cfg.get("auto_vent", True)) and (co2 > int(cfg.get("co2_alert_ppm", 1200)) or pm > float(cfg.get("pm25_alert", 35)))
    rt["vent_on"] = vent
    return {"co2_ppm": co2, "pm25_ug": pm, "vent_on": vent}


def _tick_pool(cfg: dict, rt: dict) -> dict:
    ph = round(max(6, min(8.5, rt.get("ph_level", 7.4) + random.uniform(-0.05, 0.05))), 2)
    wt = round(max(20, min(40, rt.get("water_temp_c", 28) + random.uniform(-0.3, 0.3))), 1)
    rt["ph_level"], rt["water_temp_c"] = ph, wt
    pump = abs(ph - float(cfg.get("ph_target", 7.4))) > 0.2 or abs(wt - float(cfg.get("temp_target_c", 28))) > 1
    rt["pump_on"] = pump
    return {"ph_level": ph, "water_temp_c": wt, "pump_on": pump}


def _tick_solar(cfg: dict, rt: dict) -> dict:
    solar = round(max(0, rt.get("solar_kw", 2) + random.uniform(-0.5, 0.8)), 2)
    bat = int(max(0, min(100, rt.get("battery_pct", 60) + random.uniform(-1, 2))))
    grid = round(max(0, random.uniform(0, 3)), 2) if solar < 0.5 else 0.0
    rt.update({"solar_kw": solar, "battery_pct": bat, "grid_import_kw": grid})
    return {"solar_kw": solar, "battery_pct": bat, "grid_import_kw": grid}


def _tick_ev(cfg: dict, rt: dict) -> dict:
    charging = random.random() > 0.4
    cur = round(random.uniform(0, float(cfg.get("max_current_a", 32))), 1) if charging else 0.0
    rt["charging"] = charging
    rt["charge_current_a"] = cur
    if charging:
        rt["session_kwh"] = round(rt.get("session_kwh", 0) + cur * 0.001, 2)
    return {"charge_current_a": cur, "session_kwh": rt.get("session_kwh", 0), "charging": charging}


def _tick_asset(cfg: dict, rt: dict) -> dict:
    tags = int(cfg.get("tag_count", 8))
    in_range = random.randint(max(0, tags - 2), tags)
    rssi = _r(-85, -35, 0)
    rt.update({"tags_in_range": in_range, "last_rssi": rssi, "geofence_ok": random.random() > 0.05})
    return {"tags_in_range": in_range, "last_rssi": rssi, "geofence_ok": rt["geofence_ok"]}


def _tick_machine(cfg: dict, rt: dict) -> dict:
    vib = round(max(0, rt.get("vibration_g", 0.5) + random.uniform(-0.2, 0.3)), 2)
    temp = round(max(20, min(90, rt.get("motor_temp_c", 45) + random.uniform(-1, 2))), 1)
    rt["vibration_g"], rt["motor_temp_c"] = vib, temp
    rt["service_due"] = vib > float(cfg.get("vibration_alert_g", 2))
    return {"vibration_g": vib, "motor_temp_c": temp, "service_due": rt["service_due"]}


def _tick_pest(cfg: dict, rt: dict) -> dict:
    if random.random() < 0.12:
        rt["catches_today"] = rt.get("catches_today", 0) + 1
    rt["traps_active"] = int(cfg.get("trap_count", 12))
    rt["bait_low"] = random.random() < 0.03
    return {"catches_today": rt.get("catches_today", 0), "traps_active": rt["traps_active"], "bait_low": rt["bait_low"]}


def _tick_smart_gate(cfg: dict, rt: dict) -> dict:
    flow = round(max(0, rt.get("flow_lpm", 40) + random.uniform(-8, 8)), 1)
    level = round(max(0, rt.get("level_cm", 100) + random.uniform(-5, 5)), 0)
    target = float(cfg.get("target_flow_lpm", 50))
    open_pct = int(min(100, max(0, (flow / max(1, target)) * 100)))
    rt.update({"flow_lpm": flow, "level_cm": level, "gate_open_pct": open_pct})
    return {"flow_lpm": flow, "gate_open_pct": open_pct, "level_cm": level}


def _tick_elder(cfg: dict, rt: dict) -> dict:
    t = round(max(15, min(30, rt.get("room_temp_c", 21) + random.uniform(-0.4, 0.4))), 1)
    rt["room_temp_c"] = t
    if random.random() < 0.1:
        rt["motion_today"] = rt.get("motion_today", 0) + 1
    alert = t < float(cfg.get("temp_min_c", 18)) or (cfg.get("night_check") and random.random() < 0.02)
    rt["alert_active"] = alert
    return {"room_temp_c": t, "motion_today": rt.get("motion_today", 0), "alert_active": alert}


def _tick_fire(cfg: dict, rt: dict) -> dict:
    smoke = round(max(0, rt.get("smoke_level", 0) + random.uniform(-0.5, 1)), 1)
    co = round(max(0, rt.get("co_ppm", 0) + random.uniform(0, 2)), 1)
    rt["smoke_level"], rt["co_ppm"] = smoke, co
    alarm = smoke > 8 or co > 30
    rt["alarm_active"] = alarm
    return {"smoke_level": smoke, "co_ppm": co, "alarm_active": alarm}


def _tick_parking(cfg: dict, rt: dict) -> dict:
    cap = int(cfg.get("capacity", 50))
    occ = int(max(0, min(cap, rt.get("occupancy", 20) + random.randint(-1, 2))))
    rt["occupancy"] = occ
    rt["spaces_free"] = cap - occ
    rt["barrier_open"] = bool(cfg.get("barrier_auto", True)) and random.random() > 0.7
    return {"occupancy": occ, "barrier_open": rt["barrier_open"], "spaces_free": rt["spaces_free"]}


TICK: dict[str, TickFn] = {
    "smartfarm": _tick_smartfarm, "smarthome": _tick_smarthome, "sensor-net": _tick_sensor_net,
    "telemetry": _tick_telemetry, "greenhouse": _tick_greenhouse, "security": _tick_security,
    "energy-monitor": _tick_energy, "aquaponics": _tick_aquaponics, "weather-station": _tick_weather,
    "warehouse": _tick_warehouse, "smart-lighting": _tick_lighting, "cold-chain": _tick_cold_chain,
    "livestock-barn": _tick_livestock, "irrigation-scheduler": _tick_irrigation_sched,
    "leak-detector": _tick_leak, "air-quality": _tick_air_quality, "pool-spa": _tick_pool,
    "solar-battery": _tick_solar, "ev-charger": _tick_ev, "asset-tracker": _tick_asset,
    "machine-monitor": _tick_machine, "pest-trap": _tick_pest, "smart-gate": _tick_smart_gate,
    "elder-care": _tick_elder, "fire-smoke": _tick_fire, "parking-gate": _tick_parking,
}


def _register_extended_handlers() -> None:
    import blueprint_handlers_ext as ext

    for bid in ext.NEW_APPS:
        fn = bid.replace("-", "_")
        init_fn = getattr(ext, f"_init_{fn}", None)
        tick_fn = getattr(ext, f"_tick_{fn}", None)
        if init_fn:
            INIT[bid] = init_fn
        if tick_fn:
            TICK[bid] = tick_fn


_register_extended_handlers()
