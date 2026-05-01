#include <iostream>
#include "ServerType.h"

using namespace std;

// التحقق لو السيرفر فاضي
bool ServerType::isFree() const
{
    return (status == "free");
}

// تغيير حالة السيرفر لمشغول
void ServerType::setBusy()
{
    status = "busy";
}

// تغيير حالة السيرفر لفاضي
void ServerType::setFree()
{
    status = "free";
}

// ضبط وقت الخدمة بقيمة معينة
void ServerType::setTransactionTime(int t)
{
    if (t >= 0)
    {
        transactionTime = t;
    }
    else
    {
        transactionTime = 0;
    }
}

// ضبط وقت الخدمة بناءً على وقت العميل الحالي
void ServerType::setTransactionTime()
{
    if (status == "busy")
    {
        transactionTime = currentCustomer.getTransactionTime();
    }
    else
    {
        transactionTime = 0;
    }
}

// إرجاع الوقت المتبقي لانتهاء الخدمة
int ServerType::getRemainingTransactionTime() const
{
    return transactionTime;
}

// تقليل وقت الخدمة بمقدار 1 (مع كل لفة في الـ Simulation)
void ServerType::decreaseTransactionTime()
{
    if (transactionTime > 0)
    {
        transactionTime--;
    }
}

// تعيين العميل اللي السيرفر بيخدمه حالياً
void ServerType::setCurrentCustomer(const CustomerType& cCustomer)
{
    currentCustomer = cCustomer;
}

// إرجاع رقم العميل الحالي
int ServerType::getCurrentCustomerNumber() const
{
    return currentCustomer.getCustomerNumber();
}

// إرجاع وقت وصول العميل الحالي
int ServerType::getCurrentCustomerArrivalTime() const
{
    return currentCustomer.getArrivalTime();
}

// إرجاع وقت انتظار العميل الحالي
int ServerType::getCurrentCustomerWaitingTime() const
{
    return currentCustomer.getWaitingTime();
}

// إرجاع وقت خدمة العميل الحالي
int ServerType::getCurrentCustomerTransactionTime() const
{
    return currentCustomer.getTransactionTime();
}

// Constructor
ServerType::ServerType()
{
    status = "free";
    transactionTime = 0;
}