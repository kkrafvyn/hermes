const API = window.location.origin;

let nodes = [];
let apps = [];
let logs = [];
let blueprints = [];
let selectedBlueprint = null;
let blueprintDetail = null;
let configuringApp = null;
let boardMode = false;
let primaryNode = null;
let sseActive = false;
let ws = null;
let authToken = localStorage.getItem('hermes_token') || 'hermes-dev';
let sensorCatalog = [];

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

function nodeLabel(nodeId) {
  const n = nodes.find(x => x.id === nodeId);
  return n ? `${n.name} (node ${n.id})` : `node ${nodeId}`;
}

function mcuFamily(nodeType) {
  const t = (nodeType || '').toLowerCase();
  if (t.includes('esp')) return 'espressif';
  if (t.includes('stm')) return 'stm32';
  if (t.includes('nrf')) return 'nordic';
  if (t.includes('avr') || t.includes('atmega')) return 'avr';
  if (t.includes('rp')) return 'rpi';
  return 'mcu';
}

function applyStatus(status) {
  nodes = status.nodes || [];
  apps = status.apps || [];
  boardMode = Boolean(status.board_mode);
  primaryNode = status.primary_node || null;
  window._meshStatus = status.mesh || null;
  if (status.logs?.length) {
    logs = status.logs;
    document.getElementById('log-output').textContent = logs.join('\n');
  }
  renderBoard();
  renderNodes();
  renderApps();
  updateMetrics(status.metrics, status);
  if (blueprintDetail) refreshStudioFields();
}

function slugifyName(value) {
  return String(value || '').trim().toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-+|-+$/g, '');
}

let toastTimer = null;
function toast(message, kind = 'success') {
  const el = document.getElementById('toast');
  if (!el) return;
  el.textContent = message;
  el.className = `toast ${kind}`;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => el.classList.add('hidden'), 3200);
}

async function api(path, opts = {}) {
  const headers = authHeaders(opts.headers || {});
  if (opts.body) headers['Content-Type'] = 'application/json';
  const res = await fetch(`${API}${path}`, { ...opts, headers });
  let data = {};
  try {
    data = await res.json();
  } catch (e) {
    throw new Error(`Invalid response from ${path}`);
  }
  if (!res.ok && !data.error) {
    data.error = `Request failed (${res.status})`;
  }
  return data;
}

function log(msg) {
  const ts = new Date().toISOString().slice(11, 19);
  logs.unshift(`[${ts}] ${msg}`);
  if (logs.length > 50) logs.pop();
  document.getElementById('log-output').textContent = logs.join('\n');
}

function renderBoard() {
  const hero = document.getElementById('board-hero');
  const hint = document.getElementById('no-board-hint');
  const heading = document.getElementById('nodes-heading');
  const subtitle = document.getElementById('header-subtitle');
  const status = { nodes, mesh: window._meshStatus };

  if (!nodes.length) {
    hero.classList.add('hidden');
    hint.classList.remove('hidden');
    heading.textContent = 'Your Board';
    if (subtitle) subtitle.textContent = 'Waiting for hardware…';
    return;
  }

  hint.classList.add('hidden');
  if (boardMode && primaryNode) {
    hero.classList.remove('hidden');
    hero.className = `board-hero family-${mcuFamily(primaryNode.node_type)}`;
    document.getElementById('board-name').textContent = primaryNode.name;
    document.getElementById('board-type').textContent = primaryNode.node_type;
    document.getElementById('board-cpu').textContent = `${Math.round(primaryNode.cpu)}%`;
    document.getElementById('board-ram').textContent = `${primaryNode.ram_kb} KB`;
    document.getElementById('board-temp').textContent = `${Math.round(primaryNode.temp_c)}°C`;
    document.getElementById('board-power').textContent = `${(primaryNode.power_mw / 1000).toFixed(2)} W`;
    document.getElementById('board-status').textContent = primaryNode.online ? 'Online' : 'Offline';
    document.getElementById('board-status').className = primaryNode.online ? 'status-chip online' : 'status-chip';
    heading.textContent = 'Board Dashboard';
    if (subtitle) subtitle.textContent = `${primaryNode.name} · ${primaryNode.node_type}`;
    const meshEl = document.getElementById('mesh-panel');
    if (meshEl && status.mesh?.nodes?.length) {
      meshEl.classList.remove('hidden');
      meshEl.innerHTML = '<h3 class="group-title">Remote sensor nodes</h3>' + status.mesh.nodes.map(m =>
        `<div class="view-lead">Node ${m.node_id} ${m.name || ''} — ${m.online ? 'online' : 'offline'} ${m.sensors || ''}</div>`
      ).join('');
    } else if (meshEl) meshEl.classList.add('hidden');
  } else {
    hero.classList.add('hidden');
    heading.textContent = nodes.length === 1 ? 'Your Board' : 'Connected Boards';
    if (subtitle) subtitle.textContent = `${nodes.length} board${nodes.length === 1 ? '' : 's'} connected`;
  }
}

