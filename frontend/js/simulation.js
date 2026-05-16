const API_BASE = "/api";

async function request(path, options = {}) {
  const res = await fetch(`${API_BASE}${path}`, {
    headers: { "Content-Type": "application/json" },
    ...options,
  });

  if (!res.ok) throw new Error(`Backend error: ${res.status}`);
  return await res.json();
}

export const local = {
  running: false,

  numServers: 4,
  arrivalMin: 1,
  arrivalMax: 3,
  serviceMin: 8,
  serviceMax: 15,
  maxCustomers: 100,

  async reset() {
    await request("/reset", { method: "POST" });
    return await this.getState();
  },

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

  async pause() {
    const state = await request("/pause", { method: "POST" });
    this.running = state.running;
    return await this.getState();
  },

  async tick_fn() {
    return await this.getState();
  },

  async getState() {
    const state = await request("/state");
    const normalized = normalizeState(state);
    this.running = normalized.running;
    return normalized;
  },

  async rebuildServers() {
    return await this.reset();
  },

  async resizeQueue(newMax) {
    this.maxCustomers = Number(newMax);
    return await this.reset();
  },
};

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