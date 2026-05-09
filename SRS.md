# Software Requirements Specification (SRS)

## Queuing System Simulation — CSC 121

**Document Version:** 2.0  
**Date:** May 2026  
**Institution:** E-JUST · Faculty of Computer Science & Information Technology  
**Course:** CSC 121 — Data Structures  
**Instructor:** Dr. Reda Elbasiony

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Overall Description](#2-overall-description)
3. [Functional Requirements](#3-functional-requirements)
4. [Non-Functional Requirements](#4-non-functional-requirements)
5. [System Architecture Requirements](#5-system-architecture-requirements)
6. [External Interface Requirements](#6-external-interface-requirements)
7. [Data Requirements](#7-data-requirements)
8. [Constraints & Assumptions](#8-constraints--assumptions)

---

## 1. Introduction

### 1.1 Purpose

This document specifies the software requirements for the **Queuing System Simulation**, a discrete-time simulation of a real-world service environment. It defines what the system must do (functional requirements), how it must perform (non-functional requirements), and the constraints under which it operates.

### 1.2 Scope

The system simulates customers arriving at a service centre, waiting in a FIFO queue, and being served by one or more parallel servers. It provides:

- A **C++ simulation engine** implementing the core data structures and tick logic
- A **Node.js REST API** bridging the engine to a browser-based interface
- A **Web frontend** delivering real-time animated visualisation

### 1.3 Definitions

| Term                | Definition                                                                       |
| :------------------ | :------------------------------------------------------------------------------- |
| **Tick**            | One discrete time unit; the clock advances by 1 per simulation step              |
| **Customer**        | An entity that arrives, waits in queue, and is served                            |
| **Total Customers** | The exact number of customers that will arrive in a simulation run               |
| **Queue**           | FIFO circular array holding customers waiting for service                        |
| **Server**          | A service unit; can be **Free** or **Busy** at any tick                          |
| **Round-robin**     | Assignment strategy that distributes customers evenly across all servers         |
| **Shadow state**    | A mirror of server busy/idle state maintained by the engine for consistent logic |
| **stateQueue**      | Node.js buffer of all STATE snapshots; popped one-per-poll for animated replay   |

### 1.4 References

- Course materials: CSC 121, E-JUST Spring 2026
- Gross & Harris, *Fundamentals of Queueing Theory*, 4th ed.
- ISO/IEC 14882:2023 (C++23 Standard)

---

## 2. Overall Description

### 2.1 Product Perspective

The system operates as a standalone educational tool. It has two independent run modes:

```mathematica
┌──────────────────────────────────────────────────────┐
│                   Run Mode                           │
│                                                      │
│  ┌─────────────────┐      ┌────────────────────────┐ │
│  │  Local (JS)     │      │  Backend (C++ + Node)  │ │
│  │                 │      │                        │ │
│  │ Browser-only    │      │ C++ engine → Node.js   │ │
│  │ No server needed│      │ REST API → Browser     │ │
│  └─────────────────┘      └────────────────────────┘ │
└──────────────────────────────────────────────────────┘
```

### 2.2 Product Functions Summary

1. Accept user-defined simulation parameters
2. Simulate customer arrivals, queue management, and service
3. Terminate automatically when all customers are fully served
4. Visualise the simulation state in real time
5. Report final statistics on completion

### 2.3 User Classes

| User Class     | Description                                                          |
| :------------- | :------------------------------------------------------------------- |
| **Student**    | Primary user; runs simulations to observe queuing behaviour          |
| **Instructor** | Reviews outputs and statistics; may set specific parameter scenarios |
| **Developer**  | Extends or maintains the codebase                                    |

---

## 3. Functional Requirements

### FR-1: Simulation Parameters

| ID     | Requirement                                                                                                  |
| :----- | :----------------------------------------------------------------------------------------------------------- |
| FR-1.1 | The system SHALL accept **Total Customers** (N): the exact count of customers who will arrive (range: 1–500) |
| FR-1.2 | The system SHALL accept **Number of Servers** (range: 1–10)                                                  |
| FR-1.3 | The system SHALL accept **Arrival Interval** in ticks (min and max for randomised mode)                      |
| FR-1.4 | The system SHALL accept **Service Time** in ticks (min and max)                                              |
| FR-1.5 | Parameters SHALL be adjustable at any time via the web UI without restarting the application                 |

### FR-2: Customer Arrivals

| ID     | Requirement                                                                                        |
| :----- | :------------------------------------------------------------------------------------------------- |
| FR-2.1 | A new customer SHALL arrive every `arrivalInterval` ticks until exactly `N` customers have arrived |
| FR-2.2 | Once `N` customers have arrived, no further arrivals SHALL occur                                   |
| FR-2.3 | Each customer SHALL be assigned a unique sequential ID                                             |
| FR-2.4 | Each customer SHALL record its arrival tick for waiting-time calculation                           |

### FR-3: Queue Management

| ID     | Requirement                                                                                        |
| :----- | :------------------------------------------------------------------------------------------------- |
| FR-3.1 | The waiting queue SHALL be implemented as a **circular array** with FIFO ordering                  |
| FR-3.2 | Queue capacity SHALL be automatically set equal to `N` (Total Customers)                           |
| FR-3.3 | Since capacity = `N` and only `N` customers ever arrive, **no customer shall ever be turned away** |
| FR-3.4 | The queue SHALL correctly maintain `front` and `rear` pointers using modular arithmetic            |
| FR-3.5 | Every customer remaining in queue SHALL have their waiting time incremented once per tick          |

### FR-4: Server Management

| ID     | Requirement                                                                                                                                                 |
| :----- | :---------------------------------------------------------------------------------------------------------------------------------------------------------- |
| FR-4.1 | Each server SHALL maintain a **Free** or **Busy** state                                                                                                     |
| FR-4.2 | When a server becomes free and the queue is non-empty, it SHALL immediately accept the next customer                                                        |
| FR-4.3 | Server assignment SHALL use **round-robin** — a `next_server_hint` pointer advances after each assignment, distributing customers evenly across all servers |
| FR-4.4 | A busy server SHALL decrement its remaining service time by 1 each tick                                                                                     |
| FR-4.5 | When remaining service time reaches 0, the server SHALL transition to **Free**                                                                              |
| FR-4.6 | The system SHALL track two parallel server state representations: real `ServerType` objects and a shadow `server_states[]` array. Both MUST agree.          |

### FR-5: Simulation Termination

| ID     | Requirement                                                                                                                                      |
| :----- | :----------------------------------------------------------------------------------------------------------------------------------------------- |
| FR-5.1 | The simulation SHALL automatically stop when ALL three conditions are true: (a) `totalArrived ≥ N`, (b) queue is empty, (c) all servers are free |
| FR-5.2 | A **safety cap** (`N × arrivalInterval × 20` ticks) SHALL prevent infinite loops in degenerate configurations                                    |
| FR-5.3 | The final tick SHALL emit `running: false` in its state snapshot                                                                                 |

### FR-6: Statistics & Output

| ID     | Requirement                                                                                              |
| :----- | :------------------------------------------------------------------------------------------------------- |
| FR-6.1 | The system SHALL report **total customers arrived**                                                      |
| FR-6.2 | The system SHALL report **total customers served** (customers_served SHALL equal N on normal completion) |
| FR-6.3 | The system SHALL report **peak queue length**                                                            |
| FR-6.4 | The system SHALL report **average waiting time** per served customer                                     |
| FR-6.5 | The C++ engine SHALL emit one `STATE:{JSON}` line per tick to stdout                                     |
| FR-6.6 | The C++ engine SHALL emit one `FINAL:{JSON}` line on completion                                          |

### FR-7: Frontend Visualisation

| ID     | Requirement                                                                                                |
| :----- | :--------------------------------------------------------------------------------------------------------- |
| FR-7.1 | The frontend SHALL display a **circular ring canvas** showing occupied/empty slots and front/rear pointers |
| FR-7.2 | The ring SHALL dynamically scale to the configured `totalCustomers` (10–500 slots)                         |
| FR-7.3 | The frontend SHALL display an **arrival progress bar** showing `arrived / totalCustomers %`                |
| FR-7.4 | The frontend SHALL display a **FIFO strip** of the next 20 customers, colour-coded by assigned server      |
| FR-7.5 | The frontend SHALL display an animated **server card** per server showing busy/idle state and countdown    |
| FR-7.6 | The frontend SHALL display six **stat cards**: Tick, Queue, Served, Avg Wait, Next Arrival, Throughput     |
| FR-7.7 | The frontend SHALL support two modes: **Local** (JS simulation) and **Backend** (C++ via Node.js)          |
| FR-7.8 | In Backend mode, the frontend SHALL poll `GET /api/state` and animate one tick per poll interval           |

### FR-8: Backend API

| ID     | Requirement                                                                                               |
| :----- | :-------------------------------------------------------------------------------------------------------- |
| FR-8.1 | `POST /api/start` SHALL spawn the C++ process with current configuration as CLI arguments                 |
| FR-8.2 | `GET /api/state` SHALL pop and return the next snapshot from `stateQueue`; if empty, return current state |
| FR-8.3 | `POST /api/config` SHALL update the server-side configuration object                                      |
| FR-8.4 | `POST /api/reset` SHALL kill the child process and clear all state                                        |
| FR-8.5 | The server SHALL buffer ALL C++ stdout lines into `stateQueue[]` and serve them sequentially              |

---

## 4. Non-Functional Requirements

### NFR-1: Performance

| ID      | Requirement                                                                               |
| :------ | :---------------------------------------------------------------------------------------- |
| NFR-1.1 | The C++ engine SHALL complete a 500-customer simulation in under 100 ms on any modern PC  |
| NFR-1.2 | The Node.js server SHALL handle state polling at ≥ 10 requests/second without queuing lag |
| NFR-1.3 | The frontend canvas SHALL render at ≥ 30 fps during animation                             |

### NFR-2: Correctness

| ID      | Requirement                                                                                                     |
| :------ | :-------------------------------------------------------------------------------------------------------------- |
| NFR-2.1 | On normal completion, `customersServed` SHALL equal `totalCustomers`                                            |
| NFR-2.2 | The circular array MUST NOT access out-of-bounds indices: all pointer arithmetic uses `% totalCustomers`        |
| NFR-2.3 | Round-robin assignment MUST be verified: each server SHALL receive an approximately equal share over a full run |
| NFR-2.4 | Waiting time MUST equal `servedTick − arrivalTick` for every customer                                           |

### NFR-3: Reliability

| ID      | Requirement                                                                                    |
| :------ | :--------------------------------------------------------------------------------------------- |
| NFR-3.1 | The C++ engine MUST NOT write anything other than `STATE:{…}` or `FINAL:{…}` to stdout         |
| NFR-3.2 | Debug `std::cout` statements are PROHIBITED in `WebSimulation.cpp`                             |
| NFR-3.3 | The Node.js server MUST handle fragmented stdout chunks without crashing (`JSON.parse` errors) |
| NFR-3.4 | Killing the C++ process (pause/reset) MUST NOT leave zombie processes                          |

### NFR-4: Maintainability

| ID      | Requirement                                                                                |
| :------ | :----------------------------------------------------------------------------------------- |
| NFR-4.1 | All C++ files SHALL comply with the project's C++23 coding standard and clang-format rules |
| NFR-4.2 | The JavaScript frontend SHALL use no external frameworks (Vanilla JS only)                 |
| NFR-4.3 | All magic numbers SHALL be named constants or derived from configuration                   |

### NFR-5: Usability

| ID      | Requirement                                                                                                        |
| :------ | :----------------------------------------------------------------------------------------------------------------- |
| NFR-5.1 | A user with no programming background SHALL be able to start a simulation within 30 seconds of opening the browser |
| NFR-5.2 | Parameter sliders SHALL update the UI value label in real time                                                     |
| NFR-5.3 | The simulation SHALL visually indicate when it has completed (auto-stop + UI banner)                               |

---

## 5. System Architecture Requirements

| ID   | Requirement                                                                                                                       |
| :--- | :-------------------------------------------------------------------------------------------------------------------------------- |
| AR-1 | The simulation engine SHALL be separated from the I/O and visualisation layers                                                    |
| AR-2 | Communication between C++ and Node.js SHALL use **newline-delimited JSON over stdout**                                            |
| AR-3 | The Node.js server SHALL act as a pure **data relay** — no simulation logic                                                       |
| AR-4 | The frontend SHALL work in standalone mode without the Node.js server                                                             |
| AR-5 | The `stateQueue` pattern SHALL be used so the C++ engine can run at full speed while the frontend animates at human-readable pace |

---

## 6. External Interface Requirements

### 6.1 User Interface

- Single-page web application served on `http://localhost:8081`
- Dark-mode design with glassmorphism card components
- Responsive layout (minimum 1024px width recommended)
- Font: Google Fonts (Inter / system sans-serif fallback)

### 6.2 Software Interfaces

| Interface         | Technology                        | Version                           |
| :---------------- | :-------------------------------- | :-------------------------------- |
| Simulation engine | C++                               | ISO C++23                         |
| Web server        | Node.js + Express                 | Node ≥ 18                         |
| IPC               | child_process.spawn (stdout pipe) | —                                 |
| Browser           | Any modern browser                | Chrome 90+, Firefox 88+, Edge 90+ |

### 6.3 Communication Interface

- HTTP/1.1 REST on `localhost:8081`
- CORS enabled for local development
- Polling interval: 350 ms (configurable in `frontend/js/main.js`)

---

## 7. Data Requirements

### 7.1 Customer Record

| Field             | Type | Description                           |
| :---------------- | :--- | :------------------------------------ |
| `customerNumber`  | int  | Sequential ID (1-based)               |
| `arrivalTime`     | int  | Tick at which customer arrived        |
| `waitingTime`     | int  | Accumulated ticks spent in queue      |
| `transactionTime` | int  | Service duration assigned at creation |

### 7.2 Server Record

| Field               | Type         | Description                     |
| :------------------ | :----------- | :------------------------------ |
| `status_`           | string       | `"free"` or `"busy"`            |
| `transaction_time_` | int          | Remaining service ticks         |
| `current_customer_` | CustomerType | Customer currently being served |

### 7.3 STATE JSON Snapshot

Emitted once per tick. Fields: `tick`, `queueSize`, `served`, `turnedAway`, `peakQueue`, `avgWait`, `nextArrival`, `running`, `totalCustomers`, `arrived`, `lastEvent`, `lastEventCustomer`, `servers[]`.

### 7.4 FINAL JSON Summary

Emitted once on simulation exit. Fields: `totalSimulationTime`, `totalCustomersArrived`, `customersServed`, `customersLeftInQueue`, `customersTurnedAway`, `peakQueueLength`, `averageWaitingTime`.

---

## 8. Constraints & Assumptions

| ID   | Constraint / Assumption                                                                                              |
| :--- | :------------------------------------------------------------------------------------------------------------------- |
| C-1  | The simulation is **deterministic in the C++ backend** (fixed arrival interval and service time)                     |
| C-2  | The JavaScript local mode uses **random service times** within [serviceMin, serviceMax]                              |
| C-3  | The system does NOT support priority queuing in this version                                                         |
| C-4  | All customers are identical in priority — pure FIFO ordering                                                         |
| C-5  | The web server runs on `localhost` only; no authentication is provided                                               |
| C-6  | Maximum supported `totalCustomers` is 500 (ring canvas performance limit)                                            |
| C-7  | The safety cap prevents but does not guarantee liveness for pathological configurations (e.g., arrival interval = 0) |
| A-1  | Users have a modern web browser with JavaScript and Canvas API support                                               |
| A-2  | `g++` (C++23) and `Node.js ≥ 18` are installed on the host machine                                                   |
| A-3  | Port 8081 is available on localhost                                                                                  |
