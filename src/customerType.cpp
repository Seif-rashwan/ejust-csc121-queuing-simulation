#include "customerType.h"

// دالة لضبط كل بيانات العميل مرة واحدة
void customerType::setCustomerInfo(int customerN, int arrTime, int wTime, int tTime) {
    customerNumber = customerN;
    arrivalTime = arrTime;
    waitingTime = wTime;
    transactionTime = tTime;
}

// إرجاع وقت الانتظار
int customerType::getWaitingTime() const {
    return waitingTime;
}

// ضبط وقت الانتظار بقيمة معينة
void customerType::setWaitingTime(int time) {
    waitingTime = time;
}

// زيادة وقت الانتظار بمقدار 1 (دي الدالة اللي الطابور بينادي عليها)
void customerType::incrementWaitingTime() {
    waitingTime++;
}

// إرجاع وقت الوصول
int customerType::getArrivalTime() const {
    return arrivalTime;
}

// إرجاع وقت الخدمة المطلوب
int customerType::getTransactionTime() const {
    return transactionTime;
}

// إرجاع رقم العميل
int customerType::getCustomerNumber() const {
    return customerNumber;
}

// الـ Constructor بينادي على setCustomerInfo عشان يختصر الكود
customerType::customerType(int customerN, int arrTime, int wTime, int tTime) {
    setCustomerInfo(customerN, arrTime, wTime, tTime);
}