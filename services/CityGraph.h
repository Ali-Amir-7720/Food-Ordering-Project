#pragma once
#ifndef CITYGRAPH_H
#define CITYGRAPH_H

#include <iostream>
#include <map>
#include <string>
#include <climits>
#include "dataStructures/Graph.h"
#include "dataStructures/LinkedList.h"
#include "../models/CityMapData.h"
#include "../CityMapDatabase.h"

using namespace std;

class CityGraph {
private:
    Graph* graph;
    map<int, string> locationNames;
    map<int, string> locationTypes;
    CityMapDatabase mapDB;
    
public:
    CityGraph(int maxNodes = 500) {
        graph = new Graph(maxNodes);
        loadFromDatabase();  // Load on construction
    }
    
    ~CityGraph() {
        saveToDatabase();
        delete graph;
    }
    
    // ✅ MADE PUBLIC: So we can reload after initialization
    void loadFromDatabase() {
        // Clear existing data first
        locationNames.clear();
        locationTypes.clear();
        
        // Load locations
        vector<LocationData> locations = mapDB.loadAllLocations();
        for (const auto& loc : locations) {
            graph->addNode(loc.nodeId);
            locationNames[loc.nodeId] = string(loc.name);
            locationTypes[loc.nodeId] = string(loc.type);
        }
        
        // Load roads
        vector<RoadData> roads = mapDB.loadAllRoads();
        for (const auto& road : roads) {
            graph->addEdge(road.fromNode, road.toNode, road.distance);
        }
        
        cout << "DEBUG: CityGraph loaded " << locationNames.size() 
             << " locations and " << roads.size() << " roads\n";
    }
    
    // Save current map to database
    void saveToDatabase() {
        // Save locations
        vector<LocationData> locations;
        for (const auto& loc : locationNames) {
            locations.push_back(LocationData(loc.first, loc.second, locationTypes[loc.first]));
        }
        mapDB.saveAllLocations(locations);
        
        // Save roads
        vector<RoadData> roads;
        for (const auto& loc : locationNames) {
            int nodeId = loc.first;
            LinkedList<int> neighbors = graph->getNeighbors(nodeId);
            auto* current = neighbors.getHead();
            while (current != nullptr) {
                int neighborId = current->data;
                // Only save each road once (avoid duplicates in undirected graph)
                if (nodeId < neighborId) {
                    int distance = graph->getEdgeWeight(nodeId, neighborId);
                    roads.push_back(RoadData(nodeId, neighborId, distance));
                }
                current = current->next;
            }
        }
        mapDB.saveAllRoads(roads);
    }
    
    // Add a location
    void addLocation(int nodeId, const string& name, const string& type) {
        graph->addNode(nodeId);
        locationNames[nodeId] = name;
        locationTypes[nodeId] = type;
    }
    
    // Remove a location
    bool removeLocation(int nodeId) {
        if (locationNames.find(nodeId) == locationNames.end()) {
            return false;
        }
        
        // Remove all edges connected to this node
        LinkedList<int> neighbors = graph->getNeighbors(nodeId);
        auto* current = neighbors.getHead();
        while (current != nullptr) {
            graph->removeEdge(nodeId, current->data);
            current = current->next;
        }
        
        locationNames.erase(nodeId);
        locationTypes.erase(nodeId);
        return true;
    }
    
    // Add a road
    void addRoad(int from, int to, int distance) {
        graph->addEdge(from, to, distance);
    }
    
    // Remove a road
    void removeRoad(int from, int to) {
        graph->removeEdge(from, to);
    }
    
    // Check if location exists
    bool locationExists(int nodeId) {
        return locationNames.find(nodeId) != locationNames.end();
    }
    
    // Get next available node ID
    int getNextAvailableNodeId() {
        int maxId = 0;
        for (const auto& loc : locationNames) {
            if (loc.first > maxId) {
                maxId = loc.first;
            }
        }
        return maxId + 1;
    }
    
