// =========================================================
// BACKEND CONFIG
// =========================================================

const BACKEND = {
  enabled: false,
  baseUrl: 'http://localhost:8081',
  pollInterval: 350,
  endpoints: {
    state:  '/api/state',
    config: '/api/config',
    pause:  '/api/pause',
    reset:  '/api/reset',
    start:  '/api/start'
  }
};

// =========================================================
// HELPERS
// =========================================================

function randomBetween(min, max){
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

// =========================================================
// CIRCULAR ARRAY RING RENDERER
// =========================================================

const RING = {
  MAX_SLOTS: 100,
  canvas: null,
  ctx: null,

  init() {
    this.canvas = document.getElementById('ring-canvas');
    this.ctx    = this.canvas.getContext('2d');
  },

  // Draw the N-slot circular array ring (N = maxSlots, dynamic)
  draw(queueSize, maxSlots, frontIdx, servers) {
    const c   = this.ctx;
    const W   = this.canvas.width;
    const H   = this.canvas.height;
    const cx  = W / 2;
    const cy  = H / 2;
    const N   = maxSlots;
    // Scale dot size down for large queues so they still fit on the ring
    const r   = N <= 100 ? 10 : N <= 200 ? 7 : N <= 500 ? 5 : 3;
    const R   = 210;          // ring radius

    c.clearRect(0, 0, W, H);

    // Glow ring track
    c.beginPath();
    c.arc(cx, cy, R, 0, Math.PI * 2);
    c.strokeStyle = 'rgba(100,120,255,0.15)';
    c.lineWidth   = 24;
    c.stroke();

    c.beginPath();
    c.arc(cx, cy, R, 0, Math.PI * 2);
    c.strokeStyle = 'rgba(100,120,255,0.08)';
    c.lineWidth   = 36;
    c.stroke();

    // Number of servers for colour assignment
    const numServers = servers ? servers.length : 1;

    // Server colours (hue per server)
    const serverHues = [210, 45, 155, 0, 280, 180, 330, 90, 30, 260];

    for (let i = 0; i < N; i++) {
      const angle = (i / N) * Math.PI * 2 - Math.PI / 2;
      const x = cx + R * Math.cos(angle);
      const y = cy + R * Math.sin(angle);

      const isOccupied = i < queueSize;
      const isFront    = isOccupied && i === 0;
      const isRear     = isOccupied && i === queueSize - 1 && queueSize > 0;

      // Determine which server "owns" this slot
      let ownerHue = 210;
      if (isOccupied && numServers > 1) {
        // Slot i (0-indexed) → customer position (i+1) → server index = i % numServers
        const serverIdx = i % numServers;
        ownerHue = serverHues[serverIdx % serverHues.length];
      }

      // Glow for occupied
      if (isOccupied) {
        c.beginPath();
        c.arc(x, y, r + 5, 0, Math.PI * 2);
        c.fillStyle = isFront
          ? 'rgba(80,230,120,0.25)'
          : `hsla(${ownerHue},80%,65%,0.18)`;
        c.fill();
      }

      // Main dot
      c.beginPath();
      c.arc(x, y, r, 0, Math.PI * 2);

      if (isFront) {
        // Front = green
        c.fillStyle   = '#50e678';
        c.shadowColor = '#50e678';
        c.shadowBlur  = 14;
      } else if (isRear) {
        // Rear = amber
        c.fillStyle   = '#f7b731';
        c.shadowColor = '#f7b731';
        c.shadowBlur  = 14;
      } else if (isOccupied) {
        c.fillStyle   = `hsl(${ownerHue},75%,62%)`;
        c.shadowColor = `hsl(${ownerHue},80%,55%)`;
        c.shadowBlur  = 8;
      } else {
        c.fillStyle   = 'rgba(255,255,255,0.08)';
        c.shadowBlur  = 0;
      }

      c.fill();
      c.shadowBlur = 0;

      // Slot index label (every 10th)
      if (i % 10 === 0) {
        c.fillStyle  = 'rgba(180,190,230,0.6)';
        c.font       = '9px Inter, sans-serif';
        c.textAlign  = 'center';
        c.textBaseline = 'middle';
        const lx = cx + (R + 22) * Math.cos(angle);
        const ly = cy + (R + 22) * Math.sin(angle);
        c.fillText(i, lx, ly);
      }
    }

    // FIFO direction arrow (curved label at top)
    c.font       = 'bold 11px Inter, sans-serif';
    c.fillStyle  = 'rgba(150,170,255,0.7)';
    c.textAlign  = 'center';
    c.textBaseline = 'middle';
    c.fillText('← FIFO', cx, cy - R - 38);

    // Center labels updated via DOM
    document.getElementById('ring-label-queue').textContent = queueSize;
    document.getElementById('ring-label-cap').textContent   = `/ ${maxSlots} slots`;
  }
};

// =========================================================
// LOCAL SIMULATION
// =========================================================

// maxCustomers is now a live property on `local` — driven by the slider

const local = {
  tick: 0,
  served: 0,
  totalWait: 0,
  peakQueueLength: 0,
  totalArrived: 0,
  turnedAway: 0,

  // Circular array: size is DYNAMIC — driven by the Customer Count slider
  maxCustomers: 100,
  circularArray: new Array(100).fill(null),
  front: 0,
  rear:  0,     // points to NEXT empty slot
  queueSize: 0,

  servers: [],
  nextId: 1,
  running: false,

  numServers:  4,
  arrivalMin:  2,
  arrivalMax:  6,
  serviceMin:  4,
  serviceMax:  10,
  nextArrivalTick: 0,
  nextServerHint: 0,   // round-robin pointer

  // ── Queue primitives ──────────────────────────────────────
  enqueue(customer) {
    if (this.queueSize >= this.maxCustomers) return false;
    this.circularArray[this.rear] = customer;
    this.rear = (this.rear + 1) % this.maxCustomers;
    this.queueSize++;
    return true;
  },

  dequeue() {
    if (this.queueSize === 0) return null;
    const c = this.circularArray[this.front];
    this.circularArray[this.front] = null;
    this.front = (this.front + 1) % this.maxCustomers;
    this.queueSize--;
    return c;
  },

  // ── Resize circular array (preserves current queue) ───────
  resizeQueue(newMax) {
    // Drain existing customers into a temp array (FIFO order)
    const existing = [];
    for (let i = 0; i < this.queueSize; i++) {
      const idx = (this.front + i) % this.maxCustomers;
      existing.push(this.circularArray[idx]);
    }
    // Rebuild with new capacity
    this.maxCustomers  = newMax;
    this.circularArray = new Array(newMax).fill(null);
    this.front = 0;
    this.queueSize = 0;
    this.rear = 0;
    // Re-enqueue up to new capacity
    for (const c of existing) {
      this.enqueue(c);
    }
  },

  // ── Reset ─────────────────────────────────────────────────
  reset() {
    this.tick            = 0;
    this.served          = 0;
    this.totalWait       = 0;
    this.peakQueueLength = 0;
    this.totalArrived    = 0;
    this.turnedAway      = 0;
    this.nextId          = 1;

    this.circularArray = new Array(this.maxCustomers).fill(null);
    this.front = 0;
    this.rear  = 0;
    this.queueSize = 0;

    this.nextArrivalTick = randomBetween(this.arrivalMin, this.arrivalMax);
    this.nextServerHint  = 0;
    this._buildServers();
  },

  _buildServers() {
    this.servers = Array.from({ length: this.numServers }, (_, i) => ({
      id:         i,
      busy:       false,
      remaining:  0,
      customerId: null,
      // Which logical queue positions this server handles
      // Server i → positions i, i+N, i+2N, … (0-indexed)
      assignLabel: this._serverLabel(i, this.numServers)
    }));
  },

  _serverLabel(idx, total) {
    if (total === 1) return 'All: 1,2,3…';
    if (total === 2) return idx === 0 ? 'Odd: 1,3,5…' : 'Even: 2,4,6…';
    // Generic: idx+1, idx+1+total, idx+1+total*2 …
    const a = idx + 1, b = a + total, c2 = b + total;
    return `${a}, ${b}, ${c2}…`;
  },

  rebuildServers() {
    this.nextServerHint = 0;
    this._buildServers();
  },

  // Round-robin: find next free server starting from nextServerHint
  _nextFreeServer() {
    const n = this.servers.length;
    for (let offset = 0; offset < n; offset++) {
      const idx = (this.nextServerHint + offset) % n;
      if (!this.servers[idx].busy) {
        this.nextServerHint = (idx + 1) % n;
        return this.servers[idx];
      }
    }
    return null;  // all busy
  },

  // ── Tick ──────────────────────────────────────────────────
  tick_fn() {
    if (!this.running) return;
    this.tick++;

    // 1. Customer arrival — stop when total target reached
    if (this.tick >= this.nextArrivalTick && this.totalArrived < this.maxCustomers) {
      this.totalArrived++;
      const customer = { id: this.nextId++, arrived: this.tick };

      if (!this.enqueue(customer)) {
        this.turnedAway++;
      }

      this.nextArrivalTick = this.tick + randomBetween(this.arrivalMin, this.arrivalMax);
    }

    // 2. Decrement busy servers; count served when DONE
    for (const s of this.servers) {
      if (s.busy) {
        s.remaining--;
        if (s.remaining <= 0) {
          s.busy       = false;
          s.customerId = null;
          this.served++;
        }
      }
    }

    // 3. Assign free servers — ROUND ROBIN
    let nextServer = this._nextFreeServer();
    while (nextServer !== null && this.queueSize > 0) {
      const person   = this.dequeue();
      this.totalWait += (this.tick - person.arrived);
      nextServer.busy       = true;
      nextServer.remaining  = randomBetween(this.serviceMin, this.serviceMax);
      nextServer.customerId = person.id;
      nextServer = this._nextFreeServer();
    }

    // 4. Auto-stop: all customers have arrived, queue drained, all servers idle
    const allArrived  = this.totalArrived >= this.maxCustomers;
    const queueDone   = this.queueSize === 0;
    const serversDone = this.servers.every(s => !s.busy);
    if (allArrived && queueDone && serversDone) {
      this.running = false;
    }

    // 4. Peak
    this.peakQueueLength = Math.max(this.peakQueueLength, this.queueSize);
  },

  getState() {
    return {
      tick:           this.tick,
      queueSize:      this.queueSize,
      served:         this.served,
      avgWait:        this.served > 0 ? this.totalWait / this.served : 0,
      nextArrival:    Math.max(0, this.nextArrivalTick - this.tick),
      running:        this.running,
      servers:        this.servers.map(s => ({
                        busy:        s.busy,
                        remaining:   s.remaining,
                        customerId:  s.customerId,
                        assignLabel: s.assignLabel
                      })),
      peakQueueLength: this.peakQueueLength,
      totalArrived:    this.totalArrived,
      turnedAway:      this.turnedAway,
      throughput:      this.tick > 0 ? this.served / this.tick : 0,
      // Expose ring internals for visualisation
      maxCustomers:    this.maxCustomers,
      totalCustomers:  this.maxCustomers,   // total that will arrive
      arrived:         this.totalArrived,   // how many have arrived so far
      _front:          this.front,
      _rear:           this.rear,
      // Snapshot of first 20 occupied slots for FIFO strip
      _fifoSlots:      this._getFifoSnapshot(20)
    };
  },

  _getFifoSnapshot(n) {
    const out = [];
    for (let i = 0; i < Math.min(n, this.queueSize); i++) {
      const idx = (this.front + i) % this.maxCustomers;
      out.push({ ...this.circularArray[idx], pos: i });
    }
    return out;
  }
};

// =========================================================
// BACKEND CALLS
// =========================================================

async function fetchState(){
  const res = await fetch(BACKEND.baseUrl + BACKEND.endpoints.state);
  if (!res.ok) throw new Error('State fetch failed');
  return res.json();
}

async function sendConfig(cfg){
  const r = await fetch(BACKEND.baseUrl + BACKEND.endpoints.config, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(cfg)
  });
  return r.json();
}

