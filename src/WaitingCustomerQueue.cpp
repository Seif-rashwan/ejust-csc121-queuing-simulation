#include "WaitingCustomerQueue.h"
#include <cassert>
#include "CustomerType.h"

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
    assert(!isEmpty());
    return queue_array_[queue_front_];
}

template <typename Type>
Type WaitingCustomerQueue<Type>::back() const {
    assert(!isEmpty());
    return queue_array_[queue_rear_];
}

template <typename Type>
void WaitingCustomerQueue<Type>::enqueue(const Type& queue_elem) {
    assert(!isFull());
    queue_rear_               = (queue_rear_ + 1) % max_queue_size_;
    queue_array_[queue_rear_] = queue_elem;
    count_++;
}

template <typename Type>
void WaitingCustomerQueue<Type>::dequeue() {
    assert(!isEmpty());
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

template class WaitingCustomerQueue<CustomerType>;
