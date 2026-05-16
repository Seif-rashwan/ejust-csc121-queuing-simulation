/**
 * @file Main.cpp
 * @brief Standalone CLI entry point for the queuing simulation.
 *
 * Note: This file contains its own main() function and is built into a separate
 * executable (simulation_cli). It is NOT linked with web_simulation.cpp.
 */

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "ServerListType.h"
#include "WaitingCustomerQueue.h"

using std::cin;
using std::cout;

// Reads simulation settings from the user
static void SetSimulationParameters(int& s_time, int& num_of_servers, int& trans_time,
                                    int& t_between_arrivals) {
    cout << "Enter simulation time: ";
    if (!(cin >> s_time)) {
        throw std::runtime_error("Invalid input: expected an integer for simulation time.");
    }

    cout << "Enter number of servers: ";
    if (!(cin >> num_of_servers)) {
        throw std::runtime_error("Invalid input: expected an integer for number of servers.");
    }

    cout << "Enter transaction time: ";
    if (!(cin >> trans_time)) {
        throw std::runtime_error("Invalid input: expected an integer for transaction time.");
    }

    cout << "Enter time between customer arrivals: ";
    if (!(cin >> t_between_arrivals)) {
        throw std::runtime_error("Invalid input: expected an integer for time between arrivals.");
    }

    if (s_time <= 0 || num_of_servers <= 0 || trans_time <= 0 || t_between_arrivals <= 0) {
        throw std::invalid_argument("All simulation parameters must be positive integers.");
    }

    if (t_between_arrivals > s_time) {
        cout << "Warning: time between arrivals (" << t_between_arrivals
             << ") exceeds simulation time (" << s_time << "). No customers will arrive.\n";
    }
}

int main() {
    int simulation_time;
    int number_of_servers;
    int transaction_time;
    int time_between_arrivals;
    int peak_queue_length = 0;

    // Read simulation settings from user
    SetSimulationParameters(simulation_time, number_of_servers, transaction_time,
                            time_between_arrivals);

    // Create servers and waiting queue
    ServerListType servers(number_of_servers);
    WaitingCustomerQueue<CustomerType> customer_queue;

    // Variables used for final statistics
    int total_wait_time       = 0;
    int customers_served      = 0;
    int customers_arrived     = 0;
    int customers_turned_away = 0;

    // Start simulation loop
    for (int clock = 1; clock <= simulation_time; clock++) {
        // Update all servers
        // Decrease remaining service time for busy servers
        servers.updateServers();

        // Handle new customer arrivals
        if (clock % time_between_arrivals == 0) {
            customers_arrived++;

            if (!customer_queue.isFull()) {
                CustomerType new_customer(customers_arrived, clock, 0, transaction_time);
                customer_queue.enqueue(new_customer);

                cout << "--> Customer " << customers_arrived << " arrived at tick " << clock
                     << '\n';
            } else {
                customers_turned_away++;

                cout << "--> Customer " << customers_arrived << " turned away (queue full) at tick "
                     << clock << '\n';
            }
        }

        // Assign waiting customers to free servers
        int free_server_id = servers.getFreeServerID();

        while (free_server_id != -1 && !customer_queue.isEmpty()) {
            // Remove first customer from queue
            CustomerType front_customer = customer_queue.front();
            customer_queue.dequeue();

            // Update statistics
            total_wait_time += front_customer.getWaitingTime();
            customers_served++;

            // Assign customer to free server
            servers.setServerBusy(free_server_id, front_customer);

            // Look for another free server
            free_server_id = servers.getFreeServerID();
        }

        // Track maximum queue size reached
        peak_queue_length = std::max(customer_queue.size(), peak_queue_length);

        // Increase waiting time for customers still in queue
        if (!customer_queue.isEmpty()) {
            customer_queue.incrementWaitingTimes();
        }
    }

    // Calculate waiting time for customers left in queue after simulation ends
    int customers_in_queue = customer_queue.size();

    while (!customer_queue.isEmpty()) {
        total_wait_time += customer_queue.front().getWaitingTime();
        customer_queue.dequeue();
    }

    // Print final simulation statistics
    cout << "\n===================================\n";
    cout << "        SIMULATION RESULTS         \n";
    cout << "===================================\n";
    cout << "Total simulation time:        " << simulation_time << '\n';
    cout << "Total customers arrived:      " << customers_arrived << '\n';
    cout << "  - Served:                   " << customers_served << '\n';
    cout << "  - Left in queue:            " << customers_in_queue << '\n';
    cout << "  - Turned away (queue full): " << customers_turned_away << '\n';
    cout << "Peak queue length:            " << peak_queue_length << '\n';

    int total_customers_counted = customers_served + customers_in_queue;

    if (total_customers_counted > 0) {
        cout << "Average waiting time:         "
             << static_cast<double>(total_wait_time) / total_customers_counted << '\n';
    }

    return 0;
}