async function sendPause(){
  await fetch(BACKEND.baseUrl + BACKEND.endpoints.pause, { method: 'POST' });
}

async function sendReset(){
  await fetch(BACKEND.baseUrl + BACKEND.endpoints.reset, { method: 'POST' });
}

async function sendStart(){
  await fetch(BACKEND.baseUrl + BACKEND.endpoints.start, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({
      simulationTime:       1000,
      servers:              local.numServers,
      transactionTime:      Math.floor((local.serviceMin + local.serviceMax) / 2),
      timeBetweenArrivals:  Math.floor((local.arrivalMin + local.arrivalMax) / 2)
    })
  });
}

// =========================================================
// RENDER
// =========================================================

// Palette per server (up to 10)
const SERVER_COLORS = [
  '#6c8aff', '#f7b731', '#50e678', '#ff6b6b',
  '#a29bfe', '#00cec9', '#fd79a8', '#55efc4',
  '#e17055', '#74b9ff'
];

function render(state) {
  // ── Stat cards ───────────────────────────────────────────
  setText('s-tick',         state.tick        ?? 0);
  setText('s-queue',        state.queueSize   ?? 0);
  setText('s-served',       state.served      ?? 0);
  setText('s-wait',         (state.avgWait ?? 0).toFixed(1));
  setText('s-next-arrival', (state.nextArrival ?? 0) + ' ticks');
  setText('s-throughput',   (state.throughput ?? 0).toFixed(2));

  const servers    = state.servers ?? [];
  const numServers = servers.length;

  // ── Circular ring (maxSlots comes from state so it's always live) ──
  const maxSlots = state.totalCustomers ?? state.maxCustomers ?? local.maxCustomers ?? 100;
  RING.draw(state.queueSize ?? 0, maxSlots, state._front ?? 0, servers);

  // ── Arrival progress bar (arrived / total) ───────────────────
  const arrived = state.arrived ?? 0;
  const total   = state.totalCustomers ?? local.maxCustomers ?? 100;
  const pct     = total > 0 ? Math.min(100, Math.round(arrived / total * 100)) : 0;
  const progEl  = document.getElementById('arrival-progress-bar');
  const progLbl = document.getElementById('arrival-progress-label');
  if (progEl)  progEl.style.width = pct + '%';
  if (progLbl) progLbl.textContent = `Arrived: ${arrived} / ${total} customers (${pct}%)`;

  // ── FIFO strip ───────────────────────────────────────────
  renderFifoStrip(state._fifoSlots ?? [], numServers);

  // ── Server lane arrows ───────────────────────────────────
  renderLanes(numServers);

  // ── Server cards ─────────────────────────────────────────
  renderServers(servers);

  // ── Stats modal ───────────────────────────────────────────
  updateDetailedStats(state);
}

