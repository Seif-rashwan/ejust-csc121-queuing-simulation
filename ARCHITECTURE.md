# System Architecture Document

## Queuing System Simulation — CSC 121

**Version:** 2.0 | **Date:** May 2026

---

## 1. Architectural Overview

The system is a **three-tier event-driven application** where each tier has a single, well-defined responsibility:

```mathematica
┌──────────────────────────────────────────────────────────────────────┐
│  Tier 1 — Presentation  │  Tier 2 — Orchestration  │  Tier 3 — Engine│
│  (Browser / Vanilla JS) │  (Node.js / Express)     │  (C++23 binary) │
│                         │                          │                 │
│  Renders state          │  Relays data             │  Computes state │
│  Accepts user input     │  Manages process life    │  Emits JSON     │
│  Polls REST API         │  Buffers state queue     │  Terminates     │
└──────────────────────────────────────────────────────────────────────┘
```

The tiers communicate over **two distinct channels**:

| Channel   | From    | To      | Protocol                   |
| :-------- | :------ | :------ | :------------------------- |
| REST/HTTP | Browser | Node.js | HTTP/1.1 JSON              |
| IPC pipe  | Node.js | C++     | child_process.spawn stdout |

---

## 2. Component Diagram

```mathematica
Browser
│
│  ┌───────────────────────────────────────────────────────────┐
│  │                  frontend/js/modules                      │
│  │                                                           │
│  │  ┌──────────────────────┐  ┌──────────────────────────┐   │
│  │  │   LocalSimulation    │  │    BackendClient         │   │
│  │  │   (object `local`)   │  │  polls /api/state        │   │
│  │  │                      │  │  every 350 ms            │   │
│  │  │  tick_fn()           │  │                          │   │
│  │  │  ├─ arrivals cap     │  │  sendConfig()            │   │
│  │  │  ├─ server decrement │  │  sendStart()             │   │
│  │  │  ├─ round-robin      │  │  sendReset()             │   │
│  │  │  └─ auto-stop        │  └──────────────────────────┘   │
│  │  └──────────────────────┘                                 │
│  │                                                           │
│  │  render(state)                                            │
│  │  ├─ RING.draw()          ← canvas circular visualisation  │
│  │  ├─ arrival progress bar                                  │
│  │  ├─ FIFO strip           ← next-20 customer slots         │
│  │  ├─ server lane cards                                     │
│  │  └─ stat cards (6)                                        │
│  └───────────────────────────────────────────────────────────┘
│
│  HTTP REST on localhost:8081
│
Node.js (server.js)
│
│  ┌───────────────────────────────────────────────────────────┐
│  │  Express App                                              │
│  │                                                           │
│  │  GET  /api/state   ──► pop stateQueue[0] → return JSON   │
│  │  POST /api/config  ──► update config{}                   │
│  │  POST /api/start   ──► kill old process                  │
│  │                        spawn simulation.exe              │
│  │                        stateQueue = []                   │
│  │  POST /api/pause   ──► kill process                      │
│  │  POST /api/reset   ──► kill + clear state                │
│  │                                                           │
│  │  stdout reader:                                           │
│  │  lineBuffer += chunk                                      │
│  │  split('\n') → parseLine(line)                           │
│  │    "STATE:{…}" → stateQueue.push(JSON.parse(...))        │
│  │    "FINAL:{…}" → log finalStats                          │
│  └───────────────────────────────────────────────────────────┘
│
│  child_process.spawn() stdout pipe
│
C++ simulation.exe
│
│  ┌───────────────────────────────────────────────────────────┐
│  │  main()                                                   │
│  │  ├─ parse argv[1..5]                                      │
│  │  │   argv[1] safetyTimeCap                                │
│  │  │   argv[2] numServers                                   │
│  │  │   argv[3] serviceTime                                  │
│  │  │   argv[4] arrivalInterval                              │
│  │  │   argv[5] totalCustomers                               │
│  │  │   │   ├── Main.cpp       ← original console entry point
│  │  │   └── WebSimulation.cpp  ← web-facing engine (JSON stdout)
│  │  │                                                        │
│  │  ├─ WebSimulation sim(...)                                │
│  │  │   ├─ ServerListType* servers      (real server pool)   │
│  │  │   ├─ CustomerType* customer_array (circular array)     │
│  │  │   ├─ server_states[]              (shadow state)       │
│  │  │   └─ next_server_hint             (round-robin ptr)    │
│  │  │                                                        │
│  │  ├─ sim.outputState()  ← tick 0                           │
│  │  │                                                        │
│  │  └─ while (!sim.isFinished())                             │
│  │       sim.tick()                                          │
│  │       │ 1. decrement shadow states; free + count_served   │
│  │       │ 2. updateServers() on real ServerListType         │
│  │       │ 3. enqueue new arrivals (≤ totalCustomers)        │
│  │       │ 4. round-robin assign free servers                │
│  │       sim.outputState()  → "STATE:{…}\n" to stdout        │
│  │                                                           │
│  └─ sim.outputFinalStats() → "FINAL:{…}\n" to stdout         │
└───────────────────────────────────────────────────────────────┘
```

