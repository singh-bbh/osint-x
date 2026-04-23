const state = { activeModule: null };

const moduleInfo = {
  ip:       { name: 'IP LOOKUP',     desc: 'Geolocation, ISP, ASN, coordinates',  hint: 'example: 8.8.8.8 or 2001:4860:4860::8888',        placeholder: '8.8.8.8' },
  phone:    { name: 'PHONE INTEL',   desc: 'Carrier, region, line type',           hint: 'always include country code — e.g. +91 for India', placeholder: '+919876543210' },
  email:    { name: 'EMAIL CHECK',   desc: 'MX validation, breach detection',      hint: 'enter full email address with domain',              placeholder: 'user@example.com' },
  username: { name: 'USERNAME SCAN', desc: '150+ platforms cross-checked',         hint: 'letters, numbers, underscores and hyphens only',   placeholder: 'john_doe' },
  whois:    { name: 'WHOIS / DNS',   desc: 'Registrar, expiry, A/MX/TXT records', hint: 'enter root domain only — e.g. google.com',          placeholder: 'example.com' },
};

const loaderMsgs = [
  'SCANNING TARGET...',
  'QUERYING INTEL SOURCES...',
  'GATHERING DATA...',
  'PROCESSING RESULTS...',
  'COMPILING REPORT...'
];

const loaderSubs = [
  'connecting to remote servers...',
  'parsing api response...',
  'cross-referencing databases...',
  'running analysis...',
  'almost done...'
];

// Elements
const welcomeScreen   = document.getElementById('welcomeScreen');
const inputArea       = document.getElementById('inputArea');
const loaderArea      = document.getElementById('loaderArea');
const resultsArea     = document.getElementById('resultsArea');
const startBtn        = document.getElementById('startBtn');
const queryInput      = document.getElementById('queryInput');
const runBtn          = document.getElementById('runBtn');
const clearBtn        = document.getElementById('clearBtn');
const inputModuleBadge= document.getElementById('inputModuleBadge');
const inputAreaDesc   = document.getElementById('inputAreaDesc');
const terminalHint    = document.getElementById('terminalHint');
const topbarPath      = document.getElementById('topbarPath');
const resultsBreadcrumb=document.getElementById('resultsBreadcrumb');
const outputBody      = document.getElementById('outputBody');
const outputMeta      = document.getElementById('outputMeta');
const loaderMsg       = document.getElementById('loaderMsg');
const loaderSub       = document.getElementById('loaderSub');
const progressBar     = document.getElementById('progressBar');

// Clock
function updateClock() {
  const now = new Date();
  const el  = document.getElementById('clock');
  const ds  = document.getElementById('dateStr');
  if (el) el.textContent = now.toLocaleTimeString();
  if (ds) ds.textContent = now.toLocaleDateString('en-GB');
}
setInterval(updateClock, 1000);
updateClock();

// Matrix
function initMatrix() {
  const canvas = document.getElementById('matrix');
  if (!canvas) return;
  const ctx = canvas.getContext('2d');
  canvas.width  = window.innerWidth;
  canvas.height = window.innerHeight;
  const chars = '01アイウエオカキクケコ'.split('');
  const cols  = Math.floor(canvas.width / 12);
  const drops = Array(cols).fill(1);
  function draw() {
    ctx.fillStyle = 'rgba(0,0,0,0.05)';
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    ctx.fillStyle = '#00ff41';
    ctx.font = '12px Share Tech Mono';
    for (let i = 0; i < drops.length; i++) {
      ctx.fillText(chars[Math.floor(Math.random() * chars.length)],
                   i * 12, drops[i] * 12);
      if (drops[i] * 12 > canvas.height && Math.random() > 0.975) drops[i] = 0;
      drops[i]++;
    }
  }
  setInterval(draw, 60);
  window.addEventListener('resize', () => {
    canvas.width  = window.innerWidth;
    canvas.height = window.innerHeight;
  });
}
initMatrix();

// Boot sequence
const bootLines = [
  { text: '> BIOS v2.1 — osint-x system boot', delay: 0 },
  { text: '> loading modules...', delay: 200 },
  { text: '> [OK] IPLookup ready', delay: 380, cls: 'ok' },
  { text: '> [OK] PhoneLookup ready — key pool active', delay: 520, cls: 'ok' },
  { text: '> [OK] EmailLookup ready — leakcheck connected', delay: 660, cls: 'ok' },
  { text: '> [OK] UsernameLookup — 150 platforms indexed', delay: 800, cls: 'ok' },
  { text: '> [OK] WHOISLookup ready — google dns active', delay: 940, cls: 'ok' },
  { text: '> [!!] select a module from the sidebar', delay: 1100, cls: 'warn' },
];

