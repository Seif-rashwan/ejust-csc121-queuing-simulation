#ifndef INCLUDE_SERVERTYPE_H_
#define INCLUDE_SERVERTYPE_H_

/**
 * @file ServerType.h
 * @brief Defines one service point in the queuing simulation.
 * @copyright Copyright 2026 Seif Rashwan
 */

#include <string>
#include "CustomerType.h"

using std::string;

/**
 * @class ServerType
 * @brief Tracks a server's status, assigned customer, and remaining service time.
 */
class ServerType {
   private:
    string status_;
    CustomerType current_customer_;
    int transaction_time_ = 0;

   public:
    /**
     * @brief Checks whether the server is idle.
     * @return true when the server status is free.
     */
    bool isFree() const;

    /**
     * @brief Marks the server as busy.
     */
    void setBusy();

    /**
     * @brief Marks the server as free.
     */
    void setFree();

    /**
     * @brief Sets the remaining service time.
     * @param time Service time in ticks; negative values are clamped to zero.
     */
    void setTransactionTime(int time);

    /**
     * @brief Copies the current customer's transaction time into the server timer.
     */
    void setTransactionTime();

    /**
     * @brief Gets the remaining service time.
     * @return Remaining transaction time in ticks.
     */
    int getRemainingTransactionTime() const;

    /**
     * @brief Decrements the remaining service time by one tick when positive.
     */
    void decreaseTransactionTime();

    /**
     * @brief Assigns the customer currently being served.
     * @param c_customer Customer assigned to this server.
     */
    void setCurrentCustomer(const CustomerType& c_customer);

    /**
     * @brief Gets the assigned customer's identifier.
     * @return Current customer number.
     */
    int getCurrentCustomerNumber() const;

    /**
     * @brief Gets the assigned customer's arrival tick.
     * @return Current customer arrival time.
     */
    int getCurrentCustomerArrivalTime() const;

    /**
     * @brief Gets the assigned customer's waiting time.
     * @return Current customer waiting time.
     */
    int getCurrentCustomerWaitingTime() const;

    /**
     * @brief Gets the assigned customer's original service time.
     * @return Current customer transaction time.
     */
    int getCurrentCustomerTransactionTime() const;

    /**
     * @brief Constructs a free server with no remaining work.
     */
    ServerType();

    /**
     * @brief Uses the default destructor.
     */
    ~ServerType()                            = default;
    ServerType(const ServerType&)            = delete;
    ServerType& operator=(const ServerType&) = delete;
};

#endif  // INCLUDE_SERVERTYPE_H_
