#pragma once
#ifndef QUEUE_H
#define QUEUE_H
#include <stdexcept>
using namespace std;

template<typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& d) : data(d), next(nullptr) {}
    };

    Node* frontPtr;
    Node* rearPtr;
    int count;

public:
    Queue() : frontPtr(nullptr), rearPtr(nullptr), count(0) {}

    ~Queue() {
        while (!isEmpty()) {
            dequeue();
        }
    }

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }

    void enqueue(const T& val) {
        Node* newNode = new Node(val);
        if (rearPtr == nullptr) {
            frontPtr = rearPtr = newNode;
        }
        else {
            rearPtr->next = newNode;
            rearPtr = newNode;
        }
        count++;
    }

    T dequeue() {
        if (isEmpty()) {
            throw out_of_range("Queue empty");
        }
        Node* temp = frontPtr;
        T val = temp->data;
        frontPtr = frontPtr->next;
        if (!frontPtr) {
            rearPtr = nullptr;
        }
        delete temp;
        count--;
        return val;
    }

    T front() const {
        if (isEmpty()) {
            throw out_of_range("Queue empty");
        }
        return frontPtr->data;
    }
    
    T peek() const {
        return front();
    }
     void traverse(function<void(const T&)> func) const {
        Node* current = front;
        while (current != nullptr) {
            func(current->data);
            current = current->next;
        }
    }
    
    void traverse(function<void(T&)> func) {
        Node* current = front;
        while (current != nullptr) {
            func(current->data);
            current = current->next;
        }
    }
};

#endif