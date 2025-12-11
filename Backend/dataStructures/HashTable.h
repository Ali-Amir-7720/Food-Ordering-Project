#pragma once
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include "LinkedList.h"
using namespace std;

template<typename T>
class HashTable {
private:
    int TABLE_SIZE;  // Make it non-const so it can be set in constructor
    LinkedList<pair<int, T>>* table;  // Change to pointer for dynamic allocation
    
    int hashFunction(int key) const {
        return key % TABLE_SIZE;
    }

public:
    // Default constructor with size 10
    HashTable() : TABLE_SIZE(10) {
        table = new LinkedList<pair<int, T>>[TABLE_SIZE];
    }
    
    // Constructor with custom size
    HashTable(int size) : TABLE_SIZE(size) {
        table = new LinkedList<pair<int, T>>[TABLE_SIZE];
    }
    
    // Destructor to clean up memory
    ~HashTable() {
        delete[] table;
    }
    
    // Copy constructor (important for rule of three)
    HashTable(const HashTable& other) : TABLE_SIZE(other.TABLE_SIZE) {
        table = new LinkedList<pair<int, T>>[TABLE_SIZE];
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i] = other.table[i];
        }
    }
    
    // Assignment operator
    HashTable& operator=(const HashTable& other) {
        if (this != &other) {
            delete[] table;
            TABLE_SIZE = other.TABLE_SIZE;
            table = new LinkedList<pair<int, T>>[TABLE_SIZE];
            for (int i = 0; i < TABLE_SIZE; i++) {
                table[i] = other.table[i];
            }
        }
        return *this;
    }
    
    bool isEmpty() const {
        for (int i = 0; i < TABLE_SIZE; i++) {
            if (!table[i].isEmpty())
                return false;
        }
        return true;
    }
    
    void insertItem(int key, const T& val) {
        int index = hashFunction(key);
        // Check if key already exists
        auto* node = table[index].getHead();
        while (node != nullptr) {
            if (node->data.first == key) {
                node->data.second = val;
                return;
            }
            node = node->next;
        }
        table[index].insertAtEnd(make_pair(key, val));
    }
    
    T* searchTable(int key) {
        int index = hashFunction(key);
        auto* node = table[index].getHead();
        
        while (node != nullptr) {
            if (node->data.first == key)
                return &(node->data.second);
            node = node->next;
        }
        return nullptr;
    }
    
    const T* searchTable(int key) const {
        int index = hashFunction(key);
        auto* node = table[index].getHead();
        
        while (node != nullptr) {
            if (node->data.first == key)
                return &(node->data.second);
            node = node->next;
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
    
    void traverse(function<void(int, const T&)> func) const {
        for (int i = 0; i < TABLE_SIZE; i++) {
            auto* node = table[i].getHead();
            while (node != nullptr) {
                func(node->data.first, node->data.second);
                node = node->next;
            }
        }
    }
    
    void traverse(function<void(int, T&)> func) {
        for (int i = 0; i < TABLE_SIZE; i++) {
            auto* node = table[i].getHead();
            while (node != nullptr) {
                func(node->data.first, node->data.second);
                node = node->next;
            }
        }
    }
    
    int getSize() const {
        int count = 0;
        for (int i = 0; i < TABLE_SIZE; i++) {
            auto* node = table[i].getHead();
            while (node != nullptr) {
                count++;
                node = node->next;
            }
        }
        return count;
    }
    
    // Get the table size (capacity)
    int getCapacity() const {
        return TABLE_SIZE;
    }
    
    // Check if key exists
    bool containsKey(int key) const {
        return searchTable(key) != nullptr;
    }
    
    // Clear all items
    void clear() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i].clear();
        }
    }
};

#endif