"""Extended blueprint handlers — auto-generated."""
from __future__ import annotations

import random

NEW_APPS = ['hydroponics', 'compost-monitor', 'vineyard', 'bee-hive', 'poultry-coop', 'grain-silo', 'hvac-controller', 'window-blinds', 'garage-door', 'lawn-irrigation', 'water-meter', 'gas-safety', 'wine-cellar', 'battery-room', 'diesel-generator', 'street-lighting', 'conveyor-monitor', 'tank-level', 'pressure-vessel', 'freezer-monitor', 'brewery', 'server-room', 'mold-risk', 'lift-elevator', 'rfid-gate']

def _r(a: float, b: float, d: int = 1) -> float:
    return round(random.uniform(a, b), d)

def _init_hydroponics(cfg: dict) -> dict:
    return {"ph_level": _r(5.2, 7.2, 1), "ec_ms": _r(0.8, 2.8, 1), "pump_active": False}

def _tick_hydroponics(cfg: dict, rt: dict) -> dict:
    ph = round(max(4, min(9, rt.get("ph_level", 6.2) + random.uniform(-0.1, 0.1))), 2)
    ec = round(max(0.2, min(4, rt.get("ec_ms", 1.8) + random.uniform(-0.15, 0.15))), 2)
    rt["ph_level"], rt["ec_ms"] = ph, ec
    pump = bool(cfg.get("auto_dose", True)) and (abs(ph - float(cfg.get("ph_target", 6.2))) > 0.3 or abs(ec - float(cfg.get("ec_target", 1.8))) > 0.4)
    rt["pump_active"] = pump
    return {"ph_level": ph, "ec_ms": ec, "pump_active": pump}

def _init_compost_monitor(cfg: dict) -> dict:
    return {"core_temp_c": _r(40, 70), "moisture_pct": _r(35, 55), "turn_due": False}

def _tick_compost_monitor(cfg: dict, rt: dict) -> dict:
    t = round(max(20, min(80, rt.get("core_temp_c", 55) + random.uniform(-2, 3))), 1)
    m = round(max(10, min(80, rt.get("moisture_pct", 45) + random.uniform(-3, 2))), 1)
    rt["core_temp_c"], rt["moisture_pct"] = t, m
    turn = t > float(cfg.get("temp_alert_c", 65)) or m < float(cfg.get("moisture_min", 40))
    rt["turn_due"] = turn
    return {"core_temp_c": t, "moisture_pct": m, "turn_due": turn}

def _init_vineyard(cfg: dict) -> dict:
    return {"soil_moisture": _r(25, 45), "leaf_wet": _r(10, 30), "frost_risk": False}

def _tick_vineyard(cfg: dict, rt: dict) -> dict:
    m = round(max(5, min(95, rt.get("soil_moisture", 35) + random.uniform(-2, 1.5))), 1)
    lw = round(max(0, min(100, rt.get("leaf_wet", 20) + random.uniform(-5, 8))), 1)
    rt["soil_moisture"], rt["leaf_wet"] = m, lw
    frost = random.random() < 0.05
    rt["frost_risk"] = frost
    return {"soil_moisture": m, "leaf_wet": lw, "frost_risk": frost}

def _init_bee_hive(cfg: dict) -> dict:
    return {"hive_weight_kg": _r(32, 52), "hive_temp_c": _r(24, 44), "swarm_alert": False}

def _tick_bee_hive(cfg: dict, rt: dict) -> dict:
    w = round(max(10, rt.get("hive_weight_kg", 42) + random.uniform(-0.5, 0.3)), 1)
    t = round(max(20, min(40, rt.get("hive_temp_c", 34) + random.uniform(-0.5, 0.5))), 1)
    rt["hive_weight_kg"], rt["hive_temp_c"] = w, t
    alert = w < float(cfg.get("weight_alert_kg", 3)) + 30
    rt["swarm_alert"] = alert
    return {"hive_weight_kg": w, "hive_temp_c": t, "swarm_alert": alert}

