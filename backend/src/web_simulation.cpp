#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <vector>

#include "ServerListType.h"
#include "WaitingCustomerQueue.h"

using std::cin;
using std::cout;

class WebSimulation {
private:
    int simulation_time;
    int number_of_servers;
    int transaction_time;
    int time_between_arrivals;
    int total_arrivals_target;  // total customers who will ever arrive
    int peak_queue_length  = 0;
    int next_server_hint   = 0; // round-robin pointer for server assignment

    ServerListType* servers;

    // Circular array queue
    CustomerType* customer_array;
    int queue_front = 0;
    int queue_rear  = 0;
    int queue_size  = 0;

    // Statistics
    int total_wait_time       = 0;
    int customers_served      = 0;
    int customers_arrived     = 0;
    int customers_turned_away = 0;
    int current_clock         = 0;
    bool running              = false;

    // Last event info for embedding into STATE JSON
    std::string last_event_type;
    int last_event_customer_id = -1;

    // Server tracking for web interface
    struct ServerState {
        bool busy;
        int  remaining;
        int  assigned_customer_id;
    };
    std::vector<ServerState> server_states;

public:
    WebSimulation(int sim_time, int num_servers, int trans_time,
                  int t_between_arrivals, int total_cust = 100) {
        simulation_time       = sim_time;
        number_of_servers     = num_servers;
        transaction_time      = trans_time;
        time_between_arrivals = t_between_arrivals;
        // total_cust = how many customers will arrive in total
        // queue buffer = same size so no one is ever turned away
        total_arrivals_target = total_cust;

        servers        = new ServerListType(number_of_servers);
        customer_array = new CustomerType[total_arrivals_target];
        server_states.resize(number_of_servers, {false, 0, -1});

        reset();
    }

    ~WebSimulation() {
        delete servers;
        delete[] customer_array;
    }

    // ── Circular array helpers ────────────────────────────────────────────────

    bool isQueueFull()  const { return queue_size >= total_arrivals_target; }
    bool isQueueEmpty() const { return queue_size == 0; }

    // Round-robin: find next free server starting from next_server_hint
    int getFreeServerRoundRobin() {
        for (int offset = 0; offset < number_of_servers; offset++) {
            int idx = (next_server_hint + offset) % number_of_servers;
            if (server_states[idx].busy == false) {
                // Also verify the real ServerType agrees
                // (use servers->getFreeServerID fallback if needed)
                next_server_hint = (idx + 1) % number_of_servers;
                return idx;
            }
        }
        return -1;  // all busy
    }

    void enqueueCustomer(const CustomerType& customer) {
        if (!isQueueFull()) {
            customer_array[queue_rear] = customer;
            queue_rear  = (queue_rear + 1) % total_arrivals_target;
            queue_size++;
        }
    }

    CustomerType dequeueCustomer() {
        if (!isQueueEmpty()) {
            CustomerType customer = customer_array[queue_front];
            queue_front = (queue_front + 1) % total_arrivals_target;
            queue_size--;
            return customer;
        }
        return CustomerType();
    }

    CustomerType peekCustomer() const {
        if (!isQueueEmpty()) {
            return customer_array[queue_front];
        }
        return CustomerType();
    }

    // ── Lifecycle ─────────────────────────────────────────────────────────────

    void reset() {
        current_clock         = 0;
        customers_served      = 0;
        total_wait_time       = 0;
        peak_queue_length     = 0;
        customers_arrived     = 0;
        customers_turned_away = 0;
        last_event_type       = "";
        last_event_customer_id = -1;
        next_server_hint      = 0;

        queue_front = 0;
        queue_rear  = 0;
        queue_size  = 0;

        // Bug fix: delete before re-allocating to prevent memory leak
        delete servers;
        servers = new ServerListType(number_of_servers);

        server_states.assign(number_of_servers, {false, 0, -1});
    }

    void start() { running = true;  }
    void pause() { running = false; }

    // ── Tick logic ────────────────────────────────────────────────────────────

