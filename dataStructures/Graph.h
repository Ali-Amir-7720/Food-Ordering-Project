#pragma once
#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <climits>
#include <vector>
#include "LinkedList.h"

using namespace std;

class Graph {
public:
    struct Edge {
        int destination;
        int weight;
        
        Edge() : destination(0), weight(0) {}
        Edge(int dest, int w) : destination(dest), weight(w) {}
        
        bool operator==(const Edge& other) const {
            return destination == other.destination && weight == other.weight;
        }
        
        bool operator!=(const Edge& other) const {
            return !(*this == other);
        }
    };

private:
    int maxNodes;
    LinkedList<Edge>* adjacencyList;
    bool* nodeExists;
    
public:
    Graph(int maxN = 500) : maxNodes(maxN) {
        adjacencyList = new LinkedList<Edge>[maxNodes];
        nodeExists = new bool[maxNodes];
        for (int i = 0; i < maxNodes; i++) {
            nodeExists[i] = false;
        }
    }
    
    ~Graph() {
        delete[] adjacencyList;
        delete[] nodeExists;
    }
    
    void addNode(int nodeId) {
        if (nodeId >= 0 && nodeId < maxNodes) {
            nodeExists[nodeId] = true;
        }
    }
    
    void addEdge(int from, int to, int weight) {
        if (from >= 0 && from < maxNodes && to >= 0 && to < maxNodes) {
            if (!nodeExists[from]) addNode(from);
            if (!nodeExists[to]) addNode(to);
            
            // Check if edge already exists
            Node<Edge>* current = adjacencyList[from].getHead();
            bool found = false;
            while (current != nullptr) {
                if (current->data.destination == to) {
                    current->data.weight = weight; // Update weight
                    found = true;
                    break;
                }
                current = current->next;
            }
            
            if (!found) {
                adjacencyList[from].insertAtEnd(Edge(to, weight));
            }
            
            // For undirected graph, add reverse edge
            current = adjacencyList[to].getHead();
            found = false;
            while (current != nullptr) {
                if (current->data.destination == from) {
                    current->data.weight = weight;
                    found = true;
                    break;
                }
                current = current->next;
            }
            
            if (!found) {
                adjacencyList[to].insertAtEnd(Edge(from, weight));
            }
        }
    }
    
    LinkedList<int> getAllNodes() const {
        LinkedList<int> nodes;
        for (int i = 0; i < maxNodes; i++) {
            if (nodeExists[i]) {
                nodes.insertAtEnd(i);
            }
        }
        return nodes;
    }
    
    int getNumNodes() const {
        int count = 0;
        for (int i = 0; i < maxNodes; i++) {
            if (nodeExists[i]) {
                count++;
            }
        }
        return count;
    }
    
    void removeEdge(int from, int to) {
        if (from >= 0 && from < maxNodes && to >= 0 && to < maxNodes) {
            // Remove from -> to
            Node<Edge>* current = adjacencyList[from].getHead();
            Node<Edge>* prev = nullptr;
            
            while (current != nullptr) {
                if (current->data.destination == to) {
                    if (prev == nullptr) {
                        // Remove head
                        adjacencyList[from].remove(current->data);
                    } else {
                        adjacencyList[from].remove(current->data);
                    }
                    break;
                }
                prev = current;
                current = current->next;
            }
            
            // Remove to -> from (undirected)
            current = adjacencyList[to].getHead();
            prev = nullptr;
            
            while (current != nullptr) {
                if (current->data.destination == from) {
                    if (prev == nullptr) {
                        adjacencyList[to].remove(current->data);
                    } else {
                        adjacencyList[to].remove(current->data);
                    }
                    break;
                }
                prev = current;
                current = current->next;
            }
        }
    }
    
    LinkedList<int> getNeighbors(int nodeId) const {
        LinkedList<int> neighbors;
        if (nodeId >= 0 && nodeId < maxNodes && nodeExists[nodeId]) {
            Node<Edge>* current = adjacencyList[nodeId].getHead();
            while (current != nullptr) {
                neighbors.insertAtEnd(current->data.destination);
                current = current->next;
            }
        }
        return neighbors;
    }
    
    int getEdgeWeight(int from, int to) const {
        if (from >= 0 && from < maxNodes && to >= 0 && to < maxNodes) {
            Node<Edge>* current = adjacencyList[from].getHead();
            while (current != nullptr) {
                if (current->data.destination == to) {
                    return current->data.weight;
                }
                current = current->next;
            }
        }
        return -1; // Edge doesn't exist
    }
    
