#pragma once
#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include <iostream>
#include <vector>
#include <functional>
#include <stdexcept>
using namespace std;

template<typename T>
class PriorityQueue {
private:
    vector<pair<T, int>> heap; 
    
    void heapifyUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (heap[parent].second > heap[index].second) {
                swap(heap[parent], heap[index]);
                index = parent;
            } else {
                break;
            }
        }
    }
    
    void heapifyDown(int index) {
        int size = heap.size();
        while (true) {
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int smallest = index;
            
            if (left < size && heap[left].second < heap[smallest].second) {
                smallest = left;
            }
            if (right < size && heap[right].second < heap[smallest].second) {
                smallest = right;
            }
            
            if (smallest != index) {
                swap(heap[index], heap[smallest]);
                index = smallest;
            } else {
                break;
            }
        }
    }

public:
    PriorityQueue() {}
    
    // Add element with priority (lower number = higher priority)
    void enqueue(const T& item, int priority) {
        heap.emplace_back(item, priority);
        heapifyUp(heap.size() - 1);
    }
    
    // Remove and return the highest priority element
    T dequeue() {
        if (isEmpty()) {
            throw runtime_error("Priority queue is empty");
        }
        
        T item = heap[0].first;
        heap[0] = heap.back();
        heap.pop_back();
        
        if (!isEmpty()) {
            heapifyDown(0);
        }
        
        return item;
    }
    
    // Peek at the highest priority element (non-const version)
    T& peek() {
        if (isEmpty()) {
            throw runtime_error("Priority queue is empty");
        }
        return heap[0].first;
    }
    
    // Peek at the highest priority element (const version)
    const T& peek() const {
        if (isEmpty()) {
            throw runtime_error("Priority queue is empty");
        }
        return heap[0].first;
    }
    
    // Get the priority of the top element
    int peekPriority() const {
        if (isEmpty()) {
            throw runtime_error("Priority queue is empty");
        }
        return heap[0].second;
    }
    
    // Check if empty
    bool isEmpty() const {
        return heap.empty();
    }
    
    // Get size
    int size() const {
        return heap.size();
    }
    
    // Clear all elements
    void clear() {
        heap.clear();
    }
    
    // Check if element exists
    bool contains(const T& item) const {
        for (const auto& pair : heap) {
            if (pair.first == item) {
                return true;
            }
        }
        return false;
    }
    
    // Update priority of an existing element
    bool updatePriority(const T& item, int newPriority) {
        for (size_t i = 0; i < heap.size(); i++) {
            if (heap[i].first == item) {
                int oldPriority = heap[i].second;
                heap[i].second = newPriority;
                
                // Re-heapify based on priority change
                if (newPriority < oldPriority) {
                    heapifyUp(i);
                } else if (newPriority > oldPriority) {
                    heapifyDown(i);
                }
                return true;
            }
        }
        return false; // Item not found
    }
    
    // Get priority of a specific element
    int getPriority(const T& item) const {
        for (const auto& pair : heap) {
            if (pair.first == item) {
                return pair.second;
            }
        }
        throw runtime_error("Item not found in priority queue");
    }
    
    // Remove a specific element
    bool remove(const T& item) {
        for (size_t i = 0; i < heap.size(); i++) {
            if (heap[i].first == item) {
                // Move last element to this position
                heap[i] = heap.back();
                heap.pop_back();
                
                // Re-heapify if needed
                if (i < heap.size()) {
                    // Check if we need to move up or down
                    if (i > 0 && heap[i].second < heap[(i-1)/2].second) {
                        heapifyUp(i);
                    } else {
                        heapifyDown(i);
                    }
                }
                return true;
            }
        }
        return false;
    }
    
    // Print the priority queue (for debugging)
    void print() const {
        cout << "Priority Queue (element, priority):\n";
        for (size_t i = 0; i < heap.size(); i++) {
            cout << "  [" << i << "]: ";
            // Try to print the element - works for types with operator<<
            cout << "Element at priority " << heap[i].second;
            cout << endl;
        }
    }
    
    // Get all elements sorted by priority (lowest priority number first)
    vector<T> getAllElements() const {
        vector<T> elements;
        // Create a copy to work with
        PriorityQueue<T> copy = *this;
        
        while (!copy.isEmpty()) {
            elements.push_back(copy.dequeue());
        }
        
        return elements;
    }
    
    // Check if the heap property is maintained (for testing)
    bool isHeapValid() const {
        for (size_t i = 0; i < heap.size(); i++) {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            
            if (left < heap.size() && heap[left].second < heap[i].second) {
                return false;
            }
            if (right < heap.size() && heap[right].second < heap[i].second) {
                return false;
            }
        }
        return true;
    }
    
    // Iterator support
    class Iterator {
    private:
        const PriorityQueue* pq;
        size_t index;
        
    public:
        Iterator(const PriorityQueue* _pq, size_t _index) 
            : pq(_pq), index(_index) {}
        
        bool operator!=(const Iterator& other) const {
            return index != other.index;
        }
        
        Iterator& operator++() {
            index++;
            return *this;
        }
        
        const T& operator*() const {
            return pq->heap[index].first;
        }
        
        int priority() const {
            return pq->heap[index].second;
        }
    };
    
    Iterator begin() const {
        return Iterator(this, 0);
    }
    
    Iterator end() const {
        return Iterator(this, heap.size());
    }
};

#endif