function renderNodes() {
  const grid = document.getElementById('node-grid');
  const section = document.getElementById('node-grid-section');
  if (boardMode) {
    section.classList.add('hidden');
    return;
  }
  section.classList.remove('hidden');
  grid.innerHTML = nodes.map(n => `
    <div class="node-card ${n.online ? 'online' : ''} family-${mcuFamily(n.node_type)}">
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
  const list = document.getElementById('app-list');
  const section = document.getElementById('apps-section');
  if (!list) return;
  if (!nodes.length) {
    if (section) section.classList.add('hidden');
    list.innerHTML = '<div class="app-empty">Connect a board to deploy applications.</div>';
    return;
  }
  if (section) section.classList.remove('hidden');
  const visibleApps = boardMode && primaryNode
    ? apps.filter(a => a.node_id === primaryNode.id)
    : apps;
  list.innerHTML = visibleApps.length ? visibleApps.map(a => `
    <article class="app-card">
      <div class="app-card-main">
        <h3>${a.display_name || a.name}</h3>
        <p>${a.blueprint || 'manifest'} · <span class="state-${a.state}">${a.state}</span></p>
      </div>
      <div class="app-card-meta">
        <div>${nodeLabel(a.node_id)}</div>
        <div class="metrics-cell">${formatMetrics(a)}</div>
      </div>
      <div class="app-card-actions">
        ${a.blueprint ? `<button class="action" onclick="openConfig('${a.name}')">Customize</button>` : ''}
        <button class="action" onclick="appAction('${a.name}','pause')">Pause</button>
        ${nodes.length > 1 ? `<button class="action" onclick="appAction('${a.name}','migrate')">Migrate</button>` : ''}
        <button class="action" onclick="appAction('${a.name}','stop')">Stop</button>
      </div>
    </article>
  `).join('') : `<div class="app-empty">No apps deployed yet. Use App Studio above to create one.</div>`;
}

function updateMetrics(metrics, status = {}) {
  const label = document.getElementById('metrics-heading');
  label.textContent = boardMode ? 'Board Metrics' : 'Cluster Metrics';
  document.getElementById('m-load').textContent = `${metrics.load_percent}%`;
  document.getElementById('m-power').textContent = `${metrics.power_w} W`;
  document.getElementById('m-temp').textContent = `${Math.round(metrics.temp_c)}°C`;
  document.getElementById('m-apps').textContent = metrics.app_count;
  const badge = document.getElementById('cluster-status');
  const agents = status.agents_connected ?? 0;
  if (!agents) {
    badge.textContent = 'No board connected';
    badge.className = 'status-chip warn';
  } else if (boardMode && primaryNode) {
    badge.textContent = `${primaryNode.name} online`;
    badge.className = 'status-chip online';
  } else {
    badge.textContent = `${agents} board${agents === 1 ? '' : 's'} connected`;
    badge.className = 'status-chip ' + (metrics.load_percent < 80 ? 'online' : 'warn');
  }
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
      toast(blueprintDetail.error, 'error');
      return;
    }
    document.getElementById('studio-title').textContent = blueprintDetail.name;
    const hint = document.getElementById('studio-hint');
    if (hint) hint.textContent = blueprintDetail.description || 'Customize fields, then deploy.';
    renderStudioFields(blueprintDetail.fields || []);
    renderMetricsPreview(blueprintDetail.metrics || []);
    document.getElementById('btn-create-app').disabled = !nodes.length;
    if (!nodes.length) toast('Connect a board before deploying apps', 'error');
  } catch (e) {
    toast(`Blueprint load failed: ${e.message}`, 'error');
  }
}

function refreshStudioFields() {
  if (!blueprintDetail) return;
  const container = document.getElementById('studio-fields');
  const current = readFormFields('studio-fields');
  renderStudioFields(blueprintDetail.fields || [], current);
  document.getElementById('btn-create-app').disabled = !nodes.length;
}

function renderMetricsPreview(metrics) {
  const el = document.getElementById('studio-metrics');
  if (!metrics.length) {
    el.innerHTML = '';
    el.classList.add('hidden');
    return;
  }
  el.classList.remove('hidden');
  el.innerHTML = `<p class="hint">Live metrics tracked by this app:</p>` +
    metrics.map(m => `<span class="metric-chip">${m.label}${m.unit ? ` (${m.unit})` : ''}</span>`).join('');
}

function renderStudioFields(fields, values = {}) {
  const hideNodePicker = nodes.length <= 1;
  const filtered = hideNodePicker
    ? fields.filter(f => f.type !== 'node')
    : fields;
  const defaults = { ...values };
  if (hideNodePicker && nodes.length === 1) {
    defaults.target_node = nodes[0].id;
  }
  const container = document.getElementById('studio-fields');
  const groups = {};
  filtered.forEach(f => {
    const g = f.group || 'App settings';
    if (!groups[g]) groups[g] = [];
    groups[g].push(f);
  });
  let html = '';
  for (const [group, items] of Object.entries(groups)) {
    html += `<div class="field-group"><h4 class="group-title">${group}</h4>`;
    html += items.map(f => fieldHtml(f, defaults[f.key] ?? f.default)).join('');
    html += '</div>';
  }
  const mesh = window._meshStatus;
  if (mesh?.nodes?.length) {
    html += `<div class="field-group"><h4 class="group-title">Live mesh nodes</h4><div class="mesh-live">`;
    html += mesh.nodes.map(m =>
      `<div class="mesh-live-row"><strong>Node ${m.node_id}</strong> ${m.name || ''} · ${m.online ? 'online' : 'offline'}<span class="hint">${m.sensors || ''}</span></div>`
    ).join('');
    html += '</div></div>';
  }
  container.innerHTML = html;
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
    const opts = nodes.map(n =>
      `<option value="${n.id}" ${Number(value) === n.id ? 'selected' : ''}>${n.name} (node ${n.id})</option>`
    ).join('');
    return `<div class="field"><label for="${id}">${field.label}</label>
      <select id="${id}" data-key="${field.key}">${opts || '<option value="">No board connected</option>'}</select></div>`;
  }
  if (field.type === 'select') {
    const opts = (field.options || []).map(o => {
      const val = typeof o === 'object' ? o.value : o;
      const label = typeof o === 'object' ? (o.label || o.value) : o;
      return `<option value="${val}" ${String(value) === String(val) ? 'selected' : ''}>${label}</option>`;
    }).join('');
    return `<div class="field"><label for="${id}">${field.label}</label>
      <select id="${id}" data-key="${field.key}">${opts}</select></div>`;
  }
  if (field.type === 'textarea') {
    return `<div class="field"><label for="${id}">${field.label}</label>
      <textarea id="${id}" data-key="${field.key}" rows="3">${value ?? ''}</textarea></div>`;
  }
  const type = field.type === 'number' ? 'number' : 'text';
  const min = field.min !== undefined ? `min="${field.min}"` : '';
  const max = field.max !== undefined ? `max="${field.max}"` : '';
  const req = field.required ? 'required' : '';
  return `<div class="field"><label for="${id}">${field.label}</label>
    <input type="${type}" id="${id}" data-key="${field.key}" value="${value ?? ''}" ${min} ${max} ${req}></div>`;
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
    else if (el.tagName === 'SELECT') {
      const raw = el.value;
      config[key] = /^\d+$/.test(raw) ? Number(raw) : raw;
    }
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
  if (!selectedBlueprint || !blueprintDetail) {
    toast('Select a blueprint first', 'error');
    return;
  }
  if (!nodes.length) {
    toast('Connect a board first (flash + serial bridge)', 'error');
    return;
  }
  const config = readFormFields('studio-fields');
  if (nodes.length === 1) config.target_node = nodes[0].id;
  const displayName = instanceNameFromConfig(config, selectedBlueprint);
  const name = slugifyName(displayName) || `${selectedBlueprint}-${Date.now().toString(36)}`;
  const btn = document.getElementById('btn-create-app');
  btn.disabled = true;
  try {
    const result = await api('/api/apps/create', {
      method: 'POST',
      body: JSON.stringify({
        blueprint: selectedBlueprint,
        name,
        config,
        node_id: config.target_node,
        auto_start: true,
      }),
    });
    if (result.error) {
      toast(result.error, 'error');
      log(`Create failed: ${result.error}`);
    } else {
      toast(`Deployed ${result.display_name || result.name}`, 'success');
      log(`Created & deployed ${result.display_name || result.name} on ${nodeLabel(result.node_id)}`);
    }
    await refresh();
  } catch (e) {
    toast(`Deploy failed: ${e.message}`, 'error');
    log(`Create error: ${e.message}`);
  } finally {
    btn.disabled = !nodes.length;
  }
}

async function openConfig(appName) {
  configuringApp = appName;
  try {
    const data = await api(`/api/apps/${appName}/config`);
    if (data.error) { log(data.error); return; }
    document.getElementById('config-modal-title').textContent = `Customize: ${data.app}`;
    renderConfigForm(data.schema, data.config);
    document.getElementById('config-modal').classList.remove('hidden');
  } catch (e) {
    log(`Config load failed: ${e.message}`);
  }
}

function renderConfigForm(schema, config) {
  const hideNodePicker = nodes.length <= 1;
  const filtered = hideNodePicker ? schema.filter(f => f.type !== 'node') : schema;
  const form = document.getElementById('config-form');
  form.innerHTML = filtered.map(f => fieldHtml(f, config[f.key] ?? f.default)).join('');
}

async function saveConfig() {
  if (!configuringApp) return;
  const config = readFormFields('config-form');
  if (nodes.length === 1) config.target_node = nodes[0].id;
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
      const others = nodes.filter(n => n.id !== app.node_id && n.online);
      if (!others.length) { log('No other board to migrate to'); return; }
      const dst = others[0].id;
      const result = await api(`/api/apps/${name}/migrate`, {
        method: 'POST',
        body: JSON.stringify({ dst_node: dst }),
      });
      if (result.error) log(`Migrate failed: ${result.error}`);
      else log(`Migrated ${name} -> ${nodeLabel(result.app.node_id)}`);
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

function initNav() {
  const items = document.querySelectorAll('.nav-item');
  const views = document.querySelectorAll('.view');
  items.forEach(btn => {
    btn.addEventListener('click', () => {
      items.forEach(b => b.classList.remove('active'));
      views.forEach(v => v.classList.remove('active'));
      btn.classList.add('active');
      const view = document.getElementById(`view-${btn.dataset.view}`);
      if (view) view.classList.add('active');
    });
  });
}

(async () => {
  initHermesThemePicker();
  initNav();
  await ensureAuth();
  await loadSensors();
  await loadBlueprints();
  refresh();
  connectWebSocket();
  setInterval(() => { if (!sseActive && !ws) refresh(); }, 2000);
  setInterval(loadAnalytics, 5000);
  log('HERMES App Studio ready — connect your board to begin');
})();
