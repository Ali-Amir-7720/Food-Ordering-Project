#pragma once
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include "LinkedList.h"
using namespace std;

template<typename T>
class HashTable {
private:
    static const int TABLE_SIZE = 10;
    LinkedList<pair<int, T>> table[TABLE_SIZE];

    int hashFunction(int key) const {
        return key % TABLE_SIZE;
    }

public:
    bool isEmpty() const {
        for (int i = 0; i < TABLE_SIZE; i++) {
            if (!table[i].isEmpty())
                return false;
        }
        return true;
    }
    void insertItem(int key, const T& val) {
        int index = hashFunction(key);
        Node<pair<int, T>>* temp = table[index].getHead();
        while (temp != nullptr) {
            if (temp->data.first == key) {
                temp->data.second = val;
                return;
            }
            temp = temp->next;
        }
        table[index].insertAtEnd(make_pair(key, val));
    }
    T* searchTable(int key) {
        int index = hashFunction(key);
        Node<pair<int, T>>* temp = table[index].getHead();

        while (temp != nullptr) {
            if (temp->data.first == key)
                return &temp->data.second;  
            temp = temp->next;
        }
        return nullptr; 
    }
    bool removeItem(int key) {
        int index = hashFunction(key);
        return table[index].removeByKey(key);
    }
    void printTable() const {
        for (int i = 0; i < TABLE_SIZE; i++) {
            cout << "Bucket " << i << ": ";
            table[i].print();
            cout << endl;
        }
    }
};

#endif
