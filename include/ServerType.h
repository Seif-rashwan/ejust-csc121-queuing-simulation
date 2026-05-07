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
    ServerType(const ServerType&)            = delete;
    ServerType& operator=(const ServerType&) = delete;

   public:
    // دوال الاستعلام عن حالة السيرفر
    bool isFree() const;
    void setBusy();
    void setFree();

    // دوال وقت الخدمة
    void setTransactionTime(int t);
    void setTransactionTime();
    int getRemainingTransactionTime() const;
    void decreaseTransactionTime();
    void setTransactionTimeFromCustomer();

    // دوال التعامل مع العميل الحالي
    void setCurrentCustomer(const CustomerType& c_customer);
    int getCurrentCustomerNumber() const;
    int getCurrentCustomerArrivalTime() const;
    int getCurrentCustomerWaitingTime() const;
    int getCurrentCustomerTransactionTime() const;

    // Constructor
    ServerType();
};

#endif  // INCLUDE_SERVERTYPE_H_
