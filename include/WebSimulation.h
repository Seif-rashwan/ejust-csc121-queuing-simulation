/**
 * @file WebSimulation.h
 * @brief Web-based queuing simulation with server management and statistics tracking.
 * @version 2.0
 */

#ifndef INCLUDE_WEBSIMULATION_H_
#define INCLUDE_WEBSIMULATION_H_

#include <string>
#include <vector>

#include "CustomerType.h"
#include "ServerListType.h"

/**
 * @class WebSimulation
 * @brief Manages a discrete-event queuing simulation with multiple servers.
 *
 * @details
 * - Uses round-robin server assignment for load balancing
 * - Maintains a circular array for customer queue storage
 * - Supports randomised arrival intervals and service times for realism
 * - Outputs state after each tick and final statistics at completion
 * - Runs until all customers are served or simulation time cap is reached
 */
class WebSimulation {
   private:
    /// @name Simulation Configuration
    /// @{
    int simulation_time_;        ///< Safety cap for simulation duration
    int number_of_servers_;      ///< Number of service points
    int transaction_time_min_;   ///< Minimum service time per customer (ticks)
    int transaction_time_max_;   ///< Maximum service time per customer (ticks)
    int arrival_interval_min_;   ///< Minimum ticks between arrivals
    int arrival_interval_max_;   ///< Maximum ticks between arrivals
    int total_arrivals_target_;  ///< Total customers expected to arrive
    int next_arrival_tick_;      ///< Pre-scheduled clock tick for next arrival
    /// @}

    /// @name Queue Management
    /// @{
    CustomerType* customer_array_;  ///< Circular array for waiting customers
    int queue_front_;               ///< Front index of circular queue
    int queue_rear_;                ///< Rear index of circular queue
    int queue_size_;                ///< Current number of customers in queue
    /// @}

    /// @name Server Management
    /// @{
    ServerListType* servers_;  ///< List of server objects
    int next_server_hint_;     ///< Round-robin pointer for server assignment
    /// @}

    /// @name Statistics Tracking
    /// @{
    int peak_queue_length_;      ///< Maximum queue length observed
    int total_wait_time_;        ///< Cumulative waiting time for all customers
    int customers_served_;       ///< Number of customers who completed service
    int customers_arrived_;      ///< Number of customers who have arrived
    int customers_turned_away_;  ///< Number of customers denied entry
    int current_clock_;          ///< Current simulation time unit
    bool running_;               ///< Whether simulation is currently running
    /// @}

    /// @name Event Tracking
    /// @{
    std::string last_event_type_;  ///< Type of last event ("arrived" | "serving" | "")
    int last_event_customer_id_;   ///< Customer ID associated with last event
    /// @}

    /**
     * @struct ServerState
     * @brief Shadow state for a single server, source of truth for UI and termination logic.
     */
    struct ServerState {
        bool busy;                 ///< Whether server is currently busy
        int remaining;             ///< Time units remaining for current service
        int assigned_customer_id;  ///< ID of customer being served (-1 if idle)
    };

    std::vector<ServerState> server_states_;  ///< Shadow state vector for all servers

    // ── Private helpers ───────────────────────────────────────────────────────
    /**
     * @brief Checks if the queue buffer is at capacity.
     * @return true if queue is full, false otherwise.
     */
    bool isQueueFull() const;

    /**
     * @brief Checks if the queue is empty.
     * @return true if queue contains no customers, false otherwise.
     */
    bool isQueueEmpty() const;

    /**
     * @brief Finds the next free server using round-robin selection.
     * @return Index of free server, or -1 if all servers are busy.
     */
    int getFreeServerRoundRobin();

    /**
     * @brief Adds a customer to the rear of the circular queue.
     * @param customer The customer to enqueue.
     */
    void enqueueCustomer(const CustomerType& customer);

    /**
     * @brief Removes and returns the customer at the front of the queue.
     * @return The front customer, or a default-constructed CustomerType if empty.
     */
    CustomerType dequeueCustomer();

    /**
     * @brief Checks if all servers are currently idle.
     * @return true if all servers are free, false otherwise.
     */
    bool allServersFree() const;

