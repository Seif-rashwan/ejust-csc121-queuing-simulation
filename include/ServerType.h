#ifndef INCLUDE_SERVERTYPE_H_
#define INCLUDE_SERVERTYPE_H_

#include <string>
#include "CustomerType.h"

// Copyright 2026 E-JUST CSC 121 Project

class ServerType {
public:
    // Functions
    bool isFree() const;
    void setBusy();
    void setFree();
    void setTransactionTime(int t);
    void setTransactionTime();
    int getRemainingTransactionTime() const;
    void decreaseTransactionTime();
    void setCurrentCustomer(CustomerType current_customer);
    int getCurrentCustomerNumber() const;
    int getCurrentCustomerArrivalTime() const;
    int getCurrentCustomerWaitingTime() const;
    int getCurrentCustomerTransactionTime() const;

    // Constructor
    ServerType();

private:
    CustomerType current_customer;
    std::string status;
    int transaction_time;
};

#endif  // INCLUDE_SERVERTYPE_H_
