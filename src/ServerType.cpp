// Copyright 2026 E-JUST CSC 121 Project

#include "ServerType.h"

bool ServerType::isFree() const {
    return status == "free";
}

void ServerType::setBusy() {
    status = "busy";
}

void ServerType::setFree() {
    status = "free";
}

void ServerType::setTransactionTime(int t) {
    transaction_time = t;
}

void ServerType::setTransactionTime() {
    transaction_time = current_customer.getTransactionTime();
}

int ServerType::getRemainingTransactionTime() const {
    return transaction_time;
}

void ServerType::decreaseTransactionTime() {
    transaction_time--;
}

void ServerType::setCurrentCustomer(CustomerType current_customer) {
    this->current_customer = current_customer;
}

int ServerType::getCurrentCustomerNumber() const {
    return current_customer.getCustomerNumber();
}

int ServerType::getCurrentCustomerArrivalTime() const {
    return current_customer.getArrivalTime();
}

int ServerType::getCurrentCustomerWaitingTime() const {
    return current_customer.getWaitingTime();
}

int ServerType::getCurrentCustomerTransactionTime() const {
    return current_customer.getTransactionTime();
}

ServerType::ServerType() {
    status = "free";
    transaction_time = 0;
}
