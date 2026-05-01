// Copyright 2026 Seif Rashwan
#include "CustomerType.h"

// دالة لضبط كل بيانات العميل مرة واحدة
void CustomerType::setCustomerInfo(int customerN, int arrTime, int wTime, int tTime) {
    customerNumber  = customerN;
    arrivalTime     = arrTime;
    waitingTime     = wTime;
    transactionTime = tTime;
}

// إرجاع وقت الانتظار
int CustomerType::getWaitingTime() const {
    return waitingTime;
}

// ضبط وقت الانتظار بقيمة معينة
void CustomerType::setWaitingTime(int time) {
    waitingTime = time;
}

// زيادة وقت الانتظار بمقدار 1 (دي الدالة اللي الطابور بينادي عليها)
void CustomerType::incrementWaitingTime() {
    waitingTime++;
}

// إرجاع وقت الوصول
int CustomerType::getArrivalTime() const {
    return arrivalTime;
}

// إرجاع وقت الخدمة المطلوب
int CustomerType::getTransactionTime() const {
    return transactionTime;
}

// إرجاع رقم العميل
int CustomerType::getCustomerNumber() const {
    return customerNumber;
}

// الـ Constructor بينادي على setCustomerInfo عشان يختصر الكود
CustomerType::CustomerType(int customerN, int arrTime, int wTime, int tTime) {
    setCustomerInfo(customerN, arrTime, wTime, tTime);
}