---

## 3. Key Design Decisions

### 3.1 State Queue Pattern (stateQueue)

**Problem:** The C++ engine runs a complete 500-tick simulation in ~10 ms. If the frontend simply reads the last state, the user never sees the simulation animate.

**Solution:** Node.js collects every `STATE:{…}` line into `stateQueue[]`. The `GET /api/state` endpoint pops ONE entry per request. Since the browser polls every 350 ms, the simulation replays one tick per poll — appearing to run in real time.

```go
C++ (10 ms)         Node.js stateQueue       Browser (350 ms/poll)
────────────         ────────────────────     ─────────────────────
emit 500 STATEs  →  [S0, S1, ..., S499]  →  S0 → animate
                                          →  S1 → animate
                                          →  ...
                                          →  S499 → show "Complete"
```

### 3.2 Dual Server State (Shadow + Real)

**Problem:** `ServerListType` (real) and the UI-facing `server_states[]` (shadow) must always agree. Using only the real object caused 1-tick lag issues in `isFinished()`.

**Solution:**
- **Shadow `server_states[]`** is the single source of truth for: assignment (`getFreeServerRoundRobin`), termination (`allServersFree()`), and JSON output
- **Real `ServerListType`** is used for: `updateServers()` (correct tick-by-tick decrement) and `setServerBusy()` (sets the customer reference)
- Both are updated every tick in the same order, maintaining synchrony

### 3.3 Round-Robin vs. First-Free Assignment

**Problem:** `getFreeServerID()` always scans from index 0, so Server 0 handles every customer when arrival rate < service rate — other servers stay idle.

**Solution:** `next_server_hint` pointer starts at 0 and advances after each assignment:

```js
Tick 3:  C1 arrives → hint=0 → assign S0 → hint=1
Tick 6:  C2 arrives → hint=1 → assign S1 → hint=2
Tick 9:  C3 arrives → hint=2 → assign S2 → hint=0
Tick 12: C4 arrives → hint=0 → assign S0 → hint=1  (S0 is free again)
```

This guarantees uniform load distribution regardless of traffic intensity.

### 3.4 Total Customers = Queue Capacity

**Problem:** Previously `maxCustomers` meant "queue buffer size". Users setting 170 expected 170 to arrive, but it just meant the queue could hold 170 at once — arrivals continued indefinitely.

**Solution:**
- `totalCustomers` = exact number of customers who will ever arrive (simulation workload)
- Queue buffer = `totalCustomers` (always large enough; no one turned away)
- Simulation stops when all N have arrived **and** been served
- "Turned away" is always 0 by construction

### 3.5 Auto-Stop Condition

The simulation terminates when all three conditions hold simultaneously:

```cpp
bool isFinished() const {
    bool all_done = (customers_arrived >= total_arrivals_target)  // all arrived
                 && isQueueEmpty()                                 // none waiting
                 && allServersFree();                              // none being served
    return all_done || (current_clock >= simulation_time);         // or safety cap
}
```

This ensures `customersServed == totalCustomers` on every normal exit.

---

## 4. Class Diagram (C++ Engine)

```cpp
┌──────────────────────────────────────────────────────────┐
│                   WebSimulation                          │
│──────────────────────────────────────────────────────────│
│ - simulation_time    : int                               │
│ - number_of_servers  : int                               │
│ - transaction_time   : int                               │
│ - time_between_arrivals : int                            │
│ - total_arrivals_target : int                            │
│ - peak_queue_length  : int                               │
│ - next_server_hint   : int        ← round-robin ptr      │
│ - servers            : ServerListType*                   │
│ - customer_array     : CustomerType*  ← circular array   │
│ - queue_front        : int                               │
│ - queue_rear         : int                               │
│ - queue_size         : int                               │
│ - server_states      : vector<ServerState>  ← shadow     │
│ - customers_arrived  : int                               │
│ - customers_served   : int                               │
│ - current_clock      : int                               │
│──────────────────────────────────────────────────────────│
│ + tick()                                                 │
│ + outputState()                                          │
│ + isFinished() : bool                                    │
│ + allServersFree() : bool                                │
│ - getFreeServerRoundRobin() : int                        │
│ - enqueueCustomer(CustomerType)                          │
│ - dequeueCustomer() : CustomerType                       │
└──────────────────────────────────────────────────────────┘
         │ uses 1..*          │ uses 1
         ▼                   ▼
┌─────────────────┐  ┌──────────────────────────┐
│  ServerListType │  │      CustomerType        │
│─────────────────│  │──────────────────────────│
│ - servers_[]    │  │ - customer_number_  : int│
│ - num_of_servers│  │ - arrival_time_     : int│
│─────────────────│  │ - waiting_time_     : int│
│ + getFreeServerID│  │ - transaction_time_ : int│
│ + setServerBusy │  │──────────────────────────│
│ + updateServers │  │ + incrementWaitingTime() │
│ + getNumberOfBusy│  │ + getTransactionTime()   │
└────────┬────────┘  └──────────────────────────┘
         │ contains 1..*
         ▼
┌─────────────────┐
│   ServerType    │
│─────────────────│
│ - status_       │  "free" | "busy"
│ - transaction_  │
│   time_    : int│
│ - current_      │
│   customer_     │
│─────────────────│
│ + isFree()      │
│ + setBusy()     │
│ + setFree()     │
│ + decreaseTxnTm │
│ + setTxnTime()  │
└─────────────────┘
```