function setText(id, val) {
  const el = document.getElementById(id);
  if (el) el.textContent = val;
}

// FIFO strip: shows the first N customers in queue order
function renderFifoStrip(fifoSlots, numServers) {
  const container = document.getElementById('fifo-slots');
  const arrowArea = document.getElementById('fifo-arrows');
  container.innerHTML = '';
  arrowArea.innerHTML = '';

  if (fifoSlots.length === 0) {
    container.innerHTML = '<span class="fifo-empty">Queue is empty</span>';
    return;
  }

  fifoSlots.forEach((c, i) => {
    // Which server will this customer go to?
    const serverIdx = numServers > 1 ? (i % numServers) : 0;
    const color     = SERVER_COLORS[serverIdx % SERVER_COLORS.length];

    const slot = document.createElement('div');
    slot.className = 'fifo-slot';
    slot.style.borderColor = color;
    slot.style.boxShadow   = `0 0 8px ${color}55`;
    if (i === 0) slot.classList.add('fifo-front');

    slot.innerHTML = `
      <div class="fifo-icon" style="color:${color}">🧍</div>
      <div class="fifo-cid"  style="color:${color}">C${c.id}</div>
      <div class="fifo-pos">#${i + 1}</div>
      ${i === 0 ? '<div class="fifo-badge">NEXT</div>' : ''}
    `;

    // Arrow below pointing to server
    const arrow = document.createElement('div');
    arrow.className   = 'lane-arrow';
    arrow.style.color = color;
    arrow.innerHTML   = `<i class="fas fa-arrow-down"></i><span>S${serverIdx + 1}</span>`;

    container.appendChild(slot);
    arrowArea.appendChild(arrow);
  });

  // "…more" badge
  if ((fifoSlots._total ?? 0) > fifoSlots.length) {
    const more = document.createElement('div');
    more.className   = 'fifo-more';
    more.textContent = `+${fifoSlots._total - fifoSlots.length} more`;
    container.appendChild(more);
  }
}

