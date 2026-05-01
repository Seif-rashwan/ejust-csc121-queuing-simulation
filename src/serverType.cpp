#include <iostream>
#include "serverType.h"

using namespace std;

// التحقق لو السيرفر فاضي
bool serverType::isFree() const {
    return (status == "free");
}

// تغيير حالة السيرفر لمشغول
void serverType::setBusy() {
    status = "busy";
}

// تغيير حالة السيرفر لفاضي
void serverType::setFree() {
    status = "free";
}

// ضبط وقت الخدمة بقيمة معينة
void serverType::setTransactionTime(int t) {
    transactionTime = t;
}

// ضبط وقت الخدمة بناءً على وقت العميل الحالي
void serverType::setTransactionTime() {
    transactionTime = currentCustomer.getTransactionTime();
}

// إرجاع الوقت المتبقي لانتهاء الخدمة
int serverType::getRemainingTransactionTime() const {
    return transactionTime;
}

// تقليل وقت الخدمة بمقدار 1 (مع كل لفة في الـ Simulation)
void serverType::decreaseTransactionTime() {
    if (transactionTime > 0) {
        transactionTime--;
    }
}

// تعيين العميل اللي السيرفر بيخدمه حالياً
void serverType::setCurrentCustomer(const customerType& cCustomer) {
    currentCustomer = cCustomer;
}

// إرجاع رقم العميل الحالي
int serverType::getCurrentCustomerNumber() const {
    return currentCustomer.getCustomerNumber();
}

// إرجاع وقت وصول العميل الحالي
int serverType::getCurrentCustomerArrivalTime() const {
    return currentCustomer.getArrivalTime();
}

// إرجاع وقت انتظار العميل الحالي
int serverType::getCurrentCustomerWaitingTime() const {
    return currentCustomer.getWaitingTime();
}

// إرجاع وقت خدمة العميل الحالي
int serverType::getCurrentCustomerTransactionTime() const {
    return currentCustomer.getTransactionTime();
}

// Constructor
serverType::serverType() {
    status = "free";
    transactionTime = 0;
}