def _init_poultry_coop(cfg: dict) -> dict:
    return {"coop_temp_c": _r(16, 36), "ammonia_ppm": _r(2, 22), "vents_on": False}

def _tick_poultry_coop(cfg: dict, rt: dict) -> dict:
    t = round(max(10, min(45, rt.get("coop_temp_c", 26) + random.uniform(-0.6, 0.6))), 1)
    nh3 = round(max(0, rt.get("ammonia_ppm", 12) + random.uniform(-2, 4)), 1)
    rt["coop_temp_c"], rt["ammonia_ppm"] = t, nh3
    vents = bool(cfg.get("auto_vent", True)) and (t > float(cfg.get("max_temp_c", 32)) or nh3 > float(cfg.get("ammonia_alert_ppm", 25)))
    rt["vents_on"] = vents
    return {"coop_temp_c": t, "ammonia_ppm": nh3, "vents_on": vents}

def _init_grain_silo(cfg: dict) -> dict:
    return {"grain_temp_c": _r(18, 38), "humidity_pct": _r(40, 70), "spoilage_risk": False}

def _tick_grain_silo(cfg: dict, rt: dict) -> dict:
    t = round(max(10, min(60, rt.get("grain_temp_c", 28) + random.uniform(-1, 2))), 1)
    h = round(max(20, min(95, rt.get("humidity_pct", 55) + random.uniform(-3, 3))), 1)
    rt["grain_temp_c"], rt["humidity_pct"] = t, h
    risk = t > float(cfg.get("max_temp_c", 40)) or h > float(cfg.get("humidity_alert", 70))
    rt["spoilage_risk"] = risk
    return {"grain_temp_c": t, "humidity_pct": h, "spoilage_risk": risk}

def _init_hvac_controller(cfg: dict) -> dict:
    return {"zone_temp_c": _r(12, 32), "hvac_on": False, "co2_ppm": _r(585, 615)}

def _tick_hvac_controller(cfg: dict, rt: dict) -> dict:
    t = round(max(10, min(35, rt.get("zone_temp_c", 22) + random.uniform(-0.5, 0.5))), 1)
    co2 = int(max(400, rt.get("co2_ppm", 600) + random.uniform(-20, 30)))
    rt["zone_temp_c"], rt["co2_ppm"] = t, co2
    hvac = bool(cfg.get("auto_climate", True)) and abs(t - float(cfg.get("target_temp_c", 22))) > float(cfg.get("deadband_c", 1))
    rt["hvac_on"] = hvac
    return {"zone_temp_c": t, "hvac_on": hvac, "co2_ppm": co2}

def _init_window_blinds(cfg: dict) -> dict:
    return {"blinds_pct": _r(45, 75), "room_lux": _r(385, 415), "auto_mode": True}

def _tick_window_blinds(cfg: dict, rt: dict) -> dict:
    lux = round(max(0, rt.get("room_lux", 400) + random.uniform(-80, 100)), 0)
    rt["room_lux"] = lux
    auto = bool(cfg.get("heat_block", True))
    blinds = 20 if lux < float(cfg.get("lux_close", 800)) else 80
    rt["blinds_pct"], rt["auto_mode"] = blinds, auto
    return {"blinds_pct": blinds, "room_lux": lux, "auto_mode": auto}

def _init_garage_door(cfg: dict) -> dict:
    return {"door_open": False, "motion_events": _r(0, 10), "auto_closed": False}

def _tick_garage_door(cfg: dict, rt: dict) -> dict:
    if random.random() < 0.08:
        rt["motion_events"] = rt.get("motion_events", 0) + 1
    open_d = random.random() < 0.15
    rt["door_open"] = open_d
    closed = not open_d and bool(cfg.get("motion_arm", True)) and random.random() < 0.2
    rt["auto_closed"] = closed
    return {"door_open": open_d, "motion_events": rt.get("motion_events", 0), "auto_closed": closed}