// Lane dividers per server
function renderLanes(numServers) {
  const wrap = document.getElementById('lanes-wrap');
  wrap.innerHTML = '';

  if (numServers <= 1) return;

  const label = document.createElement('div');
  label.className   = 'lanes-title';
  label.textContent = 'Queue Assignment by Server';
  wrap.appendChild(label);

  const row = document.createElement('div');
  row.className = 'lanes-row';

  for (let i = 0; i < numServers; i++) {
    const color = SERVER_COLORS[i % SERVER_COLORS.length];
    const lane  = document.createElement('div');
    lane.className = 'lane-card';
    lane.style.borderColor = color;
    lane.style.background  = `${color}11`;

    const positions = [];
    for (let p = i + 1; p <= Math.min(i + 1 + (numServers * 2), 20); p += numServers) {
      positions.push(p);
    }

    lane.innerHTML = `
      <div class="lane-server-id" style="color:${color}">
        <i class="fas fa-server"></i> Server ${i + 1}
      </div>
      <div class="lane-pattern" style="color:${color}">
        ${numServers === 2
          ? (i === 0 ? 'Odd positions: 1, 3, 5, 7…' : 'Even positions: 2, 4, 6, 8…')
          : `Positions: ${positions.join(', ')}…`}
      </div>
      <div class="lane-desc">Customer #${i + 1} in queue → this server</div>
    `;

    row.appendChild(lane);
  }

  wrap.appendChild(row);
}

