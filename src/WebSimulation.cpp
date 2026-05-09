#include <algorithm>
#include <iostream>
#include <sstream>

#include "WebSimulation.h"

using std::cin;
using std::cout;
using std::ostringstream;

// ────────────────────────────────────────────────────────────────────────────────
// Web Sim. Entry Point:
// ─────────────────────
int main(int argc, const char* argv[]) {
    try {
        // 'simulation_time_cap' is just a safety cap; sim ends when all customers served
        int simulation_time_cap   = WebSimulation::SIMULATION_TIME_CAP_DEFAULT;
        int total_customers       = WebSimulation::DEFAULT_TOT_CUSTOMERS;
        int number_of_servers     = 4;
        int transaction_time      = 5;
        int time_between_arrivals = 3;

        if (argc >= 6) {
            simulation_time_cap   = std::stoi(argv[1]);
            number_of_servers     = std::stoi(argv[2]);
            transaction_time      = std::stoi(argv[3]);
            time_between_arrivals = std::stoi(argv[4]);
            total_customers       = std::stoi(argv[5]);
        } else if (argc >= 5) {
            simulation_time_cap   = std::stoi(argv[1]);
            number_of_servers     = std::stoi(argv[2]);
            transaction_time      = std::stoi(argv[3]);
            time_between_arrivals = std::stoi(argv[4]);
        }

        WebSimulation sim(simulation_time_cap, number_of_servers, transaction_time,
                          time_between_arrivals, total_customers);

        // Emit initial state (tick 0, nothing has happened yet)
        sim.outputState();
        sim.start();

        while (!sim.isFinished()) {
            sim.tick();
            sim.outputState();  // sole call site: no duplicate output
        }

        sim.outputFinalStats();
    } catch (const std::exception& e) {
        // Errors go to stderr so they don't corrupt the stdout JSON stream
        std::cerr << "FATAL: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

// ────────────────────────────────────────────────────────────────────────────────
// WebSimulation Implementation:
// ─────────────────────────────
WebSimulation::WebSimulation(int sim_time, int num_servers, int trans_time, int t_between_arrivals,
                             int total_cust) {
    simulation_time_       = sim_time;
    number_of_servers_     = num_servers;
    transaction_time_      = trans_time;
    time_between_arrivals_ = t_between_arrivals;

    // total_cust = how many customers will arrive in total
    // queue buffer = same size so no one is ever turned away
    total_arrivals_target_ = total_cust;

    servers_               = new ServerListType(number_of_servers_);
    customer_array_        = new CustomerType[total_arrivals_target_];
    server_states_.resize(number_of_servers_,
                          {.busy = false, .remaining = 0, .assigned_customer_id = -1});

    reset();
}

WebSimulation::~WebSimulation() {
    delete servers_;
    delete[] customer_array_;
}

// Helpers:
bool WebSimulation::isQueueFull() const {
    return queue_size_ >= total_arrivals_target_;
}

bool WebSimulation::isQueueEmpty() const {
    return queue_size_ == 0;
}

// Round-robin:
// Find next free server starting from `next_server_hint`
int WebSimulation::getFreeServerRoundRobin() {
    for (int offset = 0; offset < number_of_servers_; offset++) {
        int idx = (next_server_hint_ + offset) % number_of_servers_;
        if (!server_states_[idx].busy) {
            // Also verify the real ServerType agrees
            // (use servers->getFreeServerID fallback if needed)

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

CustomerType WebSimulation::peekCustomer() const {
    if (!isQueueEmpty()) {
        return customer_array_[queue_front_];
    }
    return CustomerType();
}

// Lifecycle:
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

    queue_front_            = 0;
    queue_rear_             = 0;
    queue_size_             = 0;

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

// Tick Logic:
void WebSimulation::tick() {
    if (!running_ || isFinished()) {
        // Stop if paused, clock hit safety cap, or all customers done
        return;
    }

    current_clock_++;
    last_event_type_        = "";
    last_event_customer_id_ = -1;

    // 1. Decrement existing busy server states and free them when done.
    for (int i = 0; i < number_of_servers_; i++) {
        if (server_states_[i].busy) {
            server_states_[i].remaining--;
            if (server_states_[i].remaining <= 0) {
                server_states_[i].busy                 = false;
                server_states_[i].assigned_customer_id = -1;
                customers_served_++;  // count when DONE, not when assigned
            }
        }
    }

    // 2. Advance the real ServerType objects in parallel.
    servers_->updateServers();

    // 3. Customer arrivals — only until total_arrivals_target reached
    if (current_clock_ % time_between_arrivals_ == 0 &&
        customers_arrived_ < total_arrivals_target_) {
        customers_arrived_++;
        CustomerType new_customer(customers_arrived_, current_clock_, 0, transaction_time_);
        enqueueCustomer(new_customer);
        last_event_type_        = "arrived";
        last_event_customer_id_ = customers_arrived_;
    }

    // 4. Assign free servers — ROUND ROBIN so all servers get used evenly
    int rr_id = getFreeServerRoundRobin();
    while (rr_id != -1 && !isQueueEmpty()) {
        CustomerType front_customer = dequeueCustomer();

        total_wait_time_ += (current_clock_ - front_customer.getArrivalTime());
        // Note: customers_served_ incremented above when server FINISHES

        servers_->setServerBusy(rr_id, front_customer);
        server_states_[rr_id]   = {.busy                 = true,
                                   .remaining            = transaction_time_,
                                   .assigned_customer_id = front_customer.getCustomerNumber()};

        last_event_type_        = "serving";
        last_event_customer_id_ = front_customer.getCustomerNumber();

        rr_id                   = getFreeServerRoundRobin();
    }

    // 5. Increment waiting time for customers still in queue
    for (int i = 0; i < queue_size_; i++) {
        int idx = (queue_front_ + i) % total_arrivals_target_;
        customer_array_[idx].incrementWaitingTime();
    }

    // 6. Track peak queue length
    peak_queue_length_ = std::max(queue_size_, peak_queue_length_);
}

// Output:
void WebSimulation::outputState() const {
    // All output is prefixed "STATE:" followed by a single-line JSON object.
    // No other text is ever written to stdout by this program.
    ostringstream output;

    // clang-format off
    output << R"(STATE:{)"
           << R"("tick":)" << current_clock_ << ","
           << R"("queueSize":)" << queue_size_ << ","
           << R"("served":)" << customers_served_ << ","
           << R"("turnedAway":)" << customers_turned_away_ << ","
           << R"("peakQueue":)" << peak_queue_length_ << ","
           << R"("avgWait":)"
           << (customers_served_ > 0 ? static_cast<double>(total_wait_time_) / customers_served_
                                     : 0.0)
           << ","
           << R"("nextArrival":)"
           << (time_between_arrivals_ - (current_clock_ % time_between_arrivals_)) << ","
           << R"("running":)" << (running_ && !isFinished() ? "true" : "false") << ","
           << R"("totalCustomers":)" << total_arrivals_target_ << ","
           << R"("arrived":)" << customers_arrived_ << ","
           << R"("lastEvent":")" << last_event_type_ << "\","
           << R"("lastEventCustomer":)" << last_event_customer_id_ << ","
           << R"("servers":[)";

    for (int i = 0; i < number_of_servers_; i++) {
        if (i > 0) {
            output << ",";
        }

        output << R"({"busy":)" << (server_states_[i].busy ? "true" : "false") << ","
               << R"("remaining":)" << server_states_[i].remaining << ","
               << R"("customerId":)" << server_states_[i].assigned_customer_id << "}";
    }
    // clang-format on

    output << "]}\n";
    cout << output.str();
    cout.flush();
}

void WebSimulation::outputFinalStats() const {
    ostringstream output;

    // clang-format off
    output << R"(FINAL:{)"
           << R"("totalSimulationTime":)" << simulation_time_ << ","
           << R"("totalCustomersArrived":)" << customers_arrived_ << ","
           << R"("customersServed":)" << customers_served_ << ","
           << R"("customersLeftInQueue":)" << queue_size_ << ","
           << R"("customersTurnedAway":)" << customers_turned_away_ << ","
           << R"("peakQueueLength":)" << peak_queue_length_ << ","
           << R"("averageWaitingTime":)"
           << (customers_served_ > 0 ? static_cast<double>(total_wait_time_) / customers_served_
                                     : 0.0)
           << "}\n";

    // clang-format on
    cout << output.str();
    cout.flush();
}

// Use shadow state (same source of truth as assignment logic)
bool WebSimulation::allServersFree() const {
    for (int i = 0; i < number_of_servers_; i++) {
        if (server_states_[i].busy) {
            return false;
        }
    }

    return true;
}

// Simulation ends when:
//   a. all customers arrived + queue empty + all servers idle, OR
//   b. safety cap hit
bool WebSimulation::isFinished() const {
    bool all_done =
        (customers_arrived_ >= total_arrivals_target_) && isQueueEmpty() && allServersFree();
    return all_done || (current_clock_ >= simulation_time_);
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
