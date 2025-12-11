#pragma once
#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include "LinkedList.h"
#include "PriorityQueue.h"

class Graph {
private:
    struct Edge {
        int to;
        int weight;
        Edge(int t = 0, int w = 0) : to(t), weight(w) {}
        
        bool operator==(const Edge& other) const {
            return to == other.to && weight == other.weight;
        }
    };
    
    // Array of LinkedLists to store adjacency list
    LinkedList<Edge>** adj;
    int V; // Number of vertices
    int capacity; // Current capacity
    
    void resize(int newCapacity) {
        LinkedList<Edge>** newAdj = new LinkedList<Edge>*[newCapacity];
        
        // Initialize new array
        for (int i = 0; i < newCapacity; i++) {
            if (i < capacity) {
                newAdj[i] = adj[i];
            } else {
                newAdj[i] = new LinkedList<Edge>();
            }
        }
        
        // Delete old array
        delete[] adj;
        adj = newAdj;
        capacity = newCapacity;
    }

public:
    Graph(int vertices = 100) {
        V = vertices;
        capacity = vertices;
        adj = new LinkedList<Edge>*[capacity];
        
        for (int i = 0; i < capacity; i++) {
            adj[i] = new LinkedList<Edge>();
        }
    }
    
    ~Graph() {
        for (int i = 0; i < capacity; i++) {
            delete adj[i];
        }
        delete[] adj;
    }

    void addNode(int nodeId) {
        if (nodeId >= capacity) {
            resize(nodeId + 10);
        }
        if (nodeId >= V) {
            V = nodeId + 1;
        }
    }

    void addEdge(int u, int v, int w) {
        if (u >= V) addNode(u);
        if (v >= V) addNode(v);
        
        // Add edge both ways for undirected graph
        adj[u]->insertAtEnd(Edge(v, w));
        adj[v]->insertAtEnd(Edge(u, w));
    }
    
    void removeEdge(int u, int v) {
        if (u < 0 || u >= V || v < 0 || v >= V) return;
        
        // Remove edge from u to v
        LinkedList<Edge>* listU = adj[u];
        auto* currentU = listU->getHead();
        while (currentU != nullptr) {
            if (currentU->data.to == v) {
                Edge edgeToRemove(v, currentU->data.weight);
                listU->remove(edgeToRemove);
                break;
            }
            currentU = currentU->next;
        }
        
        // Remove edge from v to u
        LinkedList<Edge>* listV = adj[v];
        auto* currentV = listV->getHead();
        while (currentV != nullptr) {
            if (currentV->data.to == u) {
                Edge edgeToRemove(u, currentV->data.weight);
                listV->remove(edgeToRemove);
                break;
            }
            currentV = currentV->next;
        }
    }
    
    int getEdgeWeight(int u, int v) {
        if (u < 0 || u >= V || v < 0 || v >= V) return -1;
        
        LinkedList<Edge>* list = adj[u];
        auto* current = list->getHead();
        while (current != nullptr) {
            if (current->data.to == v) {
                return current->data.weight;
            }
            current = current->next;
        }
        return -1; // Edge not found
    }
    
    // Dijkstra's algorithm using our custom PriorityQueue
    LinkedList<int> dijkstra(int src, int dest) {
        // Create arrays for distances and previous nodes
        int* dist = new int[V];
        int* prev = new int[V];
        bool* visited = new bool[V];
        
        // Initialize arrays
        for (int i = 0; i < V; i++) {
            dist[i] = 2147483647; // INT_MAX equivalent
            prev[i] = -1;
            visited[i] = false;
        }
        dist[src] = 0;
        
        // Create priority queue (min-heap)
        PriorityQueue<std::pair<int, int>> pq; // (distance, node)
        
        // Enqueue source node
        pq.enqueue(std::make_pair(0, src), 0);
        
        while (!pq.isEmpty()) {
            // Get node with smallest distance
            auto current = pq.dequeue();
            int u = current.second;
            int d = current.first;
            
            if (visited[u]) continue;
            visited[u] = true;
            
            // If we reached destination, we can stop early
            if (u == dest) break;
            
            // Check all neighbors
            LinkedList<Edge>* neighbors = adj[u];
            auto* neighborNode = neighbors->getHead();
            
            while (neighborNode != nullptr) {
                int v = neighborNode->data.to;
                int weight = neighborNode->data.weight;
                
                if (!visited[v]) {
                    int newDist = d + weight;
                    if (newDist < dist[v]) {
                        dist[v] = newDist;
                        prev[v] = u;
                        pq.enqueue(std::make_pair(newDist, v), newDist);
                    }
                }
                neighborNode = neighborNode->next;
            }
        }
        
        // Reconstruct path if destination is reachable
        LinkedList<int> path;
        if (dist[dest] == 2147483647) {
            // No path found
            delete[] dist;
            delete[] prev;
            delete[] visited;
            return path;
        }
        
        // Build path in reverse WITHOUT using std::stack
        // Method 1: Build reverse linked list, then reverse it
        LinkedList<int> reversePath;
        int current = dest;
        while (current != -1) {
            reversePath.insertAtFront(current); // Insert at front to reverse
            current = prev[current];
        }
        
        // Method 2: Or insert at end and traverse in reverse
        // For simplicity, we'll just return reversePath since it's already correct
        
        // Clean up
        delete[] dist;
        delete[] prev;
        delete[] visited;
        
        return reversePath;
    }
    
