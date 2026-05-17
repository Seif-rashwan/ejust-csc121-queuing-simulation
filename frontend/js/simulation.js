/**
 * @file simulation.js
 * @brief Browser-side wrapper around the REST simulation API.
 */

const API_BASE = "/api";

/**
 * Sends a JSON request to the simulation API.
 * @param {string} path API path after /api.
 * @param {RequestInit} [options={}] Fetch options.
 * @returns {Promise<Object>} Parsed JSON response.
 */
async function request(path, options = {}) {
  const res = await fetch(`${API_BASE}${path}`, {
    headers: { "Content-Type": "application/json" },
    ...options,
  });

  if (!res.ok) throw new Error(`Backend error: ${res.status}`);
  return await res.json();
}

/** Local simulation facade backed by the C++ REST service. */
export const local = {
  running: false,

  numServers: 4,
  arrivalMin: 1,
  arrivalMax: 3,
  serviceMin: 8,
  serviceMax: 15,
  maxCustomers: 100,

  /**
   * Resets the backend simulation and returns normalized state.
   * @returns {Promise<Object>} Current normalized state.
   */
  async reset() {
    await request("/reset", { method: "POST" });
    return await this.getState();
  },

  /**
   * Applies configuration and starts the simulation.
   * @returns {Promise<Object>} Current normalized state.
   */
  async start() {
    await request("/config", {
      method: "POST",
      body: JSON.stringify({
        servers: this.numServers,
        arrivalMin: this.arrivalMin,
        arrivalMax: this.arrivalMax,
        serviceMin: this.serviceMin,
        serviceMax: this.serviceMax,
        maxCustomers: this.maxCustomers,
      }),
    });

    await request("/start", { method: "POST" });

    this.running = true;
    return await this.getState();
  },

  /**
   * Toggles pause/resume and returns the latest state.
   * @returns {Promise<Object>} Current normalized state.
   */
  async pause() {
    const state = await request("/pause", { method: "POST" });
    this.running = state.running;
    return await this.getState();
  },

  /**
   * Gets one state frame for compatibility with older tick callers.
   * @returns {Promise<Object>} Current normalized state.
   */
  async tick_fn() {
    return await this.getState();
  },

  /**
   * Fetches and normalizes backend state.
   * @returns {Promise<Object>} Normalized state for renderers.
   */
  async getState() {
    const state = await request("/state");
    const normalized = normalizeState(state);
    this.running = normalized.running;
    return normalized;
  },

  /**
   * Rebuilds server state by resetting the backend simulation.
   * @returns {Promise<Object>} Current normalized state.
   */
  async rebuildServers() {
    return await this.reset();
  },

  /**
   * Updates the customer cap and resets the simulation.
   * @param {number|string} newMax New customer count.
   * @returns {Promise<Object>} Current normalized state.
   */
  async resizeQueue(newMax) {
    this.maxCustomers = Number(newMax);
    return await this.reset();
  },
};

/**
 * Converts backend state variants into the shape expected by renderers.
 * @param {Object} state Raw backend state.
 * @returns {Object} Normalized UI state.
 */
function normalizeState(state) {
  return {
    tick: state.tick ?? 0,
    queueSize: state.queueSize ?? 0,
    served: state.served ?? 0,
    avgWait: state.avgWait ?? 0,
    nextArrival: state.nextArrival ?? 0,
    running: state.running ?? false,

    peakQueueLength: state.peakQueueLength ?? state.peakQueue ?? 0,
    totalArrived: state.totalArrived ?? state.arrived ?? 0,
    turnedAway: state.turnedAway ?? 0,
    throughput:
      state.tick > 0
        ? (state.served ?? 0) / state.tick
        : 0,
    maxCustomers: state.maxCustomers ?? state.totalCustomers ?? 100,
    totalCustomers: state.totalCustomers ?? state.maxCustomers ?? 100,
    arrived: state.arrived ?? state.totalArrived ?? 0,

    _front: state._front ?? 0,
    _rear: state._rear ?? 0,
    _fifoSlots: state._fifoSlots ?? [],

    servers: (state.servers ?? []).map((s, index) => ({
      busy: s.busy ?? false,
      remaining: s.remaining ?? 0,
      customerId: s.customerId ?? -1,
      assignLabel: s.assignLabel ?? `Server ${index + 1}`,
    })),
  };
}
