#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <iostream>
#include <stdexcept>
#include <functional>

// ============================================================================
// GENERIC NODE FOR LINKED STRUCTURES
// ============================================================================
template <typename T>
struct Node {
    T data;
    Node<T>* next;

    Node(const T& val) : data(val), next(nullptr) {}
};

// ============================================================================
// CUSTOM STACK IMPLEMENTATION (LIFO - Last In, First Out)
// Used for:
// 1. Bottleneck Dead-End Turnout Spur (Vehicles pushed in reverse order)
// 2. Emergency Signal Preemption Stack (Nested signal state interrupts)
// 3. Controller Action Undo/Rollback History
// ============================================================================
template <typename T>
class CustomStack {
private:
    Node<T>* topNode;
    int count;

    void copyFrom(const CustomStack& other) {
        if (other.topNode == nullptr) {
            topNode = nullptr;
            count = 0;
            return;
        }

        // To preserve stack order during deep copy, we use a temporary array or reverse copy
        Node<T>* current = other.topNode;
        T* tempArray = new T[other.count];
        int idx = 0;
        while (current != nullptr) {
            tempArray[idx++] = current->data;
            current = current->next;
        }

        topNode = nullptr;
        count = 0;
        for (int i = idx - 1; i >= 0; --i) {
            push(tempArray[i]);
        }
        delete[] tempArray;
    }

public:
    CustomStack() : topNode(nullptr), count(0) {}

    // Copy constructor
    CustomStack(const CustomStack& other) : topNode(nullptr), count(0) {
        copyFrom(other);
    }

    // Copy assignment operator
    CustomStack& operator=(const CustomStack& other) {
        if (this != &other) {
            clear();
            copyFrom(other);
        }
        return *this;
    }

    // Destructor
    ~CustomStack() {
        clear();
    }

    // O(1) Push
    void push(const T& value) {
        Node<T>* newNode = new Node<T>(value);
        newNode->next = topNode;
        topNode = newNode;
        count++;
    }

    // O(1) Pop
    T pop() {
        if (isEmpty()) {
            throw std::underflow_error("Stack Underflow: Attempted to pop from an empty stack.");
        }
        Node<T>* temp = topNode;
        T poppedData = temp->data;
        topNode = topNode->next;
        delete temp;
        count--;
        return poppedData;
    }

    // O(1) Peek
    T peek() const {
        if (isEmpty()) {
            throw std::underflow_error("Stack Underflow: Attempted to peek into an empty stack.");
        }
        return topNode->data;
    }

    // O(1) Check if empty
    bool isEmpty() const {
        return topNode == nullptr;
    }

    // O(1) Get size
    int size() const {
        return count;
    }

    // O(N) Clear
    void clear() {
        while (!isEmpty()) {
            pop();
        }
    }

    // Traversal from Top to Bottom (for visual inspection)
    void forEach(std::function<void(const T&, int index)> func) const {
        Node<T>* curr = topNode;
        int idx = 0;
        while (curr != nullptr) {
            func(curr->data, idx++);
            curr = curr->next;
        }
    }
};

// ============================================================================
// CUSTOM QUEUE IMPLEMENTATION (FIFO - First In, First Out)
// Used for:
// 1. Directional Traffic Intersection Lanes (North, South, East, West)
// 2. Regular Inflow & Outflow dispatch
// ============================================================================
template <typename T>
class CustomQueue {
private:
    Node<T>* frontNode;
    Node<T>* rearNode;
    int count;

    void copyFrom(const CustomQueue& other) {
        frontNode = nullptr;
        rearNode = nullptr;
        count = 0;
        Node<T>* curr = other.frontNode;
        while (curr != nullptr) {
            enqueue(curr->data);
            curr = curr->next;
        }
    }

public:
    CustomQueue() : frontNode(nullptr), rearNode(nullptr), count(0) {}

    // Copy Constructor
    CustomQueue(const CustomQueue& other) : frontNode(nullptr), rearNode(nullptr), count(0) {
        copyFrom(other);
    }

