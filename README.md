# Queuing System Simulation, CSC 121

![Build Status](https://github.com/Seif-rashwan/ejust-csc121-queuing-simulation/actions/workflows/code-style.yml/badge.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg?logo=unlicense)
![C++ Standard](https://img.shields.io/badge/C%2B%2B-23-blue.svg?logo=cplusplus)

**E-JUST · Faculty of Computer Science & Information Technology · Spring 2026**  
**Instructor:** Dr. Reda Elbasiony &nbsp;|&nbsp; **Standard:** C++23 &nbsp;|&nbsp; **Build:** GNU Make

---

## Overview

A discrete-time simulation of a real-life service environment (bank / hospital / call center) built on a **FIFO queue** and a configurable **server pool**. A clock steps forward one tick at a time, driving customer arrivals, queue management, and service completion.

---

## Repository Structure

```text
ejust-csc121-queuing-simulation/
├── .github/
│   ├── scripts/
│   │   └── check_naming_conventions.py   # Enforces PascalCase file naming
│   └── workflows/
│       └── code-style.yml                # CI: clang-format · cpplint · clang-tidy · cppcheck
├── include/
│   ├── CustomerType.h                    # Customer data & interface
│   ├── QueueADT.h                        # Abstract queue interface
│   ├── ServerListType.h                  # Server pool interface
│   ├── ServerType.h                      # Single-server interface
│   └── WaitingCustomerQueue.h            # Circular-array FIFO queue + wait-time increment
│   ├── CustomerType.h                    # Customer data & interface
│   ├── QueueADT.h                        # Abstract queue interface
│   ├── ServerListType.h                  # Server pool interface
│   ├── ServerType.h                      # Single-server interface
│   └── WaitingCustomerQueue.h            # Circular-array FIFO queue + wait-time increment
├── src/
│   ├── CustomerType.cpp                  # Customer implementation
│   ├── Main.cpp                          # Simulation loop & results
│   ├── ServerListType.cpp                # Server pool implementation
│   ├── ServerType.cpp                    # Server implementation
│   └── WaitingCustomerQueue.cpp          # Queue implementation
├── .clang-format                         # Google style, 100-col, C++20 base
├── .clang-tidy                           # Static analysis & naming rules
├── .editorconfig                         # Editor whitespace/encoding consistency
├── .gitignore                            # Git ignore rules
├── .pre-commit-config.yaml               # Local clang-format + clang-tidy hooks
├── CODING_STANDARD.md                    # Full team coding standard reference
├── CPPLINT.cfg                           # cpplint rule overrides
├── Doxyfile                              # Doxygen documentation config
├── Makefile                              # Build system
├── TASKS.md                              # Per-member task breakdown & Git workflow
│   ├── CustomerType.cpp                  # Customer implementation
│   ├── Main.cpp                          # Simulation loop & results
│   ├── ServerListType.cpp                # Server pool implementation
│   ├── ServerType.cpp                    # Server implementation
│   └── WaitingCustomerQueue.cpp          # Queue implementation
├── .clang-format                         # Google style, 100-col, C++20 base
├── .clang-tidy                           # Static analysis & naming rules
├── .editorconfig                         # Editor whitespace/encoding consistency
├── .gitignore                            # Git ignore rules
├── .pre-commit-config.yaml               # Local clang-format + clang-tidy hooks
├── CODING_STANDARD.md                    # Full team coding standard reference
├── CPPLINT.cfg                           # cpplint rule overrides
├── Doxyfile                              # Doxygen documentation config
├── Makefile                              # Build system
├── TASKS.md                              # Per-member task breakdown & Git workflow
└── README.md
```

---

## Building & Running

> **Requires:** g++ with C++23 support, GNU Make.

---

## Building & Running

> **Requires:** g++ with C++23 support, GNU Make.

```bash
make            # Compile all sources → simulation[.exe]
make run        # Compile + execute
make debug      # Compile with -g -O0 for debugging
make lint       # Run clang-tidy against all sources
make format     # Auto-format all sources with clang-format
make clean      # Remove build artifacts and binary
make            # Compile all sources → simulation[.exe]
make run        # Compile + execute
make debug      # Compile with -g -O0 for debugging
make lint       # Run clang-tidy against all sources
make format     # Auto-format all sources with clang-format
make clean      # Remove build artifacts and binary
```

---

---

## Simulation Parameters

Configurable constants at the top of `src/Main.cpp`:

| Constant           | Default | Description                                |
| :----------------- | :-----: | :----------------------------------------- |
| `SIM_TIME`         |  `100`  | Total clock ticks to simulate              |
| `NUM_SERVERS`      |   `3`   | Number of parallel servers                 |
| `ARRIVAL_INTERVAL` |   `4`   | Ticks between successive customer arrivals |
| `MIN_TXN_TIME`     |   `2`   | Minimum service duration (ticks)           |
| `MAX_TXN_TIME`     |   `6`   | Maximum service duration (ticks)           |

---

## Core Classes

| Class                  | File                     | Responsibility                                                                                             |
| :--------------------- | :----------------------- | :--------------------------------------------------------------------------------------------------------- |
| `CustomerType`         | `CustomerType.*`         | Stores customer number, arrival time, waiting time, and transaction time; exposes `incrementWaitingTime()` |
| `ServerType`           | `ServerType.*`           | Tracks free/busy state, current customer, and remaining service time; auto-frees on countdown expiry       |
| `ServerListType`       | `ServerListType.*`       | Manages the server pool; finds free servers, dispatches customers, ticks every busy server                 |
| `WaitingCustomerQueue` | `WaitingCustomerQueue.*` | Circular-array FIFO queue inheriting `QueueADT<T>`; adds `incrementWaitingTimes()` for bulk tick updates   |

---

## Output Metrics

The simulation prints a timestamped event log and a final summary including:

- Total customers served
- Customers remaining in queue at end of simulation
- Peak queue length recorded
- Average customer waiting time

---

## Code Quality & Tooling

This project enforces a documented [C++ Coding Standard](CODING_STANDARD.md) (C++23, Google style) through a layered tooling stack:

### Local, Pre-commit Hooks

Runs automatically on every `git commit` against staged files only.

```bash
pip install pre-commit
python -m pre_commit install
```

| Hook            | Tool               | Checks                                                 |
| :-------------- | :----------------- | :----------------------------------------------------- |
| Formatting      | `clang-format v22` | Google style, 100-col, 4-space indent                  |
| Static analysis | `clang-tidy`       | Naming conventions, `[[nodiscard]]`, readability rules |

### Remote, GitHub Actions CI

Every push and pull request triggers the `Code Style & Quality Check` workflow:

| Job                       | Tool                  | What It Checks                                         |
| :------------------------ | :-------------------- | :----------------------------------------------------- |
| `clang-format`            | clang-format          | Formatting compliance; posts PR comment on failure     |
| `cpplint`                 | cpplint               | Google style lint rules (configured via `CPPLINT.cfg`) |
| `file-naming-conventions` | custom Python script  | `PascalCase` requirement for all `.cpp` / `.h` files   |
| `code-quality`            | cppcheck + clang-tidy | Memory safety, UB, naming, and design issues           |

---

## Naming Conventions (Quick Reference)

| Construct         | Convention                   | Example                    |
| :---------------- | :--------------------------- | :------------------------- |
| Files             | `PascalCase`                 | `CustomerType.cpp`         |
| Classes & Structs | `PascalCase`                 | `class ServerListType`     |
| Private members   | `snake_case_` (trailing `_`) | `int customer_id_;`        |
| Methods           | `camelCase`                  | `void getCustomerId()`     |
| Free functions    | `PascalCase`                 | `void ProcessQueue()`      |
| Parameters        | `snake_case`                 | `int arrival_time`         |
| Constants         | `UPPER_SNAKE_CASE`           | `const int MAX_QUEUE_SIZE` |

---

## Web GUI Enhancement (Phase 2 Complete)

A modern web-based interface has been added to complement the console simulation:

### Features
- **Modern Web Interface**: Beautiful, responsive design with real-time visualization
- **Dual Mode Operation**: 
  - Local JavaScript simulation for quick testing
  - C++ backend integration for high-performance simulation
- **Real-time Statistics**: Live updates of queue metrics, server utilization, and performance indicators
- **Interactive Controls**: Adjust parameters on-the-fly with instant visual feedback
- **Detailed Analytics**: Comprehensive statistics modal with performance metrics

### Quick Start (Web GUI)

#### Option 1: Local Mode (JavaScript Only)
1. Open `index.html` in your web browser
2. The simulation will start automatically in local mode
3. Use controls to adjust parameters and observe simulation

#### Option 2: Backend Mode (C++ + Node.js)
1. Install Node.js dependencies: `npm install`
2. Build C++ simulation: `npm run build`
3. Start web server: `npm start`
4. Open `http://localhost:8080` and click "Backend" mode

### Web GUI Files
- `index.html` - Main web interface
- `style-enhanced.css` - Modern styling
- `script-enhanced.js` - Frontend logic with backend integration
- `server.js` - Node.js web server for C++ integration
- `package.json` - Node.js dependencies and scripts

### API Endpoints (Backend Mode)
- `GET /api/state` - Get current simulation state
- `POST /api/config` - Update simulation parameters
- `POST /api/start` - Start C++ simulation
- `POST /api/pause` - Pause/resume simulation
- `POST /api/reset` - Reset simulation

---

## Phase 2, Bonus Extensions/Features

| Priority | Feature                                                               | Status |
| :------: | :-------------------------------------------------------------------- | :-----: |
|  🔴 High  | Live terminal UI visualizing server states & queue length each tick   | ✅ Done |
|  🔴 High  | CSV export of per-tick statistics for external graphing               | ✅ Done |
| 🟡 Medium | Multi-scenario comparison (vary `NUM_SERVERS` and `ARRIVAL_INTERVAL`) | ✅ Done |
| 🟡 Medium | Priority queue variant (VIP customers with preemption)                | 🔄 In Progress |

---

## Team

| Name                   | Student ID |
| :--------------------- | :--------: |
| Seif Rashwan           | 120210256  |
| Hana Aly ElMaghraby    | 320240126  |
| Nour Ibrahim           | 320250325  |
| Androw Shonoda Tawfeek | 320210333  |
| @EgizianoEG            |     --     |