// Server cards
function renderServers(servers) {
  const sr = document.getElementById('servers-row');
  sr.innerHTML = '';

  servers.forEach((s, i) => {
    const color = SERVER_COLORS[i % SERVER_COLORS.length];
    const div   = document.createElement('div');
    div.className = 'server';

    const progressPct = s.busy && s._maxRemaining
      ? Math.round((s.remaining / s._maxRemaining) * 100)
      : 0;

    div.innerHTML = `
      <div class="server-label" style="color:${color}">S${i + 1}</div>
      <div class="server-icon ${s.busy ? 'busy' : 'idle'}"
           style="${s.busy ? `box-shadow:0 0 18px ${color}88;` : ''}">
        ${s.busy ? '🧑‍💼' : '🪑'}
      </div>
      <div class="server-customer-id" style="color:${color}">
        ${s.busy ? `C${s.customerId}` : '—'}
      </div>
      <div class="server-remaining">
        ${s.busy ? `${s.remaining} ticks` : 'Free'}
      </div>
      ${s.assignLabel ? `<div class="server-assign">${s.assignLabel}</div>` : ''}
    `;

    sr.appendChild(div);
  });
}

function updateDetailedStats(state){
  setText('stat-peak-queue',    state.peakQueueLength ?? 0);

  const servers = state.servers ?? [];
  const util = servers.length > 0
    ? (servers.filter(s => s.busy).length / servers.length * 100).toFixed(1)
    : 0;
  setText('stat-utilization',   util + '%');
  setText('stat-total-arrived', state.totalArrived ?? 0);
  setText('stat-turned-away',   state.turnedAway   ?? 0);

  const inBank  = (state.queueSize ?? 0) + servers.filter(s => s.busy).length;
  setText('stat-avg-service',   inBank + ' in bank');
  setText('stat-efficiency',    (state.served ?? 0) + ' left bank');
}

// =========================================================
// MAIN LOOP
// =========================================================

const statusBar = document.getElementById('status-bar');
let paused = false;
let loopInterval;

async function loop(){
  if (BACKEND.enabled) {
    try {
      const state = await fetchState();
      if (state && typeof state === 'object') {
        render(state);
        paused = !state.running;
        updatePauseButton(paused);
        updateStatusBar('● Connected to backend (real-time)', 'connected');
      }
    } catch(e) {
      updateStatusBar('● Backend unreachable — ' + e.message, 'error');
      render({ tick:0, queueSize:0, served:0, avgWait:0, nextArrival:0,
               throughput:0, servers:[], peakQueueLength:0,
               totalArrived:0, turnedAway:0 });
    }
  } else {
    local.tick_fn();
    render(local.getState());
  }
}

function updatePauseButton(isPaused){
  const btn = document.getElementById('btn-pause');
  btn.innerHTML = isPaused
    ? '<i class="fas fa-play"></i><span>Resume</span>'
    : '<i class="fas fa-pause"></i><span>Pause</span>';
  btn.classList.toggle('active', !isPaused);
}

function updateStatusBar(msg, cls = ''){
  statusBar.innerHTML = `<i class="fas fa-info-circle"></i><span>${msg}</span>`;
  statusBar.className = 'status-bar ' + cls;
}

function startLoop(){
  if (loopInterval) clearInterval(loopInterval);
  loopInterval = setInterval(loop, BACKEND.pollInterval);
}

// =========================================================
// MODE SWITCHING
// =========================================================

function switchToMode(mode){
  BACKEND.enabled = mode === 'backend';
  document.querySelectorAll('.mode-btn').forEach(b =>
    b.classList.toggle('active', b.dataset.mode === mode));
  updateStatusBar(mode === 'backend'
    ? '● Backend mode — Connecting to C++ simulation…'
    : '● Running in standalone mode');
}

