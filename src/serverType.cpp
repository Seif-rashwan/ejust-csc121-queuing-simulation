#include "serverType.h"

bool serverType::isFree() const { return status == "free"; }

void serverType::setBusy() { status = "busy"; }

void serverType::setFree() { status = "free"; }

void serverType::setTransactionTime(int t) { transactionTime = t; }

void serverType::setTransactionTime() { transactionTime = currentCustomer.getTransactionTime(); }

int serverType::getRemainingTransactionTime() const { return transactionTime; }

void serverType::decreaseTransactionTime() { transactionTime--; }

void serverType::setCurrentCustomer(customerType currentCustomer) {
    this->currentCustomer = currentCustomer;
}

int serverType::getCurrentCustomerNumber() const { return currentCustomer.getCustomerNumber(); }

int serverType::getCurrentCustomerArrivalTime() const { return currentCustomer.getArrivalTime(); }

int serverType::getCurrentCustomerWaitingTime() const { return currentCustomer.getWaitingTime(); }

int serverType::getCurrentCustomerTransactionTime() const {
    return currentCustomer.getTransactionTime();
}

serverType::serverType() {
    status          = "free";
    transactionTime = 0;
}