    // Find shortest path
    pair<LinkedList<int>, int> findShortestPath(int start, int end) {
        LinkedList<int> path = graph->dijkstra(start, end);
        
        int totalDistance = 0;
        if (!path.isEmpty()) {
            auto* current = path.getHead();
            while (current != nullptr && current->next != nullptr) {
                int u = current->data;
                int v = current->next->data;
                int weight = graph->getEdgeWeight(u, v);
                if (weight != -1) {
                    totalDistance += weight;
                }
                current = current->next;
            }
        }
        
        return {path, totalDistance};
    }
    
    int getDirectDistance(int from, int to) {
        return graph->getEdgeWeight(from, to);
    }
    
    string getLocationName(int nodeId) {
        if (locationNames.find(nodeId) != locationNames.end()) {
            return locationNames[nodeId];
        }
        return "Location " + to_string(nodeId);
    }
    
    string getLocationType(int nodeId) {
        if (locationTypes.find(nodeId) != locationTypes.end()) {
            return locationTypes[nodeId];
        }
        return "unknown";
    }
    
    // Print visual map
    void printGraph() {
        cout << "\n╔═══════════════════════════════════════╗\n";
        cout << "║           CITY MAP                     ║\n";
        cout << "╚═══════════════════════════════════════╝\n\n";
        
        if (locationNames.empty()) {
            cout << "⚠ No locations in the city map!\n";
            return;
        }
        
        for (const auto& location : locationNames) {
            int nodeId = location.first;
            string name = location.second;
            string type = locationTypes[nodeId];
            
            string icon = "📍";
            if (type == "restaurant") icon = "🍽️";
            else if (type == "home") icon = "🏠";
            else if (type == "rider") icon = "🏍️";
            
            cout << icon << " [" << nodeId << "] " << name 
                 << " (" << type << ")\n";
            
            LinkedList<int> neighbors = graph->getNeighbors(nodeId);
            if (!neighbors.isEmpty()) {
                cout << "   Connected to:\n";
                auto* current = neighbors.getHead();
                while (current != nullptr) {
                    int neighborId = current->data;
                    int distance = graph->getEdgeWeight(nodeId, neighborId);
                    cout << "   → [" << neighborId << "] " 
                         << locationNames[neighborId] 
                         << " (" << distance << "m)\n";
                    current = current->next;
                }
            }
            cout << "\n";
        }
    }
    
    // Print path with details
    void printPathDetails(const LinkedList<int>& path, int totalDistance) {
        if (path.isEmpty()) {
            cout << "⚠ No path found!\n";
            return;
        }
        
        cout << "\n╔═══════════════════════════════════════╗\n";
        cout << "║         DELIVERY ROUTE                 ║\n";
        cout << "╚═══════════════════════════════════════╝\n";
        cout << "Total Distance: " << totalDistance << " meters\n";
        cout << "Route:\n\n";
        
        auto* current = path.getHead();
        int step = 1;
        
        while (current != nullptr) {
            int nodeId = current->data;
            string name = getLocationName(nodeId);
            string type = getLocationType(nodeId);
            
            string icon = "📍";
            if (type == "restaurant") icon = "🍽️";
            else if (type == "home") icon = "🏠";
            else if (type == "rider") icon = "🏍️";
            
            cout << "  " << step++ << ". " << icon << " " << name 
                 << " [" << nodeId << "]\n";
            
            if (current->next != nullptr) {
                int nextId = current->next->data;
                int distance = graph->getEdgeWeight(nodeId, nextId);
                cout << "     ↓ " << distance << "m\n";
            } else {
                cout << "     ✓ DESTINATION\n";
            }
            
            current = current->next;
        }
        cout << "\n═══════════════════════════════════════\n";
    }
    
    // Get all locations
    map<int, string> getAllLocations() {
        return locationNames;
    }
};

#endif