    // Alternative method to get path without stack
    LinkedList<int> getPathFromPrev(int dest, int* prev) {
        LinkedList<int> path;
        
        // If no path exists
        if (prev[dest] == -1 && dest != -1) {
            return path; // Empty path
        }
        
        // Collect path nodes in a temporary array first
        int* tempPath = new int[V];
        int pathLength = 0;
        int current = dest;
        
        // Collect all nodes in reverse order
        while (current != -1) {
            tempPath[pathLength++] = current;
            current = prev[current];
        }
        
        // Add to linked list in correct order (reverse of tempPath)
        for (int i = pathLength - 1; i >= 0; i--) {
            path.insertAtEnd(tempPath[i]);
        }
        
        delete[] tempPath;
        return path;
    }
    
    // Version of Dijkstra using the new path reconstruction
    LinkedList<int> dijkstra2(int src, int dest) {
        // Create arrays for distances and previous nodes
        int* dist = new int[V];
        int* prev = new int[V];
        bool* visited = new bool[V];
        
        // Initialize arrays
        for (int i = 0; i < V; i++) {
            dist[i] = 2147483647;
            prev[i] = -1;
            visited[i] = false;
        }
        dist[src] = 0;
        
        // Create priority queue
        PriorityQueue<std::pair<int, int>> pq;
        pq.enqueue(std::make_pair(0, src), 0);
        
        while (!pq.isEmpty()) {
            auto current = pq.dequeue();
            int u = current.second;
            int d = current.first;
            
            if (visited[u]) continue;
            visited[u] = true;
            
            if (u == dest) break;
            
            // Process neighbors
            LinkedList<Edge>* neighbors = adj[u];
            auto* neighborNode = neighbors->getHead();
            
            while (neighborNode != nullptr) {
                int v = neighborNode->data.to;
                int weight = neighborNode->data.weight;
                
                if (!visited[v]) {
                    int newDist = d + weight;
                    if (newDist < dist[v]) {
                        dist[v] = newDist;
                        prev[v] = u;
                        pq.enqueue(std::make_pair(newDist, v), newDist);
                    }
                }
                neighborNode = neighborNode->next;
            }
        }
        
        // Reconstruct path using the new method
        LinkedList<int> path = getPathFromPrev(dest, prev);
        
        // Clean up
        delete[] dist;
        delete[] prev;
        delete[] visited;
        
        return path;
    }

    void printGraph() {
        for (int i = 0; i < V; i++) {
            LinkedList<Edge>* list = adj[i];
            if (!list->isEmpty()) {
                std::cout << "Node " << i << ": ";
                list->traverse([](const Edge& e) {
                    std::cout << "(" << e.to << "," << e.weight << ") ";
                });
                std::cout << "\n";
            }
        }
    }
    
    int getNumNodes() const {
        return V;
    }
    
    // Get all nodes as LinkedList (not vector)
    LinkedList<int> getAllNodes() {
        LinkedList<int> nodes;
        for (int i = 0; i < V; i++) {
            nodes.insertAtEnd(i);
        }
        return nodes;
    }
    
    // Get all edges
    LinkedList<std::pair<int, int>> getAllEdges() {
        LinkedList<std::pair<int, int>> edges;
        for (int i = 0; i < V; i++) {
            LinkedList<Edge>* list = adj[i];
            auto* current = list->getHead();
            while (current != nullptr) {
                // Add each edge (only once since it's undirected)
                if (i <= current->data.to) { // Avoid duplicates
                    edges.insertAtEnd(std::make_pair(i, current->data.to));
                }
                current = current->next;
            }
        }
        return edges;
    }
    
    // Check if edge exists
    bool hasEdge(int u, int v) {
        return getEdgeWeight(u, v) != -1;
    }
    
    // Get neighbors of a node
    LinkedList<int> getNeighbors(int node) {
        LinkedList<int> neighbors;
        if (node < 0 || node >= V) return neighbors;
        
        LinkedList<Edge>* list = adj[node];
        auto* current = list->getHead();
        while (current != nullptr) {
            neighbors.insertAtEnd(current->data.to);
            current = current->next;
        }
        return neighbors;
    }
};

#endif