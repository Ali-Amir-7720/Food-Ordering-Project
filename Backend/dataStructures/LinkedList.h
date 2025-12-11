#pragma once
#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <functional>
#include <iostream>
using namespace std;

template<typename T>
struct Node {
    T data;
    Node* next;
    Node(const T& d) : data(d), next(nullptr) {}
};

template<typename T>
class LinkedList {
private:
    Node<T>* head;
    Node<T>* tail;
    int size;

public:
    LinkedList() : head(nullptr), tail(nullptr), size(0) {}
    
    ~LinkedList() {
        clear();
    }

    bool isEmpty() const {
        return head == nullptr;
    }

    Node<T>* getHead() const {
        return head;
    }
    
    Node<T>* getTail() const {
        return tail;
    }
    
    int getSize() const {
        return size;
    }

    void insertAtEnd(const T& val) {
        Node<T>* newNode = new Node<T>(val);
        if (head == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        size++;
    }
    
    void push_back(const T& val) {
        insertAtEnd(val);
    }
    
    T removeFront() {
        if (head == nullptr) {
            throw std::runtime_error("Cannot remove from empty list");
        }
        
        Node<T>* temp = head;
        T data = head->data;
        head = head->next;
        
        // If list becomes empty, update tail
        if (head == nullptr) {
            tail = nullptr;
        }
        
        delete temp;
        size--;
        return data;
    }
    
    void insertAtFront(const T& val) {
        Node<T>* newNode = new Node<T>(val);
        newNode->next = head;
        head = newNode;
        if (tail == nullptr) {
            tail = newNode;
        }
        size++;
    }

    // bool removeByKey(int key) {
    //     if (!head) return false;

    //     // Check if data has a 'first' member (for pairs)
    //     if constexpr (requires { head->data.first; }) {
    //         if (head->data.first == key) {
    //             Node<T>* del = head;
    //             head = head->next;
    //             if (head == nullptr) tail = nullptr;
    //             delete del;
    //             size--;
    //             return true;
    //         }

    //         Node<T>* temp = head;
    //         while (temp->next != nullptr && temp->next->data.first != key)
    //             temp = temp->next;

    //         if (temp->next == nullptr)
    //             return false;

    //         Node<T>* del = temp->next;
    //         temp->next = temp->next->next;
    //         if (del == tail) tail = temp;
    //         delete del;
    //         size--;
    //         return true;
    //     }
    //     return false;
    // }
    // In LinkedList.h, replace the removeByKey function:

bool removeByKey(int key) {
    if (!head) return false;
    
    Node<T>* temp = head;
    Node<T>* prev = nullptr;
    
    while (temp != nullptr) {
        // Use the helper function for comparison
        if (compareDataWithKey(temp->data, key)) {
            if (prev == nullptr) {
                // Removing head
                head = head->next;
                if (head == nullptr) tail = nullptr;
            } else {
                prev->next = temp->next;
                if (temp == tail) tail = prev;
            }
            delete temp;
            size--;
            return true;
        }
        prev = temp;
        temp = temp->next;
    }
    
    return false;
}
    
    bool remove(const T& val) {
        if (!head) return false;
        
        if (head->data == val) {
            Node<T>* del = head;
            head = head->next;
            if (head == nullptr) tail = nullptr;
            delete del;
            size--;
            return true;
        }
        
        Node<T>* temp = head;
        while (temp->next != nullptr && temp->next->data != val)
            temp = temp->next;
            
        if (temp->next == nullptr) return false;
        
        Node<T>* del = temp->next;
        temp->next = temp->next->next;
        if (del == tail) tail = temp;
        delete del;
        size--;
        return true;
    }

    void print() const {
        Node<T>* temp = head;
        while (temp != nullptr) {
            cout << temp->data << " ";
            temp = temp->next;
        }
        cout << endl;
    }
    
    void traverse(function<void(const T&)> func) const {
        Node<T>* temp = head;
        while (temp != nullptr) {
            func(temp->data);
            temp = temp->next;
        }
    }

    void clear() {
        while (head != nullptr) {
            Node<T>* temp = head;
            head = head->next;
            delete temp;
        }
        head = tail = nullptr;
        size = 0;
    }
    
    // Find an element
    T* find(function<bool(const T&)> predicate) {
        Node<T>* temp = head;
        while (temp != nullptr) {
            if (predicate(temp->data)) {
                return &(temp->data);
            }
            temp = temp->next;
        }
        return nullptr;
    }
    
    // Iterator support
    class Iterator {
    private:
        Node<T>* current;
    public:
        Iterator(Node<T>* node) : current(node) {}
        
        T& operator*() { return current->data; }
        Iterator& operator++() { 
            if (current) current = current->next; 
            return *this; 
        }
        bool operator!=(const Iterator& other) const { 
            return current != other.current; 
        }
    };
    
    Iterator begin() { return Iterator(head); }
    Iterator end() { return Iterator(nullptr); }
    
    // Add the friend function declaration (correct syntax for template friend)
    friend ostream& operator<<(ostream& os, const LinkedList<T>& list) {
        if (list.isEmpty()) {
            os << "Empty List";
            return os;
        }
        
        Node<T>* current = list.head;
        os << "[";
        while (current != nullptr) {
            os << current->data;
            if (current->next != nullptr) {
                os << ", ";
            }
            current = current->next;
        }
        os << "]";
        return os;
    }
};

#endif
// Implementation of helper functions
template<typename T>
bool compareDataWithKey(const T& data, int key) {
    return data == key;
}

template<typename U>
bool compareDataWithKey(const std::pair<int, U>& data, int key) {
    return data.first == key;
}