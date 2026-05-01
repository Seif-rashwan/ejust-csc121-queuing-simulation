#include "customerType.h"

void customerType::setCustomerInfo(int customerNumber, int arrivalTime, int waitingTime,
                                   int transactionTime) {
    this->customerNumber  = customerNumber;
    this->arrivalTime     = arrivalTime;
    this->waitingTime     = waitingTime;
    this->transactionTime = transactionTime;
}

int customerType::getWaitingTime() const { return waitingTime; }

void customerType::setWaitingTime(int time) { waitingTime = time; }

void customerType::incrementWaitingTime() { waitingTime++; }

int customerType::getArrivalTime() const { return arrivalTime; }

int customerType::getTransactionTime() const { return transactionTime; }

int customerType::getCustomerNumber() const { return customerNumber; }

customerType::customerType(int customerNumber, int arrivalTime, int waitingTime,
                           int transactionTime) {
    this->customerNumber  = customerNumber;
    this->arrivalTime     = arrivalTime;
    this->waitingTime     = waitingTime;
    this->transactionTime = transactionTime;
}