def _init_lawn_irrigation(cfg: dict) -> dict:
    return {"soil_moisture": _r(28, 48), "zones_active": _r(0, 10), "rain_skip_active": False}

def _tick_lawn_irrigation(cfg: dict, rt: dict) -> dict:
    m = round(max(5, min(95, rt.get("soil_moisture", 38) + random.uniform(-2, 1.5))), 1)
    rain = random.random() < 0.1
    skip = bool(cfg.get("rain_skip", True)) and rain
    zones = 0 if skip else random.randint(0, int(cfg.get("zones", 4)))
    rt.update({"soil_moisture": m, "rain_skip_active": skip, "zones_active": zones})
    return {"soil_moisture": m, "zones_active": zones, "rain_skip_active": skip}

def _init_water_meter(cfg: dict) -> dict:
    return {"flow_lpm": _r(1.5, 3.5, 1), "daily_liters": _r(105, 135), "leak_detected": False}

def _tick_water_meter(cfg: dict, rt: dict) -> dict:
    flow = round(max(0, rt.get("flow_lpm", 2.5) + random.uniform(-0.5, 1)), 2)
    daily = round(rt.get("daily_liters", 120) + flow * 0.05, 1)
    leak = flow > float(cfg.get("leak_flow_lpm", 0.5)) and random.random() < 0.3
    rt.update({"flow_lpm": flow, "daily_liters": daily, "leak_detected": leak})
    return {"flow_lpm": flow, "daily_liters": daily, "leak_detected": leak}

def _init_gas_safety(cfg: dict) -> dict:
    return {"gas_level": _r(0, 12), "co_ppm": _r(0, 15), "shutoff_active": False}

def _tick_gas_safety(cfg: dict, rt: dict) -> dict:
    gas = round(max(0, rt.get("gas_level", 2) + random.uniform(-0.5, 1.5)), 1)
    co = round(max(0, rt.get("co_ppm", 5) + random.uniform(0, 3)), 1)
    shut = bool(cfg.get("auto_shutoff", True)) and (gas > 5 or co > float(cfg.get("co_alert_ppm", 35)))
    rt.update({"gas_level": gas, "co_ppm": co, "shutoff_active": shut})
    return {"gas_level": gas, "co_ppm": co, "shutoff_active": shut}

def _init_wine_cellar(cfg: dict) -> dict:
    return {"cellar_temp_c": _r(4, 24), "humidity_pct": _r(50, 80), "in_range": True}

def _tick_wine_cellar(cfg: dict, rt: dict) -> dict:
    t = round(max(8, min(20, rt.get("cellar_temp_c", 14) + random.uniform(-0.4, 0.4))), 1)
    h = round(max(40, min(90, rt.get("humidity_pct", 65) + random.uniform(-2, 2))), 1)
    ok = abs(t - float(cfg.get("target_temp_c", 14))) <= 2 and abs(h - float(cfg.get("target_humidity", 65))) <= 8
    rt.update({"cellar_temp_c": t, "humidity_pct": h, "in_range": ok})
    return {"cellar_temp_c": t, "humidity_pct": h, "in_range": ok}

def _init_battery_room(cfg: dict) -> dict:
    return {"bank_voltage": _r(36, 66), "bank_temp_c": _r(18, 38), "vent_on": False}

def _tick_battery_room(cfg: dict, rt: dict) -> dict:
    v = round(max(40, min(58, rt.get("bank_voltage", 51) + random.uniform(-0.5, 0.5))), 1)
    t = round(max(20, min(55, rt.get("bank_temp_c", 28) + random.uniform(-1, 2))), 1)
    vent = v < float(cfg.get("min_voltage", 48)) or t > float(cfg.get("max_temp_c", 40))
    rt.update({"bank_voltage": v, "bank_temp_c": t, "vent_on": vent})
    return {"bank_voltage": v, "bank_temp_c": t, "vent_on": vent}

def _init_diesel_generator(cfg: dict) -> dict:
    return {"vibration_g": _r(-0.6, 1.4, 1), "output_kw": _r(0, 10), "running": False}

