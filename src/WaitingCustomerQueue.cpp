#include <iostream>
#include <cassert>
#include "waitingCustomerQueue.h"

using namespace std;

// 1. التحقق لو الطابور فاضي
template <class Type>
bool waitingCustomerQueue<Type>::isEmptyQueue() const {
    return (queueFront == nullptr);
}

// 2. التحقق لو الطابور مليان (في الـ Linked List عمره ما بيكون مليان)
template <class Type>
bool waitingCustomerQueue<Type>::isFullQueue() const {
    return false;
}

// 3. تهيئة الطابور (مسح كل العناصر اللي فيه)
template <class Type>
void waitingCustomerQueue<Type>::initializeQueue() {
    nodeType<Type> *temp;
    while (queueFront != nullptr) {
        temp = queueFront;
        queueFront = queueFront->link;
        delete temp;
    }
    queueRear = nullptr;
}

// 4. إرجاع أول عنصر
template <class Type>
Type waitingCustomerQueue<Type>::front() const {
    assert(queueFront != nullptr); // التأكد إن الطابور مش فاضي
    return queueFront->info;
}

// 5. إرجاع آخر عنصر
template <class Type>
Type waitingCustomerQueue<Type>::back() const {
    assert(queueRear != nullptr);
    return queueRear->info;
}

// 6. إضافة عنصر جديد للطابور (عميل جديد)
template <class Type>
void waitingCustomerQueue<Type>::addQueue(const Type& newElement) {
    nodeType<Type> *newNode;
    newNode = new nodeType<Type>;
    newNode->info = newElement;
    newNode->link = nullptr;

    if (queueFront == nullptr) { // لو الطابور فاضي
        queueFront = newNode;
        queueRear = newNode;
    } else { // لو فيه ناس واقفة
        queueRear->link = newNode;
        queueRear = queueRear->link;
    }
}

// 7. حذف عنصر من الطابور (عميل دخل للسيرفر)
template <class Type>
void waitingCustomerQueue<Type>::deleteQueue() {
    nodeType<Type> *temp;
    if (!isEmptyQueue()) {
        temp = queueFront;
        queueFront = queueFront->link;
        delete temp;

        if (queueFront == nullptr) // لو مسحنا آخر عنصر
            queueRear = nullptr;
    } else {
        cout << "Cannot remove from an empty queue." << endl;
    }
}

// 8. الدالة الخاصة: تحديث وقت الانتظار لكل العملاء في الطابور
template <class Type>
void waitingCustomerQueue<Type>::updateWaitingTimes() {
    nodeType<Type> *current = queueFront;
    while (current != nullptr) {
        // بننادي على دالة زيادة وقت الانتظار اللي جوه كلاس العميل
        current->info.incrementWaitingTime(); 
        current = current->link;
    }
}

// 9. Constructor
template <class Type>
waitingCustomerQueue<Type>::waitingCustomerQueue() {
    queueFront = nullptr;
    queueRear = nullptr;
}

// 10. Destructor
template <class Type>
waitingCustomerQueue<Type>::~waitingCustomerQueue() {
    initializeQueue();
}

// ---------------------------------------------------------
// Explicit Template Instantiation
// عشان الـ Linker بتاع الـ C++ ميضربش error وإحنا بنبني المشروع
#include "customerType.h"
template class waitingCustomerQueue<customerType>;