# Queuing System Simulation — CSC 121 Project
**E-JUST · Faculty of Computer Science & Information Technology · Spring 2026**

## Project Overview
Simulates a real-life service environment (bank / hospital / call center) using a **FIFO Queue**.  
A time-driven clock steps through each tick, managing customer arrivals, waiting, and service.

## File Structure
```
queuing system simulation/
├── include/
│   ├── customerType.h          # Customer data & methods
│   ├── queueADT.h              # Queue abstract data type
│   ├── serverListType.h        # Collection of servers
│   ├── serverType.h            # Single server logic
│   └── waitingCustomerQueue.h  # FIFO queue + wait-time increment
├── src/
│   ├── customerType.cpp        # Customer implementation
│   ├── main.cpp                # Simulation loop & results
│   ├── serverListType.cpp      # Server list implementation
│   ├── serverType.cpp          # Server implementation
│   └── waitingCustomerQueue.cpp # Queue implementation
├── Makefile
└── README.md
```

## How to Build & Run
```bash
make        # compile
make run    # compile + run
make clean  # remove binary
```

## Simulation Parameters (edit in `main.cpp`)
| Constant | Default | Meaning |
|---|---|---|
| `SIM_TIME` | 100 | Total clock ticks |
| `NUM_SERVERS` | 3 | Number of servers |
| `ARRIVAL_INTERVAL` | 4 | Ticks between customer arrivals |
| `MIN_SERVICE_TIME` | 2 | Min service duration (ticks) |
| `MAX_SERVICE_TIME` | 6 | Max service duration (ticks) |

## Core Classes
- **`customerType`** — number, arrival time, waiting time, transaction time  
- **`serverType`** — free/busy state, current customer, remaining time  
- **`serverListType`** — finds free servers, dispatches customers, updates each tick  
- **`waitingCustomerQueue`** — FIFO queue with bulk waiting-time increment  

## Output Metrics
- Total customers served  
- Customers still in queue at end  
- Peak queue length  
- Average waiting time  

## Bonus Ideas (for extra grade)
- [ ] GUI / terminal UI using ncurses or a web-based interface  
- [ ] CSV export of per-tick stats for graphing  
- [ ] Priority queue variant (VIP customers)  
- [ ] Multiple queue lanes  
- [ ] Variable arrival rate (rush hours)  

## Team
| Name | ID | Role |
|---|---|---|
|Seif Rashwan|120210256|--|
|Hana Aly ElMaghraby|320240126|--|
|Nour Ibrahim|320250325|-- |
|Androw shonoda tawfeek|320210333|--|
| | | |
