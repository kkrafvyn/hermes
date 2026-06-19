const HERMES_THEMES = [
  { id: 'apple', label: 'Apple' },
  { id: 'midnight', label: 'Midnight' },
  { id: 'ocean', label: 'Ocean' },
  { id: 'sunset', label: 'Sunset' },
  { id: 'forest', label: 'Forest' },
  { id: 'slate', label: 'Slate' },
  { id: 'light', label: 'Light' },
  { id: 'contrast', label: 'High contrast' },
];

function getHermesTheme() {
  const saved = localStorage.getItem('hermes_theme');
  if (saved && HERMES_THEMES.some(t => t.id === saved)) return saved;
  return 'apple';
}

function applyHermesTheme(themeId) {
  const id = HERMES_THEMES.some(t => t.id === themeId) ? themeId : 'apple';
  document.documentElement.setAttribute('data-theme', id);
  localStorage.setItem('hermes_theme', id);
  const sel = document.getElementById('theme-select');
  if (sel && sel.value !== id) sel.value = id;
}

function initHermesThemePicker() {
  applyHermesTheme(getHermesTheme());
  const sel = document.getElementById('theme-select');
  if (!sel) return;
  sel.innerHTML = HERMES_THEMES.map(t =>
    `<option value="${t.id}">${t.label}</option>`
  ).join('');
  sel.value = getHermesTheme();
  sel.addEventListener('change', () => applyHermesTheme(sel.value));
}
