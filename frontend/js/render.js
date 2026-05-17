/**
 * @file render.js
 * All DOM rendering. Reads state objects and updates the UI.
 * Never calls fetch. Never runs simulation logic.
 */

import { RING } from "./ring.js";

/**
 * Writes text into an element when it exists.
 * @param {string} id Element id.
 * @param {*} val Value to render.
 */
function setText(id, val) {
  const el = document.getElementById(id);
  if (el) el.textContent = val;
}

/** Stable palette used to associate queue positions with servers. */
const SERVER_COLORS = [
  "#6c8aff",
  "#f7b731",
  "#50e678",
  "#ff6b6b",
  "#a29bfe",
  "#00cec9",
  "#fd79a8",
  "#55efc4",
  "#e17055",
  "#74b9ff",
];

/**
 * Master render function — updates all UI panels from a state snapshot.
 * @param {Object} state
 */
export function refreshVisuals(state) {
  // Stat cards
  setText("s-tick", state.tick ?? 0);
  setText("s-queue", state.queueSize ?? 0);
  setText("s-served", state.served ?? 0);
  setText("s-wait", (state.avgWait ?? 0).toFixed(1));
  setText("s-next-arrival", (state.nextArrival ?? 0) + " ticks");
  setText("s-throughput", (state.throughput ?? 0).toFixed(2));

  const servers = state.servers ?? [];
  const numServers = servers.length;
  const maxSlots = state.totalCustomers ?? state.maxCustomers ?? 100;

  // Circular ring
  RING.draw(state.queueSize ?? 0, maxSlots, state._front ?? 0, servers);

  // Arrival progress bar
  const arrived = state.arrived ?? 0;
  const total = state.totalCustomers ?? 100;
  const pct = total > 0 ? Math.min(100, Math.round((arrived / total) * 100)) : 0;
  const progEl = document.getElementById("arrival-progress-bar");
  const progLbl = document.getElementById("arrival-progress-label");
  if (progEl) progEl.style.width = pct + "%";
  if (progLbl) progLbl.textContent = `Arrived: ${arrived} / ${total} customers (${pct}%)`;

  renderFifoStrip(state._fifoSlots ?? [], numServers);
  renderLanes(numServers);
  renderServers(servers);
  updateDetailedStats(state);
}

/**
 * Renders the FIFO queue strip showing the first N customers.
 * @param {Object[]} fifoSlots
 * @param {number}   numServers
 */
export function renderFifoStrip(fifoSlots, numServers) {
  const container = document.getElementById("fifo-slots");
  const arrowArea = document.getElementById("fifo-arrows");
  container.innerHTML = "";
  arrowArea.innerHTML = "";

  if (fifoSlots.length === 0) {
    container.innerHTML = '<span class="fifo-empty">Queue is empty</span>';
    return;
  }

  fifoSlots.forEach((c, i) => {
    const serverIdx = numServers > 1 ? i % numServers : 0;
    const color = SERVER_COLORS[serverIdx % SERVER_COLORS.length];

    const slot = document.createElement("div");
    slot.className = "fifo-slot";
    slot.style.borderColor = color;
    slot.style.boxShadow = `0 0 8px ${color}55`;
    if (i === 0) slot.classList.add("fifo-front");

    slot.innerHTML = `
      <div class="fifo-icon" style="color:${color}">🧍</div>
      <div class="fifo-cid"  style="color:${color}">C${c.id}</div>
      <div class="fifo-pos">#${i + 1}</div>
      ${i === 0 ? '<div class="fifo-badge">NEXT</div>' : ""}
    `;

    const arrow = document.createElement("div");
    arrow.className = "lane-arrow";
    arrow.style.color = color;
    arrow.innerHTML = `<i class="fas fa-arrow-down"></i><span>S${serverIdx + 1}</span>`;

    container.appendChild(slot);
    arrowArea.appendChild(arrow);
  });

  if ((fifoSlots._total ?? 0) > fifoSlots.length) {
    const more = document.createElement("div");
    more.className = "fifo-more";
    more.textContent = `+${fifoSlots._total - fifoSlots.length} more`;
    container.appendChild(more);
  }
}

/**
 * Renders the server lane assignment cards.
 * @param {number} numServers
 */
export function renderLanes(numServers) {
  const wrap = document.getElementById("lanes-wrap");
  wrap.innerHTML = "";
  if (numServers <= 1) return;

  const label = document.createElement("div");
  label.className = "lanes-title";
  label.textContent = "Queue Assignment by Server";
  wrap.appendChild(label);

  const row = document.createElement("div");
  row.className = "lanes-row";

  for (let i = 0; i < numServers; i++) {
    const color = SERVER_COLORS[i % SERVER_COLORS.length];
    const positions = [];
    for (let p = i + 1; p <= Math.min(i + 1 + numServers * 2, 20); p += numServers) {
      positions.push(p);
    }

    const lane = document.createElement("div");
    lane.className = "lane-card";
    lane.style.borderColor = color;
    lane.style.background = `${color}11`;
    lane.innerHTML = `
      <div class="lane-server-id" style="color:${color}">
        <i class="fas fa-server"></i> Server ${i + 1}
      </div>
      <div class="lane-pattern" style="color:${color}">
        ${
          numServers === 2
            ? i === 0
              ? "Odd positions: 1, 3, 5, 7…"
              : "Even positions: 2, 4, 6, 8…"
            : `Positions: ${positions.join(", ")}…`
        }
      </div>
      <div class="lane-desc">Customer #${i + 1} in queue → this server</div>
    `;

    row.appendChild(lane);
  }

  wrap.appendChild(row);
}

/**
 * Renders the server status cards.
 * @param {Object[]} servers
 */
export function renderServers(servers) {
  const row = document.getElementById("servers-row");
  row.innerHTML = "";

  servers.forEach((s, i) => {
    const color = SERVER_COLORS[i % SERVER_COLORS.length];
    const div = document.createElement("div");
    div.className = "server";
    div.innerHTML = `
      <div class="server-label" style="color:${color}">S${i + 1}</div>
      <div class="server-icon ${s.busy ? "busy" : "idle"}"
           style="${s.busy ? `box-shadow:0 0 18px ${color}88;` : ""}">
        ${s.busy ? "🧑‍💼" : "🪑"}
      </div>
      <div class="server-remaining">
        ${s.busy ? `C${s.customerId} · ${s.remaining}t` : "Free"}
      </div>
      ${s.assignLabel ? `<div class="server-assign">${s.assignLabel}</div>` : ""}
    `;

    row.appendChild(div);
  });
}

/**
 * Updates the detailed stats modal panel.
 * @param {Object} state
 */
export function updateDetailedStats(state) {
  setText("stat-peak-queue", state.peakQueueLength ?? 0);

  const servers = state.servers ?? [];
  const util =
    servers.length > 0
      ? ((servers.filter((s) => s.busy).length / servers.length) * 100).toFixed(1)
      : 0;

  setText("stat-utilization", util + "%");
  setText("stat-total-arrived", state.totalArrived ?? 0);
  setText("stat-turned-away", state.turnedAway ?? 0);

  const inBank = (state.queueSize ?? 0) + servers.filter((s) => s.busy).length;
  setText("stat-avg-service", inBank + " in bank");
  setText("stat-efficiency", (state.served ?? 0) + " left bank");
}

