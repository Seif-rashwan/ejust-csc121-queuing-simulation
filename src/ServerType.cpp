#include "ServerType.h"

// Checks if the server is free
bool ServerType::isFree() const {
    return (status_ == "free");
}

// Sets the server status to busy
void ServerType::setBusy() {
    status_ = "busy";
}

// Sets the server status to free
void ServerType::setFree() {
    status_ = "free";
}

// Sets the transaction/service time manually
void ServerType::setTransactionTime(int time) {
    if (time >= 0) {
        transaction_time_ = time;
    } else {
        transaction_time_ = 0;
    }
}

// Sets service time based on current customer transaction time
void ServerType::setTransactionTime() {
    transaction_time_ = current_customer_.getTransactionTime();
}

// Returns remaining service time
int ServerType::getRemainingTransactionTime() const {
    return transaction_time_;
}

// Decreases remaining service time by 1 tick
void ServerType::decreaseTransactionTime() {
    if (transaction_time_ > 0) {
        transaction_time_--;
    }
}

// Assigns a customer to the server
void ServerType::setCurrentCustomer(const CustomerType& c_customer) {
    current_customer_ = c_customer;
}

// Returns current customer ID
int ServerType::getCurrentCustomerNumber() const {
    return current_customer_.getCustomerNumber();
}

// Returns arrival time of current customer
int ServerType::getCurrentCustomerArrivalTime() const {
    return current_customer_.getArrivalTime();
}

// Returns waiting time of current customer
int ServerType::getCurrentCustomerWaitingTime() const {
    return current_customer_.getWaitingTime();
}

// Returns transaction time of current customer
int ServerType::getCurrentCustomerTransactionTime() const {
    return current_customer_.getTransactionTime();
}

// Constructor initializes server as free
ServerType::ServerType() : status_("free"), transaction_time_(0) {}