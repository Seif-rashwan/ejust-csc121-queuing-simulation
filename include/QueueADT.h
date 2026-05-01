#ifndef QUEUEADT_H
#define QUEUEADT_H

template <class Type>
class QueueADT {
public:
    // يتحقق إذا كان الطابور فارغ
    virtual bool isEmptyQueue() const = 0;

    // يتحقق إذا كان الطابور ممتلئ
    virtual bool isFullQueue() const = 0;

    // تهيئة الطابور (تفريغه)
    virtual void initializeQueue() = 0;

    // إرجاع أول عنصر في الطابور
    virtual Type front() const = 0;

    // إرجاع آخر عنصر في الطابور
    virtual Type back() const = 0;

    // إضافة عنصر في نهاية الطابور
    virtual void addQueue(const Type& queueElement) = 0;

    // حذف عنصر من بداية الطابور
    virtual void deleteQueue() = 0;

    // Destructor
    virtual ~QueueADT() {}
};

#endif