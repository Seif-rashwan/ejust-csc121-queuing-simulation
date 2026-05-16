// Copyright 2026 Seif Rashwan
#ifndef INCLUDE_WAITINGCUSTOMERQUEUE_H_
#define INCLUDE_WAITINGCUSTOMERQUEUE_H_

#include <stdexcept>
#include "QueueADT.h"

/**
 * @class WaitingCustomerQueue
 * @brief Circular array implementation of a FIFO queue for the simulation.
 * @tparam Type The type of elements stored in the queue.
 */
template <typename Type>
class WaitingCustomerQueue : public QueueADT<Type> {
   private:
    int max_queue_size_;
    int count_;
    int queue_front_;
    int queue_rear_;
    Type* queue_array_;

    void initialize();

   public:
    static const int DEFAULT_MAX_SIZE = 100;

    explicit WaitingCustomerQueue(int max_size = DEFAULT_MAX_SIZE);
    ~WaitingCustomerQueue() override;

    WaitingCustomerQueue& operator=(const WaitingCustomerQueue&) = delete;
    WaitingCustomerQueue(const WaitingCustomerQueue&)            = delete;

    void enqueue(const Type& queue_elem) override;
    void dequeue() override;

    Type front() const override;
    Type back() const override;

    bool isEmpty() const override;
    bool isFull() const override;
    int size() const override;

    void incrementWaitingTimes();

    /**
     * @brief Retrieves an element at a specific logical position in the queue.
     * @details Implemented in header to resolve compilation visibility issues.
     * @param index 0-based logical index (0 is front).
     * @return Copy of the element.
     */
    Type getAt(int index) const {
        if (index < 0 || index >= count_) {
            throw std::out_of_range("WaitingCustomerQueue::getAt(): index out of range");
        }

        int real_index = (queue_front_ + index) % max_queue_size_;
        return queue_array_[real_index];
    }
};

#endif  // INCLUDE_WAITINGCUSTOMERQUEUE_H_