// =========================================================
// BUTTONS
// =========================================================

document.getElementById('btn-pause').onclick = async () => {
  if (BACKEND.enabled) {
    await sendPause();
  } else {
    local.running = !local.running;
    paused = !local.running;
    updatePauseButton(paused);
  }
};

document.getElementById('btn-reset').onclick = async () => {
  if (BACKEND.enabled) {
    await sendReset();
  } else {
    local.reset();
    render(local.getState());
  }
};

document.getElementById('btn-start').onclick = async () => {
  if (BACKEND.enabled) {
    await sendStart();
  } else {
    local.running = true;
    paused = false;
    updatePauseButton(paused);
  }
};

document.getElementById('local-mode').onclick  = () => switchToMode('local');
document.getElementById('backend-mode').onclick = () => switchToMode('backend');

document.getElementById('btn-stats').onclick = () =>
  document.getElementById('stats-modal').classList.add('show');
document.getElementById('close-stats').onclick = () =>
  document.getElementById('stats-modal').classList.remove('show');
document.getElementById('stats-modal').onclick = e => {
  if (e.target.id === 'stats-modal')
    document.getElementById('stats-modal').classList.remove('show');
};

// =========================================================
// SLIDERS
// =========================================================

function updateArrivalLabel(){
  setText('v-arrival', local.arrivalMin + '-' + local.arrivalMax);
}
function updateServiceLabel(){
  setText('v-service', local.serviceMin + '-' + local.serviceMax);
}

document.getElementById('r-servers').oninput = async () => {
  local.numServers = +document.getElementById('r-servers').value;
  setText('v-servers', local.numServers);
  local.rebuildServers();
  if (BACKEND.enabled) {
    await sendConfig({ servers: local.numServers, serviceMin: local.serviceMin,
                       arrivalMin: local.arrivalMin });
    updateStatusBar('● Live parameter update sent', 'connected');
  }
};

document.getElementById('r-arrival-min').oninput = async () => {
  local.arrivalMin = +document.getElementById('r-arrival-min').value;
  if (local.arrivalMin >= local.arrivalMax) {
    local.arrivalMax = local.arrivalMin + 1;
    document.getElementById('r-arrival-max').value = local.arrivalMax;
  }
  updateArrivalLabel();
};

document.getElementById('r-arrival-max').oninput = async () => {
  local.arrivalMax = +document.getElementById('r-arrival-max').value;
  if (local.arrivalMax <= local.arrivalMin) {
    local.arrivalMin = local.arrivalMax - 1;
    document.getElementById('r-arrival-min').value = local.arrivalMin;
  }
  updateArrivalLabel();
};

document.getElementById('r-service-min').oninput = () => {
  local.serviceMin = +document.getElementById('r-service-min').value;
  if (local.serviceMin >= local.serviceMax) {
    local.serviceMax = local.serviceMin + 1;
    document.getElementById('r-service-max').value = local.serviceMax;
  }
  updateServiceLabel();
};

document.getElementById('r-service-max').oninput = () => {
  local.serviceMax = +document.getElementById('r-service-max').value;
  if (local.serviceMax <= local.serviceMin) {
    local.serviceMin = local.serviceMax - 1;
    document.getElementById('r-service-min').value = local.serviceMin;
  }
  updateServiceLabel();
};

document.getElementById('r-customers').oninput = async () => {
  const newMax = +document.getElementById('r-customers').value;
  setText('v-customers', newMax);

  // ── Local mode: resize circular array live ────────────────
  local.resizeQueue(newMax);

  // ── Backend mode: send updated config ────────────────────
  if (BACKEND.enabled) {
    await sendConfig({
      servers:      local.numServers,
      serviceMin:   local.serviceMin,
      arrivalMin:   local.arrivalMin,
      maxCustomers: newMax
    });
    updateStatusBar('● Customer count updated → ' + newMax, 'connected');
  }
};

document.getElementById('r-speed').oninput = () => {
  const speed = +document.getElementById('r-speed').value;
  setText('v-speed', speed + 'ms');
  BACKEND.pollInterval = speed;
  startLoop();
};

// =========================================================
// INIT
// =========================================================

RING.init();
local.reset();
render(local.getState());
startLoop();
updateArrivalLabel();
updateServiceLabel();