def _tick_diesel_generator(cfg: dict, rt: dict) -> dict:
    run = random.random() > 0.6
    vib = round(max(0, rt.get("vibration_g", 0.4) + random.uniform(-0.1, 0.4)), 2)
    kw = round(random.uniform(0, 80), 1) if run else 0.0
    rt.update({"running": run, "vibration_g": vib, "output_kw": kw})
    return {"vibration_g": vib, "output_kw": kw, "running": run}

def _init_street_lighting(cfg: dict) -> dict:
    return {"fixtures_on": _r(0, 10), "circuit_kw": _r(0, 10), "schedule_active": True}

def _tick_street_lighting(cfg: dict, rt: dict) -> dict:
    count = int(cfg.get("lights_count", 12))
    on = random.randint(0, count) if random.random() > 0.3 else count
    kw = round(on * random.uniform(0.05, 0.15), 2)
    rt.update({"fixtures_on": on, "circuit_kw": kw, "schedule_active": True})
    return {"fixtures_on": on, "circuit_kw": kw, "schedule_active": True}

def _init_conveyor_monitor(cfg: dict) -> dict:
    return {"vibration_g": _r(-0.4, 1.6, 1), "motor_temp_c": _r(38, 58), "fault": False}

def _tick_conveyor_monitor(cfg: dict, rt: dict) -> dict:
    vib = round(max(0, rt.get("vibration_g", 0.6) + random.uniform(-0.2, 0.3)), 2)
    temp = round(max(25, min(90, rt.get("motor_temp_c", 48) + random.uniform(-1, 2))), 1)
    fault = vib > float(cfg.get("vibration_alert_g", 2)) or temp > float(cfg.get("motor_max_c", 70))
    rt.update({"vibration_g": vib, "motor_temp_c": temp, "fault": fault})
    return {"vibration_g": vib, "motor_temp_c": temp, "fault": fault}

def _init_tank_level(cfg: dict) -> dict:
    return {"level_cm": _r(165, 195), "volume_pct": _r(57, 87), "refill_due": False}

def _tick_tank_level(cfg: dict, rt: dict) -> dict:
    lvl = round(max(0, min(500, rt.get("level_cm", 180) + random.uniform(-8, 5))), 0)
    hi, lo = float(cfg.get("high_level_cm", 400)), float(cfg.get("low_level_cm", 50))
    pct = int(min(100, max(0, (lvl / max(1, hi)) * 100)))
    refill = lvl < lo
    rt.update({"level_cm": lvl, "volume_pct": pct, "refill_due": refill})
    return {"level_cm": lvl, "volume_pct": pct, "refill_due": refill}

def _init_pressure_vessel(cfg: dict) -> dict:
    return {"pressure_kpa": _r(405, 435), "vessel_temp_c": _r(25, 45), "over_pressure": False}

def _tick_pressure_vessel(cfg: dict, rt: dict) -> dict:
    p = round(max(50, min(900, rt.get("pressure_kpa", 420) + random.uniform(-20, 25))), 0)
    t = round(max(15, min(80, rt.get("vessel_temp_c", 35) + random.uniform(-1, 2))), 1)
    over = p > float(cfg.get("max_pressure_kpa", 600))
    rt.update({"pressure_kpa": p, "vessel_temp_c": t, "over_pressure": over})
    return {"pressure_kpa": p, "vessel_temp_c": t, "over_pressure": over}

def _init_freezer_monitor(cfg: dict) -> dict:
    return {"freezer_temp_c": _r(-19.0, -17.0, 1), "door_open_min": _r(0, 10), "breach": False}

def _tick_freezer_monitor(cfg: dict, rt: dict) -> dict:
    t = round(max(-30, min(-5, rt.get("freezer_temp_c", -18) + random.uniform(-0.5, 0.5))), 1)
    door = rt.get("door_open_min", 0)
    if random.random() < 0.06:
        door += 1
    elif door > 0 and random.random() < 0.4:
        door = 0
    breach = t > float(cfg.get("target_temp_c", -18)) + 3 or door >= int(cfg.get("door_alert_min", 5))
    rt.update({"freezer_temp_c": t, "door_open_min": door, "breach": breach})
    return {"freezer_temp_c": t, "door_open_min": door, "breach": breach}

