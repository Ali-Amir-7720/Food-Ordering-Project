#pragma once
#pragma once
#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H
#include <vector>
#include <stdexcept>
using namespace std;
template<typename T>
class PriorityQueue {
private:
    struct Node {
        T data;
        int priority;
        Node(const T& d, int p) : data(d), priority(p) {}
    };

    vector<Node> heap;

    int parent(int i) { return (i - 1) / 2; }
    int left(int i) { return 2 * i + 1; }
    int right(int i) { return 2 * i + 2; }

    void heapifyUp(int index) {
        while (index != 0 && heap[parent(index)].priority > heap[index].priority) {
            swap(heap[index], heap[parent(index)]);
            index = parent(index);
        }
    }

    void heapifyDown(int index) {
        int smallest = index;
        int l = left(index);
        int r = right(index);

        if (l < heap.size() && heap[l].priority < heap[smallest].priority)
            smallest = l;

        if (r < heap.size() && heap[r].priority < heap[smallest].priority)
            smallest = r;

        if (smallest != index) {
            std::swap(heap[index], heap[smallest]);
            heapifyDown(smallest);
        }
    }

public:
    bool isEmpty() const { return heap.empty(); }

    int size() const { return heap.size(); }

    void push(const T& val, int priority) {
        heap.push_back(Node(val, priority));
        heapifyUp(heap.size() - 1);
    }

    T pop() {
        if (heap.empty())
            throw out_of_range("PriorityQueue empty");

        T root = heap[0].data;
        heap[0] = heap.back();
        heap.pop_back();
        if (!heap.empty()) heapifyDown(0);
        return root;
    }

    T top() const {
        if (heap.empty())
            throw out_of_range("PriorityQueue empty");
        return heap[0].data;
    }
};

#endif
