/**
 * @file main.js
 * Front-end entry point that wires controls, sliders, theme, and initialization.
 */

import { RING } from "./ring.js";
import { local } from "./simulation.js";
import { invalidateRenderCache, refreshVisuals } from "./render.js";
import {
  BACKEND_CONFIG,
  activeMode,
  sessionActive,
  setSessionActive,
  startLoop,
  switchToMode,
  updatePauseButton,
  updateStatusBar,
} from "./backend.js";

/**
 * Writes text into an element when it exists.
 * @param {string} id Element id.
 * @param {*} val Value to render.
 */
function setText(id, val) {
  const el = document.getElementById(id);
  if (el) el.textContent = val;
}

RING.init();

switchToMode("local");
await local.configure();
await local.reset();
refreshVisuals(await local.getState());
updateArrivalLabel();
updateServiceLabel();
updatePauseButton(true);
setSessionActive(false);
startLoop();

document.getElementById("btn-pause").onclick = async () => {
  await local.pause();
  updatePauseButton(!local.running);
};

document.getElementById("btn-reset").onclick = async () => {
  const defaults = [
    { slider: "r-servers", label: "v-servers", val: 4, text: "4" },
    { slider: "r-arrival-min", label: null, val: 2, text: null },
    { slider: "r-arrival-max", label: "v-arrival", val: 6, text: "2-6" },
    { slider: "r-service-min", label: null, val: 4, text: null },
    { slider: "r-service-max", label: "v-service", val: 10, text: "4-10" },
    { slider: "r-customers", label: "v-customers", val: 100, text: "100" },
    { slider: "r-queue-capacity", label: "v-queue-capacity", val: 50, text: "50 waiting spots" },
    { slider: "r-speed", label: "v-speed", val: 350, text: "350ms" },
  ];

  defaults.forEach(({ slider, label, val, text }) => {
    const el = document.getElementById(slider);
    if (el) el.value = val;
    if (label && text) setText(label, text);
  });

  local.numServers = 4;
  local.arrivalMin = 2;
  local.arrivalMax = 6;
  local.serviceMin = 4;
  local.serviceMax = 10;
  local.maxCustomers = 100;
  local.maxQueueSize = 50;
  document.getElementById("n-queue-capacity").value = 50;
  BACKEND_CONFIG.pollInterval = 350;

  await local.configure();
  await local.reset();
  setSessionActive(false);
  refreshVisuals(await local.getState());
  updatePauseButton(true);
  startLoop();
  updateStatusBar(
    activeMode === "backend"
      ? "Backend simulation reset"
      : "Local simulation reset",
    activeMode === "backend" ? "connected" : "",
  );
};

document.getElementById("btn-start").onclick = async () => {
  await local.start();
  setSessionActive(true);
  updatePauseButton(false);
  startLoop();
  updateStatusBar(
    activeMode === "backend"
      ? "Backend simulation running"
      : "Local simulation running",
    activeMode === "backend" ? "connected" : "",
  );
};

/**
 * Attempts to switch simulation mode when no active session is locked.
 * @param {string} mode Requested mode.
 */
function requestModeSwitch(mode) {
  if (sessionActive) {
    updateStatusBar("Reset or finish the active simulation before switching modes", "error");
    return;
  }

  switchToMode(mode);
}

document.getElementById("local-mode").onclick = () => requestModeSwitch("local");
document.getElementById("backend-mode").onclick = () => requestModeSwitch("backend");

document.getElementById("btn-stats").onclick = () =>
  document.getElementById("stats-modal").classList.add("show");

document.getElementById("close-stats").onclick = () =>
  document.getElementById("stats-modal").classList.remove("show");

document.getElementById("stats-modal").onclick = (e) => {
  if (e.target.id === "stats-modal") {
    document.getElementById("stats-modal").classList.remove("show");
  }
};

/**
 * Resets the backend and resumes it when it was running before a control change.
 * @param {string} message Status message shown after reset.
 * @returns {Promise<void>}
 */
async function resetAndResumeIfNeeded(message) {
  const wasRunning = local.running;
  const wasSessionActive = sessionActive;

  await local.configure();
  await local.reset();

  if (wasRunning) {
    await local.start();
    updatePauseButton(false);
  } else {
    refreshVisuals(await local.getState());
    updatePauseButton(true);
  }

  setSessionActive(wasSessionActive);
  updateStatusBar(message, activeMode === "backend" ? "connected" : "");
}