def _init_brewery(cfg: dict) -> dict:
    return {"ferment_temp_c": _r(8, 28), "ph_level": _r(4.2, 6.2, 1), "co2_ppm": _r(785, 815)}

def _tick_brewery(cfg: dict, rt: dict) -> dict:
    t = round(max(10, min(28, rt.get("ferment_temp_c", 18) + random.uniform(-0.4, 0.4))), 1)
    ph = round(max(4, min(6, rt.get("ph_level", 5.2) + random.uniform(-0.05, 0.05))), 2)
    co2 = int(max(400, rt.get("co2_ppm", 800) + random.uniform(-50, 80)))
    rt.update({"ferment_temp_c": t, "ph_level": ph, "co2_ppm": co2})
    return {"ferment_temp_c": t, "ph_level": ph, "co2_ppm": co2}

def _init_server_room(cfg: dict) -> dict:
    return {"rack_temp_c": _r(14, 34), "humidity_pct": _r(38, 58), "cooling_on": False}

def _tick_server_room(cfg: dict, rt: dict) -> dict:
    t = round(max(18, min(40, rt.get("rack_temp_c", 24) + random.uniform(-0.6, 0.8))), 1)
    h = round(max(20, min(80, rt.get("humidity_pct", 48) + random.uniform(-3, 3))), 1)
    cool = t > float(cfg.get("max_temp_c", 27)) or h > float(cfg.get("humidity_max", 60))
    rt.update({"rack_temp_c": t, "humidity_pct": h, "cooling_on": cool})
    return {"rack_temp_c": t, "humidity_pct": h, "cooling_on": cool}

def _init_mold_risk(cfg: dict) -> dict:
    return {"humidity_pct": _r(47, 77), "dew_risk": False, "mold_alert": False}

def _tick_mold_risk(cfg: dict, rt: dict) -> dict:
    h = round(max(30, min(99, rt.get("humidity_pct", 62) + random.uniform(-4, 5))), 1)
    dew = h > 70 and random.random() < 0.2
    mold = h > float(cfg.get("humidity_alert", 75)) or dew
    rt.update({"humidity_pct": h, "dew_risk": dew, "mold_alert": mold})
    return {"humidity_pct": h, "dew_risk": dew, "mold_alert": mold}

def _init_lift_elevator(cfg: dict) -> dict:
    return {"vibration_g": _r(-0.5, 1.5, 1), "door_cycles": _r(105, 135), "fault": False}

def _tick_lift_elevator(cfg: dict, rt: dict) -> dict:
    vib = round(max(0, rt.get("vibration_g", 0.5) + random.uniform(-0.15, 0.25)), 2)
    cycles = rt.get("door_cycles", 120) + (1 if random.random() < 0.2 else 0)
    fault = vib > float(cfg.get("vibration_alert_g", 2)) or cycles > int(cfg.get("door_cycles_max", 800))
    rt.update({"vibration_g": vib, "door_cycles": cycles, "fault": fault})
    return {"vibration_g": vib, "door_cycles": cycles, "fault": fault}

def _init_rfid_gate(cfg: dict) -> dict:
    return {"tags_scanned": _r(0, 10), "throughput_hr": _r(0, 10), "unauthorized": False}

def _tick_rfid_gate(cfg: dict, rt: dict) -> dict:
    scanned = rt.get("tags_scanned", 0) + (1 if random.random() < 0.25 else 0)
    thr = int(scanned * random.uniform(2, 6))
    bad = bool(cfg.get("geofence_alert", True)) and random.random() < 0.04
    rt.update({"tags_scanned": scanned, "throughput_hr": thr, "unauthorized": bad})
    return {"tags_scanned": scanned, "throughput_hr": thr, "unauthorized": bad}

