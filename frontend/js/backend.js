/**
 * @file backend.js
 * Backend configuration, API calls, polling, and shared UI runtime state.
 */

import { local } from "./simulation.js";
import { refreshVisuals } from "./render.js";

const PORT = 8081;
const MIN_POLL_INTERVAL = 150;

/** Runtime backend connection settings and endpoint paths. */
export const BACKEND_CONFIG = {
  enabled: false,
  pollInterval: 350,
  baseUrl: `http://localhost:${PORT}`,
  endpoints: {
    state: "/api/state",
    config: "/api/config",
    pause: "/api/pause",
    reset: "/api/reset",
    start: "/api/start",
  },
};

const url = (endpoint) => BACKEND_CONFIG.baseUrl + BACKEND_CONFIG.endpoints[endpoint];

/**
 * Fetches the latest simulation state from the backend.
 * @returns {Promise<Object>} Current simulation state.
 */
export async function fetchState() {
  const res = await fetch(url("state"));
  if (!res.ok) throw new Error("State fetch failed");
  return res.json();
}

/**
 * Sends a new simulation configuration to the backend.
 * @param {Object} cfg Simulation configuration values.
 * @returns {Promise<Object>} Backend response.
 */
export async function sendConfig(cfg) {
  const res = await fetch(url("config"), {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(cfg),
  });
  return res.json();
}

/**
 * Toggles backend pause state.
 * @returns {Promise<void>}
 */
export async function sendPause() {
  await fetch(url("pause"), { method: "POST" });
}

/**
 * Resets the backend simulation process and buffered state.
 * @returns {Promise<void>}
 */
export async function sendReset() {
  await fetch(url("reset"), { method: "POST" });
}

/**
 * Starts the backend simulation process.
 * @returns {Promise<Object>} Backend response.
 */
export async function sendStart() {
  const res = await fetch(url("start"), { method: "POST" });
  if (!res.ok) throw new Error("Start failed");
  return res.json();
}

const statusBar = document.getElementById("status-bar");
const modeButtons = [...document.querySelectorAll(".mode-btn")];

export let paused = false;
export let loopInterval = null;
export let activeMode = "local";
export let sessionActive = false;

let pollInFlight = false;

/**
 * Returns whether a state snapshot represents natural completion.
 * @param {Object} state Normalized state snapshot.
 * @returns {boolean} True when all customers are out of the system.
 */
function isSimulationComplete(state) {
  const total = state.totalCustomers ?? state.maxCustomers ?? 0;
  const arrived = state.arrived ?? state.totalArrived ?? 0;
  const servedOrLeft = (state.served ?? 0) + (state.turnedAway ?? 0);
  const busyServers = (state.servers ?? []).some((server) => server.busy);

  return (
    total > 0 &&
    arrived >= total &&
    servedOrLeft >= total &&
    !busyServers &&
    (state.queueSize ?? 0) === 0
  );
}

/**
 * Updates whether mode controls are locked by an active simulation session.
 * @param {boolean} value True while a started simulation should lock mode changes.
 */
export function setSessionActive(value) {
  sessionActive = value;

  modeButtons.forEach((button) => {
    button.disabled = value;
    button.classList.toggle("locked", value);
    button.title = value
      ? "Reset or finish the active simulation before switching modes"
      : "";
  });
}

/**
 * Applies mode styling and records the selected adapter path.
 * @param {string} mode Mode name from a mode button.
 */
export function switchToMode(mode) {
  activeMode = mode === "backend" ? "backend" : "local";
  BACKEND_CONFIG.enabled = activeMode === "backend";

  modeButtons.forEach((button) => {
    button.classList.toggle("active", button.dataset.mode === activeMode);
  });

  updateStatusBar(
    activeMode === "backend"
      ? "Backend mode connected to the C++ service"
      : "Local mode selected",
    activeMode === "backend" ? "connected" : "",
  );
}

/**
 * Polls state, renders the UI, and updates connection status.
 * @returns {Promise<void>}
 */
export async function loop() {
  if (pollInFlight) return;

  pollInFlight = true;
  try {
    const state = await local.getState();
    refreshVisuals(state);

    paused = !state.running;
    updatePauseButton(paused);

    if (sessionActive && isSimulationComplete(state)) {
      setSessionActive(false);
      updateStatusBar("Simulation complete. Mode switching is available again.", "connected");
    } else if (sessionActive) {
      const runState = state.running ? "running" : "paused";
      updateStatusBar(
        activeMode === "backend"
          ? `Backend simulation ${runState}`
          : `Local simulation ${runState}`,
        activeMode === "backend" ? "connected" : "",
      );
    }
  } catch (e) {
    updateStatusBar("Backend unreachable - " + e.message, "error");
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
  } finally {
    pollInFlight = false;
  }
}

/**
 * Starts or restarts the polling interval.
 */
export function startLoop() {
  if (loopInterval) clearInterval(loopInterval);
  BACKEND_CONFIG.pollInterval = Math.max(MIN_POLL_INTERVAL, BACKEND_CONFIG.pollInterval);
  loopInterval = setInterval(loop, BACKEND_CONFIG.pollInterval);
}

/**
 * Updates the pause/resume control state.
 * @param {boolean} isPaused Whether the simulation is paused.
 */
export function updatePauseButton(isPaused) {
  const btn = document.getElementById("btn-pause");
  btn.classList.toggle("active", !isPaused);
  btn.innerHTML = isPaused
    ? '<i class="fas fa-play"></i><span>Resume</span>'
    : '<i class="fas fa-pause"></i><span>Pause</span>';
}

/**
 * Writes a status message into the status bar.
 * @param {string} msg Message to display.
 * @param {string} [cls=""] Additional status class.
 */
export function updateStatusBar(msg, cls = "") {
  statusBar.replaceChildren();

  const icon = document.createElement("i");
  const text = document.createElement("span");
  icon.className = "fas fa-info-circle";
  text.textContent = msg;

  statusBar.append(icon, text);
  statusBar.className = "status-bar " + cls;
}
