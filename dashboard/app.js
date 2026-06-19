const API = window.location.origin;

let nodes = [];
let apps = [];
let logs = [];
let blueprints = [];
let selectedBlueprint = null;
let blueprintDetail = null;
let configuringApp = null;
let sseActive = false;
let ws = null;
let authToken = localStorage.getItem('hermes_token') || 'hermes-dev';
let sensorCatalog = [];

const NODE_NAMES = ['STM32', 'ESP32', 'nRF'];

function authHeaders(extra = {}) {
  return { 'X-Hermes-Token': authToken, ...extra };
}

async function ensureAuth() {
  if (authToken) return;
  const res = await fetch(`${API}/api/auth/login`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ username: 'admin' }),
  });
  const data = await res.json();
  authToken = data.token;
  localStorage.setItem('hermes_token', authToken);
}

function connectWebSocket() {
  const proto = location.protocol === 'https:' ? 'wss:' : 'ws:';
  ws = new WebSocket(`${proto}//${location.host}/ws`);
  ws.onmessage = (ev) => {
    try { applyStatus(JSON.parse(ev.data)); } catch (e) { /* ignore */ }
  };
  ws.onclose = () => { ws = null; connectSSE(); };
}

function connectSSE() {
  if (typeof EventSource === 'undefined' || ws) return;
  const es = new EventSource(`${API}/api/events`);
  es.onmessage = (ev) => {
    try {
      applyStatus(JSON.parse(ev.data));
      sseActive = true;
    } catch (e) { /* ignore */ }
  };
  es.onerror = () => { sseActive = false; es.close(); };
}

function applyStatus(status) {
  nodes = status.nodes;
  apps = status.apps;
  if (status.logs?.length) {
    logs = status.logs;
    document.getElementById('log-output').textContent = logs.join('\n');
  }
  renderNodes();
  renderApps();
  updateMetrics(status.metrics, status);
}

async function api(path, opts = {}) {
  const headers = authHeaders(opts.headers || {});
  if (opts.body) headers['Content-Type'] = 'application/json';
  const res = await fetch(`${API}${path}`, { ...opts, headers });
  return res.json();
}

function log(msg) {
  const ts = new Date().toISOString().slice(11, 19);
  logs.unshift(`[${ts}] ${msg}`);
  if (logs.length > 50) logs.pop();
  document.getElementById('log-output').textContent = logs.join('\n');
}

function renderNodes() {
  const grid = document.getElementById('node-grid');
  grid.innerHTML = nodes.map(n => `
    <div class="node-card ${n.online ? 'online' : ''}">
      <h3>${n.name}</h3>
      <div class="stat">
        Node ${n.id} · ${n.node_type}<br>
        CPU: ${Math.round(n.cpu)}%<br>
        RAM: ${n.ram_kb} KB<br>
        Temp: ${Math.round(n.temp_c)}°C<br>
        Status: ${n.online ? 'Online' : 'Offline'}
      </div>
    </div>
  `).join('');
}

function formatMetrics(app) {
  if (!app.metrics || Object.keys(app.metrics).length === 0) {
    return app.cpu_percent ? `${Math.round(app.cpu_percent)}% CPU` : '—';
  }
  return Object.entries(app.metrics).map(([k, v]) => {
    const label = k.replace(/_/g, ' ');
    const val = typeof v === 'boolean' ? (v ? 'on' : 'off') : v;
    return `<span class="metric-val">${label}: ${val}</span>`;
  }).join('<br>');
}

function renderApps() {
  const body = document.getElementById('app-body');
  body.innerHTML = apps.map(a => `
    <tr>
      <td>${a.display_name || a.name}</td>
      <td>${a.blueprint || 'manifest'}</td>
      <td>${NODE_NAMES[a.node_id] ?? a.node_id}</td>
      <td class="state-${a.state}">${a.state}</td>
      <td class="metrics-cell">${formatMetrics(a)}</td>
      <td>
        ${a.blueprint ? `<button class="action" onclick="openConfig('${a.name}')">Configure</button>` : ''}
        <button class="action" onclick="appAction('${a.name}','pause')">Pause</button>
        <button class="action" onclick="appAction('${a.name}','migrate')">Migrate</button>
        <button class="action" onclick="appAction('${a.name}','stop')">Stop</button>
      </td>
    </tr>
  `).join('');
}

function updateMetrics(metrics, status = {}) {
  document.getElementById('m-load').textContent = `${metrics.load_percent}%`;
  document.getElementById('m-power').textContent = `${metrics.power_w} W`;
  document.getElementById('m-temp').textContent = `${Math.round(metrics.temp_c)}°C`;
  document.getElementById('m-apps').textContent = metrics.app_count;
  const badge = document.getElementById('cluster-status');
  const agents = status.agents_connected ?? 0;
  badge.textContent = agents > 0 ? `Cluster Online (${agents} agents)` : 'Cluster Online';
  badge.className = 'badge ' + (metrics.load_percent < 80 ? 'online' : 'warn');
}

