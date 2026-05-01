// Copyright 2026 Seif Rashwan
#ifndef INCLUDE_WAITINGCUSTOMERQUEUE_H_
#define INCLUDE_WAITINGCUSTOMERQUEUE_H_

#include <iostream>
#include "QueueADT.h"

template <class Type>
struct nodeType {
    Type info;
    nodeType<Type> *link;
};

template <class Type>
class WaitingCustomerQueue : public QueueADT<Type> {
private:
    nodeType<Type> *queueFront; // مؤشر لأول عنصر في الطابور
    nodeType<Type> *queueRear;  // مؤشر لآخر عنصر في الطابور

public:
    // الدوال الأساسية الموروثة من queueADT
    bool isEmptyQueue() const;
    bool isFullQueue() const;
    void initializeQueue();
    Type front() const;
    Type back() const;
    void addQueue(const Type& queueElement);
    void deleteQueue();

    // الدالة الإضافية المطلوبة للـ Simulation لزيادة وقت الانتظار
    void incrementWaitingTimes();

    // Constructor & Destructor
    WaitingCustomerQueue();
    ~WaitingCustomerQueue();
};

#endif  // INCLUDE_WAITINGCUSTOMERQUEUE_H_