    void tick() {
        // Stop if paused, clock hit safety cap, or all customers done
        if (!running || isFinished()) return;

        current_clock++;
        last_event_type        = "";
        last_event_customer_id = -1;

        // 1. Decrement existing busy server states and free them when done.
        for (int i = 0; i < number_of_servers; i++) {
            if (server_states[i].busy) {
                server_states[i].remaining--;
                if (server_states[i].remaining <= 0) {
                    server_states[i].busy                 = false;
                    server_states[i].assigned_customer_id = -1;
                    customers_served++;   // count when DONE, not when assigned
                }
            }
        }

        // 2. Advance the real ServerType objects in parallel.
        servers->updateServers();

        // 3. Customer arrivals — only until total_arrivals_target reached
        if (current_clock % time_between_arrivals == 0
            && customers_arrived < total_arrivals_target) {
            customers_arrived++;
            CustomerType new_customer(customers_arrived, current_clock, 0, transaction_time);
            enqueueCustomer(new_customer);
            last_event_type        = "arrived";
            last_event_customer_id = customers_arrived;
        }

        // 4. Assign free servers — ROUND ROBIN so all servers get used evenly
        int rr_id = getFreeServerRoundRobin();
        while (rr_id != -1 && !isQueueEmpty()) {
            CustomerType front_customer = dequeueCustomer();

            total_wait_time += (current_clock - front_customer.getArrivalTime());
            // Note: customers_served incremented above when server FINISHES

            servers->setServerBusy(rr_id, front_customer);
            server_states[rr_id] = {true, transaction_time,
                                     front_customer.getCustomerNumber()};

            last_event_type        = "serving";
            last_event_customer_id = front_customer.getCustomerNumber();

            rr_id = getFreeServerRoundRobin();
        }

        // 5. Increment waiting time for customers still in queue
        for (int i = 0; i < queue_size; i++) {
            int idx = (queue_front + i) % total_arrivals_target;
            customer_array[idx].incrementWaitingTime();
        }

        // 6. Track peak queue length
        peak_queue_length = std::max(queue_size, peak_queue_length);
    }

    // ── Output ────────────────────────────────────────────────────────────────

    void outputState() const {
        // All output is prefixed "STATE:" followed by a single-line JSON object.
        // No other text is ever written to stdout by this program.
        cout << "STATE:{";
        cout << "\"tick\":"        << current_clock         << ",";
        cout << "\"queueSize\":"   << queue_size            << ",";
        cout << "\"served\":"      << customers_served      << ",";
        cout << "\"turnedAway\":"  << customers_turned_away << ",";
        cout << "\"peakQueue\":"   << peak_queue_length     << ",";
        cout << "\"avgWait\":"     << (customers_served > 0
                                        ? (double)total_wait_time / customers_served
                                        : 0.0)              << ",";
        cout << "\"nextArrival\":" << (time_between_arrivals
                                       - (current_clock % time_between_arrivals)) << ",";
        cout << "\"running\":"     << (running && !isFinished()
                                        ? "true" : "false") << ",";
        cout << "\"totalCustomers\":" << total_arrivals_target << ",";
        cout << "\"arrived\":"     << customers_arrived << ",";
        cout << "\"lastEvent\":\"" << last_event_type       << "\",";
        cout << "\"lastEventCustomer\":" << last_event_customer_id << ",";

        cout << "\"servers\":[";
        for (int i = 0; i < number_of_servers; i++) {
            if (i > 0) cout << ",";
            cout << "{\"busy\":"       << (server_states[i].busy ? "true" : "false") << ","
                 << "\"remaining\":"   << server_states[i].remaining                  << ","
                 << "\"customerId\":"  << server_states[i].assigned_customer_id       << "}";
        }
        cout << "]}\n";
        cout.flush();
    }

    void outputFinalStats() const {
        cout << "FINAL:{";
        cout << "\"totalSimulationTime\":"   << simulation_time     << ",";
        cout << "\"totalCustomersArrived\":" << customers_arrived   << ",";
        cout << "\"customersServed\":"       << customers_served    << ",";
        cout << "\"customersLeftInQueue\":"  << queue_size          << ",";
        cout << "\"customersTurnedAway\":"   << customers_turned_away << ",";
        cout << "\"peakQueueLength\":"       << peak_queue_length   << ",";
        cout << "\"averageWaitingTime\":"    << (customers_served > 0
                                                  ? (double)total_wait_time / customers_served
                                                  : 0.0);
        cout << "}\n";
        cout.flush();
    }

    // Use shadow state (same source of truth as assignment logic)
    bool allServersFree() const {
        for (int i = 0; i < number_of_servers; i++)
            if (server_states[i].busy) return false;
        return true;
    }

    // Simulation ends when:
    //   (a) all customers arrived + queue empty + all servers idle, OR
    //   (b) safety cap hit
    bool isFinished() const {
        bool all_done = (customers_arrived >= total_arrivals_target)
                     && isQueueEmpty()
                     && allServersFree();
        return all_done || (current_clock >= simulation_time);
    }

    void runFullSimulation() {
        reset();
        start();
        while (!isFinished()) {
            tick();
            outputState();
        }
        outputFinalStats();
    }
};

// ── Entry point ───────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    try {
        int total_customers       = 100;
        int number_of_servers     = 4;
        int transaction_time      = 5;
        int time_between_arrivals = 3;
        // simulation_time is just a safety cap; sim ends when all customers served
        int simulation_time_cap   = 99999;

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

        WebSimulation sim(simulation_time_cap, number_of_servers,
                          transaction_time, time_between_arrivals, total_customers);

        // Emit initial state (tick 0, nothing has happened yet)
        sim.outputState();

        sim.start();
        while (!sim.isFinished()) {
            sim.tick();
            sim.outputState();   // ← sole call site: no duplicate output
        }

        sim.outputFinalStats();

    } catch (const std::exception& e) {
        // Errors go to stderr so they don't corrupt the stdout JSON stream
        std::cerr << "FATAL: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
