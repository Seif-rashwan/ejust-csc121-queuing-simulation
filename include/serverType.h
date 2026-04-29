#ifndef SERVERTYPE_H
#define SERVERTYPE_H

#include <string>
#include "customerType.h"

using namespace std;

class serverType {
private:
    customerType currentCustomer;
    string status;
    int transactionTime;

public:
    // Functions
    bool isFree() const;
    void setBusy();
    void setFree();
    void setTransactionTime(int t);
    void setTransactionTime(); 
    int getRemainingTransactionTime() const;
    void decreaseTransactionTime();
    void setCurrentCustomer(customerType cCustomer);
    int getCurrentCustomerNumber() const;
    int getCurrentCustomerArrivalTime() const;
    int getCurrentCustomerWaitingTime() const;
    int getCurrentCustomerTransactionTime() const;

    // Constructor
    serverType();
};

#endif