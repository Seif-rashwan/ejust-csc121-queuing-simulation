/**
 * Front-End entry point. Wires all event handlers, sliders, and initialization.
 * Shall contain NO logic.
 */

import { RING } from "./ring.js";
import { local } from "./simulation.js";
import { setText } from "./helpers.js";
import { BACKEND_CONFIG } from "./config.js";
import { refreshVisuals } from "./render.js";
import { sendConfig, sendPause, sendReset, sendStart } from "./api.js";
import { startLoop, switchToMode, updatePauseButton, updateStatusBar } from "./loop.js";

// Initialization:
RING.init();
local.reset();
refreshVisuals(local.getState());
startLoop();
updateArrivalLabel();
updateServiceLabel();

// Control buttons:
document.getElementById("btn-pause").onclick = async () => {
  if (BACKEND_CONFIG.enabled) {
    await sendPause();
  } else {
    local.running = !local.running;
    updatePauseButton(!local.running);
  }
};

document.getElementById("btn-reset").onclick = async () => {
  if (BACKEND_CONFIG.enabled) {
    await sendReset();
  } else {
    local.reset();
    refreshVisuals(local.getState());
  }
};

document.getElementById("btn-start").onclick = async () => {
  if (BACKEND_CONFIG.enabled) {
    await sendStart();
  } else {
    local.running = true;
    updatePauseButton(false);
  }
};

// Stats modal:
document.getElementById("btn-stats").onclick = () =>
  document.getElementById("stats-modal").classList.add("show");

document.getElementById("close-stats").onclick = () =>
  document.getElementById("stats-modal").classList.remove("show");

document.getElementById("stats-modal").onclick = (e) => {
  if (e.target.id === "stats-modal")
    document.getElementById("stats-modal").classList.remove("show");
};

// Sliders:
document.getElementById("r-servers").oninput = async () => {
  local.numServers = +document.getElementById("r-servers").value;
  setText("v-servers", local.numServers);
  local.rebuildServers();
  if (BACKEND_CONFIG.enabled) {
    await sendConfig({ servers: local.numServers });
    updateStatusBar("● Live parameter update sent", "connected");
  }
};

document.getElementById("r-arrival-min").oninput = () => {
  local.arrivalMin = +document.getElementById("r-arrival-min").value;
  if (local.arrivalMin >= local.arrivalMax) {
    local.arrivalMax = local.arrivalMin + 1;
    document.getElementById("r-arrival-max").value = local.arrivalMax;
  }
  updateArrivalLabel();
};

document.getElementById("r-arrival-max").oninput = () => {
  local.arrivalMax = +document.getElementById("r-arrival-max").value;
  if (local.arrivalMax <= local.arrivalMin) {
    local.arrivalMin = local.arrivalMax - 1;
    document.getElementById("r-arrival-min").value = local.arrivalMin;
  }
  updateArrivalLabel();
};

document.getElementById("r-service-min").oninput = () => {
  local.serviceMin = +document.getElementById("r-service-min").value;
  if (local.serviceMin >= local.serviceMax) {
    local.serviceMax = local.serviceMin + 1;
    document.getElementById("r-service-max").value = local.serviceMax;
  }
  updateServiceLabel();
};

document.getElementById("r-service-max").oninput = () => {
  local.serviceMax = +document.getElementById("r-service-max").value;
  if (local.serviceMax <= local.serviceMin) {
    local.serviceMin = local.serviceMax - 1;
    document.getElementById("r-service-min").value = local.serviceMin;
  }
  updateServiceLabel();
};

document.getElementById("r-customers").oninput = async () => {
  const newMax = +document.getElementById("r-customers").value;
  setText("v-customers", newMax);
  local.resizeQueue(newMax);
  if (BACKEND_CONFIG.enabled) {
    await sendConfig({ servers: local.numServers, maxCustomers: newMax });
    updateStatusBar("● Customer count updated → " + newMax, "connected");
  }
};

document.getElementById("r-speed").oninput = () => {
  const speed = +document.getElementById("r-speed").value;
  setText("v-speed", speed + "ms");
  BACKEND_CONFIG.pollInterval = speed;
  startLoop();
};

// Label helpers:
function updateArrivalLabel() {
  setText("v-arrival", local.arrivalMin + "-" + local.arrivalMax);
}

function updateServiceLabel() {
  setText("v-service", local.serviceMin + "-" + local.serviceMax);
}

// ── THEME TOGGLE ──────────────────────────────────────────
const themeBtn = document.getElementById("theme-toggle");
if (themeBtn) {
  const themeIcon = themeBtn.querySelector("i");

  const savedTheme = localStorage.getItem("theme");
  if (savedTheme === "light") {
    document.body.classList.add("light");
    if (themeIcon) themeIcon.className = "fas fa-moon";
  }

  themeBtn.addEventListener("click", () => {
    const isLight = document.body.classList.toggle("light");
    if (themeIcon) themeIcon.className = isLight ? "fas fa-moon" : "fas fa-sun";
    localStorage.setItem("theme", isLight ? "light" : "dark");
  });
}
