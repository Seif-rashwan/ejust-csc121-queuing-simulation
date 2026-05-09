# Queuing System Simulation — CSC 121

![Build Status](https://github.com/Seif-rashwan/ejust-csc121-queuing-simulation/actions/workflows/code-style.yml/badge.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg?logo=unlicense)
![C++ Standard](https://img.shields.io/badge/C%2B%2B-23-blue.svg?logo=cplusplus)
![Node.js](https://img.shields.io/badge/Node.js-18%2B-brightgreen.svg?logo=nodedotjs)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)

**E-JUST · Faculty of Computer Science & Information Technology · Spring 2026**  
**Instructor:** Dr. Reda Elbasiony | **Standard:** C++23 | **Build:** g++ / GNU Make

---

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Repository Structure](#repository-structure)
4. [Features](#features)
5. [Quick Start](#quick-start)
6. [Build System](#build-system)
7. [Simulation Parameters](#simulation-parameters)
8. [API Reference](#api-reference)
9. [Documentation & Standards](#documentation--standards)
10. [Team](#team)

---

## Overview

A **discrete-time queuing system simulation** modelling a real-life service environment (bank, hospital, call centre). A logical clock advances one *tick* at a time, driving:

- Customer **arrivals** at a configurable interval
- FIFO **queue management** via a circular array
- **Multi-server dispatch** with round-robin load balancing
- Automatic **simulation termination** when every customer has been fully served

The project ships in two integrated layers:

| Layer        | Technology          | Role                                       |
| :----------- | :------------------ | :----------------------------------------- |
| **Engine**   | Modern C++23        | Deterministic tick-based simulation core   |
| **Server**   | Node.js / Express   | Spawns & streams the C++ process; REST API |
| **Frontend** | Vanilla HTML/CSS/JS | Real-time animated visualisation           |

---

## Architecture

The system is a **three-tier event-driven application**:

- **Tier 1 (Presentation):** Vanilla HTML/CSS/JS frontend running in the browser
- **Tier 2 (Orchestration):** Node.js/Express server managing the C++ process and REST API
- **Tier 3 (Engine):** C++23 binary performing deterministic tick-based simulation

**For detailed architecture diagrams, design decisions, class structure, and data flow sequences, see [ARCHITECTURE.md](ARCHITECTURE.md).**

**Quick overview:**
- Browser polls `/api/state` every 350 ms (default) for animation frames
- Node.js spawns the C++ engine, which runs the full simulation, after frontend config push
- C++ emits one `STATE:{…JSON…}` line per tick to stdout
- Node.js buffers these states and serves them to the frontend on demand
- Simulation auto-terminates when: all customers arrived + queue empty + all servers free

---

## Repository Structure

```mathematica
ejust-csc121-queuing-simulation/
│
├── build/
│   └── simulation.exe              ← compiled C++ engine (auto-generated)
├── include/
│   ├── CustomerType.h
│   ├── QueueADT.h
│   ├── ServerListType.h
│   ├── ServerType.h
│   ├── WaitingCustomerQueue.h
│   └── WebSimulation.h
├── src/
│   ├── CustomerType.cpp            ← customer data model
│   ├── ServerType.cpp              ← single-server FSM
│   ├── ServerListType.cpp          ← server pool (round-robin dispatch)
│   ├── WaitingCustomerQueue.cpp    ← circular-array FIFO queue
│   ├── Main.cpp                    ← original console entry point
│   └── WebSimulation.cpp           ← web-facing engine (JSON stdout)
├── server/
│   ├── node_modules/
│   ├── package.json
│   └── server.js                       ← Node.js REST API + child process manager
│
├── frontend/
│   ├── index.html                      ← single-page application shell
│   ├── style.css                       ← dark-mode design system
│   └── js/                             ← modular simulation controller & renderer
│
├── .github/
│   ├── scripts/
│   │   └── check_naming_conventions.py
│   └── workflows/
│       └── code-style.yml              ← CI: clang-format · cpplint · clang-tidy
│
├── .clang-format                       ← Google style, 100-col
├── .clang-tidy                         ← static analysis rules
├── .editorconfig
├── .gitignore
├── .pre-commit-config.yaml
├── CODING_STANDARD.md
├── CPPLINT.cfg
├── Doxyfile
├── Makefile
├── TASKS.md
└── README.md
```

---

## Features

### Simulation Engine (C++)

| Feature                   | Details                                                                                              |
| :------------------------ | :--------------------------------------------------------------------------------------------------- |
| **Tick-based clock**      | Deterministic discrete-time loop; each tick = one time unit                                          |
| **Circular array queue**  | Fixed-capacity FIFO; size = `totalCustomers` (auto-sized, never turns anyone away)                   |
| **Total-arrivals target** | `totalCustomers` = exact number of customers who will arrive; simulation ends when all are served    |
| **Round-robin dispatch**  | `next_server_hint` pointer rotates after every assignment — no server starves                        |
| **Shadow server states**  | Separate `server_states[]` array mirrors real `ServerListType` for consistent UI + termination logic |
| **Auto termination**      | Stops exactly when: `allArrived ∧ queueEmpty ∧ allServersFree` — no wasted ticks                     |
| **JSON stdout stream**    | Every tick emits `STATE:{…}` on stdout; final `FINAL:{…}` on completion                              |
| **Safety cap**            | `simulation_time = totalCustomers × arrivalInterval × 20` prevents infinite loops                    |

### Node.js Server

| Feature                     | Details                                                                                    |
| :-------------------------- | :----------------------------------------------------------------------------------------- |
| **State queue replay**      | All `STATE:` lines buffered into `stateQueue[]`; served one-per-poll for animated playback |
| **Line buffering**          | Handles fragmented stdout chunks correctly — no JSON parse errors                          |
| **Live config push**        | `POST /api/config` updates parameters before next run                                      |
| **Clean process lifecycle** | Old child process killed before new run; stateQueue cleared on reset                       |

### Frontend Visualisation

| Feature                    | Details                                                                         |
| :------------------------- | :------------------------------------------------------------------------------ |
| **Circular ring canvas**   | Animated arc shows occupied/empty/front/rear slots; scales dot size to capacity |
| **Arrival progress bar**   | Green→blue gradient bar: `Arrived X / N customers (%)`                          |
| **FIFO strip**             | Next 20 customers colour-coded by assigned server                               |
| **Server lane cards**      | Explicit assignment pattern per server (e.g. S1: C1, C4, C7…)                   |
| **Live server cards**      | Animated busy/idle state with remaining service time countdown                  |
| **Stat cards**             | Tick · Queue size · Served · Avg wait · Next arrival · Throughput               |
| **Dual mode**              | **Local** (pure JS, no server needed) · **Backend** (C++ engine via Node.js)    |
| **Live parameter sliders** | Servers · Arrival rate · Service rate · Total customers — all hot-reload        |
| **Auto-stop UI**           | Detects `running: false` and shows completion banner                            |

---

## Quick Start

### Prerequisites

| Tool      | Version      | Purpose                     |
| :-------- | :----------- | :-------------------------- |
| `g++`     | ≥ 11 (C++23) | Compile C++ engine          |
| `Node.js` | ≥ 18         | Run Express server          |
| `npm`     | ≥ 9          | Install JS dependencies     |
| `python`  | ≥ 3.7        | Pre-commit hooks (optional) |

### Option A — Local Mode (No server required)

```bash
# Just open the HTML file directly
start frontend/index.html        # Windows
open  frontend/index.html        # macOS
xdg-open frontend/index.html     # Linux
```

Click **Local** (default) → **Start**. The pure-JavaScript simulation runs entirely in the browser.

### Option B — Backend Mode (C++ engine)

```bash
# 1. One-time setup (installs pre-commit hooks + Node.js deps)
make install

# 2. Build the C++ binaries
make build          # Build both web and CLI binaries
# OR build individually:
make build-web      # Build web server binary only
make build-cli      # Build CLI binary only

# 3. Start the web server
make run            # Build and start Node.js server
```

Open **<http://localhost:8081>**, click **Backend** → **Start**.

### Option C — Console Mode (standalone CLI)

```bash
# Build and run CLI version
make run-cli        # Build CLI binary and run it
# OR manually:
make build-cli
./build/bin/simulation_cli.exe
```

---

## Build System

The project uses GNU Make for building. The build system is organized with separate directories for clarity:

```mathematica
build/
├── bin/           # Final executables
│   ├── simulation.exe        # Web server mode (Node.js spawns this)
│   └── simulation_cli.exe    # Standalone CLI mode
└── obj/           # Intermediate object files (.o) and dependencies (.d)
    ├── CustomerType.o
    ├── ServerType.o
    ├── ServerListType.o
    ├── WaitingCustomerQueue.o
    ├── WebSimulation.o
    └── Main.o
```

### Available Build Targets

| Target               | Description                                             |
| :------------------- | :------------------------------------------------------ |
| `make` or `make all` | Build both binaries                                     |
| `make build`         | Same as `make all`                                      |
| `make build-web`     | Build web server binary only                            |
| `make build-cli`     | Build CLI binary only                                   |
| `make run`           | Build web binary and start Node.js server               |
| `make run-cli`       | Build CLI binary and run it                             |
| `make install`       | One-time setup: install pre-commit hooks + Node.js deps |
| `make debug`         | Build with debug symbols (`-g -O0`)                     |
| `make lint`          | Run clang-tidy static analysis                          |
| `make format`        | Auto-format all C++ code                                |
| `make clean`         | Remove all build artifacts                              |

### Development Workflow

```bash
# First time setup
make install

# Development cycle
make build        # Compile changes
make run          # Test web interface
make lint         # Check code quality
make format       # Fix formatting issues
```

---

## Simulation Parameters

All parameters are adjustable at runtime via the web UI sliders and pushed to the server via `POST /api/config`.

| Parameter          | Slider ID       | Default | Range  | Description                               |
| :----------------- | :-------------- | :-----: | :----: | :---------------------------------------- |
| Total Customers    | `r-customers`   |   100   | 10–500 | Exact number of customers who will arrive |
| Servers            | `r-servers`     |    4    |  1–10  | Number of parallel service windows        |
| Arrival Rate (min) | `r-arrival-min` |    2    |  1–20  | Min ticks between successive arrivals     |
| Service Time (min) | `r-service-min` |    4    |  1–20  | Min ticks a server spends per customer    |
| Service Time (max) | `r-service-max` |   10    |  1–30  | Max ticks a server spends per customer    |

> **Note:** "Total Customers" is the *simulation workload*; how many people walk through the door. The queue buffer is auto-sized to this number so no customer is ever turned away due to capacity.

---

---

## Documentation & Standards

**For detailed specifications, conventions, and quality standards:**

| Document                                 | Purpose                                                                                                  |
| :--------------------------------------- | :------------------------------------------------------------------------------------------------------- |
| [CODING_STANDARD.md](CODING_STANDARD.md) | Naming conventions, formatting rules, header guards, Doxygen docs, error handling, pre-commit & CI setup |
| [ARCHITECTURE.md](ARCHITECTURE.md)       | System tiers, component diagrams, class structure, design decisions, tick execution, error handling      |
| [SRS.md](SRS.md)                         | Full software requirements: functional specs, performance targets, constraints, user stories             |
| [TASKS.md](TASKS.md)                     | Student task assignments with dependencies and submission tracking                                       |

---

## API Reference

Base URL: `http://localhost:8081`

| Method | Endpoint      | Body         | Response    | Description                                   |
| :----- | :------------ | :----------- | :---------- | :-------------------------------------------- |
| `GET`  | `/api/state`  | —            | `StateJSON` | Pop & return next tick snapshot from queue    |
| `POST` | `/api/config` | `ConfigJSON` | `{success}` | Update simulation parameters                  |
| `POST` | `/api/start`  | —            | `{success}` | Kill existing process; spawn new C++ run      |
| `POST` | `/api/pause`  | —            | `{success}` | Kill C++ process (pause = stop current run)   |
| `POST` | `/api/reset`  | —            | `{success}` | Kill process + clear state + clear stateQueue |

### StateJSON Schema

```jsonc
{
  "tick":            559,      // current clock tick
  "queueSize":       12,       // customers waiting in queue
  "served":          47,       // total customers fully served
  "turnedAway":      0,        // customers rejected (always 0 with auto-sized queue)
  "peakQueue":       18,       // maximum queue size ever recorded
  "avgWait":         6.4,      // average waiting time (ticks) per served customer
  "nextArrival":     2,        // ticks until next customer arrives
  "running":         true,     // false when simulation is complete
  "totalCustomers":  100,      // simulation workload target
  "arrived":         59,       // customers who have arrived so far
  "lastEvent":       "serving",// "arrived" | "serving" | ""
  "lastEventCustomer": 59,     // customer ID of last event
  "servers": [                 // one entry per server
    { "busy": true,  "remaining": 3, "customerId": 52 },
    { "busy": false, "remaining": 0, "customerId": -1 }
  ]
}
```

---

## Team

| Name                   | Student ID |
| :--------------------- | :--------: |
| Seif Rashwan           | 120210256  |
| Hana Aly ElMaghraby    | 320240126  |
| Nour Ibrahim           | 320250325  |
| Androw Shonoda Tawfeek | 320210333  |
| @EgizianoEG            |     —      |
