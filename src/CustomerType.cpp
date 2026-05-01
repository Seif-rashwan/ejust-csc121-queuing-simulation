// Copyright 2026 E-JUST CSC 121 Project

#include "CustomerType.h"

void CustomerType::setCustomerInfo(int customer_number, int arrival_time, 
                                    int waiting_time, int transaction_time) {
    this->customer_number = customer_number;
    this->arrival_time = arrival_time;
    this->waiting_time = waiting_time;
    this->transaction_time = transaction_time;
}

int CustomerType::getWaitingTime() const {
    return waiting_time;
}

void CustomerType::setWaitingTime(int time) {
    waiting_time = time;
}

void CustomerType::incrementWaitingTime() {
    waiting_time++;
}

int CustomerType::getArrivalTime() const {
    return arrival_time;
}

int CustomerType::getTransactionTime() const {
    return transaction_time;
}

int CustomerType::getCustomerNumber() const {
    return customer_number;
}

CustomerType::CustomerType(int customer_number, int arrival_time, 
                            int waiting_time, int transaction_time) {
    this->customer_number = customer_number;
    this->arrival_time = arrival_time;
    this->waiting_time = waiting_time;
    this->transaction_time = transaction_time;
}
