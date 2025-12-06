#pragma once
#pragma once
#ifndef QUEUE_H
#define QUEUE_H

template<typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& d) : data(d), next(nullptr) {}
    };

    Node* front;
    Node* rear;
    int count;

public:
    Queue() : front(nullptr), rear(nullptr), count(0) {}

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }

    void enqueue(const T& val) {
        Node* newNode = new Node(val);
        if (rear == nullptr) {
            front = rear = newNode;
        }
        else {
            rear->next = newNode;
            rear = newNode;
        }
        count++;
    }

    T dequeue() {
        if (isEmpty()) {
            throw out_of_range("Queue empty");
        }
        Node* temp = front;
        T val = temp->data;
        front = front->next;
        if (!front) {
            rear = nullptr;
        }
        delete temp;
        count--;
        return val;
    }

    T front() const {
        if (isEmpty()) {
            throw std::out_of_range("Queue empty");
        }
        return front->data;
    }
};

#endif

