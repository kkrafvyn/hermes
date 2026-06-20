/** Blueprint categories, icons, and favorites for App Studio */
const BLUEPRINT_CATEGORIES = [
  {
    id: 'ag',
    label: 'Agriculture',
    icon: 'ag',
    ids: [
      'smartfarm', 'greenhouse', 'aquaponics', 'irrigation-scheduler', 'livestock-barn',
      'pest-trap', 'smart-gate', 'weather-station', 'hydroponics', 'compost-monitor',
      'vineyard', 'bee-hive', 'poultry-coop', 'grain-silo', 'lawn-irrigation',
    ],
  },
  {
    id: 'home',
    label: 'Home',
    icon: 'home',
    ids: [
      'smarthome', 'smart-lighting', 'air-quality', 'leak-detector', 'pool-spa',
      'fire-smoke', 'elder-care', 'hvac-controller', 'window-blinds', 'garage-door',
      'water-meter', 'gas-safety', 'wine-cellar', 'mold-risk',
    ],
  },
  {
    id: 'energy',
    label: 'Energy',
    icon: 'energy',
    ids: [
      'energy-monitor', 'solar-battery', 'ev-charger', 'asset-tracker', 'parking-gate',
      'battery-room', 'diesel-generator', 'street-lighting',
    ],
  },
  {
    id: 'industrial',
    label: 'Industrial',
    icon: 'industrial',
    ids: [
      'warehouse', 'cold-chain', 'machine-monitor', 'sensor-net', 'telemetry', 'security',
      'conveyor-monitor', 'tank-level', 'pressure-vessel', 'freezer-monitor', 'brewery',
      'server-room', 'lift-elevator', 'rfid-gate',
    ],
  },
];

function blueprintCategory(bpId) {
  for (const cat of BLUEPRINT_CATEGORIES) {
    if (cat.ids.includes(bpId)) return cat;
  }
  return { id: 'other', label: 'Other', icon: 'other', ids: [] };
}

function getFavoriteBlueprints() {
  try {
    return JSON.parse(localStorage.getItem('hermes_favorites') || '[]');
  } catch {
    return [];
  }
}

function setFavoriteBlueprint(id, on) {
  let favs = getFavoriteBlueprints();
  if (on && !favs.includes(id)) favs = [id, ...favs].slice(0, 12);
  else if (!on) favs = favs.filter(x => x !== id);
  localStorage.setItem('hermes_favorites', JSON.stringify(favs));
  return favs;
}

function getRecentBlueprints() {
  try {
    return JSON.parse(localStorage.getItem('hermes_recent_blueprints') || '[]');
  } catch {
    return [];
  }
}

function pushRecentBlueprint(id) {
  let rec = getRecentBlueprints().filter(x => x !== id);
  rec.unshift(id);
  localStorage.setItem('hermes_recent_blueprints', JSON.stringify(rec.slice(0, 8)));
}