document.getElementById("r-servers").oninput = async () => {
  local.numServers = +document.getElementById("r-servers").value;
  setText("v-servers", local.numServers);

  await resetAndResumeIfNeeded("Server count updated");
};

document.getElementById("r-arrival-min").oninput = async () => {
  local.arrivalMin = +document.getElementById("r-arrival-min").value;

  if (local.arrivalMin >= local.arrivalMax) {
    local.arrivalMax = local.arrivalMin + 1;
    document.getElementById("r-arrival-max").value = local.arrivalMax;
  }

  updateArrivalLabel();
  await resetAndResumeIfNeeded("Arrival interval updated");
};

document.getElementById("r-arrival-max").oninput = async () => {
  local.arrivalMax = +document.getElementById("r-arrival-max").value;

  if (local.arrivalMax <= local.arrivalMin) {
    local.arrivalMin = local.arrivalMax - 1;
    document.getElementById("r-arrival-min").value = local.arrivalMin;
  }

  updateArrivalLabel();
  await resetAndResumeIfNeeded("Arrival interval updated");
};

document.getElementById("r-service-min").oninput = async () => {
  local.serviceMin = +document.getElementById("r-service-min").value;

  if (local.serviceMin >= local.serviceMax) {
    local.serviceMax = local.serviceMin + 1;
    document.getElementById("r-service-max").value = local.serviceMax;
  }

  updateServiceLabel();
  await resetAndResumeIfNeeded("Service time updated");
};

document.getElementById("r-service-max").oninput = async () => {
  local.serviceMax = +document.getElementById("r-service-max").value;

  if (local.serviceMax <= local.serviceMin) {
    local.serviceMin = local.serviceMax - 1;
    document.getElementById("r-service-min").value = local.serviceMin;
  }

  updateServiceLabel();
  await resetAndResumeIfNeeded("Service time updated");
};

document.getElementById("r-customers").oninput = async () => {
  const newMax = +document.getElementById("r-customers").value;

  local.maxCustomers = newMax;
  setText("v-customers", newMax);

  await resetAndResumeIfNeeded("Customer count updated -> " + newMax);
};

document.getElementById("r-queue-capacity").oninput = async () => {
  await updateQueueCapacity(document.getElementById("r-queue-capacity").value);
};

document.getElementById("n-queue-capacity").onchange = async () => {
  await updateQueueCapacity(document.getElementById("n-queue-capacity").value);
};

document.getElementById("r-speed").oninput = () => {
  const speed = +document.getElementById("r-speed").value;

  setText("v-speed", speed + "ms");
  BACKEND_CONFIG.pollInterval = speed;

  startLoop();
};

/**
 * Refreshes the displayed arrival interval label.
 */
function updateArrivalLabel() {
  setText("v-arrival", `${local.arrivalMin}-${local.arrivalMax}`);
}

/**
 * Refreshes the displayed service-time label.
 */
function updateServiceLabel() {
  setText("v-service", `${local.serviceMin}-${local.serviceMax}`);
}

/**
 * Applies queue capacity from either the slider or exact number input.
 * @param {number|string} rawCapacity Requested capacity.
 * @returns {Promise<void>}
 */
async function updateQueueCapacity(rawCapacity) {
  const newCapacity = Math.max(1, Math.min(5000, Number(rawCapacity) || 1));
  const rangeValue = Math.min(1000, newCapacity);

  local.maxQueueSize = newCapacity;
  document.getElementById("r-queue-capacity").value = rangeValue;
  document.getElementById("n-queue-capacity").value = newCapacity;
  setText("v-queue-capacity", `${newCapacity} waiting spots`);

  await resetAndResumeIfNeeded("Queue capacity updated -> " + newCapacity);
}

const themeBtn = document.getElementById("theme-toggle");
const themeIcon = themeBtn.querySelector("i");

if (localStorage.getItem("theme") === "light") {
  document.body.classList.add("light");
  themeIcon.className = "fas fa-moon";
}

themeBtn.addEventListener("click", async () => {
  const isLight = document.body.classList.toggle("light");
  themeIcon.className = isLight ? "fas fa-moon" : "fas fa-sun";
  localStorage.setItem("theme", isLight ? "light" : "dark");
  invalidateRenderCache();
  refreshVisuals(await local.getState());
});
