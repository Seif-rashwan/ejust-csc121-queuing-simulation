// Copyright 2026 E-JUST CSC 121 Project

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

#include "CustomerType.h"
#include "ServerType.h"
#include "ServerListType.h"
#include "WaitingCustomerQueue.h"

// ──────────────────────────────────────────────
//  Simulation parameters (edit these freely)
// ──────────────────────────────────────────────
const int sim_time          = 100;   // total clock ticks to simulate
const int num_servers       = 3;     // number of servers
const int arrival_interval  = 4;     // a new customer arrives every N ticks
const int min_service_time  = 2;     // minimum service (transaction) time
const int max_service_time  = 6;     // maximum service (transaction) time
// ──────────────────────────────────────────────

int random_service_time() {
    return min_service_time
         + rand() % (max_service_time - min_service_time + 1);
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    ServerListType       server_list(num_servers);
    WaitingCustomerQueue wait_queue;

    int customer_count   = 0;
    int total_wait_time   = 0;
    int total_customers  = 0;
    int max_queue_length  = 0;

    std::cout << "========================================\n";
    std::cout << "   Queuing System Simulation — E-JUST  \n";
    std::cout << "========================================\n";
    std::cout << "Servers: "  << num_servers      << "  |  "
              << "Sim time: " << sim_time         << " ticks  |  "
              << "Arrival every " << arrival_interval << " ticks\n\n";

    // ── Main simulation loop ─────────────────
    for (int clock = 1; clock <= sim_time; clock++) {

        // 2.1  Update servers (decrement transaction time)
        server_list.updateServers(std::cout);

        // 2.2  Increment waiting time of every queued customer
        if (!wait_queue.empty())
            wait_queue.incrementWaitingTimes();

        // 2.3  New customer arrives?
        if (clock % arrival_interval == 0) {
            customer_count++;
            CustomerType new_customer(customer_count, clock, 0, random_service_time());
            wait_queue.enqueue(new_customer);
            total_customers++;
            std::cout << "  [Clock " << clock << "] Customer #"
                      << customer_count << " arrived (service time: "
                      << new_customer.getTransactionTime() << " units)\n";
        }

        // Track peak queue length
        if (wait_queue.size() > max_queue_length)
            max_queue_length = wait_queue.size();

        // 2.4  Assign waiting customers to free servers
        int free_id;
        while (!wait_queue.empty() && (free_id = server_list.getFreeServerID()) != -1) {
            CustomerType next = wait_queue.dequeue();
            total_wait_time += next.getWaitingTime();
            server_list.setServerBusy(free_id, next, clock);
        }
    }

    // ── Results ─────────────────────────────
    std::cout << "\n========================================\n";
    std::cout << "           Simulation Results           \n";
    std::cout << "========================================\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Total customers served : " << total_customers  << "\n";
    std::cout << "Customers still queued : " << wait_queue.size() << "\n";
    std::cout << "Peak queue length      : " << max_queue_length   << "\n";

    double avg_wait = total_customers > 0
                   ? static_cast<double>(total_wait_time) / total_customers
                   : 0.0;
    std::cout << "Average waiting time   : " << avg_wait << " ticks\n";
    std::cout << "========================================\n";

    return 0;
}
