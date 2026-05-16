/**
 * @file WebSimulation.cpp
 * @brief Implementation of the WebSimulation queuing system.
 *
 * Key Features:
 * - Configurable number of servers/cashiers
 * - Random customer arrival intervals and service times
 * - Round-robin server assignment strategy
 * - Circular queue implementation for efficient memory usage
 * - Real-time tick-by-tick state output in JSON format
 * - Comprehensive final statistics reporting
 *
 * The simulation operates on discrete time steps (ticks) and can be controlled
 * via command-line arguments for custom parameter overrides.
 */

#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>

#include "WebSimulation.h"

using std::cout;
using std::ostringstream;

/// Maximum number of queued customers to display in state output preview
constexpr int MAX_FIFO_SLOTS_PREVIEW = 20;

// ── Constructor ───────────────────────────────────────────────────────────────
WebSimulation::WebSimulation(int sim_time, int num_servers, int trans_min, int trans_max,
                             int arrival_min, int arrival_max, int total_cust) {
    simulation_time_       = sim_time;
    number_of_servers_     = num_servers;
    transaction_time_min_  = trans_min;
    transaction_time_max_  = trans_max;
    arrival_interval_min_  = arrival_min;
    arrival_interval_max_  = arrival_max;
    total_arrivals_target_ = total_cust;

    servers_               = new ServerListType(number_of_servers_);
    customer_array_        = new CustomerType[total_arrivals_target_];
    server_states_.resize(number_of_servers_,
                          {.busy = false, .remaining = 0, .assigned_customer_id = -1});

    // reset() initialises everything else including next_arrival_tick_
    reset();
}

WebSimulation::~WebSimulation() {
    delete servers_;
    delete[] customer_array_;
}

