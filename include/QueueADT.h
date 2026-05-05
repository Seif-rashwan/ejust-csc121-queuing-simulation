#ifndef INCLUDE_QUEUEADT_H_
#define INCLUDE_QUEUEADT_H_

/**
 * @class QueueADT
 * @brief Abstract base class for a queue.
 *
 * The `QueueADT` class defines the interface for a queue data structure.
 * Derived classes must implement all pure virtual methods to provide
 * functionality for queue operations such as enqueue, dequeue, and checking
 * if the queue is empty or full.
 *
 * @tparam Type The type of elements stored in the queue.
 */
template <typename Type>
class QueueADT {
   protected:
    /**
     * @brief Initializes the queue by setting it to an empty state.
     */
    virtual void initialize() = 0;

   public:
    /**
     * @brief Adds an element to the queue.
     * @param queueElement The element to be added to the queue.
     */
    virtual void queue(const Type& queueElement) = 0;

    /**
     * @brief Removes the front element from the queue.
     */
    virtual void dequeue() = 0;

    /**
     * @brief Checks if the queue is empty.
     * @return `true` if the queue is empty, `false` otherwise.
     */
    virtual bool isEmpty() const = 0;

    /**
     * @brief Checks if the queue is full.
     * @return `true` if the queue is full, `false` otherwise.
     * @throws std::exception If the queue is full, throws an exception.
     */
    virtual bool isFull() const = 0;

    /**
     * @brief Retrieves the front element of the queue.
     * @return The element at the front of the queue.
     * @throws std::exception If the queue is empty, throws an exception.
     */
    virtual Type front() const = 0;

    /**
     * @brief Retrieves the back element of the queue.
     * @return The element at the back of the queue.
     */
    virtual Type back() const = 0;

    virtual ~QueueADT()       = default;
};

#endif  // INCLUDE_QUEUEADT_H_
