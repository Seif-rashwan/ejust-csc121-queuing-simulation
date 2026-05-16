/** Backend configuration, API calls, and simulation loop. */

import { local } from "./simulation.js";
import { refreshVisuals } from "./render.js";

const PORT = 8081;

export const BACKEND_CONFIG = {
  enabled: false,
  pollInterval: 350,
  baseUrl: `http://localhost:${PORT}`,
  endpoints: {
    state:  "/api/state",
    config: "/api/config",
    pause:  "/api/pause",
    reset:  "/api/reset",
    start:  "/api/start",
  },
};

const url = (endpoint) => BACKEND_CONFIG.baseUrl + BACKEND_CONFIG.endpoints[endpoint];

// ── API Calls ────────────────────────────────────────────

export async function fetchState() {
  const res = await fetch(url("state"));
  if (!res.ok) throw new Error("State fetch failed");
  return res.json();
}

export async function sendConfig(cfg) {
  const res = await fetch(url("config"), {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(cfg),
  });
  return res.json();
}

export async function sendPause() {
  await fetch(url("pause"), { method: "POST" });
}

export async function sendReset() {
  await fetch(url("reset"), { method: "POST" });
}

export async function sendStart() {
  const res = await fetch(url("start"), { method: "POST" });
  if (!res.ok) throw new Error("Start failed");
  return res.json();
}

// ── Loop & UI State ───────────────────────────────────────

const statusBar = document.getElementById("status-bar");
export let paused = false;
export let loopInterval = null;

export async function loop() {
  try {
    const state = await local.getState();
    refreshVisuals(state);

    paused = !state.running;
    updatePauseButton(paused);
    updateStatusBar("● Connected to backend (real-time)", "connected");
  } catch (e) {
    updateStatusBar("● Backend unreachable — " + e.message, "error");
    refreshVisuals({
      tick: 0,
      queueSize: 0,
      served: 0,
      avgWait: 0,
      nextArrival: 0,
      throughput: 0,
      servers: [],
      peakQueueLength: 0,
      totalArrived: 0,
      turnedAway: 0,
    });
  }
}
export function startLoop() {
  if (loopInterval) clearInterval(loopInterval);
  loopInterval = setInterval(loop, BACKEND_CONFIG.pollInterval);
}

export function switchToMode(mode) {
  BACKEND_CONFIG.enabled = mode === "backend";
  document
    .querySelectorAll(".mode-btn")
    .forEach((b) => b.classList.toggle("active", b.dataset.mode === mode));
  updateStatusBar(
    mode === "backend"
      ? "● Backend mode — Connecting to C++ simulation…"
      : "● Running in standalone mode"
  );
}

export function updatePauseButton(isPaused) {
  const btn = document.getElementById("btn-pause");
  btn.classList.toggle("active", !isPaused);
  btn.innerHTML = isPaused
    ? '<i class="fas fa-play"></i><span>Resume</span>'
    : '<i class="fas fa-pause"></i><span>Pause</span>';
}

export function updateStatusBar(msg, cls = "") {
  statusBar.innerHTML = `<i class="fas fa-info-circle"></i><span>${msg}</span>`;
  statusBar.className = "status-bar " + cls;
}