#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

#include "customerType.h"
#include "serverType.h"
#include "serverListType.h"
#include "waitingCustomerQueue.h"

// ──────────────────────────────────────────────
//  Simulation parameters (edit these freely)
// ──────────────────────────────────────────────
const int SIM_TIME          = 100;   // total clock ticks to simulate
const int NUM_SERVERS       = 3;     // number of servers
const int ARRIVAL_INTERVAL  = 4;     // a new customer arrives every N ticks
const int MIN_SERVICE_TIME  = 2;     // minimum service (transaction) time
const int MAX_SERVICE_TIME  = 6;     // maximum service (transaction) time
// ──────────────────────────────────────────────

int randomServiceTime() {
    return MIN_SERVICE_TIME
         + rand() % (MAX_SERVICE_TIME - MIN_SERVICE_TIME + 1);
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    serverListType       serverList(NUM_SERVERS);
    waitingCustomerQueue waitQueue;

    int customerCount   = 0;
    int totalWaitTime   = 0;
    int totalCustomers  = 0;
    int maxQueueLength  = 0;

    std::cout << "========================================\n";
    std::cout << "   Queuing System Simulation — E-JUST  \n";
    std::cout << "========================================\n";
    std::cout << "Servers: "  << NUM_SERVERS      << "  |  "
              << "Sim time: " << SIM_TIME         << " ticks  |  "
              << "Arrival every " << ARRIVAL_INTERVAL << " ticks\n\n";

    // ── Main simulation loop ─────────────────
    for (int clock = 1; clock <= SIM_TIME; clock++) {

        // 2.1  Update servers (decrement transaction time)
        serverList.updateServers(std::cout, clock);

        // 2.2  Increment waiting time of every queued customer
        if (!waitQueue.empty())
            waitQueue.incrementWaitingTimes();

        // 2.3  New customer arrives?
        if (clock % ARRIVAL_INTERVAL == 0) {
            customerCount++;
            customerType newCustomer(customerCount, clock, randomServiceTime());
            waitQueue.enqueue(newCustomer);
            totalCustomers++;
            std::cout << "  [Clock " << clock << "] Customer #"
                      << customerCount << " arrived (service time: "
                      << newCustomer.getTransactionTime() << " units)\n";
        }

        // Track peak queue length
        if (waitQueue.size() > maxQueueLength)
            maxQueueLength = waitQueue.size();

        // 2.4  Assign waiting customers to free servers
        int freeID;
        while (!waitQueue.empty() && (freeID = serverList.getFreeServerID()) != -1) {
            customerType next = waitQueue.dequeue();
            totalWaitTime += next.getWaitingTime();
            serverList.setServerBusy(freeID, next, clock);
        }
    }

    // ── Results ─────────────────────────────
    std::cout << "\n========================================\n";
    std::cout << "           Simulation Results           \n";
    std::cout << "========================================\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Total customers served : " << totalCustomers  << "\n";
    std::cout << "Customers still queued : " << waitQueue.size() << "\n";
    std::cout << "Peak queue length      : " << maxQueueLength   << "\n";

    double avgWait = totalCustomers > 0
                   ? static_cast<double>(totalWaitTime) / totalCustomers
                   : 0.0;
    std::cout << "Average waiting time   : " << avgWait << " ticks\n";
    std::cout << "========================================\n";

    return 0;
}