async function loadSensors() {
  try {
    const data = await api('/api/sensors');
    sensorCatalog = data.sensors || [];
  } catch (e) {
    log(`Sensor catalog load failed: ${e.message}`);
  }
}

async function loadBlueprints() {
  try {
    const data = await api('/api/blueprints');
    blueprints = data.blueprints || [];
    renderBlueprintList();
  } catch (e) {
    log(`Blueprints load failed: ${e.message}`);
  }
}

function renderBlueprintList() {
  const el = document.getElementById('blueprint-list');
  el.innerHTML = blueprints.map(bp => `
    <button type="button" class="bp-card ${selectedBlueprint === bp.id ? 'active' : ''}"
            onclick="selectBlueprint('${bp.id}')">
      <strong>${bp.name}</strong>
      <span>${bp.description}</span>
    </button>
  `).join('');
}

async function selectBlueprint(id) {
  selectedBlueprint = id;
  renderBlueprintList();
  try {
    blueprintDetail = await api(`/api/blueprints/${id}`);
    if (blueprintDetail.error) {
      log(blueprintDetail.error);
      return;
    }
    document.getElementById('studio-title').textContent = `Configure: ${blueprintDetail.name}`;
    renderStudioFields(blueprintDetail.fields || []);
    document.getElementById('btn-create-app').disabled = false;
  } catch (e) {
    log(`Blueprint load failed: ${e.message}`);
  }
}

function renderStudioFields(fields, values = {}) {
  const container = document.getElementById('studio-fields');
  container.innerHTML = fields.map(f => fieldHtml(f, values[f.key] ?? f.default)).join('');
}

function fieldHtml(field, value) {
  const id = `field-${field.key}`;
  if (field.type === 'toggle') {
    const checked = value ? 'checked' : '';
    return `<div class="field field-toggle">
      <input type="checkbox" id="${id}" data-key="${field.key}" ${checked}>
      <label for="${id}">${field.label}</label>
    </div>`;
  }
  if (field.type === 'sensor_list') {
    const selected = Array.isArray(value) ? value : (value ? [value] : []);
    const opts = sensorCatalog.map(s => {
      const on = selected.includes(s.id) ? 'checked' : '';
      return `<label class="sensor-opt"><input type="checkbox" data-key="${field.key}" data-sensor="${s.id}" ${on}> ${s.name}</label>`;
    }).join('');
    return `<div class="field"><label>${field.label}</label><div class="sensor-grid" id="${id}">${opts || '<span class="hint">Loading sensors…</span>'}</div></div>`;
  }
  if (field.type === 'node') {
    const opts = NODE_NAMES.map((n, i) =>
      `<option value="${i}" ${Number(value) === i ? 'selected' : ''}>${n} (node ${i})</option>`
    ).join('');
    return `<div class="field"><label for="${id}">${field.label}</label>
      <select id="${id}" data-key="${field.key}">${opts}</select></div>`;
  }
  const type = field.type === 'number' ? 'number' : 'text';
  const min = field.min !== undefined ? `min="${field.min}"` : '';
  const max = field.max !== undefined ? `max="${field.max}"` : '';
  return `<div class="field"><label for="${id}">${field.label}</label>
    <input type="${type}" id="${id}" data-key="${field.key}" value="${value ?? ''}" ${min} ${max}></div>`;
}

function readFormFields(containerId) {
  const container = document.getElementById(containerId);
  const config = {};
  const sensorLists = {};
  container.querySelectorAll('[data-key]').forEach(el => {
    const key = el.dataset.key;
    if (el.dataset.sensor) {
      if (!sensorLists[key]) sensorLists[key] = [];
      if (el.checked) sensorLists[key].push(el.dataset.sensor);
      return;
    }
    if (el.type === 'checkbox') config[key] = el.checked;
    else if (el.type === 'number') config[key] = Number(el.value);
    else if (el.tagName === 'SELECT') config[key] = Number(el.value);
    else config[key] = el.value;
  });
  Object.assign(config, sensorLists);
  return config;
}

function instanceNameFromConfig(config, blueprintId) {
  const keys = [
    'zone_name', 'home_name', 'network_name', 'hub_name',
    'greenhouse_name', 'site_name', 'system_name', 'station_name',
    'facility_name', 'building_name', 'barn_name', 'pool_name',
    'charger_name', 'tracker_name', 'machine_name', 'trap_name',
    'gate_name', 'care_site_name', 'lot_name',
  ];
  for (const key of keys) {
    if (config[key]) return String(config[key]).trim();
  }
  return `${blueprintId}-${Date.now().toString(36)}`;
}

