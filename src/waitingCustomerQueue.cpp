#ifndef WAITING_CUSTOMER_QUEUE_H
#define WAITING_CUSTOMER_QUEUE_H

#include "customerType.h"
#include <queue>
#include <stdexcept>

// Extends std::queue with a helper to increment every customer's waiting time
class waitingCustomerQueue {
private:
    std::queue<customerType> q;

public:
    bool empty() const { return q.empty(); }
    int  size()  const { return static_cast<int>(q.size()); }

    void enqueue(const customerType& c) { q.push(c); }

    customerType dequeue() {
        if (q.empty()) throw std::runtime_error("Queue is empty");
        customerType front = q.front();
        q.pop();
        return front;
    }

    const customerType& front() const {
        if (q.empty()) throw std::runtime_error("Queue is empty");
        return q.front();
    }

    // Called every clock tick: rebuild queue with incremented wait times
    void incrementWaitingTimes() {
        std::queue<customerType> temp;
        while (!q.empty()) {
            customerType c = q.front(); q.pop();
            c.incrementWaitingTime();
            temp.push(c);
        }
        q = temp;
    }
};

#endif
