// Copyright 2026 Seif Rashwan
#ifndef INCLUDE_SERVERTYPE_H_
#define INCLUDE_SERVERTYPE_H_

#include <string>
#include "CustomerType.h"

using std::string;

class ServerType {
   private:
    CustomerType currentCustomer;
    string status;
    int transactionTime;

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

    // دوال التعامل مع العميل الحالي
    void setCurrentCustomer(const CustomerType& cCustomer);
    int getCurrentCustomerNumber() const;
    int getCurrentCustomerArrivalTime() const;
    int getCurrentCustomerWaitingTime() const;
    int getCurrentCustomerTransactionTime() const;

    // Constructor
    ServerType();
};

#endif  // INCLUDE_SERVERTYPE_H_
