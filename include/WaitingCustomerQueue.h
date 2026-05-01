#ifndef WAITINGCUSTOMERQUEUE_H
#define WAITINGCUSTOMERQUEUE_H

#include <iostream>
#include "queueADT.h"
using namespace std;

template <class Type>
struct nodeType {
    Type info;
    nodeType<Type> *link;
};

template <class Type>
class waitingCustomerQueue : public queueADT<Type> {
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
    void updateWaitingTimes();

    // Constructor & Destructor
    waitingCustomerQueue();
    ~waitingCustomerQueue();
};

#endif