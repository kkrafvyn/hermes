#!/usr/bin/env python3
"""One-shot generator for extended blueprint catalog."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "blueprints"

BLUEPRINTS = [
    ("hydroponics", "Hydroponics", "Nutrient film / DWC pH, EC, and water level control", "96KB", "system_name", "Grow Rack 1", [
        ("ph_target", "pH target", "number", 5.5, 7.5, 6.2),
        ("ec_target", "EC target (mS/cm)", "number", 0.5, 4.0, 1.8),
        ("auto_dose", "Auto nutrient dosing", "toggle", None, None, True),
    ], ["ph", "soil_ec", "water_level", "temp_humidity", "flow_meter"], [("ph_level", "pH", "pH"), ("ec_ms", "EC", "mS/cm"), ("pump_active", "Pump", "bool")]),
    ("compost-monitor", "Compost Monitor", "Compost pile temperature and moisture tracking", "64KB", "site_name", "Compost Bay A", [
        ("temp_alert_c", "High temp alert (C)", "number", 40, 80, 65),
        ("moisture_min", "Min moisture (%)", "number", 20, 60, 40),
    ], ["ds18b20", "soil_moisture", "temp_humidity"], [("core_temp_c", "Core temp", "C"), ("moisture_pct", "Moisture", "%"), ("turn_due", "Turn due", "bool")]),
    ("vineyard", "Vineyard Monitor", "Vineyard soil, leaf wetness, and microclimate", "96KB", "zone_name", "Block 3", [
        ("frost_alert_c", "Frost alert (C)", "number", -5, 5, 2),
        ("irrigate_on_dry", "Irrigate when dry", "toggle", None, None, True),
    ], ["soil_moisture", "leaf_wetness", "temp_humidity", "lux", "wind_speed"], [("soil_moisture", "Soil moisture", "%"), ("leaf_wet", "Leaf wetness", "%"), ("frost_risk", "Frost risk", "bool")]),
    ("bee-hive", "Bee Hive", "Hive weight, temp, and humidity for colony health", "64KB", "hive_name", "Hive 7", [
        ("weight_alert_kg", "Swarm weight drop (kg)", "number", 1, 10, 3),
        ("check_interval_sec", "Check interval (sec)", "number", 60, 3600, 300),
    ], ["weight_loadcell", "temp_humidity", "lux"], [("hive_weight_kg", "Hive weight", "kg"), ("hive_temp_c", "Hive temp", "C"), ("swarm_alert", "Swarm alert", "bool")]),
    ("poultry-coop", "Poultry Coop", "Coop climate, ammonia, and door security", "80KB", "coop_name", "Coop North", [
        ("max_temp_c", "Max temp (C)", "number", 25, 40, 32),
        ("ammonia_alert_ppm", "Ammonia alert (ppm)", "number", 10, 50, 25),
        ("auto_vent", "Auto ventilation", "toggle", None, None, True),
    ], ["temp_humidity", "ammonia", "door_contact", "motion_pir"], [("coop_temp_c", "Coop temp", "C"), ("ammonia_ppm", "Ammonia", "ppm"), ("vents_on", "Vents on", "bool")]),
    ("grain-silo", "Grain Silo", "Silo temperature, humidity, and tilt monitoring", "96KB", "silo_name", "Silo 2", [
        ("max_temp_c", "Max grain temp (C)", "number", 30, 60, 40),
        ("humidity_alert", "Humidity alert (%)", "number", 50, 90, 70),
    ], ["ds18b20", "temp_humidity", "shock_tilt"], [("grain_temp_c", "Grain temp", "C"), ("humidity_pct", "Humidity", "%"), ("spoilage_risk", "Spoilage risk", "bool")]),
    ("hvac-controller", "HVAC Controller", "Multi-zone heating and cooling control", "96KB", "building_name", "Office Wing", [
        ("target_temp_c", "Target temp (C)", "number", 16, 28, 22),
        ("deadband_c", "Deadband (C)", "number", 0.5, 3, 1),
        ("auto_climate", "Auto climate", "toggle", None, None, True),
    ], ["temp_humidity", "pressure", "co2"], [("zone_temp_c", "Zone temp", "C"), ("hvac_on", "HVAC on", "bool"), ("co2_ppm", "CO2", "ppm")]),
    ("window-blinds", "Window Blinds", "Automated blinds from lux and temperature", "64KB", "room_name", "Living Room", [
        ("lux_close", "Close below (lux)", "number", 100, 5000, 800),
        ("heat_block", "Block heat when hot", "toggle", None, None, True),
    ], ["lux", "temp_humidity"], [("blinds_pct", "Blinds open", "%"), ("room_lux", "Room lux", "lux"), ("auto_mode", "Auto mode", "bool")]),
    ("garage-door", "Garage Door", "Garage door state, motion, and auto-close", "64KB", "garage_name", "Main Garage", [
        ("auto_close_min", "Auto-close after (min)", "number", 1, 60, 10),
        ("motion_arm", "Arm on motion", "toggle", None, None, True),
    ], ["door_contact", "motion_pir", "ultrasonic"], [("door_open", "Door open", "bool"), ("motion_events", "Motion events", ""), ("auto_closed", "Auto closed", "bool")]),
    ("lawn-irrigation", "Lawn Irrigation", "Lawn zone irrigation with rain skip", "80KB", "zone_name", "Front Lawn", [
        ("moisture_threshold", "Moisture threshold (%)", "number", 20, 60, 35),
        ("zones", "Zones", "number", 1, 8, 4),
        ("rain_skip", "Skip when raining", "toggle", None, None, True),
    ], ["soil_moisture", "rain_gauge", "flow_meter"], [("soil_moisture", "Soil moisture", "%"), ("zones_active", "Zones active", ""), ("rain_skip_active", "Rain skip", "bool")]),
    ("water-meter", "Water Meter", "Domestic water flow and leak detection", "64KB", "meter_name", "Main Supply", [
        ("leak_flow_lpm", "Leak threshold (L/min)", "number", 0.1, 5, 0.5),
        ("daily_alert_l", "Daily use alert (L)", "number", 100, 2000, 500),
    ], ["flow_meter", "water_leak", "pressure"], [("flow_lpm", "Flow", "L/min"), ("daily_liters", "Daily use", "L"), ("leak_detected", "Leak", "bool")]),
    ("gas-safety", "Gas Safety", "Combustible gas and CO monitoring with shutoff", "80KB", "site_name", "Kitchen", [
        ("co_alert_ppm", "CO alert (ppm)", "number", 20, 100, 35),
        ("auto_shutoff", "Auto gas shutoff", "toggle", None, None, True),
    ], ["gas_mq", "co_gas", "smoke"], [("gas_level", "Gas level", ""), ("co_ppm", "CO", "ppm"), ("shutoff_active", "Shutoff", "bool")]),
    ("wine-cellar", "Wine Cellar", "Cellar temperature and humidity for wine storage", "64KB", "cellar_name", "Cellar A", [
        ("target_temp_c", "Target temp (C)", "number", 10, 18, 14),
        ("target_humidity", "Target humidity (%)", "number", 50, 80, 65),
    ], ["temp_humidity", "ds18b20"], [("cellar_temp_c", "Cellar temp", "C"), ("humidity_pct", "Humidity", "%"), ("in_range", "In range", "bool")]),
    ("battery-room", "Battery Room", "Battery bank voltage, temp, and safety", "80KB", "room_name", "Battery Room", [
        ("min_voltage", "Min voltage (V)", "number", 40, 52, 48),
        ("max_temp_c", "Max temp (C)", "number", 30, 55, 40),
    ], ["voltage", "current_clamp", "temp_humidity", "gas_mq"], [("bank_voltage", "Bank voltage", "V"), ("bank_temp_c", "Bank temp", "C"), ("vent_on", "Vent on", "bool")]),
    ("diesel-generator", "Diesel Generator", "Generator vibration, voltage, and runtime", "96KB", "gen_name", "Backup Gen", [
        ("vibration_alert_g", "Vibration alert (g)", "number", 1, 5, 2),
        ("auto_start", "Auto start on outage", "toggle", None, None, False),
    ], ["vibration", "voltage", "current_clamp", "temp_humidity"], [("vibration_g", "Vibration", "g"), ("output_kw", "Output", "kW"), ("running", "Running", "bool")]),
    ("street-lighting", "Street Lighting", "Dusk-to-dawn street and path lighting", "64KB", "circuit_name", "Block 12", [
        ("lights_count", "Fixtures", "number", 1, 50, 12),
        ("lux_on", "On below (lux)", "number", 5, 100, 30),
    ], ["lux", "motion_pir", "current_clamp"], [("fixtures_on", "Fixtures on", ""), ("circuit_kw", "Circuit power", "kW"), ("schedule_active", "Schedule", "bool")]),
    ("conveyor-monitor", "Conveyor Monitor", "Belt conveyor vibration and motor health", "96KB", "line_name", "Line 3", [
        ("vibration_alert_g", "Vibration alert (g)", "number", 0.5, 4, 2),
        ("motor_max_c", "Motor max temp (C)", "number", 50, 90, 70),
    ], ["vibration", "accelerometer", "current_clamp", "temp_humidity"], [("vibration_g", "Vibration", "g"), ("motor_temp_c", "Motor temp", "C"), ("fault", "Fault", "bool")]),
    ("tank-level", "Tank Level", "Tank or reservoir level and refill alerts", "64KB", "tank_name", "Water Tank", [
        ("low_level_cm", "Low level (cm)", "number", 10, 200, 50),
        ("high_level_cm", "High level (cm)", "number", 100, 500, 400),
    ], ["ultrasonic", "water_level", "pressure"], [("level_cm", "Level", "cm"), ("volume_pct", "Volume", "%"), ("refill_due", "Refill due", "bool")]),
    ("pressure-vessel", "Pressure Vessel", "Pressure vessel monitoring and relief alerts", "80KB", "vessel_name", "Tank 1", [
        ("max_pressure_kpa", "Max pressure (kPa)", "number", 100, 1000, 600),
        ("relief_auto", "Auto relief alert", "toggle", None, None, True),
    ], ["pressure", "temp_humidity", "vibration"], [("pressure_kpa", "Pressure", "kPa"), ("vessel_temp_c", "Vessel temp", "C"), ("over_pressure", "Over pressure", "bool")]),
    ("freezer-monitor", "Freezer Monitor", "Deep freezer and cold storage compliance", "64KB", "unit_name", "Walk-in Freezer", [
        ("target_temp_c", "Target temp (C)", "number", -30, -10, -18),
        ("door_alert_min", "Door open alert (min)", "number", 1, 30, 5),
    ], ["ds18b20", "door_contact", "temp_humidity"], [("freezer_temp_c", "Freezer temp", "C"), ("door_open_min", "Door open", "min"), ("breach", "Breach", "bool")]),
    ("brewery", "Brewery", "Fermentation temp, pressure, and pH monitoring", "96KB", "batch_name", "Batch 42", [
        ("ferment_temp_c", "Ferment temp (C)", "number", 10, 25, 18),
        ("ph_target", "pH target", "number", 4, 6, 5.2),
    ], ["ph", "temp_humidity", "pressure", "co2"], [("ferment_temp_c", "Ferment temp", "C"), ("ph_level", "pH", "pH"), ("co2_ppm", "CO2", "ppm")]),
    ("server-room", "Server Room", "Datacenter rack temp, humidity, and power", "96KB", "room_name", "DC Row A", [
        ("max_temp_c", "Max temp (C)", "number", 22, 35, 27),
        ("humidity_max", "Max humidity (%)", "number", 40, 80, 60),
    ], ["temp_humidity", "current_clamp", "co2", "water_leak"], [("rack_temp_c", "Rack temp", "C"), ("humidity_pct", "Humidity", "%"), ("cooling_on", "Cooling on", "bool")]),
    ("mold-risk", "Mold Risk", "Indoor mold risk from humidity and temperature", "64KB", "room_name", "Basement", [
        ("humidity_alert", "Humidity alert (%)", "number", 60, 95, 75),
        ("temp_min_c", "Min temp (C)", "number", 10, 20, 15),
    ], ["temp_humidity", "voc", "water_leak"], [("humidity_pct", "Humidity", "%"), ("dew_risk", "Dew risk", "bool"), ("mold_alert", "Mold alert", "bool")]),
    ("lift-elevator", "Lift & Elevator", "Elevator vibration, door cycles, and faults", "80KB", "lift_name", "Lift B", [
        ("vibration_alert_g", "Vibration alert (g)", "number", 1, 4, 2),
        ("door_cycles_max", "Max door cycles/day", "number", 100, 5000, 800),
    ], ["vibration", "door_contact", "accelerometer"], [("vibration_g", "Vibration", "g"), ("door_cycles", "Door cycles", ""), ("fault", "Fault", "bool")]),
    ("rfid-gate", "RFID Gate", "RFID gate access and tag throughput", "80KB", "gate_name", "Loading Gate", [
        ("tags_expected", "Expected tags/hr", "number", 10, 500, 60),
        ("geofence_alert", "Geofence alerts", "toggle", None, None, True),
    ], ["rfid", "ble_rssi", "motion_pir", "door_contact"], [("tags_scanned", "Tags scanned", ""), ("throughput_hr", "Throughput/hr", ""), ("unauthorized", "Unauthorized", "bool")]),
]


def main() -> None:
    for spec in BLUEPRINTS:
        bid, name, desc, mem, name_key, name_default, extra_fields, sensors, metrics = spec
        fields = [
            {"key": name_key, "label": name_key.replace("_", " ").title(), "type": "text", "default": name_default, "required": True},
        ]
        for key, label, ftype, mn, mx, default in extra_fields:
            f: dict = {"key": key, "label": label, "type": ftype, "default": default}
            if ftype == "number":
                f["min"], f["max"] = mn, mx
            fields.append(f)
        fields.append({"key": "sensors", "label": "Sensors", "type": "sensor_list", "default": sensors})
        fields.append({"key": "target_node", "label": "Target node", "type": "node", "default": 1})
        mlist = []
        for mk, ml, mu in metrics:
            m: dict = {"key": mk, "label": ml}
            if mu == "bool":
                m["type"] = "bool"
            elif mu:
                m["unit"] = mu
            mlist.append(m)
        data = {
            "id": bid,
            "name": name,
            "description": desc,
            "version": "1.0.0",
            "memory": mem,
            "priority": 4,
            "fields": fields,
            "metrics": mlist,
        }
        (OUT / f"{bid}.json").write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
        print("wrote", bid)
    print("total", len(list(OUT.glob("*.json"))))


if __name__ == "__main__":
    main()
