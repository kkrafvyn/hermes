#!/usr/bin/env python3
"""Append init/tick handlers for extended blueprint catalog."""
from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HANDLERS = Path(__file__).resolve().parent / "control-plane" / "blueprint_handlers_ext.py"

NEW_APPS = [
    "hydroponics", "compost-monitor", "vineyard", "bee-hive", "poultry-coop", "grain-silo",
    "hvac-controller", "window-blinds", "garage-door", "lawn-irrigation", "water-meter",
    "gas-safety", "wine-cellar", "battery-room", "diesel-generator", "street-lighting",
    "conveyor-monitor", "tank-level", "pressure-vessel", "freezer-monitor", "brewery",
    "server-room", "mold-risk", "lift-elevator", "rfid-gate",
]

# (init defaults as dict keys, tick logic uses cfg keys where relevant)
INIT_RT = {
    "hydroponics": {"ph_level": "6.2", "ec_ms": "1.8", "pump_active": "False"},
    "compost-monitor": {"core_temp_c": "55", "moisture_pct": "45", "turn_due": "False"},
    "vineyard": {"soil_moisture": "35", "leaf_wet": "20", "frost_risk": "False"},
    "bee-hive": {"hive_weight_kg": "42", "hive_temp_c": "34", "swarm_alert": "False"},
    "poultry-coop": {"coop_temp_c": "26", "ammonia_ppm": "12", "vents_on": "False"},
    "grain-silo": {"grain_temp_c": "28", "humidity_pct": "55", "spoilage_risk": "False"},
    "hvac-controller": {"zone_temp_c": "22", "hvac_on": "False", "co2_ppm": "600"},
    "window-blinds": {"blinds_pct": "60", "room_lux": "400", "auto_mode": "True"},
    "garage-door": {"door_open": "False", "motion_events": "0", "auto_closed": "False"},
    "lawn-irrigation": {"soil_moisture": "38", "zones_active": "0", "rain_skip_active": "False"},
    "water-meter": {"flow_lpm": "2.5", "daily_liters": "120", "leak_detected": "False"},
    "gas-safety": {"gas_level": "2", "co_ppm": "5", "shutoff_active": "False"},
    "wine-cellar": {"cellar_temp_c": "14", "humidity_pct": "65", "in_range": "True"},
    "battery-room": {"bank_voltage": "51", "bank_temp_c": "28", "vent_on": "False"},
    "diesel-generator": {"vibration_g": "0.4", "output_kw": "0", "running": "False"},
    "street-lighting": {"fixtures_on": "0", "circuit_kw": "0", "schedule_active": "True"},
    "conveyor-monitor": {"vibration_g": "0.6", "motor_temp_c": "48", "fault": "False"},
    "tank-level": {"level_cm": "180", "volume_pct": "72", "refill_due": "False"},
    "pressure-vessel": {"pressure_kpa": "420", "vessel_temp_c": "35", "over_pressure": "False"},
    "freezer-monitor": {"freezer_temp_c": "-18", "door_open_min": "0", "breach": "False"},
    "brewery": {"ferment_temp_c": "18", "ph_level": "5.2", "co2_ppm": "800"},
    "server-room": {"rack_temp_c": "24", "humidity_pct": "48", "cooling_on": "False"},
    "mold-risk": {"humidity_pct": "62", "dew_risk": "False", "mold_alert": "False"},
    "lift-elevator": {"vibration_g": "0.5", "door_cycles": "120", "fault": "False"},
    "rfid-gate": {"tags_scanned": "0", "throughput_hr": "0", "unauthorized": "False"},
}

