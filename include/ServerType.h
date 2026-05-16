// Copyright 2026 Seif Rashwan
#ifndef INCLUDE_SERVERTYPE_H_
#define INCLUDE_SERVERTYPE_H_

#include <string>
#include "CustomerType.h"

using std::string;

class ServerType {
   private:
    CustomerType current_customer_;
    string status_;
    int transaction_time_;

   public:
    bool isFree() const;
    void setBusy();
    void setFree();

    void setTransactionTime(int time);
    void setTransactionTime();
    int getRemainingTransactionTime() const;
    void decreaseTransactionTime();

    void setCurrentCustomer(const CustomerType& c_customer);
    int getCurrentCustomerNumber() const;
    int getCurrentCustomerArrivalTime() const;
    int getCurrentCustomerWaitingTime() const;
    int getCurrentCustomerTransactionTime() const;

    ServerType();
    ~ServerType()                            = default;
    ServerType(const ServerType&)            = delete;
    ServerType& operator=(const ServerType&) = delete;
};

#endif  // INCLUDE_SERVERTYPE_H_