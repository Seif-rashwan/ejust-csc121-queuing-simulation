// Copyright 2026 Seif Rashwan
#ifndef INCLUDE_WAITINGCUSTOMERQUEUE_H_
#define INCLUDE_WAITINGCUSTOMERQUEUE_H_

#include <cassert>
#include "QueueADT.h"

template <typename Type>
class WaitingCustomerQueue : public QueueADT<Type> {
   private:
    int max_queue_size_;
    int count_;
    int queue_front_;
    int queue_rear_;
    Type* queue_array_;

    void initialize() override;

   public:
    void enqueue(const Type& queue_elem) override;
    void dequeue() override;

    Type front() const override;
    Type back() const override;

    bool isEmpty() const override;
    bool isFull() const override;
    int size() const override;

    void incrementWaitingTimes();

    explicit WaitingCustomerQueue(int max_size = 100);
    ~WaitingCustomerQueue() override;
};

#endif  // INCLUDE_WAITINGCUSTOMERQUEUE_H_