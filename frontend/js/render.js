/**
 * @file render.js
 * All DOM rendering. Reads state objects and updates the UI.
 * Never calls fetch. Never runs simulation logic.
 */

import { RING } from "./ring.js";

const FIFO_PREVIEW_LIMIT = 14;

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
  "#4f7cff",
  "#f59e0b",
  "#14b88a",
  "#ef5d5d",
  "#8b6cff",
  "#0891b2",
  "#db5a91",
  "#2bbf9f",
  "#d97745",
  "#3b8eea",
];

const renderCache = {
  fifo: "",
  lanes: "",
  ring: "",
  servers: "",
};

/**
 * Clears region signatures so the next refresh rebuilds all visual regions.
 */
export function invalidateRenderCache() {
  renderCache.fifo = "";
  renderCache.lanes = "";
  renderCache.ring = "";
  renderCache.servers = "";
}

/**
 * Builds a stable signature for render cache comparisons.
 * @param {*} value Value to serialize.
 * @returns {string} Stable enough signature for UI render inputs.
 */
function signature(value) {
  return JSON.stringify(value);
}

/**
 * Master render function - updates all UI panels from a state snapshot.
 * @param {Object} state
 */
export function refreshVisuals(state) {
  setText("s-tick", state.tick ?? 0);
  setText("s-queue", state.queueSize ?? 0);
  setText("s-served", state.served ?? 0);
  setText("s-turned-away", state.turnedAway ?? 0);
  setText("s-wait", (state.avgWait ?? 0).toFixed(1));
  setText("s-next-arrival", (state.nextArrival ?? 0) + " ticks");
  setText("s-throughput", (state.throughput ?? 0).toFixed(2));

  const servers = state.servers ?? [];
  const numServers = servers.length;
  const maxSlots = state.maxQueueSize ?? state.queueCapacity ?? 50;
  const queueSize = state.queueSize ?? 0;

  const ringSig = signature({
    queueSize,
    maxSlots,
    front: state._front ?? 0,
    servers: servers.map((server) => Boolean(server.busy)),
  });
  if (renderCache.ring !== ringSig) {
    RING.draw(queueSize, maxSlots, state._front ?? 0, servers);
    renderCache.ring = ringSig;
  }

  const arrived = state.arrived ?? 0;
  const total = state.totalCustomers ?? 100;
  const pct = total > 0 ? Math.min(100, Math.round((arrived / total) * 100)) : 0;
  const progEl = document.getElementById("arrival-progress-bar");
  const progLbl = document.getElementById("arrival-progress-label");
  if (progEl) progEl.style.width = pct + "%";
  if (progLbl) progLbl.textContent = `Arrived: ${arrived} / ${total} customers (${pct}%)`;

  const fifoSlots = state._fifoSlots ?? [];
  const fifoSig = signature({
    slots: fifoSlots.slice(0, FIFO_PREVIEW_LIMIT).map((slot) => slot.id),
    total: queueSize,
    capacity: maxSlots,
    numServers,
  });
  if (renderCache.fifo !== fifoSig) {
    renderFifoStrip(fifoSlots, numServers, queueSize);
    renderCache.fifo = fifoSig;
  }

  const lanesSig = String(numServers);
  if (renderCache.lanes !== lanesSig) {
    renderLanes(numServers);
    renderCache.lanes = lanesSig;
  }

  const serversSig = signature(
    servers.map((server) => ({
      busy: server.busy,
      remaining: server.remaining,
      customerId: server.customerId,
      assignLabel: server.assignLabel,
    })),
  );
  if (renderCache.servers !== serversSig) {
    renderServers(servers);
    renderCache.servers = serversSig;
  }

  updateDetailedStats(state);
}

/**
 * Renders the FIFO queue strip showing a bounded customer preview.
 * @param {Object[]} fifoSlots FIFO preview from the backend.
 * @param {number} numServers Number of active servers.
 * @param {number} totalQueueSize Current queue length.
 */