function runBoot() {
  const container = document.getElementById('welcomeBoot');
  bootLines.forEach(line => {
    setTimeout(() => {
      const div = document.createElement('div');
      div.className = 'boot-line' + (line.cls ? ' ' + line.cls : '');
      div.textContent = line.text;
      container.appendChild(div);
    }, line.delay);
  });
  const last = bootLines[bootLines.length - 1].delay;
  setTimeout(() => {
    startBtn.classList.remove('hidden');
  }, last + 400);
}
runBoot();

startBtn.addEventListener('click', () => {
  welcomeScreen.classList.add('hidden');
  // activate first nav item
  const first = document.querySelector('.nav-item');
  if (first) first.click();
});

// Sidebar nav
document.querySelectorAll('.nav-item').forEach(item => {
  item.addEventListener('click', () => {
    document.querySelectorAll('.nav-item').forEach(n => n.classList.remove('active'));
    item.classList.add('active');

    state.activeModule = item.dataset.module;
    const info = moduleInfo[state.activeModule];

    inputModuleBadge.textContent  = info.name;
    inputAreaDesc.textContent     = info.desc;
    terminalHint.textContent      = info.hint;
    queryInput.placeholder        = info.placeholder;
    queryInput.value              = '';
    topbarPath.textContent        = `root@osint-x:~$ osint-x ${state.activeModule}`;

    // Hide others show input
    welcomeScreen.classList.add('hidden');
    loaderArea.classList.add('hidden');
    resultsArea.classList.add('hidden');
    inputArea.classList.remove('hidden');
    hideMap();

    setTimeout(() => queryInput.focus(), 100);
  });
});

// Clear
clearBtn.addEventListener('click', () => {
  resultsArea.classList.add('hidden');
  inputArea.classList.remove('hidden');
  queryInput.value = '';
  queryInput.focus();
  hideMap();
});

// Enter key
queryInput.addEventListener('keydown', e => {
  if (e.key === 'Enter') runBtn.click();
});

// Execute
runBtn.addEventListener('click', async () => {
  const query = queryInput.value.trim();
  if (!query || !state.activeModule) return;

  inputArea.classList.add('hidden');
  resultsArea.classList.add('hidden');
  loaderArea.classList.remove('hidden');
  hideMap();

  loaderMsg.textContent = loaderMsgs[0];
  loaderSub.textContent = loaderSubs[0];
  progressBar.style.width = '0%';

  let msgIdx = 0, subIdx = 0, progress = 0;
  const t1 = setInterval(() => {
    msgIdx = Math.min(msgIdx + 1, loaderMsgs.length - 1);
    loaderMsg.textContent = loaderMsgs[msgIdx];
  }, 900);
  const t2 = setInterval(() => {
    subIdx = (subIdx + 1) % loaderSubs.length;
    loaderSub.textContent = loaderSubs[subIdx];
  }, 600);
  const t3 = setInterval(() => {
    progress = Math.min(progress + Math.random() * 14, 88);
    progressBar.style.width = Math.round(progress) + '%';
  }, 250);

  try {
    const res  = await fetch(`/api/${state.activeModule}/${encodeURIComponent(query)}`);
    const data = await res.json();

    clearInterval(t1); clearInterval(t2); clearInterval(t3);
    progressBar.style.width = '100%';

    setTimeout(() => {
      loaderArea.classList.add('hidden');
      resultsArea.classList.remove('hidden');
      topbarPath.textContent = `root@osint-x:~$ osint-x ${state.activeModule} ${query}`;
      resultsBreadcrumb.textContent = `root@osint-x:~$ osint-x ${state.activeModule} ${query}`;
      renderResults(data, query);
    }, 300);

  } catch (err) {
    clearInterval(t1); clearInterval(t2); clearInterval(t3);
    loaderArea.classList.add('hidden');
    resultsArea.classList.remove('hidden');
    outputBody.innerHTML =
      `<div class="error-msg">CONNECTION ERROR — ${err.message}<br/>
       make sure ./osint-web is running on port 8080</div>`;
  }
});

