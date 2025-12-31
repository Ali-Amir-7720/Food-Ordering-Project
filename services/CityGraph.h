#pragma once
#ifndef CITYGRAPH_H
#define CITYGRAPH_H

#include <iostream>
#include <map>
#include <string>
#include <climits>
#include <vector>
#include <algorithm>
#include "../dataStructures/Graph.h"
#include "../dataStructures/LinkedList.h"
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
    
    // Load map data from database
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
    
    // Get total number of roads
    int getRoadCount() const {
        int count = 0;
        
        // Get all nodes
        LinkedList<int> nodes = graph->getAllNodes();
        Node<int>* node = nodes.getHead();
        
        while (node != nullptr) {
            int nodeId = node->data;
            
            // Get neighbors for this node
            LinkedList<int> neighbors = graph->getNeighbors(nodeId);
            auto* neighbor = neighbors.getHead();
            
            // Count edges (each edge is counted from both directions, so divide by 2)
            while (neighbor != nullptr) {
                // Only count each edge once (avoid counting A->B and B->A separately)
                if (nodeId < neighbor->data) {
                    count++;
                }
                neighbor = neighbor->next;
            }
            
            node = node->next;
        }
        
        return count;
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
    
    // Find shortest path - FIXED VERSION
    pair<LinkedList<int>, int> findShortestPath(int start, int end) {
        // cout << "DEBUG findShortestPath: start=" << start << ", end=" << end << endl;
        
        LinkedList<int> path = graph->dijkstra(start, end);
        
        int totalDistance = 0;
        
        if (!path.isEmpty()) {
            // Debug: Print the path
            // cout << "DEBUG: Path nodes: ";
            // Node<int>* node = path.getHead();
            // while (node != nullptr) {
            //     cout << node->data << " ";
            //     node = node->next;
            // }
            // cout << endl;
            
            // Calculate total distance
            Node<int>* current = path.getHead();
            while (current != nullptr && current->next != nullptr) {
                int u = current->data;
                int v = current->next->data;
                int weight = graph->getEdgeWeight(u, v);
                if (weight != -1) {
                    totalDistance += weight;
                } else {
                    // cout << "DEBUG: No edge between " << u << " and " << v << endl;
                }
                current = current->next;
            }
        } else {
            // cout << "DEBUG: No path found from " << start << " to " << end << endl;
        }
        
        return make_pair(path, totalDistance);
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
        
        // Print all locations
        for (const auto& location : locationNames) {
            int nodeId = location.first;
            string name = location.second;
            string type = locationTypes[nodeId];
            
            string icon = "📍";
            if (type == "restaurant") icon = "🍽️";
            else if (type == "home") icon = "🏠";
            else if (type == "rider") icon = "🏍️";
            else if (type == "center") icon = "🏙️";
            
            cout << icon << " [" << nodeId << "] " << name 
                 << " (" << type << ")\n";
            
            // Print connections
            LinkedList<int> neighbors = graph->getNeighbors(nodeId);
            if (!neighbors.isEmpty()) {
                cout << "   Connected to: ";
                auto* current = neighbors.getHead();
                bool first = true;
                while (current != nullptr) {
                    if (!first) cout << ", ";
                    cout << "[" << current->data << "]";
                    first = false;
                    current = current->next;
                }
                cout << "\n";
            }
        }
        
        // Print total stats
        cout << "\n═══════════════════════════════════════\n";
        cout << "Total Locations: " << locationNames.size() << "\n";
        cout << "Total Roads: " << getRoadCount() << "\n";
        cout << "═══════════════════════════════════════\n";
    }
    
    // Print path with details - ENHANCED VERSION
    void printPathDetails(const LinkedList<int>& path, int totalDistance) {
        if (path.isEmpty()) {
            cout << "\n⚠ No path found between the specified locations!\n";
            cout << "Possible reasons:\n";
            cout << "  1. Locations are not connected\n";
            cout << "  2. No roads exist between them\n";
            cout << "  3. One or both locations don't exist\n";
            return;
        }
        
        cout << "\n╔═══════════════════════════════════════╗\n";
        cout << "║         DELIVERY ROUTE                 ║\n";
        cout << "╚═══════════════════════════════════════╝\n";
        cout << "Total Distance: " << totalDistance << " meters\n";
        cout << "Total Stops: " << path.getSize() << "\n";
        cout << "═══════════════════════════════════════\n";
        cout << "Route:\n\n";
        
        auto* current = path.getHead();
        int step = 1;
        int cumulativeDistance = 0;
        
        while (current != nullptr) {
            int nodeId = current->data;
            string name = getLocationName(nodeId);
            string type = getLocationType(nodeId);
            
            string icon = "📍";
            if (type == "restaurant") icon = "🍽️";
            else if (type == "home") icon = "🏠";
            else if (type == "rider") icon = "🏍️";
            else if (type == "center") icon = "🏙️";
            
            cout << "  " << step++ << ". " << icon << " " << name 
                 << " [" << nodeId << "]\n";
            
            if (current->next != nullptr) {
                int nextId = current->next->data;
                int segmentDistance = graph->getEdgeWeight(nodeId, nextId);
                cumulativeDistance += segmentDistance;
                cout << "     ↓ " << segmentDistance << "m ";
                cout << "(Total: " << cumulativeDistance << "m)\n";
            } else {
                cout << "     ✓ DESTINATION REACHED\n";
            }
            
            current = current->next;
        }
        cout << "\n═══════════════════════════════════════\n";
    }
    
    // Get all locations
    map<int, string> getAllLocations() {
        return locationNames;
    }
    
    // Print all locations in a compact format
    void printAllLocations() {
        cout << "\n📍 City Map Locations:\n\n";
        int count = 1;
        for (const auto& location : locationNames) {
            int nodeId = location.first;
            string name = location.second;
            string type = locationTypes[nodeId];
            
            cout << "  " << count++ << ". [" << nodeId << "] " << name;
            if (!type.empty()) {
                cout << " (" << type << ")";
            }
            cout << "\n";
        }
        cout << "\nTotal: " << locationNames.size() << " locations\n";
    }
    
    // Test connectivity between two nodes
    bool testConnectivity(int start, int end) {
        LinkedList<int> path = graph->dijkstra(start, end);
        return !path.isEmpty();
    }
    
    // Print adjacency matrix
    void printAdjacencyMatrix() {
        graph->printAdjacencyMatrix();
    }
    
    // Initialize sample map data
    void initializeSampleMap() {
        cout << "Initializing sample city map...\n";
        
        // Clear existing data
        locationNames.clear();
        locationTypes.clear();
        
        // Add sample locations
        addLocation(101, "Pizza Palace Area", "restaurant");
        addLocation(102, "Burger Barn Area", "restaurant");
        addLocation(103, "Sushi Spot Area", "restaurant");
        addLocation(201, "Customer District A", "home");
        addLocation(202, "Customer District B", "home");
        addLocation(203, "Customer District C", "home");
        addLocation(301, "Rider Station 1", "rider");
        addLocation(302, "Rider Station 2", "rider");
        addLocation(400, "City Center", "center");
        
        // Add roads/connections (create a connected graph)
        addRoad(101, 102, 500);   // Pizza Palace to Burger Barn
        addRoad(102, 103, 600);   // Burger Barn to Sushi Spot
        addRoad(101, 400, 800);   // Pizza Palace to City Center
        addRoad(102, 400, 700);   // Burger Barn to City Center
        addRoad(103, 400, 900);   // Sushi Spot to City Center
        
        addRoad(400, 201, 400);   // City Center to Customer District A
        addRoad(400, 202, 500);   // City Center to Customer District B
        addRoad(400, 203, 600);   // City Center to Customer District C
        
        addRoad(301, 101, 300);   // Rider Station 1 to Pizza Palace
        addRoad(301, 400, 600);   // Rider Station 1 to City Center
        addRoad(302, 103, 400);   // Rider Station 2 to Sushi Spot
        addRoad(302, 400, 550);   // Rider Station 2 to City Center
        
        addRoad(201, 202, 350);   // Connect customer districts
        addRoad(202, 203, 400);
        
        cout << "✓ Sample city map initialized with " << locationNames.size() 
             << " locations and " << getRoadCount() << " roads\n";
    }
    // In CityGraph class, add this method in the public section:

// Find the nearest node of a specific type (or any type if empty)
int findNearestNode(int fromNode, const string& targetType = "") {
    auto allLocations = getAllLocations();
    if (allLocations.empty()) return -1;
    
    int nearestNode = -1;
    int minDistance = INT_MAX;
    
    for (const auto& loc : allLocations) {
        int toNode = loc.first;
        
        // Skip if it's the same node
        if (toNode == fromNode) continue;
        
        // If targetType is specified, only check nodes of that type
        if (!targetType.empty()) {
            string nodeType = getLocationType(toNode);
            if (nodeType != targetType) continue;
        }
        
        // Try to find path and distance
        try {
            auto pathResult = findShortestPath(fromNode, toNode);
            int distance = pathResult.second;
            
            if (distance > 0 && distance < minDistance) {
                minDistance = distance;
                nearestNode = toNode;
            }
        } catch (...) {
            // Skip if path not found
            continue;
        }
    }
    
    return nearestNode;
}

// Also add this helper method for multiple connections:
void connectToExistingNetwork(int newNode, int maxConnections = 3) {
    auto locations = getAllLocations();
    if (locations.empty()) return;
    
    vector<int> existingNodes;
    for (const auto& loc : locations) {
        if (loc.first != newNode) {
            existingNodes.push_back(loc.first);
        }
    }
    
    // Shuffle to get random connections
    random_shuffle(existingNodes.begin(), existingNodes.end());
    
    int connections = 0;
    for (int existingNode : existingNodes) {
        if (connections >= maxConnections) break;
        
        // Don't connect to self
        if (existingNode == newNode) continue;
        
        // Calculate reasonable distance based on node IDs
        int distance = 500 + abs(newNode - existingNode) * 10;
        if (distance > 2000) distance = 2000; // Cap at 2km
        
        addRoad(newNode, existingNode, distance);
        connections++;
        
        cout << "  Connected node " << newNode << " to " << existingNode 
             << " (" << getLocationName(existingNode) << ") - " << distance << "m\n";
    }
}
};

#endif // CITYGRAPH_H