TICK_BODY = {
    "hydroponics": """
    ph = round(max(4, min(9, rt.get("ph_level", 6.2) + random.uniform(-0.1, 0.1))), 2)
    ec = round(max(0.2, min(4, rt.get("ec_ms", 1.8) + random.uniform(-0.15, 0.15))), 2)
    rt["ph_level"], rt["ec_ms"] = ph, ec
    pump = bool(cfg.get("auto_dose", True)) and (abs(ph - float(cfg.get("ph_target", 6.2))) > 0.3 or abs(ec - float(cfg.get("ec_target", 1.8))) > 0.4)
    rt["pump_active"] = pump
    return {"ph_level": ph, "ec_ms": ec, "pump_active": pump}""",
    "compost-monitor": """
    t = round(max(20, min(80, rt.get("core_temp_c", 55) + random.uniform(-2, 3))), 1)
    m = round(max(10, min(80, rt.get("moisture_pct", 45) + random.uniform(-3, 2))), 1)
    rt["core_temp_c"], rt["moisture_pct"] = t, m
    turn = t > float(cfg.get("temp_alert_c", 65)) or m < float(cfg.get("moisture_min", 40))
    rt["turn_due"] = turn
    return {"core_temp_c": t, "moisture_pct": m, "turn_due": turn}""",
    "vineyard": """
    m = round(max(5, min(95, rt.get("soil_moisture", 35) + random.uniform(-2, 1.5))), 1)
    lw = round(max(0, min(100, rt.get("leaf_wet", 20) + random.uniform(-5, 8))), 1)
    rt["soil_moisture"], rt["leaf_wet"] = m, lw
    frost = random.random() < 0.05
    rt["frost_risk"] = frost
    return {"soil_moisture": m, "leaf_wet": lw, "frost_risk": frost}""",
    "bee-hive": """
    w = round(max(10, rt.get("hive_weight_kg", 42) + random.uniform(-0.5, 0.3)), 1)
    t = round(max(20, min(40, rt.get("hive_temp_c", 34) + random.uniform(-0.5, 0.5))), 1)
    rt["hive_weight_kg"], rt["hive_temp_c"] = w, t
    alert = w < float(cfg.get("weight_alert_kg", 3)) + 30
    rt["swarm_alert"] = alert
    return {"hive_weight_kg": w, "hive_temp_c": t, "swarm_alert": alert}""",
    "poultry-coop": """
    t = round(max(10, min(45, rt.get("coop_temp_c", 26) + random.uniform(-0.6, 0.6))), 1)
    nh3 = round(max(0, rt.get("ammonia_ppm", 12) + random.uniform(-2, 4)), 1)
    rt["coop_temp_c"], rt["ammonia_ppm"] = t, nh3
    vents = bool(cfg.get("auto_vent", True)) and (t > float(cfg.get("max_temp_c", 32)) or nh3 > float(cfg.get("ammonia_alert_ppm", 25)))
    rt["vents_on"] = vents
    return {"coop_temp_c": t, "ammonia_ppm": nh3, "vents_on": vents}""",
    "grain-silo": """
    t = round(max(10, min(60, rt.get("grain_temp_c", 28) + random.uniform(-1, 2))), 1)
    h = round(max(20, min(95, rt.get("humidity_pct", 55) + random.uniform(-3, 3))), 1)
    rt["grain_temp_c"], rt["humidity_pct"] = t, h
    risk = t > float(cfg.get("max_temp_c", 40)) or h > float(cfg.get("humidity_alert", 70))
    rt["spoilage_risk"] = risk
    return {"grain_temp_c": t, "humidity_pct": h, "spoilage_risk": risk}""",
    "hvac-controller": """
    t = round(max(10, min(35, rt.get("zone_temp_c", 22) + random.uniform(-0.5, 0.5))), 1)
    co2 = int(max(400, rt.get("co2_ppm", 600) + random.uniform(-20, 30)))
    rt["zone_temp_c"], rt["co2_ppm"] = t, co2
    hvac = bool(cfg.get("auto_climate", True)) and abs(t - float(cfg.get("target_temp_c", 22))) > float(cfg.get("deadband_c", 1))
    rt["hvac_on"] = hvac
    return {"zone_temp_c": t, "hvac_on": hvac, "co2_ppm": co2}""",
    "window-blinds": """
    lux = round(max(0, rt.get("room_lux", 400) + random.uniform(-80, 100)), 0)
    rt["room_lux"] = lux
    auto = bool(cfg.get("heat_block", True))
    blinds = 20 if lux < float(cfg.get("lux_close", 800)) else 80
    rt["blinds_pct"], rt["auto_mode"] = blinds, auto
    return {"blinds_pct": blinds, "room_lux": lux, "auto_mode": auto}""",
    "garage-door": """
    if random.random() < 0.08:
        rt["motion_events"] = rt.get("motion_events", 0) + 1
    open_d = random.random() < 0.15
    rt["door_open"] = open_d
    closed = not open_d and bool(cfg.get("motion_arm", True)) and random.random() < 0.2
    rt["auto_closed"] = closed
    return {"door_open": open_d, "motion_events": rt.get("motion_events", 0), "auto_closed": closed}""",
    "lawn-irrigation": """
    m = round(max(5, min(95, rt.get("soil_moisture", 38) + random.uniform(-2, 1.5))), 1)
    rain = random.random() < 0.1
    skip = bool(cfg.get("rain_skip", True)) and rain
    zones = 0 if skip else random.randint(0, int(cfg.get("zones", 4)))
    rt.update({"soil_moisture": m, "rain_skip_active": skip, "zones_active": zones})
    return {"soil_moisture": m, "zones_active": zones, "rain_skip_active": skip}""",
    "water-meter": """
    flow = round(max(0, rt.get("flow_lpm", 2.5) + random.uniform(-0.5, 1)), 2)
    daily = round(rt.get("daily_liters", 120) + flow * 0.05, 1)
    leak = flow > float(cfg.get("leak_flow_lpm", 0.5)) and random.random() < 0.3
    rt.update({"flow_lpm": flow, "daily_liters": daily, "leak_detected": leak})
    return {"flow_lpm": flow, "daily_liters": daily, "leak_detected": leak}""",
    "gas-safety": """
    gas = round(max(0, rt.get("gas_level", 2) + random.uniform(-0.5, 1.5)), 1)
    co = round(max(0, rt.get("co_ppm", 5) + random.uniform(0, 3)), 1)
    shut = bool(cfg.get("auto_shutoff", True)) and (gas > 5 or co > float(cfg.get("co_alert_ppm", 35)))
    rt.update({"gas_level": gas, "co_ppm": co, "shutoff_active": shut})
    return {"gas_level": gas, "co_ppm": co, "shutoff_active": shut}""",
    "wine-cellar": """
    t = round(max(8, min(20, rt.get("cellar_temp_c", 14) + random.uniform(-0.4, 0.4))), 1)
    h = round(max(40, min(90, rt.get("humidity_pct", 65) + random.uniform(-2, 2))), 1)
    ok = abs(t - float(cfg.get("target_temp_c", 14))) <= 2 and abs(h - float(cfg.get("target_humidity", 65))) <= 8
    rt.update({"cellar_temp_c": t, "humidity_pct": h, "in_range": ok})
    return {"cellar_temp_c": t, "humidity_pct": h, "in_range": ok}""",
    "battery-room": """
    v = round(max(40, min(58, rt.get("bank_voltage", 51) + random.uniform(-0.5, 0.5))), 1)
    t = round(max(20, min(55, rt.get("bank_temp_c", 28) + random.uniform(-1, 2))), 1)
    vent = v < float(cfg.get("min_voltage", 48)) or t > float(cfg.get("max_temp_c", 40))
    rt.update({"bank_voltage": v, "bank_temp_c": t, "vent_on": vent})
    return {"bank_voltage": v, "bank_temp_c": t, "vent_on": vent}""",
    "diesel-generator": """
    run = random.random() > 0.6
    vib = round(max(0, rt.get("vibration_g", 0.4) + random.uniform(-0.1, 0.4)), 2)
    kw = round(random.uniform(0, 80), 1) if run else 0.0
    rt.update({"running": run, "vibration_g": vib, "output_kw": kw})
    return {"vibration_g": vib, "output_kw": kw, "running": run}""",
    "street-lighting": """
    count = int(cfg.get("lights_count", 12))
    on = random.randint(0, count) if random.random() > 0.3 else count
    kw = round(on * random.uniform(0.05, 0.15), 2)
    rt.update({"fixtures_on": on, "circuit_kw": kw, "schedule_active": True})
    return {"fixtures_on": on, "circuit_kw": kw, "schedule_active": True}""",
    "conveyor-monitor": """
    vib = round(max(0, rt.get("vibration_g", 0.6) + random.uniform(-0.2, 0.3)), 2)
    temp = round(max(25, min(90, rt.get("motor_temp_c", 48) + random.uniform(-1, 2))), 1)
    fault = vib > float(cfg.get("vibration_alert_g", 2)) or temp > float(cfg.get("motor_max_c", 70))
    rt.update({"vibration_g": vib, "motor_temp_c": temp, "fault": fault})
    return {"vibration_g": vib, "motor_temp_c": temp, "fault": fault}""",
    "tank-level": """
    lvl = round(max(0, min(500, rt.get("level_cm", 180) + random.uniform(-8, 5))), 0)
    hi, lo = float(cfg.get("high_level_cm", 400)), float(cfg.get("low_level_cm", 50))
    pct = int(min(100, max(0, (lvl / max(1, hi)) * 100)))
    refill = lvl < lo
    rt.update({"level_cm": lvl, "volume_pct": pct, "refill_due": refill})
    return {"level_cm": lvl, "volume_pct": pct, "refill_due": refill}""",
    "pressure-vessel": """
    p = round(max(50, min(900, rt.get("pressure_kpa", 420) + random.uniform(-20, 25))), 0)
    t = round(max(15, min(80, rt.get("vessel_temp_c", 35) + random.uniform(-1, 2))), 1)
    over = p > float(cfg.get("max_pressure_kpa", 600))
    rt.update({"pressure_kpa": p, "vessel_temp_c": t, "over_pressure": over})
    return {"pressure_kpa": p, "vessel_temp_c": t, "over_pressure": over}""",
    "freezer-monitor": """
    t = round(max(-30, min(-5, rt.get("freezer_temp_c", -18) + random.uniform(-0.5, 0.5))), 1)
    door = rt.get("door_open_min", 0)
    if random.random() < 0.06:
        door += 1
    elif door > 0 and random.random() < 0.4:
        door = 0
    breach = t > float(cfg.get("target_temp_c", -18)) + 3 or door >= int(cfg.get("door_alert_min", 5))
    rt.update({"freezer_temp_c": t, "door_open_min": door, "breach": breach})
    return {"freezer_temp_c": t, "door_open_min": door, "breach": breach}""",
    "brewery": """
    t = round(max(10, min(28, rt.get("ferment_temp_c", 18) + random.uniform(-0.4, 0.4))), 1)
    ph = round(max(4, min(6, rt.get("ph_level", 5.2) + random.uniform(-0.05, 0.05))), 2)
    co2 = int(max(400, rt.get("co2_ppm", 800) + random.uniform(-50, 80)))
    rt.update({"ferment_temp_c": t, "ph_level": ph, "co2_ppm": co2})
    return {"ferment_temp_c": t, "ph_level": ph, "co2_ppm": co2}""",
    "server-room": """
    t = round(max(18, min(40, rt.get("rack_temp_c", 24) + random.uniform(-0.6, 0.8))), 1)
    h = round(max(20, min(80, rt.get("humidity_pct", 48) + random.uniform(-3, 3))), 1)
    cool = t > float(cfg.get("max_temp_c", 27)) or h > float(cfg.get("humidity_max", 60))
    rt.update({"rack_temp_c": t, "humidity_pct": h, "cooling_on": cool})
    return {"rack_temp_c": t, "humidity_pct": h, "cooling_on": cool}""",
    "mold-risk": """
    h = round(max(30, min(99, rt.get("humidity_pct", 62) + random.uniform(-4, 5))), 1)
    dew = h > 70 and random.random() < 0.2
    mold = h > float(cfg.get("humidity_alert", 75)) or dew
    rt.update({"humidity_pct": h, "dew_risk": dew, "mold_alert": mold})
    return {"humidity_pct": h, "dew_risk": dew, "mold_alert": mold}""",
    "lift-elevator": """
    vib = round(max(0, rt.get("vibration_g", 0.5) + random.uniform(-0.15, 0.25)), 2)
    cycles = rt.get("door_cycles", 120) + (1 if random.random() < 0.2 else 0)
    fault = vib > float(cfg.get("vibration_alert_g", 2)) or cycles > int(cfg.get("door_cycles_max", 800))
    rt.update({"vibration_g": vib, "door_cycles": cycles, "fault": fault})
    return {"vibration_g": vib, "door_cycles": cycles, "fault": fault}""",
    "rfid-gate": """
    scanned = rt.get("tags_scanned", 0) + (1 if random.random() < 0.25 else 0)
    thr = int(scanned * random.uniform(2, 6))
    bad = bool(cfg.get("geofence_alert", True)) and random.random() < 0.04
    rt.update({"tags_scanned": scanned, "throughput_hr": thr, "unauthorized": bad})
    return {"tags_scanned": scanned, "throughput_hr": thr, "unauthorized": bad}""",
}