// ── Helpers ───────────────────────────────────────────────────────────
int WebSimulation::randomBetween(int min, int max) {
    if (min >= max) {
        return min;
    }

    static std::random_device random_device;
    static std::mt19937 generator(random_device());
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

bool WebSimulation::isQueueFull() const {
    return queue_size_ >= total_arrivals_target_;
}

bool WebSimulation::isQueueEmpty() const {
    return queue_size_ == 0;
}

int WebSimulation::getFreeServerRoundRobin() {
    for (int offset = 0; offset < number_of_servers_; offset++) {
        int idx = (next_server_hint_ + offset) % number_of_servers_;
        if (!server_states_[idx].busy) {
            next_server_hint_ = (idx + 1) % number_of_servers_;
            return idx;
        }
    }
    return -1;
}

void WebSimulation::enqueueCustomer(const CustomerType& customer) {
    if (!isQueueFull()) {
        customer_array_[queue_rear_] = customer;
        queue_rear_                  = (queue_rear_ + 1) % total_arrivals_target_;
        queue_size_++;
    }
}

CustomerType WebSimulation::dequeueCustomer() {
    if (!isQueueEmpty()) {
        CustomerType customer = customer_array_[queue_front_];
        queue_front_          = (queue_front_ + 1) % total_arrivals_target_;
        queue_size_--;
        return customer;
    }
    return CustomerType();
}

bool WebSimulation::allServersFree() const {
    for (int i = 0; i < number_of_servers_; i++) {
        if (server_states_[i].busy) {
            return false;
        }
    }
    return true;
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────
void WebSimulation::reset() {
    current_clock_          = 0;
    customers_served_       = 0;
    total_wait_time_        = 0;
    peak_queue_length_      = 0;
    customers_arrived_      = 0;
    customers_turned_away_  = 0;
    last_event_type_        = "";
    last_event_customer_id_ = -1;
    next_server_hint_       = 0;
    running_                = false;

    queue_front_            = 0;
    queue_rear_             = 0;
    queue_size_             = 0;

    // Schedule the first arrival using a random interval
    next_arrival_tick_ = randomBetween(arrival_interval_min_, arrival_interval_max_);

    delete servers_;
    servers_ = new ServerListType(number_of_servers_);

    server_states_.assign(number_of_servers_,
                          {.busy = false, .remaining = 0, .assigned_customer_id = -1});
}

void WebSimulation::start() {
    running_ = true;
}

void WebSimulation::pause() {
    running_ = false;
}

bool WebSimulation::isFinished() const {
    bool all_done =
        (customers_arrived_ >= total_arrivals_target_) && isQueueEmpty() && allServersFree();
    return all_done || (current_clock_ >= simulation_time_);
}

// ── Tick ──────────────────────────────────────────────────────────────────────
void WebSimulation::tick() {
    if (!running_ || isFinished()) {
        return;
    }

    current_clock_++;
    last_event_type_        = "";
    last_event_customer_id_ = -1;

    // 1. Decrement shadow server states; count customers served on completion
    for (int i = 0; i < number_of_servers_; i++) {
        if (server_states_[i].busy) {
            server_states_[i].remaining--;
            if (server_states_[i].remaining <= 0) {
                server_states_[i].busy                 = false;
                server_states_[i].assigned_customer_id = -1;
                customers_served_++;
            }
        }
    }

    // 2. Advance real ServerListType objects in parallel
    servers_->updateServers();

    // 3. Customer arrivals — only until total_arrivals_target_ reached
    if (current_clock_ >= next_arrival_tick_ && customers_arrived_ < total_arrivals_target_) {
        customers_arrived_++;

        // Each customer gets an individually randomised service time
        int service_time = randomBetween(transaction_time_min_, transaction_time_max_);
        CustomerType new_customer(customers_arrived_, current_clock_, 0, service_time);

        enqueueCustomer(new_customer);
        last_event_type_        = "arrived";
        last_event_customer_id_ = customers_arrived_;

        // Schedule the next arrival
        next_arrival_tick_ =
            current_clock_ + randomBetween(arrival_interval_min_, arrival_interval_max_);
    }

    // 4. Assign free servers via round-robin
    int rr_id = getFreeServerRoundRobin();
    while (rr_id != -1 && !isQueueEmpty()) {
        CustomerType front_customer = dequeueCustomer();

        total_wait_time_ += (current_clock_ - front_customer.getArrivalTime());

        servers_->setServerBusy(rr_id, front_customer);
        server_states_[rr_id]   = {.busy                 = true,
                                   .remaining            = front_customer.getTransactionTime(),
                                   .assigned_customer_id = front_customer.getCustomerNumber()};

        last_event_type_        = "serving";
        last_event_customer_id_ = front_customer.getCustomerNumber();

        rr_id                   = getFreeServerRoundRobin();
    }

    // 5. Increment waiting time for customers still queued
    for (int i = 0; i < queue_size_; i++) {
        int idx = (queue_front_ + i) % total_arrivals_target_;
        customer_array_[idx].incrementWaitingTime();
    }

    // 6. Track peak queue length
    peak_queue_length_ = std::max(queue_size_, peak_queue_length_);
}

// ── Output ────────────────────────────────────────────────────────────────────
void WebSimulation::outputState() const {
    ostringstream out;

    // clang-format off
    out << R"(STATE:{)"
        << R"("tick":)"          << current_clock_         << ","
        << R"("queueSize":)"     << queue_size_            << ","
        << R"("served":)"        << customers_served_      << ","
        << R"("turnedAway":)"    << customers_turned_away_ << ","
        << R"("peakQueue":)"     << peak_queue_length_     << ","
        << R"("avgWait":)"
            << (customers_served_ > 0
                    ? static_cast<double>(total_wait_time_) / customers_served_
                    : 0.0)
        << ","
        << R"("nextArrival":)"
            << std::max(0, next_arrival_tick_ - current_clock_)
        << ","
        << R"("running":)"           << (running_ && !isFinished() ? "true" : "false") << ","
        << R"("totalCustomers":)"    << total_arrivals_target_  << ","
        << R"("arrived":)"           << customers_arrived_      << ","
        << R"("lastEvent":")"        << last_event_type_        << "\","
        << R"("lastEventCustomer":)" << last_event_customer_id_ << ","
        << R"("throughput":)"
            << (current_clock_ > 0
                    ? static_cast<double>(customers_served_) / current_clock_
                    : 0.0)
        << ","
        << R"("_front":)"            << queue_front_            << ","
        << R"("_rear":)"             << queue_rear_             << ","
        << R"("_fifoSlots":[)";

    int limit = std::min(queue_size_, MAX_FIFO_SLOTS_PREVIEW);
    for (int i = 0; i < limit; i++) {
        if (i > 0) {
            out << ",";
        }

        int idx = (queue_front_ + i) % total_arrivals_target_;
        out << R"({"id":)"           << customer_array_[idx].getCustomerNumber()
            << R"(,"pos":)"          << i
            << "}";
    }

    out << R"(],"servers":[)";

    for (int i = 0; i < number_of_servers_; i++) {
        if (i > 0) {
            out << ",";
        }

        out << R"({"busy":)"      << (server_states_[i].busy ? "true" : "false") << ","
            << R"("remaining":)"  << server_states_[i].remaining                 << ","
            << R"("customerId":)" << server_states_[i].assigned_customer_id      << "}";
    }
    // clang-format on

    out << "]}\n";
    cout << out.str();
    cout.flush();
}

void WebSimulation::outputFinalStats() const {
    ostringstream out;

    // clang-format off
    out << R"(FINAL:{)"
        << R"("totalSimulationTime":)"    << simulation_time_       << ","
        << R"("totalCustomersArrived":)"  << customers_arrived_     << ","
        << R"("customersServed":)"        << customers_served_      << ","
        << R"("customersLeftInQueue":)"   << queue_size_            << ","
        << R"("customersTurnedAway":)"    << customers_turned_away_ << ","
        << R"("peakQueueLength":)"        << peak_queue_length_     << ","
        << R"("averageWaitingTime":)"
            << (customers_served_ > 0
                    ? static_cast<double>(total_wait_time_) / customers_served_
                    : 0.0)
        << "}\n";
    // clang-format on

    cout << out.str();
    cout.flush();
}

void WebSimulation::runFullSimulation() {
    reset();
    start();

    while (!isFinished()) {
        tick();
        outputState();
    }

    outputFinalStats();
}
