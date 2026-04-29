# 📋 Task Assignment — Queuing System Simulation

**CSC 121 · Data Structures and Algorithms · E-JUST Spring 2026**
**Instructor:** Dr. Reda Elbasiony
**Repo:** https://github.com/Seif-rashwan/ejust-csc121-queuing-simulation
**Deadline:** 2 weeks from assignment date

---

## 🔁 Push Order (follow this sequence!)

```
T1 customerType  →  T2 serverType + T3 Queue (parallel)  →  T4 serverListType  →  T5 main.cpp
```

---

## 📌 Task Table

| Task | Core File | Difficulty | Depends On | Assignee |
| :--: | :--- | :---: | :--- | :--- |
| **T1** | `customerType.cpp` + **Report** | 🟢 Light + Docs | None — start immediately | ................ |
| **T2** | `serverType.cpp` + **Bonus UI** | 🟢 Light + Creative | `customerType.h` (T1) | ................ |
| **T3** | `waitingCustomerQueue.cpp` | 🔴 Heavy (Core DS) | `customerType.h` (T1) · `queueADT.h` | ................ |
| **T4** | `serverListType.cpp` | 🟡 Medium | `serverType.h` (T2) | ................ |
| **T5** | `main.cpp` | 🔴 Heavy (Engine) | ALL other tasks done | ................ |

---

## 📝 Job Descriptions

### T1 · `customerType.cpp` + Report 🟢
> **Can start immediately — no dependencies**

**Code (`customerType.cpp`):**
- Implement all getters/setters for customer data (slide 99 UML)
- `setCustomerInfo(int, int, int, int)` — sets number, arrival, waiting, transaction time
- `incrementWaitingTime()` — adds 1 to waiting time each tick

**Report (final deliverable):**
- Write the project PDF report describing the simulation design
- Analyze how changing `NUM_SERVERS`, `ARRIVAL_INTERVAL`, and `TXN_TIME` affects:
  - Average waiting time
  - Peak queue length
  - Server utilization

---

### T2 · `serverType.cpp` + Bonus UI 🟢
> **Needs:** `customerType.h` from T1

**Code (`serverType.cpp`):**
- Implement free/busy status: `isFree()`, `setBusy()`, `setFree()`
- `setCurrentCustomer(customerType)` — assign customer, mark busy, start countdown
- `decreaseTransactionTime()` — decrement each tick; auto-call `setFree()` when it hits 0
- `setTransactionTime(int)` and `setTransactionTime()` overloads
- All `getCurrentCustomer*()` getters (slide 100)

**Bonus UI:**
- Build a terminal/text-based UI that visualizes the simulation live each tick
- Show server statuses (free/busy + remaining time) and queue length as the clock runs

---

### T3 · `waitingCustomerQueue.cpp` 🔴
> **Needs:** `customerType.h` from T1 · `queueADT.h` (already in repo)

**Code (`waitingCustomerQueue.cpp` + `queueType.h`):**
- Implement the full circular-array `queueType<T>` inheriting from `queueADT` (slides 67–87)
- `initializeQueue()` → `queueFront=0`, `queueRear=maxQueueSize-1`, `count=0`
- `isEmptyQueue()` → `count == 0` · `isFullQueue()` → `count == maxQueueSize`
- `front()` / `back()` → assert/terminate if empty (slide 84)
- `addQueue(e)` → advance `queueRear` circularly using `% maxQueueSize`, store, `count++`
- `deleteQueue()` → `count--`, advance `queueFront` circularly
- Extend with `incrementWaitingTimes()` — rotate through all elements calling `incrementWaitingTime()` on each (slide 102)

> ⚠️ Must use our own `queueType`, **not** `std::queue`

---

### T4 · `serverListType.cpp` 🟡
> **Needs:** `serverType.h` from T2

**Code (`serverListType.cpp`):**
- Manage dynamic array of `serverType` objects (slide 101 UML)
- `getFreeServerID()` — scan array, return index of first free server or `-1`
- `getNumberOfBusyServers()` — count busy servers
- `setServerBusy(int id, customerType, int clock)` — assign customer, log the event
- `updateServers(ostream&, int clock)` — tick every busy server; **save customer number BEFORE calling `decreaseTransactionTime()`**, then log if server becomes free

---

### T5 · `main.cpp` 🔴
> **Needs:** ALL other tasks complete and merged to `main`

**Code (`main.cpp`):**
- Wire all classes into the simulation loop (slide 103 — 4 steps in order):
  1. `serverList.updateServers()` — decrement busy servers
  2. `waitQueue.incrementWaitingTimes()` — if queue non-empty
  3. If `clock % ARRIVAL_INTERVAL == 0` — create customer, `addQueue()`
  4. While free server exists AND queue non-empty — `front()` + `deleteQueue()` + `setServerBusy()`
- Configurable constants at top of file: `SIM_TIME`, `NUM_SERVERS`, `ARRIVAL_INTERVAL`, `MIN_TXN_TIME`, `MAX_TXN_TIME`
- Console output: arrivals, assignments, completions, final summary (avg wait, peak queue, total customers)

---

## 🌿 Git Workflow

```bash
# 1. Clone the repo
git clone https://github.com/Seif-rashwan/ejust-csc121-queuing-simulation.git

# 2. Create your branch
git checkout -b feature/task-N     # e.g. feature/task-1

# 3. Open your files
#    → src/yourFile.cpp
#    → include/yourFile.h

# 4. Commit and push
git add .
git commit -m "feat: implement customerType getters and setters"
git push origin feature/task-N

# 5. Open a Pull Request → main on GitHub
#    At least one teammate must review before merge
```

### Branch Names
| Task | Branch |
| :--: | :--- |
| T1 | `feature/task-1-customer` |
| T2 | `feature/task-2-server` |
| T3 | `feature/task-3-queue` |
| T4 | `feature/task-4-serverlist` |
| T5 | `feature/task-5-main` |

---

## ✅ Phase 1 Done When...

- [ ] Code compiles with zero warnings: `make` runs clean
- [ ] Circular queue handles `AAADADADA...` without phantom-full bug
- [ ] `decreaseTransactionTime()` calls `setFree()` exactly when time hits 0
- [ ] Main loop steps run in the correct order (slide 103)
- [ ] All 5 members have commits on their branch
- [ ] All PRs reviewed and merged before the demo

---

## 🚀 Phase 2 — Bonus (after Phase 1 is done)

| Priority | Feature | Owner |
| :---: | :--- | :--- |
| High | Terminal UI (T2 owner) | T2 |
| High | CSV stats export for graphing | TBD |
| Medium | Multi-scenario comparison (vary server count) | TBD |
| Medium | Priority queue variant (VIP customers) | TBD |

---

*Good luck everyone! Push `customerType` first so the rest can start* 🚀
