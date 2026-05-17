#ifndef INCLUDE_CUSTOMERTYPE_H_
#define INCLUDE_CUSTOMERTYPE_H_

/**
 * @file CustomerType.h
 * @brief Defines the customer record used by the queuing simulation.
 */

/**
 * @class CustomerType
 * @brief Stores identifying and timing data for a single customer.
 */
class CustomerType {
   private:
    int customer_number_  = 0;
    int arrival_time_     = 0;
    int waiting_time_     = 0;
    int transaction_time_ = 0;

   public:
    /**
     * @brief Sets all customer fields at once.
     * @param customer_no Customer identifier.
     * @param arr_time Tick when the customer entered the system.
     * @param w_time Current accumulated waiting time.
     * @param time_spent Required service time.
     */
    void setCustomerInfo(int customer_no = 0, int arr_time = 0, int w_time = 0, int time_spent = 0);

    /**
     * @brief Gets the accumulated waiting time.
     * @return Waiting time in ticks.
     */
    int getWaitingTime() const;

    /**
     * @brief Sets the accumulated waiting time.
     * @param time Waiting time in ticks.
     */
    void setWaitingTime(int time);

    /**
     * @brief Increments the waiting time by one tick.
     */
    void incrementWaitingTime();

    /**
     * @brief Gets the arrival tick.
     * @return Arrival time in ticks.
     */
    int getArrivalTime() const;

    /**
     * @brief Sets the arrival tick.
     * @param time Arrival time in ticks.
     */
    void setArrivalTime(int time);

    /**
     * @brief Gets the required service time.
     * @return Transaction time in ticks.
     */
    int getTransactionTime() const;

    /**
     * @brief Gets the customer identifier.
     * @return Customer number.
     */
    int getCustomerNumber() const;

    /**
     * @brief Constructs a customer with optional initial timing values.
     * @param customer_no Customer identifier.
     * @param arr_time Tick when the customer entered the system.
     * @param w_time Initial waiting time.
     * @param t_time Required service time.
     */
    explicit CustomerType(int customer_no = 0, int arr_time = 0, int w_time = 0, int t_time = 0);
};

#endif  // INCLUDE_CUSTOMERTYPE_H_
