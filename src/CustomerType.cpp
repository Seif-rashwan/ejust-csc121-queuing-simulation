// Copyright 2026 Seif Rashwan
#include "CustomerType.h"

// دالة لضبط كل بيانات العميل مرة واحدة
// NOLINT(bugprone-easily-swappable-parameters)
void CustomerType::setCustomerInfo(int customer_no, int arr_time, int waiting_time,  // NOLINT
                                   int time_spent) {
    customer_number_  = customer_no;
    arrival_time_     = arr_time;
    waiting_time_     = waiting_time;
    transaction_time_ = time_spent;
}

// إرجاع وقت الانتظار
int CustomerType::getWaitingTime() const {
    return waiting_time_;
}

// ضبط وقت الانتظار بقيمة معينة
void CustomerType::setWaitingTime(int time) {
    waiting_time_ = time;
}

// زيادة وقت الانتظار بمقدار 1 (دي الدالة اللي الطابور بينادي عليها)
void CustomerType::incrementWaitingTime() {
    waiting_time_++;
}

// إرجاع وقت الوصول
void CustomerType::setArrivalTime(int time) {
    arrival_time_ = time;
}

// إرجاع وقت الوصول
int CustomerType::getArrivalTime() const {
    return arrival_time_;
}

// إرجاع وقت الخدمة المطلوب
int CustomerType::getTransactionTime() const {
    return transaction_time_;
}

// إرجاع رقم العميل
int CustomerType::getCustomerNumber() const {
    return customer_number_;
}

// الـ Constructor بينادي على setCustomerInfo عشان يختصر الكود
CustomerType::CustomerType(int customer_no, int arr_time, int w_time, int t_time) {
    setCustomerInfo(customer_no, arr_time, w_time, t_time);
}