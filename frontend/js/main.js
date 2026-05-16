/** Front-end entry point. Wires all event handlers, sliders, and initialization. */

import { RING } from "./ring.js";
import { local } from "./simulation.js";
import { refreshVisuals } from "./render.js";
import {
  BACKEND_CONFIG,
  startLoop,
  switchToMode,
  updatePauseButton,
  updateStatusBar,
} from "./backend.js";

// ── Utility ───────────────────────────────────────────────

function setText(id, val) {
  const el = document.getElementById(id);
  if (el) el.textContent = val;
}

// ── Initialization ────────────────────────────────────────

RING.init();

await local.reset();
refreshVisuals(await local.getState());

startLoop();
updateArrivalLabel();
updateServiceLabel();

BACKEND_CONFIG.enabled = true;

// ── Control Buttons ───────────────────────────────────────

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
  BACKEND_CONFIG.pollInterval = 350;

  await local.reset();
  refreshVisuals(await local.getState());
  updatePauseButton(true);
  startLoop();
};

document.getElementById("btn-start").onclick = async () => {
  await local.start();
  updatePauseButton(false);
  startLoop();
};

// ── Mode Switching ────────────────────────────────────────

document.getElementById("local-mode").onclick = () => {
  BACKEND_CONFIG.enabled = true;
  switchToMode("backend");
  updateStatusBar("● Local view is using C++ backend engine", "connected");
};

document.getElementById("backend-mode").onclick = () => {
  BACKEND_CONFIG.enabled = true;
  switchToMode("backend");
  updateStatusBar("● Backend mode connected", "connected");
};

// ── Stats Modal ───────────────────────────────────────────

document.getElementById("btn-stats").onclick = () =>
  document.getElementById("stats-modal").classList.add("show");

document.getElementById("close-stats").onclick = () =>
  document.getElementById("stats-modal").classList.remove("show");

document.getElementById("stats-modal").onclick = (e) => {
  if (e.target.id === "stats-modal") {
    document.getElementById("stats-modal").classList.remove("show");
  }
};

// ── Sliders ───────────────────────────────────────────────

async function resetAndResumeIfNeeded(message) {
  const wasRunning = local.running;

  await local.reset();

  if (wasRunning) {
    await local.start();
    updatePauseButton(false);
  } else {
    refreshVisuals(await local.getState());
    updatePauseButton(true);
  }

  updateStatusBar(message, "connected");
}

document.getElementById("r-servers").oninput = async () => {
  local.numServers = +document.getElementById("r-servers").value;
  setText("v-servers", local.numServers);

  await resetAndResumeIfNeeded("● Server count updated");
};

document.getElementById("r-arrival-min").oninput = async () => {
  local.arrivalMin = +document.getElementById("r-arrival-min").value;

  if (local.arrivalMin >= local.arrivalMax) {
    local.arrivalMax = local.arrivalMin + 1;
    document.getElementById("r-arrival-max").value = local.arrivalMax;
  }

  updateArrivalLabel();
  await resetAndResumeIfNeeded("● Arrival interval updated");
};

document.getElementById("r-arrival-max").oninput = async () => {
  local.arrivalMax = +document.getElementById("r-arrival-max").value;

  if (local.arrivalMax <= local.arrivalMin) {
    local.arrivalMin = local.arrivalMax - 1;
    document.getElementById("r-arrival-min").value = local.arrivalMin;
  }

  updateArrivalLabel();
  await resetAndResumeIfNeeded("● Arrival interval updated");
};

document.getElementById("r-service-min").oninput = async () => {
  local.serviceMin = +document.getElementById("r-service-min").value;

  if (local.serviceMin >= local.serviceMax) {
    local.serviceMax = local.serviceMin + 1;
    document.getElementById("r-service-max").value = local.serviceMax;
  }

  updateServiceLabel();
  await resetAndResumeIfNeeded("● Service time updated");
};

document.getElementById("r-service-max").oninput = async () => {
  local.serviceMax = +document.getElementById("r-service-max").value;

  if (local.serviceMax <= local.serviceMin) {
    local.serviceMin = local.serviceMax - 1;
    document.getElementById("r-service-min").value = local.serviceMin;
  }

  updateServiceLabel();
  await resetAndResumeIfNeeded("● Service time updated");
};

document.getElementById("r-customers").oninput = async () => {
  const newMax = +document.getElementById("r-customers").value;

  local.maxCustomers = newMax;
  setText("v-customers", newMax);

  await resetAndResumeIfNeeded("● Customer count updated → " + newMax);
};

document.getElementById("r-speed").oninput = () => {
  const speed = +document.getElementById("r-speed").value;

  setText("v-speed", speed + "ms");
  BACKEND_CONFIG.pollInterval = speed;

  startLoop();
};

// ── Label Helpers ─────────────────────────────────────────

function updateArrivalLabel() {
  setText("v-arrival", local.arrivalMin + "-" + local.arrivalMax);
}

function updateServiceLabel() {
  setText("v-service", local.serviceMin + "-" + local.serviceMax);
}

// ── Theme Toggle ──────────────────────────────────────────

const themeBtn = document.getElementById("theme-toggle");
const themeIcon = themeBtn.querySelector("i");

if (localStorage.getItem("theme") === "light") {
  document.body.classList.add("light");
  themeIcon.className = "fas fa-moon";
}

themeBtn.addEventListener("click", () => {
  const isLight = document.body.classList.toggle("light");
  themeIcon.className = isLight ? "fas fa-moon" : "fas fa-sun";
  localStorage.setItem("theme", isLight ? "light" : "dark");
});