    bool hasEdge(int from, int to) const {
        if (from >= 0 && from < maxNodes && to >= 0 && to < maxNodes) {
            Node<Edge>* current = adjacencyList[from].getHead();
            while (current != nullptr) {
                if (current->data.destination == to) {
                    return true;
                }
                current = current->next;
            }
        }
        return false;
    }
    
    // Dijkstra's algorithm for shortest path - FIXED VERSION
    LinkedList<int> dijkstra(int start, int end) const {
        LinkedList<int> path;
        
        // Debug: Check parameters
        // cout << "DEBUG dijkstra: start=" << start << ", end=" << end << endl;
        
        if (start < 0 || start >= maxNodes || end < 0 || end >= maxNodes) {
            // cout << "DEBUG: Invalid node IDs" << endl;
            return path; // Empty path
        }
        
        if (!nodeExists[start] || !nodeExists[end]) {
            // cout << "DEBUG: Nodes don't exist" << endl;
            return path;
        }
        
        if (start == end) {
            path.insertAtEnd(start);
            return path;
        }
        
        vector<int> dist(maxNodes, INT_MAX);
        vector<int> prev(maxNodes, -1);
        vector<bool> visited(maxNodes, false);
        
        dist[start] = 0;
        
        // Dijkstra's algorithm
        for (int count = 0; count < maxNodes; count++) {
            // Find the unvisited node with the smallest distance
            int u = -1;
            int minDist = INT_MAX;
            
            for (int i = 0; i < maxNodes; i++) {
                if (!visited[i] && nodeExists[i] && dist[i] < minDist) {
                    minDist = dist[i];
                    u = i;
                }
            }
            
            if (u == -1) break; // No more reachable nodes
            
            visited[u] = true;
            
            // If we reached the destination, break early
            if (u == end) {
                break;
            }
            
            // Update distances to neighbors
            Node<Edge>* current = adjacencyList[u].getHead();
            while (current != nullptr) {
                int v = current->data.destination;
                int weight = current->data.weight;
                
                if (!visited[v] && dist[u] != INT_MAX && 
                    dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    prev[v] = u;
                }
                
                current = current->next;
            }
        }
        
        // Debug: Check if path exists
        // cout << "DEBUG: Distance to end: " << dist[end] << endl;
        
        // Reconstruct path if it exists
        if (dist[end] != INT_MAX) {
            vector<int> reversePath;
            int current = end;
            
            while (current != -1) {
                reversePath.push_back(current);
                current = prev[current];
            }
            
            // Reverse to get correct order
            for (int i = reversePath.size() - 1; i >= 0; i--) {
                path.insertAtEnd(reversePath[i]);
            }
            
            // Debug: Print path
            // cout << "DEBUG: Path found: ";
            // Node<int>* node = path.getHead();
            // while (node != nullptr) {
            //     cout << node->data << " ";
            //     node = node->next;
            // }
            // cout << endl;
        } else {
            // cout << "DEBUG: No path found" << endl;
        }
        
        return path;
    }
    
    void printGraph() const {
        cout << "\n=== Graph Structure ===\n";
        int totalEdges = 0;
        
        for (int i = 0; i < maxNodes; i++) {
            if (nodeExists[i]) {
                cout << "Node " << i << " -> ";
                Node<Edge>* current = adjacencyList[i].getHead();
                if (current == nullptr) {
                    cout << "No connections";
                }
                while (current != nullptr) {
                    cout << "[" << current->data.destination 
                         << ", weight:" << current->data.weight << "] ";
                    totalEdges++;
                    current = current->next;
                }
                cout << "\n";
            }
        }
        cout << "Total edges: " << totalEdges << "\n";
    }
    
    // Print adjacency matrix
    void printAdjacencyMatrix() const {
        int nodeCount = getNumNodes();
        if (nodeCount == 0) {
            cout << "Graph is empty!\n";
            return;
        }
        
        // Get all nodes
        LinkedList<int> nodes = getAllNodes();
        vector<int> nodeList;
        Node<int>* current = nodes.getHead();
        while (current != nullptr) {
            nodeList.push_back(current->data);
            current = current->next;
        }
        
        cout << "\n=== Adjacency Matrix ===\n";
        cout << "    ";
        for (int node : nodeList) {
            cout << node << " ";
        }
        cout << "\n";
        
        for (int i = 0; i < nodeList.size(); i++) {
            cout << nodeList[i] << " | ";
            for (int j = 0; j < nodeList.size(); j++) {
                int weight = getEdgeWeight(nodeList[i], nodeList[j]);
                if (weight == -1) {
                    cout << "0 ";
                } else {
                    cout << weight << " ";
                }
            }
            cout << "\n";
        }
    }
};

#endif // GRAPH_H