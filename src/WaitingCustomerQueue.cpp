#include "WaitingCustomerQueue.h"
#include <stdexcept>

template <typename Type>
bool WaitingCustomerQueue<Type>::isEmpty() const {
    return (front_ == nullptr);
}

template <typename Type>
void WaitingCustomerQueue<Type>::initialize() {
    while (front_ != nullptr) {
        LinkedNode<Type>* temp = front_;
        front_                 = front_->next;
        delete temp;
    }

    rear_ = nullptr;
}
// incrementWaitingTime

template <typename Type>
Type WaitingCustomerQueue<Type>::front() const {
    if (front_ == nullptr) {
        throw std::runtime_error("ERROR: Cannot access front of empty queue.");
    }

    return front_->data;
}

template <typename Type>
Type WaitingCustomerQueue<Type>::back() const {
    if (rear_ == nullptr) {
        throw std::runtime_error("ERROR: Cannot access back of empty queue.");
    }

    return rear_->data;
}

template <typename Type>
void WaitingCustomerQueue<Type>::enqueue(const Type& customer) {
    auto* new_node = new LinkedNode<Type>;
    new_node->data = customer;
    new_node->next = nullptr;

    if (front_ == nullptr) {
        front_ = new_node;
        rear_  = new_node;
    } else {
        rear_->next = new_node;
        rear_       = new_node;
    }
}

template <typename Type>
void WaitingCustomerQueue<Type>::dequeue() {
    if (!isEmpty()) {
        LinkedNode<Type>* temp = front_;
        front_                 = front_->next;
        delete temp;

        if (front_ == nullptr) {
            rear_ = nullptr;
        }
    } else {
        throw std::runtime_error("ERROR: Cannot remove from empty queue.");
    }
}

template <typename Type>
void WaitingCustomerQueue<Type>::incrementWaitingTimes(const double& inc_amount = 1.0) {
    LinkedNode<Type>* current = front_;
    while (current != nullptr) {
        current->data.incrementWaitingTime(inc_amount);
        current = current->next;
    }
}

template <typename Type>
WaitingCustomerQueue<Type>::WaitingCustomerQueue() {
    front_ = nullptr;
    rear_  = nullptr;
}

template <typename Type>
WaitingCustomerQueue<Type>::~WaitingCustomerQueue() {
    initialize();
}

#include "CustomerType.h"
template class WaitingCustomerQueue<CustomerType>;