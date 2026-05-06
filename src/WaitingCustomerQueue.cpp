#include "WaitingCustomerQueue.h"
#include <cassert>
#include <iostream>

template <class Type>
bool WaitingCustomerQueue<Type>::is_empty_queue() const {
    return (queue_front == nullptr);
}

template <class Type>
bool WaitingCustomerQueue<Type>::is_full_queue() const {
    return false; 
}

template <class Type>
void WaitingCustomerQueue<Type>::initialize_queue() {
    NodeType<Type>* temp;
    while (queue_front != nullptr) {
        temp = queue_front;
        queue_front = queue_front->link;
        delete temp;
    }
    queue_rear = nullptr;
}

template <class Type>
Type WaitingCustomerQueue<Type>::front() const {
    assert(queue_front != nullptr);
    return queue_front->info;
}

template <class Type>
Type WaitingCustomerQueue<Type>::back() const {
    assert(queue_rear != nullptr);
    return queue_rear->info;
}

template <class Type>
void WaitingCustomerQueue<Type>::add_queue(const Type& queue_element) {
    NodeType<Type>* new_node = new NodeType<Type>;
    new_node->info = queue_element;
    new_node->link = nullptr;
    
    if (queue_front == nullptr) {
        queue_front = new_node;
        queue_rear = new_node;
    } else {
        queue_rear->link = new_node;
        queue_rear = new_node;
    }
}

template <class Type>
void WaitingCustomerQueue<Type>::delete_queue() {
    if (!is_empty_queue()) {
        NodeType<Type>* temp = queue_front;
        queue_front = queue_front->link;
        delete temp;
        
        if (queue_front == nullptr) {
            queue_rear = nullptr;
        }
    } else {
        std::cout << "ERROR: Cannot remove from empty queue." << std::endl;
    }
}

template <class Type>
void WaitingCustomerQueue<Type>::increment_all_waiting_times() {
    NodeType<Type>* current = queue_front;
    while (current != nullptr) {
        current->info.increment_waiting_time();
        current = current->link;
    }
}

template <class Type>
WaitingCustomerQueue<Type>::WaitingCustomerQueue() {
    queue_front = nullptr;
    queue_rear = nullptr;
}

template <class Type>
WaitingCustomerQueue<Type>::~WaitingCustomerQueue() {
    initialize_queue();
}

#include "CustomerType.h"
template class WaitingCustomerQueue<CustomerType>;
