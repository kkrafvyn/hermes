#ifndef HERMES_BLUEPRINTS_DATA_H
#define HERMES_BLUEPRINTS_DATA_H

#include <stddef.h>
#include <string.h>

#define HERMES_BOARD_BLUEPRINT_COUNT 51

static const char HERMES_BLUEPRINTS_JSON[] PROGMEM =
    "{\"blueprints\":[{\"id\":\"air-quality\",\"name\":\"Air Quality\",\"description\":\"CO\\u2082,"
    " VOC, PM2.5 and comfort monitoring\"},{\"id\":\"aquaponics\",\"name\":\"Aquaponics\",\"des"
    "cription\":\"Fish tank, pH balance, and automated feeding\"},{\"id\":\"asset-tracker\","
    "\"name\":\"Asset Tracker\",\"description\":\"BLE/GPS asset tracking and geofence alerts"
    "\"},{\"id\":\"battery-room\",\"name\":\"Battery Room\",\"description\":\"Battery bank voltag"
    "e, temp, and safety\"},{\"id\":\"bee-hive\",\"name\":\"Bee Hive\",\"description\":\"Hive wei"
    "ght, temp, and humidity for colony health\"},{\"id\":\"brewery\",\"name\":\"Brewery\",\"de"
    "scription\":\"Fermentation temp, pressure, and pH monitoring\"},{\"id\":\"cold-chain\","
    "\"name\":\"Cold Chain\",\"description\":\"Temperature compliance for food, pharma, and "
    "logistics\"},{\"id\":\"compost-monitor\",\"name\":\"Compost Monitor\",\"description\":\"Comp"
    "ost pile temperature and moisture tracking\"},{\"id\":\"conveyor-monitor\",\"name\":\"Co"
    "nveyor Monitor\",\"description\":\"Belt conveyor vibration and motor health\"},{\"id\":"
    "\"diesel-generator\",\"name\":\"Diesel Generator\",\"description\":\"Generator vibration,"
    " voltage, and runtime\"},{\"id\":\"elder-care\",\"name\":\"Elder Care Monitor\",\"descript"
    "ion\":\"Room comfort, motion, and fall-risk monitoring\"},{\"id\":\"energy-monitor\",\"n"
    "ame\":\"Energy Monitor\",\"description\":\"Power usage tracking, peak alerts, and load"
    " shedding\"},{\"id\":\"ev-charger\",\"name\":\"EV Charger\",\"description\":\"EV charging wi"
    "th load management and session tracking\"},{\"id\":\"fire-smoke\",\"name\":\"Fire & Smok"
    "e\",\"description\":\"Smoke, heat, CO detection and zone alarms\"},{\"id\":\"freezer-mon"
    "itor\",\"name\":\"Freezer Monitor\",\"description\":\"Deep freezer and cold storage comp"
    "liance\"},{\"id\":\"garage-door\",\"name\":\"Garage Door\",\"description\":\"Garage door sta"
    "te, motion, and auto-close\"},{\"id\":\"gas-safety\",\"name\":\"Gas Safety\",\"description"
    "\":\"Combustible gas and CO monitoring with shutoff\"},{\"id\":\"grain-silo\",\"name\":\"G"
    "rain Silo\",\"description\":\"Silo temperature, humidity, and tilt monitoring\"},{\"id"
    "\":\"greenhouse\",\"name\":\"Greenhouse\",\"description\":\"Climate, humidity, and ventila"
    "tion for greenhouse zones\"},{\"id\":\"hvac-controller\",\"name\":\"HVAC Controller\",\"de"
    "scription\":\"Multi-zone heating and cooling control\"},{\"id\":\"hydroponics\",\"name\":"
    "\"Hydroponics\",\"description\":\"Nutrient film / DWC pH, EC, and water level control"
    "\"},{\"id\":\"irrigation-scheduler\",\"name\":\"Irrigation Scheduler\",\"description\":\"Mul"
    "ti-zone irrigation with rain skip and soil feedback\"},{\"id\":\"lawn-irrigation\",\"n"
    "ame\":\"Lawn Irrigation\",\"description\":\"Lawn zone irrigation with rain skip\"},{\"id"
    "\":\"leak-detector\",\"name\":\"Leak Detector\",\"description\":\"Water leak detection wit"
    "h optional auto shutoff\"},{\"id\":\"lift-elevator\",\"name\":\"Lift & Elevator\",\"descri"
    "ption\":\"Elevator vibration, door cycles, and faults\"},{\"id\":\"livestock-barn\",\"na"
    "me\":\"Livestock Barn\",\"description\":\"Barn climate, ammonia, water, and ventilatio"
    "n\"},{\"id\":\"machine-monitor\",\"name\":\"Machine Monitor\",\"description\":\"Vibration, r"
    "untime, and predictive maintenance\"},{\"id\":\"mold-risk\",\"name\":\"Mold Risk\",\"descr"
    "iption\":\"Indoor mold risk from humidity and temperature\"},{\"id\":\"parking-gate\",\""
    "name\":\"Parking & Gate\",\"description\":\"Parking occupancy, barrier control, access"
    " counting\"},{\"id\":\"pest-trap\",\"name\":\"Pest Trap Monitor\",\"description\":\"Field tr"
    "ap counts and bait status\"},{\"id\":\"pool-spa\",\"name\":\"Pool & Spa\",\"description\":\""
    "pH, ORP, pump and heater control\"},{\"id\":\"poultry-coop\",\"name\":\"Poultry Coop\",\"d"
    "escription\":\"Coop climate, ammonia, and door security\"},{\"id\":\"pressure-vessel\","
    "\"name\":\"Pressure Vessel\",\"description\":\"Pressure vessel monitoring and relief al"
    "erts\"},{\"id\":\"rfid-gate\",\"name\":\"RFID Gate\",\"description\":\"RFID gate access and "
    "tag throughput\"},{\"id\":\"security\",\"name\":\"Security Monitor\",\"description\":\"Motio"
    "n detection, entry points, and alarm control\"},{\"id\":\"sensor-net\",\"name\":\"Sensor"
    " Network\",\"description\":\"Multi-sensor mesh with local + remote nodes\"},{\"id\":\"se"
    "rver-room\",\"name\":\"Server Room\",\"description\":\"Datacenter rack temp, humidity, a"
    "nd power\"},{\"id\":\"smart-gate\",\"name\":\"Smart Gate\",\"description\":\"Canal/gate leve"
    "l, flow control, and irrigation gates\"},{\"id\":\"smart-lighting\",\"name\":\"Smart Lig"
    "hting\",\"description\":\"Brightness, schedules, and sunset automation\"},{\"id\":\"smar"
    "tfarm\",\"name\":\"Smart Farm\",\"description\":\"Soil moisture monitoring and automated"
    " irrigation\"},{\"id\":\"smarthome\",\"name\":\"Smart Home\",\"description\":\"Climate contr"
    "ol, lighting, and room automation\"},{\"id\":\"solar-battery\",\"name\":\"Solar & Batter"
    "y\",\"description\":\"Solar output, battery state, grid import/export\"},{\"id\":\"stree"
    "t-lighting\",\"name\":\"Street Lighting\",\"description\":\"Dusk-to-dawn street and path"
    " lighting\"},{\"id\":\"tank-level\",\"name\":\"Tank Level\",\"description\":\"Tank or reserv"
    "oir level and refill alerts\"},{\"id\":\"telemetry\",\"name\":\"Telemetry Hub\",\"descript"
    "ion\":\"Network and cluster telemetry collection\"},{\"id\":\"vineyard\",\"name\":\"Vineya"
    "rd Monitor\",\"description\":\"Vineyard soil, leaf wetness, and microclimate\"},{\"id\""
    ":\"warehouse\",\"name\":\"Warehouse\",\"description\":\"Zone monitoring, cold storage, an"
    "d door alerts\"},{\"id\":\"water-meter\",\"name\":\"Water Meter\",\"description\":\"Domestic"
    " water flow and leak detection\"},{\"id\":\"weather-station\",\"name\":\"Weather Station"
    "\",\"description\":\"Outdoor temperature, humidity, wind, and rainfall\"},{\"id\":\"wind"
    "ow-blinds\",\"name\":\"Window Blinds\",\"description\":\"Automated blinds from lux and t"
    "emperature\"},{\"id\":\"wine-cellar\",\"name\":\"Wine Cellar\",\"description\":\"Cellar temp"
    "erature and humidity for wine storage\"}]}";

