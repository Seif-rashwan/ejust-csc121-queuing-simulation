/**
 * @file WaitingCustomerQueue.cpp
 * @brief Implements the circular-array waiting queue template.
 */

#include <stdexcept>

#include "CustomerType.h"
#include "WaitingCustomerQueue.h"

template <typename Type>
WaitingCustomerQueue<Type>::WaitingCustomerQueue(int max_size) {
    max_queue_size_ = (max_size > 0) ? max_size : DEFAULT_MAX_SIZE;
    queue_array_    = new Type[max_queue_size_];
    initialize();
}

template <typename Type>
WaitingCustomerQueue<Type>::~WaitingCustomerQueue() {
    delete[] queue_array_;
}

template <typename Type>
void WaitingCustomerQueue<Type>::initialize() {
    queue_front_ = 0;
    queue_rear_  = max_queue_size_ - 1;
    count_       = 0;
}

template <typename Type>
bool WaitingCustomerQueue<Type>::isEmpty() const {
    return (count_ == 0);
}

template <typename Type>
bool WaitingCustomerQueue<Type>::isFull() const {
    return (count_ == max_queue_size_);
}

template <typename Type>
int WaitingCustomerQueue<Type>::size() const {
    return count_;
}

template <typename Type>
Type WaitingCustomerQueue<Type>::front() const {
    if (isEmpty()) {
        throw std::underflow_error("WaitingCustomerQueue::front(): queue is empty");
    }
    return queue_array_[queue_front_];
}

template <typename Type>
Type WaitingCustomerQueue<Type>::back() const {
    if (isEmpty()) {
        throw std::underflow_error("WaitingCustomerQueue::back(): queue is empty");
    }
    return queue_array_[queue_rear_];
}

template <typename Type>
void WaitingCustomerQueue<Type>::enqueue(const Type& queue_elem) {
    if (isFull()) {
        throw std::overflow_error("WaitingCustomerQueue::enqueue(): queue is full");
    }
    queue_rear_               = (queue_rear_ + 1) % max_queue_size_;
    queue_array_[queue_rear_] = queue_elem;
    count_++;
}

template <typename Type>
void WaitingCustomerQueue<Type>::dequeue() {
    if (isEmpty()) {
        throw std::underflow_error("WaitingCustomerQueue::dequeue(): queue is empty");
    }
    queue_front_ = (queue_front_ + 1) % max_queue_size_;
    count_--;
}

template <typename Type>
void WaitingCustomerQueue<Type>::incrementWaitingTimes() {
    int index = queue_front_;
    for (int i = 0; i < count_; i++) {
        queue_array_[index].incrementWaitingTime();
        index = (index + 1) % max_queue_size_;
    }
}

// Explicit instantiation for the type used by the simulation.
template class WaitingCustomerQueue<CustomerType>;