function renderResults(data, query) {
  if (!data.success) {
    outputMeta.textContent = 'status: error';
    outputBody.innerHTML =
      `<div class="error-msg">ERROR — ${data.error || 'Unknown error'}</div>`;
    return;
  }

  // Map
  if (data.hasCoords && data.latitude && data.longitude) {
    showMap(parseFloat(data.latitude), parseFloat(data.longitude), query);
    document.getElementById('mapCoords').textContent =
      `${parseFloat(data.latitude).toFixed(4)}, ${parseFloat(data.longitude).toFixed(4)}`;
  }

  let fieldCount = 0;
  let html = '';

  // Fields
  if (data.fields) {
    for (const [key, val] of Object.entries(data.fields)) {
      if (!val || val === 'N/A' || val === '') continue;
      const cls = getValueClass(key, val);
      let valHtml = `<span class="field-val ${cls}">${esc(val)}`;
      if (cls === 'danger' && key.toLowerCase().includes('breach'))
        valHtml += `<span class="tag-compromised">COMPROMISED</span>`;
      if (cls === 'safe' && key.toLowerCase().includes('breach'))
        valHtml += `<span class="tag-clean">CLEAN</span>`;
      valHtml += '</span>';
      html += `<div class="result-field">
        <span class="field-key">${esc(key)}</span>${valHtml}
      </div>`;
      fieldCount++;
    }
  }

  // List
  if (data.list && data.list.length > 0) {
    let inSection = false;
    for (const item of data.list) {
      if (!item.trim()) { html += '<div style="height:5px"></div>'; continue; }

      if (item.startsWith('===') && item.endsWith('===')) {
        if (inSection) html += '</div>';
        const title = item.replace(/===/g, '').trim();
        html += `<div class="result-section">
          <div class="section-title">${esc(title)}</div>`;
        inSection = true;
        continue;
      }

      const clean = item.startsWith('• ') ? item.slice(2) : item;
      let cls = '';
      if (item.startsWith('[FOUND]'))                       cls = 'found';
      else if (item.includes('CRITICAL') ||
               item.toLowerCase().includes('breach #'))    cls = 'breach';
      else if (item.startsWith('A:') || item.startsWith('MX:') ||
               item.startsWith('NS:') || item.startsWith('TXT:')) cls = 'dns';
      else if (item.toLowerCase().includes('guideline') ||
               item.toLowerCase().includes('password') ||
               item.toLowerCase().includes('2fa') ||
               item.startsWith('  '))                      cls = 'guide';

      let content = esc(clean);
      if (cls === 'found' && item.includes('→')) {
        const parts = clean.replace('[FOUND] ', '').split(' → ');
        if (parts[1])
          content = `+ ${esc(parts[0])}&nbsp;<a href="${esc(parts[1])}"
            target="_blank"
            style="color:var(--green)">${esc(parts[1])}</a>`;
      }

      html += `<div class="result-list-item ${cls}">${content}</div>`;
    }
    if (inSection) html += '</div>';
  }

  outputMeta.textContent = `${fieldCount} fields returned`;
  outputBody.innerHTML = html ||
    '<div style="padding:1rem;color:var(--text-muted)">no data returned.</div>';
}

function getValueClass(key, val) {
  const k = key.toLowerCase(), v = val.toLowerCase();
  if ((k.includes('breach') || k.includes('status')) && v.includes('compromised')) return 'danger';
  if ((k.includes('breach') || k.includes('status')) && (v.includes('clean') || v.includes('no breach'))) return 'safe';
  if ((k.includes('valid') || k.includes('mx')) && (v.includes('valid') || v.includes('likely'))) return 'safe';
  if ((k.includes('valid') || k.includes('mx')) && (v.includes('invalid') || v.includes('none'))) return 'warning';
  if (k.includes('disposable') && v.includes('yes')) return 'danger';
  if (k.includes('spam')) return 'danger';
  return '';
}

// Map
let leafletMap = null;

function showMap(lat, lon, label) {
  document.getElementById('mapContainer').classList.remove('hidden');
  if (leafletMap) { leafletMap.remove(); leafletMap = null; }
  leafletMap = L.map('map').setView([lat, lon], 10);
  L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: 'OSM', maxZoom: 18
  }).addTo(leafletMap);
  const icon = L.divIcon({
    html: `<div style="width:12px;height:12px;background:#00ff41;
           border:2px solid #000;border-radius:50%;
           box-shadow:0 0 10px #00ff41;"></div>`,
    iconSize: [12,12], iconAnchor: [6,6], className: ''
  });
  L.marker([lat, lon], { icon })
    .addTo(leafletMap)
    .bindPopup(`<b>${label}</b><br>${lat.toFixed(4)}, ${lon.toFixed(4)}`)
    .openPopup();
}

function hideMap() {
  document.getElementById('mapContainer').classList.add('hidden');
  if (leafletMap) { leafletMap.remove(); leafletMap = null; }
}

function esc(str) {
  return String(str)
    .replace(/&/g,'&amp;').replace(/</g,'&lt;')
    .replace(/>/g,'&gt;').replace(/"/g,'&quot;');
}