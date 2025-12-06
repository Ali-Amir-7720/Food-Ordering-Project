#pragma once
#ifndef LINKEDLIST_H
#define LINKEDLIST_H
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

public:
    LinkedList() : head(nullptr) {}

    bool isEmpty() const {
        return head == nullptr;
    }

    Node<T>* getHead() const {
        return head;
    }

    void insertAtEnd(const T& val) {
        Node<T>* newNode = new Node<T>(val);
        if (head == nullptr) {
            head = newNode;
            return;
        }
        Node<T>* temp = head;
        while (temp->next != nullptr)
            temp = temp->next;
        temp->next = newNode;
    }

    bool removeByKey(int key) {
        if (!head) return false;

        if (head->data.first == key) {
            Node<T>* del = head;
            head = head->next;
            delete del;
            return true;
        }

        Node<T>* temp = head;
        while (temp->next != nullptr && temp->next->data.first != key)
            temp = temp->next;

        if (temp->next == nullptr)
            return false;

        Node<T>* del = temp->next;
        temp->next = temp->next->next;
        delete del;
        return true;
    }

    void print() const {
        Node<T>* temp = head;
        while (temp != nullptr) {
            cout << "[" << temp->data.first << ": " << temp->data.second << "] ";
            temp = temp->next;
        }
    }
};

#endif