export function renderFifoStrip(fifoSlots, numServers, totalQueueSize = fifoSlots.length) {
  const container = document.getElementById("fifo-slots");
  const arrowArea = document.getElementById("fifo-arrows");
  container.replaceChildren();
  arrowArea.replaceChildren();

  if (totalQueueSize === 0 || fifoSlots.length === 0) {
    const empty = document.createElement("span");
    empty.className = "fifo-empty";
    empty.textContent = "Queue is empty";
    container.appendChild(empty);
    return;
  }

  fifoSlots.slice(0, FIFO_PREVIEW_LIMIT).forEach((customer, index) => {
    const serverIdx = numServers > 1 ? index % numServers : 0;
    const color = SERVER_COLORS[serverIdx % SERVER_COLORS.length];

    const slot = document.createElement("div");
    slot.className = "fifo-slot";
    slot.style.borderColor = color;
    slot.style.boxShadow = `0 0 8px ${color}55`;
    if (index === 0) slot.classList.add("fifo-front");

    const icon = document.createElement("i");
    icon.className = "fas fa-user fifo-icon";
    icon.style.color = color;

    const cid = document.createElement("div");
    cid.className = "fifo-cid";
    cid.style.color = color;
    cid.textContent = `C${customer.id}`;
    cid.title = `Customer ${customer.id}`;

    const pos = document.createElement("div");
    pos.className = "fifo-pos";
    pos.textContent = `#${index + 1}`;

    const arrow = document.createElement("div");
    arrow.className = "fifo-arrow";
    arrow.style.color = color;
    arrow.innerHTML = `<i class="fas fa-arrow-down"></i><span>S${serverIdx + 1}</span>`;

    slot.append(icon, cid, pos, arrow);

    if (index === 0) {
      const badge = document.createElement("div");
      badge.className = "fifo-badge";
      badge.textContent = "NEXT";
      slot.appendChild(badge);
    }

    container.appendChild(slot);
  });

  if (totalQueueSize > FIFO_PREVIEW_LIMIT) {
    const more = document.createElement("div");
    more.className = "fifo-more";
    more.textContent = `+${totalQueueSize - FIFO_PREVIEW_LIMIT} more`;
    container.appendChild(more);
  }
}

/**
 * Renders the server lane assignment cards.
 * @param {number} numServers
 */
export function renderLanes(numServers) {
  const wrap = document.getElementById("lanes-wrap");
  wrap.replaceChildren();
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
    lane.style.background = `${color}14`;
    lane.innerHTML = `
      <div class="lane-server-id" style="color:${color}">
        <i class="fas fa-server"></i> Server ${i + 1}
      </div>
      <div class="lane-pattern" style="color:${color}">
        ${
          numServers === 2
            ? i === 0
              ? "Odd positions: 1, 3, 5, 7..."
              : "Even positions: 2, 4, 6, 8..."
            : `Positions: ${positions.join(", ")}...`
        }
      </div>
      <div class="lane-desc">Customer #${i + 1} in queue -> this server</div>
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
  row.replaceChildren();

  servers.forEach((server, index) => {
    const color = SERVER_COLORS[index % SERVER_COLORS.length];
    const div = document.createElement("div");
    div.className = "server";

    const label = document.createElement("div");
    label.className = "server-label";
    label.style.color = color;
    label.textContent = `S${index + 1}`;

    const icon = document.createElement("div");
    icon.className = `server-icon ${server.busy ? "busy" : "idle"}`;
    if (server.busy) icon.style.boxShadow = `0 0 18px ${color}88`;
    icon.innerHTML = server.busy
      ? '<i class="fas fa-user-tie"></i>'
      : '<i class="fas fa-chair"></i>';

    const remaining = document.createElement("div");
    remaining.className = "server-remaining";
    remaining.textContent = server.busy ? `C${server.customerId} - ${server.remaining}t` : "Free";
    remaining.title = remaining.textContent;

    div.append(label, icon, remaining);

    if (server.assignLabel) {
      const assign = document.createElement("div");
      assign.className = "server-assign";
      assign.textContent = server.assignLabel;
      div.appendChild(assign);
    }

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
      ? ((servers.filter((server) => server.busy).length / servers.length) * 100).toFixed(1)
      : 0;

  setText("stat-utilization", util + "%");
  setText("stat-total-arrived", state.totalArrived ?? 0);
  setText("stat-turned-away", state.turnedAway ?? 0);

  const inBank = (state.queueSize ?? 0) + servers.filter((server) => server.busy).length;
  setText("stat-avg-service", inBank + " in bank");
  setText("stat-efficiency", (state.served ?? 0) + " left bank");
}