static const char HERMES_BP_IDS[51][24] PROGMEM = {
    "air-quality",
    "aquaponics",
    "asset-tracker",
    "battery-room",
    "bee-hive",
    "brewery",
    "cold-chain",
    "compost-monitor",
    "conveyor-monitor",
    "diesel-generator",
    "elder-care",
    "energy-monitor",
    "ev-charger",
    "fire-smoke",
    "freezer-monitor",
    "garage-door",
    "gas-safety",
    "grain-silo",
    "greenhouse",
    "hvac-controller",
    "hydroponics",
    "irrigation-scheduler",
    "lawn-irrigation",
    "leak-detector",
    "lift-elevator",
    "livestock-barn",
    "machine-monitor",
    "mold-risk",
    "parking-gate",
    "pest-trap",
    "pool-spa",
    "poultry-coop",
    "pressure-vessel",
    "rfid-gate",
    "security",
    "sensor-net",
    "server-room",
    "smart-gate",
    "smart-lighting",
    "smartfarm",
    "smarthome",
    "solar-battery",
    "street-lighting",
    "tank-level",
    "telemetry",
    "vineyard",
    "warehouse",
    "water-meter",
    "weather-station",
    "window-blinds",
    "wine-cellar",
};

static const char HERMES_BP_NAMES[51][32] PROGMEM = {
    "Air Quality",
    "Aquaponics",
    "Asset Tracker",
    "Battery Room",
    "Bee Hive",
    "Brewery",
    "Cold Chain",
    "Compost Monitor",
    "Conveyor Monitor",
    "Diesel Generator",
    "Elder Care Monitor",
    "Energy Monitor",
    "EV Charger",
    "Fire & Smoke",
    "Freezer Monitor",
    "Garage Door",
    "Gas Safety",
    "Grain Silo",
    "Greenhouse",
    "HVAC Controller",
    "Hydroponics",
    "Irrigation Scheduler",
    "Lawn Irrigation",
    "Leak Detector",
    "Lift & Elevator",
    "Livestock Barn",
    "Machine Monitor",
    "Mold Risk",
    "Parking & Gate",
    "Pest Trap Monitor",
    "Pool & Spa",
    "Poultry Coop",
    "Pressure Vessel",
    "RFID Gate",
    "Security Monitor",
    "Sensor Network",
    "Server Room",
    "Smart Gate",
    "Smart Lighting",
    "Smart Farm",
    "Smart Home",
    "Solar & Battery",
    "Street Lighting",
    "Tank Level",
    "Telemetry Hub",
    "Vineyard Monitor",
    "Warehouse",
    "Water Meter",
    "Weather Station",
    "Window Blinds",
    "Wine Cellar",
};

static inline int hermes_board_blueprint_index(const char *id) {
    if (!id || !id[0]) return -1;
    for (size_t i = 0; i < 51; i++) {
        char buf[24];
        strncpy_P(buf, HERMES_BP_IDS[i], sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        if (strcmp(buf, id) == 0) return (int)i;
    }
    return -1;
}

#endif
