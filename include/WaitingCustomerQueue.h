#ifndef INCLUDE_WAITINGCUSTOMERQUEUE_H_
#define INCLUDE_WAITINGCUSTOMERQUEUE_H_

#include <queue>
#include <stdexcept>
#include "CustomerType.h"

// Copyright 2026 E-JUST CSC 121 Project

// Extends std::queue with a helper to increment every customer's waiting time
class WaitingCustomerQueue {
public:
    bool empty() const { return q.empty(); }
    int size() const { return static_cast<int>(q.size()); }

    void enqueue(const CustomerType& c) { q.push(c); }

    CustomerType dequeue() {
        if (q.empty()) throw std::runtime_error("Queue is empty");
        CustomerType front = q.front();
        q.pop();
        return front;
    }

    const CustomerType& front() const {
        if (q.empty()) throw std::runtime_error("Queue is empty");
        return q.front();
    }

    // Called every clock tick: rebuild queue with incremented wait times
    void incrementWaitingTimes() {
        std::queue<CustomerType> temp;
        while (!q.empty()) {
            CustomerType c = q.front();
            q.pop();
            c.incrementWaitingTime();
            temp.push(c);
        }
        q = temp;
    }

private:
    std::queue<CustomerType> q;
};

#endif  // INCLUDE_WAITINGCUSTOMERQUEUE_H_