---

## 5. Sequence Diagram — Backend Mode Single Tick

```dsconfig
Browser          Node.js           C++ Process
   │                │                   │
   │  POST /start   │                   │
   │───────────────►│                   │
   │                │  spawn(exe, args)  │
   │                │──────────────────►│
   │                │                   │ tick() × N (fast)
   │                │◄──────────────────│ "STATE:{…}\n" × N
   │                │  stateQueue[0..N] │ "FINAL:{…}\n"
   │                │                   │ exit(0)
   │                │                   │
   │  GET /state    │                   │
   │───────────────►│                   │
   │                │ pop stateQueue[0] │
   │◄───────────────│ return JSON       │
   │  render(state) │                   │
   │                │                   │
   │  GET /state    │   (350 ms later)  │
   │───────────────►│                   │
   │                │ pop stateQueue[1] │
   │◄───────────────│ return JSON       │
   │  render(state) │                   │
   │       ·        │                   │
   │       ·  (repeats N times)         │
   │       ·        │                   │
   │  GET /state    │                   │
   │───────────────►│                   │
   │                │ stateQueue empty  │
   │◄───────────────│ running=false     │
   │  show "Done"   │                   │
```

---

## 6. Tick Execution Order

Each call to `tick()` in the C++ engine executes the following steps in strict order:

```d
tick N
│
├─ 1. DECREMENT SHADOW SERVER STATES
│     for each server i:
│       if server_states[i].busy:
│         remaining--
│         if remaining <= 0:
│           busy = false
│           customers_served++      ← counted on COMPLETION
│
├─ 2. UPDATE REAL ServerListType
│     updateServers():
│       for each real server:
│         decreaseTransactionTime()
│         if transactionTime == 0: setFree()
│
├─ 3. CUSTOMER ARRIVALS
│     if (clock % arrivalInterval == 0) AND (arrived < totalCustomers):
│       arrived++
│       enqueue(new CustomerType(...))
│
├─ 4. ROUND-ROBIN SERVER ASSIGNMENT
│     rr_id = getFreeServerRoundRobin()   ← checks shadow state
│     while rr_id != -1 AND queue non-empty:
│       customer = dequeue()
│       totalWait += clock - customer.arrivalTime
│       setServerBusy(rr_id, customer)    ← real server
│       server_states[rr_id] = {busy, remaining=txnTime, customerId}
│       rr_id = getFreeServerRoundRobin()
│
├─ 5. INCREMENT QUEUE WAITING TIMES
│     for each customer in circular array:
│       customer.waitingTime++
│
└─ 6. TRACK PEAK QUEUE LENGTH
      peakQueue = max(peakQueue, queueSize)
```

---

## 7. Error Handling

| Scenario                 | Handling                                                               |
| :----------------------- | :--------------------------------------------------------------------- |
| C++ process crashes      | Node.js `close` event sets `running: false`; error logged to `stderr`  |
| Invalid JSON from C++    | `parseLine` catch block logs error; stateQueue not updated             |
| Port 8081 already in use | Node.js exits with port-binding error message                          |
| Config with 0 servers    | `ServerListType` constructor clamps to minimum 1                       |
| arrivalMin > arrivalMax  | UI sliders have `min/max` constraints; server accepts as-is            |
| totalCustomers = 0       | Queue allocated with size 0; simulation finishes immediately at tick 0 |

---

## 8. Technology Stack

| Component         | Technology                           | Rationale                                         |
| :---------------- | :----------------------------------- | :------------------------------------------------ |
| Simulation engine | C++23                                | Performance, determinism, educational requirement |
| Web server        | Node.js + Express                    | Lightweight, good child_process IPC support       |
| Frontend          | Vanilla HTML/CSS/JS                  | No framework dependency; maximum transparency     |
| Canvas            | HTML5 Canvas API                     | Hardware-accelerated 2D ring rendering            |
| IPC               | stdout pipe (newline-delimited JSON) | Simple, language-agnostic, debuggable             |
| Build             | GNU Make + g++                       | Standard academic toolchain                       |
| CI                | GitHub Actions                       | clang-format, cpplint, clang-tidy, cppcheck       |
