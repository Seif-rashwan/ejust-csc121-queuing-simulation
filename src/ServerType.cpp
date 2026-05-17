/**
 * @file ServerType.cpp
 * @brief Implements one simulation server's status and customer state.
 */

#include "ServerType.h"

bool ServerType::isFree() const {
    return (status_ == "free");
}

void ServerType::setBusy() {
    status_ = "busy";
}

void ServerType::setFree() {
    status_ = "free";
}

void ServerType::setTransactionTime(int time) {
    if (time >= 0) {
        transaction_time_ = time;
    } else {
        transaction_time_ = 0;
    }
}

void ServerType::setTransactionTime() {
    transaction_time_ = current_customer_.getTransactionTime();
}

int ServerType::getRemainingTransactionTime() const {
    return transaction_time_;
}

void ServerType::decreaseTransactionTime() {
    if (transaction_time_ > 0) {
        transaction_time_--;
    }
}

void ServerType::setCurrentCustomer(const CustomerType& c_customer) {
    current_customer_ = c_customer;
}

int ServerType::getCurrentCustomerNumber() const {
    return current_customer_.getCustomerNumber();
}

int ServerType::getCurrentCustomerArrivalTime() const {
    return current_customer_.getArrivalTime();
}

int ServerType::getCurrentCustomerWaitingTime() const {
    return current_customer_.getWaitingTime();
}

int ServerType::getCurrentCustomerTransactionTime() const {
    return current_customer_.getTransactionTime();
}

ServerType::ServerType() : status_("free") {}
