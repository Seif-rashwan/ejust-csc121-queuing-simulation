#ifndef INCLUDE_WAITINGCUSTOMERQUEUE_H_
#define INCLUDE_WAITINGCUSTOMERQUEUE_H_

/**
 * @file WaitingCustomerQueue.h
 * @brief Declares the circular-array FIFO queue used by waiting customers.
 * @copyright Copyright 2026 Seif Rashwan
 */

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

    /**
     * @brief Resets circular-array indices and element count.
     */
    void initialize();

   public:
    static constexpr int DEFAULT_MAX_SIZE = 100;  ///< Default queue capacity.

    /**
     * @brief Constructs an empty queue with the requested capacity.
     * @param max_size Maximum number of elements; non-positive values use DEFAULT_MAX_SIZE.
     */
    explicit WaitingCustomerQueue(int max_size = DEFAULT_MAX_SIZE);

    /**
     * @brief Releases queue storage.
     */
    ~WaitingCustomerQueue() override;
    WaitingCustomerQueue& operator=(const WaitingCustomerQueue&) = delete;
    WaitingCustomerQueue(const WaitingCustomerQueue&)            = delete;

    /**
     * @brief Adds an element to the rear of the queue.
     * @param queue_elem Element to enqueue.
     * @throws std::overflow_error if the queue is full.
     */
    void enqueue(const Type& queue_elem) override;

    /**
     * @brief Removes the element at the front of the queue.
     * @throws std::underflow_error if the queue is empty.
     */
    void dequeue() override;

    /**
     * @brief Removes all elements from the queue.
     */
    void clear() {
        initialize();
    }

    /**
     * @brief Retrieves the front element.
     * @return Copy of the front element.
     * @throws std::underflow_error if the queue is empty.
     */
    Type front() const override;

    /**
     * @brief Retrieves the rear element.
     * @return Copy of the rear element.
     * @throws std::underflow_error if the queue is empty.
     */
    Type back() const override;

    /**
     * @brief Checks whether the queue contains no elements.
     * @return true when empty.
     */
    bool isEmpty() const override;

    /**
     * @brief Checks whether the queue has reached capacity.
     * @return true when full.
     */
    bool isFull() const override;

    /**
     * @brief Gets the current queue size.
     * @return Number of queued elements.
     */
    int size() const override;

    /**
     * @brief Increments waiting time for every queued customer-like element.
     */
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