   public:
    static constexpr int DEFAULT_TOT_CUSTOMERS       = 100;
    static constexpr int SIMULATION_TIME_CAP_DEFAULT = 99999;
    static constexpr int DEFAULT_TRANS_MIN           = 4;
    static constexpr int DEFAULT_TRANS_MAX           = 8;
    static constexpr int DEFAULT_ARRIVAL_MIN         = 2;
    static constexpr int DEFAULT_ARRIVAL_MAX         = 5;

    /**
     * @brief Constructs a WebSimulation with randomised timing ranges.
     * @param sim_time          Safety cap for total simulation duration (ticks).
     * @param num_servers       Number of parallel servers.
     * @param trans_min         Minimum service time per customer (ticks).
     * @param trans_max         Maximum service time per customer (ticks).
     * @param arrival_min       Minimum interval between arrivals (ticks).
     * @param arrival_max       Maximum interval between arrivals (ticks).
     * @param total_cust        Total number of customers to simulate.
     */
    explicit WebSimulation(int sim_time, int num_servers, int trans_min, int trans_max,
                           int arrival_min, int arrival_max,
                           int total_cust = DEFAULT_TOT_CUSTOMERS);

    /**
     * @brief Destructor - releases dynamically allocated resources.
     */
    ~WebSimulation();

    WebSimulation(const WebSimulation&)            = delete;
    WebSimulation& operator=(const WebSimulation&) = delete;

    /**
     * @brief Returns a random integer in [min, max] (inclusive).
     * @param min Lower bound.
     * @param max Upper bound.
     * @return Random integer between min and max.
     */
    static int randomBetween(int min, int max);

    // ── Lifecycle ─────────────────────────────────────────────────────────────
    /**
     * @brief Resets the simulation to its initial state.
     *
     * Clears the queue, resets all statistics, reinitialises servers, resets the
     * clock to 0, and schedules the first arrival.
     */
    void reset();

    /**
     * @brief Starts or resumes the simulation.
     */
    void start();

    /**
     * @brief Pauses the simulation.
     */
    void pause();

    // ── Simulation Execution ──────────────────────────────────────────────────
    /**
     * @brief Advances the simulation by exactly one time unit.
     *
     * Execution order per tick:
     * 1. Decrement shadow server states; count served on completion.
     * 2. Update real ServerListType objects in parallel.
     * 3. Process customer arrivals (if scheduled and target not reached).
     * 4. Assign free servers via round-robin until queue is empty.
     * 5. Increment waiting times for customers still in queue.
     * 6. Update peak queue length.
     */
    void tick();

    /**
     * @brief Runs the simulation to completion in a single blocking call.
     */
    void runFullSimulation();

    /**
     * @brief Checks whether the simulation has completed.
     *
     * Returns true when all customers have arrived, the queue is empty, and all
     * servers are idle, or when the safety time cap is reached.
     *
     * @return true if simulation should end, false otherwise.
     */
    bool isFinished() const;

    // ── Output ────────────────────────────────────────────────────────────────
    int getQueueSize() const {
        return queue_size_;
    }

    int getCustomersServed() const {
        return customers_served_;
    }

    int getCustomersArrived() const {
        return customers_arrived_;
    }

    int getCustomersTurnedAway() const {
        return customers_turned_away_;
    }

    int getPeakQueueLength() const {
        return peak_queue_length_;
    }

    int getTotalWaitTime() const {
        return total_wait_time_;
    }

    double getAverageWaitingTime() const {
        return customers_served_ > 0 ? static_cast<double>(total_wait_time_) / customers_served_
                                     : 0.0;
    }

    /**
     * @brief Emits current simulation state as a single JSON line to stdout.
     *
     * Format: `STATE:{…JSON…}\n`
     * No other content is ever written to stdout by this program.
     */
    void outputState() const;

    /**
     * @brief Emits final simulation statistics as a single JSON line to stdout.
     *
     * Format: `FINAL:{…JSON…}\n`
     */
    void outputFinalStats() const;
};

#endif  // INCLUDE_WEBSIMULATION_H_