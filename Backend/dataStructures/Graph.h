#pragma once
#pragma once
#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <vector>
#include <limits>
#include "PriorityQueue.h"

using namespace std;

class Graph {
private:
    struct Edge {
        int to;
        int weight;
        Edge(int t, int w) : to(t), weight(w) {}
    };

    int V;
    vector<vector<Edge>> adj;

public:
    Graph(int vertices) {
        V = vertices;
        adj.resize(V);
    }

    void addEdge(int u, int v, int w) {
        if (u >= 0 && u < V && v >= 0 && v < V) {
            adj[u].push_back(Edge(v, w));
            adj[v].push_back(Edge(u, w)); // undirected
        }
    }

    vector<int> dijkstra(int src, int dest) {
        vector<int> dist(V, numeric_limits<int>::max());
        vector<int> parent(V, -1);
        vector<bool> visited(V, false);

        PriorityQueue<int> pq;
        dist[src] = 0;
        pq.push(src, 0);

        while (!pq.isEmpty()) {
            int u = pq.pop();

            if (visited[u]) continue;
            visited[u] = true;

            for (auto& e : adj[u]) {
                if (!visited[e.to] && dist[u] + e.weight < dist[e.to]) {
                    dist[e.to] = dist[u] + e.weight;
                    parent[e.to] = u;
                    pq.push(e.to, dist[e.to]);
                }
            }
        }

        vector<int> path;
        if (dist[dest] == numeric_limits<int>::max()) return path;

        for (int cur = dest; cur != -1; cur = parent[cur])
            path.insert(path.begin(), cur);

        return path;
    }

    void printGraph() {
        for (int i = 0; i < V; i++) {
            cout << i << ": ";
            for (auto& e : adj[i])
                cout << "(" << e.to << "," << e.weight << ") ";
            cout << "\n";
        }
    }
};
#endif