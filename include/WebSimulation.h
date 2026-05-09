/**
 * @file WebSimulation.h
 * @brief Web-based queuing simulation with server management and statistics tracking.
 * @version 1.0
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
 * - Outputs state after each tick and final statistics at completion
 * - Runs until all customers are served or simulation time cap is reached
 */
class WebSimulation {
   private:
    /// @name Simulation Configuration
    /// @{
    int simulation_time_;        ///< Safety cap for simulation duration
    int number_of_servers_;      ///< Number of service points
    int transaction_time_;       ///< Time units per service
    int time_between_arrivals_;  ///< Interval between customer arrivals
    int total_arrivals_target_;  ///< Total customers expected to arrive
    /// @}

    /// @name Queue Management
    /// @{
    CustomerType* customer_array_;  ///< Circular array buffer for waiting customers
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
    std::string last_event_type_;  ///< Type of last event (e.g., "arrived", "serving")
    int last_event_customer_id_;   ///< Customer ID associated with last event
    /// @}

    /**
     * @struct ServerState
     * @brief Tracks the state of a single server.
     */
    struct ServerState {
        bool busy;                 ///< Whether server is currently busy
        int remaining;             ///< Time units remaining for current service
        int assigned_customer_id;  ///< ID of customer being served (-1 if idle)
    };

    std::vector<ServerState> server_states_;  ///< State vector for all servers

    //--------------------------------------------------------------------------------
    // Helpers:
    /**
     * @brief Checks if the queue buffer is at capacity.
     * @return true if queue is full, false otherwise
     */
    bool isQueueFull() const;

    /**
     * @brief Checks if the queue is empty.
     * @return true if queue contains no customers, false otherwise
     */
    bool isQueueEmpty() const;

    /**
     * @brief Finds the next free server using round-robin selection.
     * @return Index of free server, or -1 if all servers are busy
     */
    int getFreeServerRoundRobin();

    /**
     * @brief Adds a customer to the rear of the queue.
     * @param customer The customer to enqueue
     */
    void enqueueCustomer(const CustomerType& customer);

    /**
     * @brief Removes and returns the customer from the front of the queue.
     * @return The customer from the front, or default-constructed CustomerType if empty
     */
    CustomerType dequeueCustomer();

    /**
     * @brief Returns the customer at the front without removing.
     * @return The customer at front, or default-constructed CustomerType if empty
     */
    CustomerType peekCustomer() const;

    /**
     * @brief Checks if all servers are currently idle.
     * @return true if all servers are free, false otherwise
     */
    bool allServersFree() const;

   public:
    static const int DEFAULT_TOT_CUSTOMERS       = 100;
    static const int SIMULATION_TIME_CAP_DEFAULT = 99999;

    /**
     * @brief Constructs a WebSimulation with specified parameters.
     * @param sim_time Safety cap for simulation duration (time units)
     * @param num_servers Number of parallel servers
     * @param trans_time Transaction/service time per customer
     * @param t_between_arrivals Time interval between customer arrivals
     * @param total_cust Total number of customers expected (default: 100)
     */
    WebSimulation(int sim_time, int num_servers, int trans_time, int t_between_arrivals,
                  int total_cust = DEFAULT_TOT_CUSTOMERS);

    /**
     * @brief Destructor; cleans up dynamically allocated resources.
     */
    ~WebSimulation();

    //--------------------------------------------------------------------------------
    // Lifecycle:
    /**
     * @brief Resets the simulation to initial state.
     *
     * Clears all queues, resets statistics, reinitializes servers, and sets clock to 0.
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

    //--------------------------------------------------------------------------------
    // Simulation Execution:
    /**
     * @brief Executes one simulation time unit.
     *
     * Performs:
     * - Server state updates and completion tracking
     * - Customer arrivals
     * - Round-robin server assignment
     * - Queue wait time increments
     * - Peak queue tracking
     */
    void tick();

    /**
     * @brief Runs the complete simulation from start to finish.
     *
     * Resets state, starts simulation, and runs ticks until completion,
     * outputting state after each tick and final statistics.
     */
    void runFullSimulation();

    /**
     * @brief Checks if the simulation has completed.
     *
     * Completes when all customers have arrived, queue is empty, and all servers
     * are idle, or when the simulation time cap is reached.
     *
     * @return true if simulation should end, false otherwise
     */
    bool isFinished() const;

    //--------------------------------------------------------------------------------
    // Output Methods:
    /**
     * @brief Outputs current simulation state as a JSON object to stdout.
     *
     * Emits a single line prefixed with "STATE:" containing JSON with current
     * tick, queue size, customers served, peak queue, average wait time, and
     * detailed server states.
     */
    void outputState() const;

    /**
     * @brief Outputs final simulation statistics as a JSON object to stdout.
     *
     * Emits a single line prefixed with "FINAL:" containing JSON with
     * comprehensive final statistics.
     */
    void outputFinalStats() const;
};

#endif  // INCLUDE_WEBSIMULATION_H_
