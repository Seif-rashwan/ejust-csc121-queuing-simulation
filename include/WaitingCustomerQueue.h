// Copyright 2026 Seif Rashwan
#ifndef INCLUDE_WAITINGCUSTOMERQUEUE_H_
#define INCLUDE_WAITINGCUSTOMERQUEUE_H_

#include "QueueADT.h"

template <typename Type>
struct LinkedNode {
    Type data;
    LinkedNode<Type>* next;
};

template <typename Type>
class WaitingCustomerQueue : public QueueADT<Type> {
   private:
    LinkedNode<Type>* front_;
    LinkedNode<Type>* rear_;
    void initialize();

   public:
    void enqueue(const Type& customer) override;
    void dequeue() override;

    Type front() const override;
    Type back() const override;
    bool isEmpty() const override;

    void incrementWaitingTimes(const double& inc_amount);

    WaitingCustomerQueue();
    ~WaitingCustomerQueue();
};

#endif  // INCLUDE_WAITINGCUSTOMERQUEUE_H_
