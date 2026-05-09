/**
 * Self-contained local simulation. Runs entirely in the browser with no backend.
 * Exposes reset(), tick_fn(), getState(), rebuildServers(), resizeQueue().
 */
import { randomBetween } from "./helpers.js";

export const local = {
  tick: 0,
  served: 0,
  totalWait: 0,
  peakQueueLength: 0,
  totalArrived: 0,
  turnedAway: 0,

  maxCustomers: 100,
  circularArray: new Array(100).fill(null),
  front: 0,
  rear: 0,
  queueSize: 0,

  servers: [],
  nextId: 1,
  running: false,

  numServers: 4,
  arrivalMin: 2,
  arrivalMax: 6,
  serviceMin: 4,
  serviceMax: 10,
  nextArrivalTick: 0,
  nextServerHint: 0,

  // ── Queue primitives ────────────────────────────────────────

  enqueue(customer) {
    if (this.queueSize >= this.maxCustomers) return false;
    this.circularArray[this.rear] = customer;
    this.rear = (this.rear + 1) % this.maxCustomers;
    this.queueSize++;
    return true;
  },

  dequeue() {
    if (this.queueSize === 0) return null;
    const c = this.circularArray[this.front];
    this.circularArray[this.front] = null;
    this.front = (this.front + 1) % this.maxCustomers;
    this.queueSize--;
    return c;
  },

  resizeQueue(newMax) {
    const existing = [];
    for (let i = 0; i < this.queueSize; i++) {
      existing.push(this.circularArray[(this.front + i) % this.maxCustomers]);
    }
    this.maxCustomers = newMax;
    this.circularArray = new Array(newMax).fill(null);
    this.front = 0;
    this.queueSize = 0;
    this.rear = 0;
    for (const c of existing) this.enqueue(c);
  },

  // ── Lifecycle ───────────────────────────────────────────────

  reset() {
    this.tick = 0;
    this.served = 0;
    this.totalWait = 0;
    this.peakQueueLength = 0;
    this.totalArrived = 0;
    this.turnedAway = 0;
    this.nextId = 1;

    this.circularArray = new Array(this.maxCustomers).fill(null);
    this.front = 0;
    this.rear = 0;
    this.queueSize = 0;

    this.nextArrivalTick = randomBetween(this.arrivalMin, this.arrivalMax);
    this.nextServerHint = 0;
    this._buildServers();
  },

  rebuildServers() {
    this.nextServerHint = 0;
    this._buildServers();
  },

  _buildServers() {
    this.servers = Array.from({ length: this.numServers }, (_, i) => ({
      id: i,
      busy: false,
      remaining: 0,
      customerId: null,
      assignLabel: this._serverLabel(i, this.numServers),
    }));
  },

  _serverLabel(idx, total) {
    if (total === 1) return "All: 1,2,3…";
    if (total === 2) return idx === 0 ? "Odd: 1,3,5…" : "Even: 2,4,6…";
    const a = idx + 1,
      b = a + total,
      c = b + total;
    return `${a}, ${b}, ${c}…`;
  },

  _nextFreeServer() {
    const n = this.servers.length;
    for (let offset = 0; offset < n; offset++) {
      const idx = (this.nextServerHint + offset) % n;
      if (!this.servers[idx].busy) {
        this.nextServerHint = (idx + 1) % n;
        return this.servers[idx];
      }
    }
    return null;
  },

  // ── Tick ────────────────────────────────────────────────────

  tick_fn() {
    if (!this.running) return;
    this.tick++;

    // 1. Customer arrival
    if (this.tick >= this.nextArrivalTick && this.totalArrived < this.maxCustomers) {
      this.totalArrived++;
      const customer = { id: this.nextId++, arrived: this.tick };
      if (!this.enqueue(customer)) this.turnedAway++;
      this.nextArrivalTick = this.tick + randomBetween(this.arrivalMin, this.arrivalMax);
    }

    // 2. Decrement busy servers
    for (const s of this.servers) {
      if (s.busy) {
        s.remaining--;
        if (s.remaining <= 0) {
          s.busy = false;
          s.customerId = null;
          this.served++;
        }
      }
    }

    // 3. Assign free servers (round-robin)
    let next = this._nextFreeServer();
    while (next !== null && this.queueSize > 0) {
      const person = this.dequeue();
      this.totalWait += this.tick - person.arrived;
      next.busy = true;
      next.remaining = randomBetween(this.serviceMin, this.serviceMax);
      next.customerId = person.id;
      next = this._nextFreeServer();
    }

    // 4. Auto-stop when all customers fully served
    if (
      this.totalArrived >= this.maxCustomers &&
      this.queueSize === 0 &&
      this.servers.every((s) => !s.busy)
    ) {
      this.running = false;
    }

    // 5. Peak tracking
    this.peakQueueLength = Math.max(this.peakQueueLength, this.queueSize);
  },

  // ── State snapshot ───────────────────────────────────────────

  getState() {
    return {
      tick: this.tick,
      queueSize: this.queueSize,
      served: this.served,
      avgWait: this.served > 0 ? this.totalWait / this.served : 0,
      nextArrival: Math.max(0, this.nextArrivalTick - this.tick),
      running: this.running,
      peakQueueLength: this.peakQueueLength,
      totalArrived: this.totalArrived,
      turnedAway: this.turnedAway,
      throughput: this.tick > 0 ? this.served / this.tick : 0,
      maxCustomers: this.maxCustomers,
      totalCustomers: this.maxCustomers,
      arrived: this.totalArrived,
      _front: this.front,
      _rear: this.rear,
      _fifoSlots: this._getFifoSnapshot(20),
      servers: this.servers.map((s) => ({
        busy: s.busy,
        remaining: s.remaining,
        customerId: s.customerId,
        assignLabel: s.assignLabel,
      })),
    };
  },

  _getFifoSnapshot(n) {
    const out = [];
    for (let i = 0; i < Math.min(n, this.queueSize); i++) {
      out.push({ ...this.circularArray[(this.front + i) % this.maxCustomers], pos: i });
    }
    return out;
  },
};
