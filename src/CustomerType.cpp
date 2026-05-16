// Copyright 2026 Seif Rashwan

#include "CustomerType.h"

// Sets all customer data together
void CustomerType::setCustomerInfo(int customer_no, int arr_time, int w_time,  // NOLINT
                                   int time_spent) {
    customer_number_  = customer_no;
    arrival_time_     = arr_time;
    waiting_time_     = w_time;
    transaction_time_ = time_spent;
}

// Returns the current waiting time
int CustomerType::getWaitingTime() const {
    return waiting_time_;
}

// Sets the waiting time
// cppcheck-suppress unusedFunction
void CustomerType::setWaitingTime(int time) {
    waiting_time_ = time;
}

// Increases waiting time by 1 tick
void CustomerType::incrementWaitingTime() {
    waiting_time_++;
}

// Sets the arrival time
// cppcheck-suppress unusedFunction
void CustomerType::setArrivalTime(int time) {
    arrival_time_ = time;
}

// Returns the arrival time
int CustomerType::getArrivalTime() const {
    return arrival_time_;
}

// Returns the required service time
int CustomerType::getTransactionTime() const {
    return transaction_time_;
}

// Returns the customer ID
int CustomerType::getCustomerNumber() const {
    return customer_number_;
}

// Constructor initializes customer information
CustomerType::CustomerType(int customer_no, int arr_time, int w_time, int t_time) {
    setCustomerInfo(customer_no, arr_time, w_time, t_time);
}