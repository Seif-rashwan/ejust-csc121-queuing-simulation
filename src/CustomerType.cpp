/**
 * @file CustomerType.cpp
 * @brief Implements customer data accessors and timing updates.
 * @copyright Copyright 2026 Seif Rashwan & The Project Team Members
 */

#include "CustomerType.h"

void CustomerType::setCustomerInfo(int customer_no, int arr_time, int w_time,  // NOLINT
                                   int time_spent) {
    customer_number_  = customer_no;
    arrival_time_     = arr_time;
    waiting_time_     = w_time;
    transaction_time_ = time_spent;
}

int CustomerType::getWaitingTime() const {
    return waiting_time_;
}

void CustomerType::setWaitingTime(int time) {
    waiting_time_ = time;
}

void CustomerType::incrementWaitingTime() {
    waiting_time_++;
}

void CustomerType::setArrivalTime(int time) {
    arrival_time_ = time;
}

int CustomerType::getArrivalTime() const {
    return arrival_time_;
}

int CustomerType::getTransactionTime() const {
    return transaction_time_;
}

int CustomerType::getCustomerNumber() const {
    return customer_number_;
}

CustomerType::CustomerType(int customer_no, int arr_time, int w_time, int t_time) {
    setCustomerInfo(customer_no, arr_time, w_time, t_time);
}
