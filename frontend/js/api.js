/**
 * All fetch calls to the C++ backend. Nothing else in the project calls fetch.
 * See `config.js` for base configuration.
 */

import { BACKEND_CONFIG } from "./config.js";
import { local } from "./simulation.js";
const url = (endpoint) => BACKEND_CONFIG.baseUrl + BACKEND_CONFIG.endpoints[endpoint];

/**
 * Fetches current simulation state from the backend.
 * @returns {Promise<Object>}
 */
export async function fetchState() {
  const res = await fetch(url("state"));
  if (!res.ok) throw new Error("State fetch failed");
  return res.json();
}

/**
 * Sends updated simulation parameters to the backend.
 * @param {Object} cfg
 */
export async function sendConfig(cfg) {
  const res = await fetch(url("config"), {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(cfg),
  });
  return res.json();
}

/** Sends a pause signal to the backend. */
export async function sendPause() {
  await fetch(url("pause"), { method: "POST" });
}

/** Sends a reset signal to the backend. */
export async function sendReset() {
  await fetch(url("reset"), { method: "POST" });
}

/**
 * Starts the backend simulation with current local parameters.
 * Uses local simulation values as the source of truth for config.
 */
export async function sendStart() {
  await fetch(url("start"), {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      simulationTime: 1000,
      servers: local.numServers,
      transactionTime: Math.floor((local.serviceMin + local.serviceMax) / 2),
      timeBetweenArrivals: Math.floor((local.arrivalMin + local.arrivalMax) / 2),
    }),
  });
}
