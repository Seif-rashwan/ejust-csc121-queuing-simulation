#include "ServerType.h"
#include <iostream>

using std::cout;

// التحقق لو السيرفر فاضي
bool ServerType::isFree() const {
    return (status_ == "free");
}

// تغيير حالة السيرفر لمشغول
void ServerType::setBusy() {
    status_ = "busy";
}

// تغيير حالة السيرفر لفاضي
void ServerType::setFree() {
    status_ = "free";
}

// ضبط وقت الخدمة بقيمة معينة
void ServerType::setTransactionTime(int time) {
    if (time >= 0) {
        transaction_time_ = time;
    } else {
        transaction_time_ = 0;
    }
}

// ضبط وقت الخدمة بناءً على وقت العميل الحالي
void ServerType::setTransactionTime() {
    if (status_ == "busy") {
        transaction_time_ = current_customer_.getTransactionTime();
    } else {
        transaction_time_ = 0;
    }
}

// إرجاع الوقت المتبقي لانتهاء الخدمة
int ServerType::getRemainingTransactionTime() const {
    return transaction_time_;
}

// تقليل وقت الخدمة بمقدار 1 (مع كل لفة في الـ Simulation)
void ServerType::decreaseTransactionTime() {
    if (transaction_time_ > 0) {
        transaction_time_--;
    }
}

// تعيين العميل اللي السيرفر بيخدمه حالياً
void ServerType::setCurrentCustomer(const CustomerType& c_customer) {
    current_customer_ = c_customer;
}

// إرجاع رقم العميل الحالي
int ServerType::getCurrentCustomerNumber() const {
    return current_customer_.getCustomerNumber();
}

// إرجاع وقت وصول العميل الحالي
int ServerType::getCurrentCustomerArrivalTime() const {
    return current_customer_.getArrivalTime();
}

// إرجاع وقت انتظار العميل الحالي
int ServerType::getCurrentCustomerWaitingTime() const {
    return current_customer_.getWaitingTime();
}

// إرجاع وقت خدمة العميل الحالي
int ServerType::getCurrentCustomerTransactionTime() const {
    return current_customer_.getTransactionTime();
}

// Constructor
ServerType::ServerType() : status_("free"), transaction_time_(0) {}