    // Copy Assignment Operator
    CustomQueue& operator=(const CustomQueue& other) {
        if (this != &other) {
            clear();
            copyFrom(other);
        }
        return *this;
    }

    // Destructor
    ~CustomQueue() {
        clear();
    }

    // O(1) Enqueue (Insert at rear)
    void enqueue(const T& value) {
        Node<T>* newNode = new Node<T>(value);
        if (rearNode == nullptr) {
            frontNode = rearNode = newNode;
        } else {
            rearNode->next = newNode;
            rearNode = newNode;
        }
        count++;
    }

    // O(1) Dequeue (Remove from front)
    T dequeue() {
        if (isEmpty()) {
            throw std::underflow_error("Queue Underflow: Attempted to dequeue from an empty queue.");
        }
        Node<T>* temp = frontNode;
        T dequeuedData = temp->data;
        frontNode = frontNode->next;

        if (frontNode == nullptr) {
            rearNode = nullptr;
        }

        delete temp;
        count--;
        return dequeuedData;
    }

    // O(1) Peek at front
    T peek() const {
        if (isEmpty()) {
            throw std::underflow_error("Queue Underflow: Attempted to peek into an empty queue.");
        }
        return frontNode->data;
    }

    // O(1) Check if empty
    bool isEmpty() const {
        return frontNode == nullptr;
    }

    // O(1) Get size
    int size() const {
        return count;
    }

    // O(N) Clear
    void clear() {
        while (!isEmpty()) {
            dequeue();
        }
    }

    // Traversal from Front to Rear
    void forEach(std::function<void(const T&, int index)> func) const {
        Node<T>* curr = frontNode;
        int idx = 0;
        while (curr != nullptr) {
            func(curr->data, idx++);
            curr = curr->next;
        }
    }
};

// ============================================================================
// CUSTOM SINGLY LINKED LIST
// Used for:
// 1. Global Registry of Active Vehicles (Audit, Search, Real-Time Stats)
// 2. Incident & Violation Audit Log
// ============================================================================
template <typename T>
class CustomLinkedList {
private:
    Node<T>* head;
    int count;

public:
    CustomLinkedList() : head(nullptr), count(0) {}

    ~CustomLinkedList() {
        clear();
    }

    // O(1) Prepend
    void prepend(const T& val) {
        Node<T>* newNode = new Node<T>(val);
        newNode->next = head;
        head = newNode;
        count++;
    }

    // O(N) Append
    void append(const T& val) {
        Node<T>* newNode = new Node<T>(val);
        if (!head) {
            head = newNode;
        } else {
            Node<T>* curr = head;
            while (curr->next != nullptr) {
                curr = curr->next;
            }
            curr->next = newNode;
        }
        count++;
    }

    // O(N) Search with predicate
    Node<T>* find(std::function<bool(const T&)> predicate) const {
        Node<T>* curr = head;
        while (curr != nullptr) {
            if (predicate(curr->data)) {
                return curr;
            }
            curr = curr->next;
        }
        return nullptr;
    }

    // O(N) Remove by predicate
    bool removeIf(std::function<bool(const T&)> predicate) {
        if (!head) return false;

        if (predicate(head->data)) {
            Node<T>* temp = head;
            head = head->next;
            delete temp;
            count--;
            return true;
        }

        Node<T>* curr = head;
        while (curr->next != nullptr && !predicate(curr->next->data)) {
            curr = curr->next;
        }

        if (curr->next != nullptr) {
            Node<T>* temp = curr->next;
            curr->next = temp->next;
            delete temp;
            count--;
            return true;
        }
        return false;
    }

    int size() const {
        return count;
    }

    bool isEmpty() const {
        return head == nullptr;
    }

    void clear() {
        while (head != nullptr) {
            Node<T>* temp = head;
            head = head->next;
            delete temp;
        }
        count = 0;
    }

    void forEach(std::function<void(const T&, int index)> func) const {
        Node<T>* curr = head;
        int idx = 0;
        while (curr != nullptr) {
            func(curr->data, idx++);
            curr = curr->next;
        }
    }
};

#endif // DATA_STRUCTURES_H
