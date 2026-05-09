/**
 * Owns the main simulation loop, mode switching, and status bar.
 * Orchestrates between api.js, simulation.js, and render.js.
 */

import { local } from "./simulation.js";
import { fetchState } from "./api.js";
import { BACKEND_CONFIG } from "./config.js";
import { refreshVisuals } from "./render.js";

const statusBar = document.getElementById("status-bar");
export let paused = false;
export let loopInterval = null;

/**
 * One loop iteration — either polls backend or advances local simulation.
 */
export async function loop() {
  if (BACKEND_CONFIG.enabled) {
    try {
      const state = await fetchState();
      if (state && typeof state === "object") {
        refreshVisuals(state);
        paused = !state.running;
        updatePauseButton(paused);
        updateStatusBar("● Connected to backend (real-time)", "connected");
      }
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
  } else {
    local.tick_fn();
    refreshVisuals(local.getState());
  }
}

/**
 * Starts or restarts the polling interval.
 * Call this whenever pollInterval changes.
 */
export function startLoop() {
  if (loopInterval) clearInterval(loopInterval);
  loopInterval = setInterval(loop, BACKEND_CONFIG.pollInterval);
}

/**
 * Switches between local and backend simulation modes.
 * @param {'local'|'backend'} mode
 */
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

/**
 * Updates the pause/resume button label and state.
 * @param {boolean} isPaused
 */
export function updatePauseButton(isPaused) {
  const btn = document.getElementById("btn-pause");
  btn.classList.toggle("active", !isPaused);
  btn.innerHTML = isPaused
    ? '<i class="fas fa-play"></i><span>Resume</span>'
    : '<i class="fas fa-pause"></i><span>Pause</span>';
}

/**
 * Updates the status bar message and CSS class.
 * @param {string} msg
 * @param {string} cls
 */
export function updateStatusBar(msg, cls = "") {
  statusBar.innerHTML = `<i class="fas fa-info-circle"></i><span>${msg}</span>`;
  statusBar.className = "status-bar " + cls;
}
