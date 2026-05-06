#include "WaitingCustomerQueue.h"
#include <cassert>
#include <iostream>

template <class Type>
bool WaitingCustomerQueue<Type>::isEmptyQueue() const {
    return (queueFront == nullptr);
}

template <class Type>
bool WaitingCustomerQueue<Type>::isFullQueue() const {
    return false; 
}

template <class Type>
void WaitingCustomerQueue<Type>::initializeQueue() {
    NodeType<Type>* temp;
    while (queueFront  != nullptr) {
        temp = queueFront ;
        queueFront  = queueFront ->link;
        delete temp;
    }
    queueRear = nullptr;
}

template <class Type>
Type WaitingCustomerQueue<Type>::front() const {
    assert(queueFront != nullptr);
    return queueFront->info;
}

template <class Type>
Type WaitingCustomerQueue<Type>::back() const {
    assert(queueRear != nullptr);
    return queueRear->info;
}

template <class Type>
void WaitingCustomerQueue<Type>::addQueue(const Type& newElement) {
    NodeType<Type>* newNode = new NodeType<Type>;
    newNode->info = queue_element;
    newNode->link = nullptr;
    
    if (queueFront == nullptr) {
        queueFront = newNode;
        queueRear = newNode;
    } else {
        queueRear->link = newNode;
        queueRear = newNode;
    }
}

template <class Type>
void WaitingCustomerQueue<Type>::deleteQueue(){
    if (!is_empty_queue()) {
        NodeType<Type>* temp = queueFront;
        queueFront = queueFront->link;
        delete temp;
        
        if (queueFront == nullptr) {
            queueRear = nullptr;
        }
    } else {
        std::cout << "ERROR: Cannot remove from empty queue." << std::endl;
    }
}

template <class Type>
void WaitingCustomerQueue<Type>::incrementWaitingTimes(){
    NodeType<Type>* current = queueFront;
    while (current != nullptr) {
        current->info.incrementWaitingTime();
        current = current->link;
    }
}

template <class Type>
WaitingCustomerQueue<Type>::WaitingCustomerQueue() {
    queueFront = nullptr;
    queueRear = nullptr;
}

template <class Type>
WaitingCustomerQueue<Type>::~WaitingCustomerQueue() {
    initializeQueue();
}

#include "CustomerType.h"
template class WaitingCustomerQueue<CustomerType>;
