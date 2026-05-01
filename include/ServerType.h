#ifndef SERVERTYPE_H
#define SERVERTYPE_H

#include <string>

#include "customerType.h"

class serverType {
public:
    // Functions
    bool isFree() const;
    void setBusy();
    void setFree();
    void setTransactionTime(int t);
    void setTransactionTime();
    int getRemainingTransactionTime() const;
    void decreaseTransactionTime();
    void setCurrentCustomer(customerType currentCustomer);
    int getCurrentCustomerNumber() const;
    int getCurrentCustomerArrivalTime() const;
    int getCurrentCustomerWaitingTime() const;
    int getCurrentCustomerTransactionTime() const;

    // Constructor
    serverType();

private:
    customerType currentCustomer;
    std::string status;
    int transactionTime;
};

#endif  // SERVERTYPE_H