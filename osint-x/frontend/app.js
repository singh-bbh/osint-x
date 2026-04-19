const state = {
  activeModule: null
};

const labels = {
  email:    '// EMAIL LOOKUP — enter target email address',
  ip:       '// IP LOOKUP — enter target IP address',
  phone:    '// PHONE LOOKUP — enter number with country code (e.g. +91...)',
  whois:    '// DOMAIN CHECK — enter domain (e.g. google.com)',
  username: '// USERNAME CHECK — enter username handle'
};

const placeholders = {
  email:    'user@example.com',
  ip:       '8.8.8.8',
  phone:    '+919876543210',
  whois:    'example.com',
  username: 'john_doe'
};

// Elements
const startBox     = document.getElementById('startBox');
const modulesGrid  = document.getElementById('modulesGrid');
const inputPanel   = document.getElementById('inputPanel');
const resultsPanel = document.getElementById('resultsPanel');
const startBtn     = document.getElementById('startBtn');
const backBtn      = document.getElementById('backBtn');
const resultsBackBtn = document.getElementById('resultsBackBtn');
const queryInput   = document.getElementById('queryInput');
const runBtn       = document.getElementById('runBtn');
const inputLabel   = document.getElementById('inputLabel');
const resultsTitle = document.getElementById('resultsTitle');
const resultsBody  = document.getElementById('resultsBody');

// Clock
function updateClock() {
  document.getElementById('clock').textContent = new Date().toLocaleTimeString();
}
setInterval(updateClock, 1000);
updateClock();

// Start button → show modules
startBtn.addEventListener('click', () => {
  startBox.classList.add('hidden');
  modulesGrid.classList.remove('hidden');
});

// Module card click → show input
document.querySelectorAll('.module-card').forEach(card => {
  card.addEventListener('click', () => {
    state.activeModule = card.dataset.module;
    inputLabel.textContent = labels[state.activeModule];
    queryInput.placeholder = placeholders[state.activeModule];
    queryInput.value = '';
    modulesGrid.classList.add('hidden');
    inputPanel.classList.remove('hidden');
    setTimeout(() => queryInput.focus(), 100);
  });
});

// Back from input → show modules
backBtn.addEventListener('click', () => {
  inputPanel.classList.add('hidden');
  modulesGrid.classList.remove('hidden');
});

// Back from results → show modules
resultsBackBtn.addEventListener('click', () => {
  resultsPanel.classList.add('hidden');
  inputPanel.classList.add('hidden');
  modulesGrid.classList.remove('hidden');
  queryInput.value = '';
});

// Enter key triggers run
queryInput.addEventListener('keydown', e => {
  if (e.key === 'Enter') runBtn.click();
});

// Execute query
runBtn.addEventListener('click', async () => {
  const query = queryInput.value.trim();
  if (!query) return;

  inputPanel.classList.add('hidden');
  resultsPanel.classList.remove('hidden');
  resultsTitle.textContent = `// ${state.activeModule.toUpperCase()} — ${query}`;
  resultsBody.innerHTML = `<div class="loading"><span class="blink">▋</span> SCANNING TARGET...</div>`;

  try {
    const res  = await fetch(`/api/${state.activeModule}/${encodeURIComponent(query)}`);
    const data = await res.json();
    renderResults(data);
  } catch (err) {
    resultsBody.innerHTML = `<div class="error-msg">⚠ CONNECTION ERROR — ${err.message}</div>`;
  }
});

function renderResults(data) {
  if (!data.success) {
    resultsBody.innerHTML = `<div class="error-msg">⚠ ERROR — ${data.error || 'Unknown error'}</div>`;
    return;
  }

  let html = '';

  // Fields
  if (data.fields && Object.keys(data.fields).length > 0) {
    for (const [key, val] of Object.entries(data.fields)) {
      const cls = getValueClass(key, val);
      html += `
        <div class="result-field">
          <span class="field-key">${key}</span>
          <span class="field-val ${cls}">${val}</span>
        </div>`;
    }
  }

  // List items (sections + entries)
  if (data.list && data.list.length > 0) {
    let inSection = false;
    for (const item of data.list) {
      if (item.startsWith('===') && item.endsWith('===')) {
        const title = item.replace(/===/g, '').trim();
        html += `<div class="result-section"><div class="section-title">${title}</div>`;
        inSection = true;
      } else {
        const clean = item.startsWith('• ') ? item.slice(2) : item;
        html += `<div class="result-list-item">${clean}</div>`;
      }
    }
    if (inSection) html += '</div>';
  }

  resultsBody.innerHTML = html || '<div class="loading">No data returned.</div>';
}

function getValueClass(key, val) {
  const k = key.toLowerCase();
  const v = val.toLowerCase();
  if (k.includes('breach') || k.includes('status')) {
    if (v.includes('compromised') || v.includes('⚠')) return 'danger';
    if (v.includes('clean') || v.includes('✓') || v.includes('no breach')) return 'safe';
  }
  if (k.includes('valid') || k.includes('mx')) {
    if (v.includes('valid') || v.includes('likely')) return 'safe';
    if (v.includes('invalid') || v.includes('none')) return 'warning';
  }
  if (k.includes('disposable')) {
    if (v.includes('yes')) return 'danger';
  }
  return '';
}