def fn_name(bid: str) -> str:
    return bid.replace("-", "_")


def gen_init(bid: str) -> str:
    fn = fn_name(bid)
    items = INIT_RT[bid]
    parts = []
    for k, v in items.items():
        if v in ("True", "False"):
            parts.append(f'"{k}": {v}')
        elif "." in v or v.startswith("-"):
            fv = float(v)
            parts.append(f'"{k}": _r({fv - 1}, {fv + 1}, 1)')
        elif int(v) > 50:
            parts.append(f'"{k}": _r({int(v) - 15}, {int(v) + 15})')
        else:
            parts.append(f'"{k}": _r({max(0, int(v) - 10)}, {int(v) + 10})')
    body = ", ".join(parts)
    return f"def _init_{fn}(cfg: dict) -> dict:\n    return {{{body}}}\n\n"


def gen_tick(bid: str) -> str:
    fn = fn_name(bid)
    return f"def _tick_{fn}(cfg: dict, rt: dict) -> dict:{TICK_BODY[bid]}\n\n"


def main() -> None:
    lines = [
        '"""Extended blueprint handlers — auto-generated."""\n',
        "from __future__ import annotations\n\n",
        "import random\n\n",
        "NEW_APPS = " + repr(NEW_APPS) + "\n\n",
        "def _r(a: float, b: float, d: int = 1) -> float:\n",
        "    return round(random.uniform(a, b), d)\n\n",
    ]
    for bid in NEW_APPS:
        lines.append(gen_init(bid))
        lines.append(gen_tick(bid))
    HANDLERS.write_text("".join(lines), encoding="utf-8")
    print("wrote", HANDLERS)


if __name__ == "__main__":
    main()