async function createApp() {
  if (!selectedBlueprint || !blueprintDetail) return;
  const config = readFormFields('studio-fields');
  const name = instanceNameFromConfig(config, selectedBlueprint);
  try {
    const result = await api('/api/apps/create', {
      method: 'POST',
      body: JSON.stringify({ blueprint: selectedBlueprint, name, config, auto_start: true }),
    });
    if (result.error) log(`Create failed: ${result.error}`);
    else log(`Created & deployed ${result.display_name || result.name} on node ${result.node_id}`);
    await refresh();
  } catch (e) {
    log(`Create error: ${e.message}`);
  }
}

async function openConfig(appName) {
  configuringApp = appName;
  try {
    const data = await api(`/api/apps/${appName}/config`);
    if (data.error) { log(data.error); return; }
    document.getElementById('config-modal-title').textContent = `Configure: ${data.app}`;
    renderConfigForm(data.schema, data.config);
    document.getElementById('config-modal').classList.remove('hidden');
  } catch (e) {
    log(`Config load failed: ${e.message}`);
  }
}

function renderConfigForm(schema, config) {
  const form = document.getElementById('config-form');
  form.innerHTML = schema.map(f => fieldHtml(f, config[f.key] ?? f.default)).join('');
}

async function saveConfig() {
  if (!configuringApp) return;
  const config = readFormFields('config-form');
  try {
    const result = await api(`/api/apps/${configuringApp}/config`, {
      method: 'POST',
      body: JSON.stringify({ config }),
    });
    if (result.error) log(`Save failed: ${result.error}`);
    else log(`Config saved for ${configuringApp}`);
    document.getElementById('config-modal').classList.add('hidden');
    configuringApp = null;
    await refresh();
  } catch (e) {
    log(`Save error: ${e.message}`);
  }
}

async function loadAnalytics() {
  try {
    const data = await api('/api/analytics');
    const pts = data.points || [];
    const el = document.getElementById('analytics-chart');
    if (!el || pts.length === 0) return;
    const loads = pts.map(p => p.load_percent);
    const max = Math.max(...loads, 1);
    el.innerHTML = loads.slice(-40).map(v =>
      `<span class="bar" style="height:${Math.round((v / max) * 100)}%" title="${v}%"></span>`
    ).join('');
  } catch (e) { /* ignore */ }
}

async function refresh() {
  try {
    const status = await api('/api/status');
    applyStatus(status);
    await loadAnalytics();
  } catch (e) {
    log(`API unreachable: ${e.message}`);
  }
}

async function appAction(name, action) {
  try {
    if (action === 'migrate') {
      const app = apps.find(a => a.name === name);
      const dst = (app.node_id + 1) % 3;
      const result = await api(`/api/apps/${name}/migrate`, {
        method: 'POST',
        body: JSON.stringify({ dst_node: dst }),
      });
      if (result.error) log(`Migrate failed: ${result.error}`);
      else log(`Migrated ${name} -> node ${result.app.node_id}`);
    } else {
      const result = await api(`/api/apps/${name}/${action}`, { method: 'POST', body: '{}' });
      if (result.error) log(`${action} failed: ${result.error}`);
      else log(`${action} ${name}: ${result.state}`);
    }
    await refresh();
  } catch (e) {
    log(`Error: ${e.message}`);
  }
}

document.getElementById('btn-create-app').addEventListener('click', createApp);
document.getElementById('btn-config-save').addEventListener('click', saveConfig);
document.getElementById('btn-config-cancel').addEventListener('click', () => {
  document.getElementById('config-modal').classList.add('hidden');
  configuringApp = null;
});

document.getElementById('btn-deploy').addEventListener('click', async () => {
  const path = prompt('Manifest path (advanced):', 'examples/smartfarm/binaries/manifest.json');
  if (!path) return;
  try {
    const result = await api('/api/apps/install', {
      method: 'POST',
      body: JSON.stringify({ manifest_path: path }),
    });
    if (result.error) log(`Deploy failed: ${result.error}`);
    else log(`Deployed ${result.name} v${result.version}`);
    await refresh();
  } catch (e) {
    log(`Deploy error: ${e.message}`);
  }
});

(async () => {
  await ensureAuth();
  await loadSensors();
  await loadBlueprints();
  refresh();
  connectWebSocket();
  setInterval(() => { if (!sseActive && !ws) refresh(); }, 2000);
  setInterval(loadAnalytics, 5000);
  log('HERMES App Studio ready');
})();
