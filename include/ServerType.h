#ifndef SERVERTYPE_H
#define SERVERTYPE_H

#include <string>
#include "CustomerType.h"

using namespace std;

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

#endif