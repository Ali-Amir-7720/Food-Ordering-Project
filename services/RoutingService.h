#pragma once
#ifndef ROUTING_SERVICE_H
#define ROUTING_SERVICE_H

#include <iostream>
#include <cmath>
#include <vector>
#include "../dataStructures/Graph.h"
#include "../models/Rider.h"
#include "../dataStructures/LinkedList.h"

using namespace std;

class RoutingService {
private:
    Graph* cityMap; // Graph representing the city map
    bool ownsGraph; // Track if we own the graph pointer

public:
    RoutingService() : cityMap(nullptr), ownsGraph(false) {}
    
    RoutingService(Graph* graph) : cityMap(graph), ownsGraph(false) {}
    
    RoutingService(int numLocations) : ownsGraph(true) {
        cityMap = new Graph(numLocations);
    }
    
    ~RoutingService() {
        if (cityMap && ownsGraph) {
            delete cityMap;
        }
    }

    // Add a location/node to the city map
    void addLocation(int nodeId) {
        if (cityMap) {
            cityMap->addNode(nodeId);
        }
    }

    // Add a road/edge between two locations with weight (e.g., distance or time)
    void addRoad(int from, int to, int weight) {
        if (cityMap) {
            cityMap->addEdge(from, to, weight);
        }
    }

    // Remove a road
    void removeRoad(int from, int to) {
        if (cityMap) {
            cityMap->removeEdge(from, to);
        }
    }

    // Calculate shortest path between two nodes using Dijkstra
    LinkedList<int> getShortestPath(int startNode, int endNode) {
        if (!cityMap) {
            return LinkedList<int>();
        }
        return cityMap->dijkstra(startNode, endNode);
    }
    
    // Get shortest path as vector (for compatibility with existing code)
    vector<int> getShortestPathVector(int startNode, int endNode) {
        LinkedList<int> pathList = getShortestPath(startNode, endNode);
        vector<int> path;
        
        // Convert LinkedList to vector
        auto* current = pathList.getHead();
        while (current != nullptr) {
            path.push_back(current->data);
            current = current->next;
        }
        
        return path;
    }

    // Calculate distance of shortest path
    int getShortestDistance(int startNode, int endNode) {
        if (!cityMap) return -1;
        
        LinkedList<int> path = getShortestPath(startNode, endNode);
        if (path.isEmpty()) return -1;
        
        int distance = 0;
        auto* current = path.getHead();
        auto* next = (current != nullptr) ? current->next : nullptr;
        
        while (current != nullptr && next != nullptr) {
            int weight = cityMap->getEdgeWeight(current->data, next->data);
            if (weight == -1) return -1; // Edge not found
            distance += weight;
            current = next;
            next = next->next;
        }
        return distance;
    }

    // Estimate delivery time based on distance and vehicle type
    int estimateDeliveryTime(int distance, const string& vehicleType) {
        if (distance <= 0) return 0;
        
        // Average speeds in meters per minute
        int speed = 200; // default: 12 km/h
        
        if (vehicleType == "motorcycle" || vehicleType == "motorbike") {
            speed = 400; // 24 km/h
        } else if (vehicleType == "car") {
            speed = 500; // 30 km/h
        } else if (vehicleType == "bike" || vehicleType == "bicycle") {
            speed = 150; // 9 km/h
        } else if (vehicleType == "walking") {
            speed = 80;  // 4.8 km/h
        }
        
        int timeMinutes = distance / speed;
        return max(5, timeMinutes); // Minimum 5 minutes
    }
    
    // Overload for Rider object
    int estimateDeliveryTime(int distance, const Rider& rider) {
        return estimateDeliveryTime(distance, string(rider.vehicle));
    }

    // Find nodes within a certain distance from a location
    LinkedList<int> findNearbyNodes(int location, int maxDistance) {
        LinkedList<int> nearbyNodes;
        if (!cityMap) return nearbyNodes;
        
        // Get all neighbors and check their distances
        LinkedList<int> neighbors = cityMap->getNeighbors(location);
        
        auto* current = neighbors.getHead();
        while (current != nullptr) {
            int nodeId = current->data;
            int distance = getShortestDistance(location, nodeId);
            if (distance != -1 && distance <= maxDistance) {
                nearbyNodes.insertAtEnd(nodeId);
            }
            current = current->next;
        }
        
        return nearbyNodes;
    }

    // Find the closest node to a given location from a list of candidates
    int findClosestNode(int location, const LinkedList<int>& candidateNodes) {
        if (candidateNodes.isEmpty()) return -1;
        
        int closestNode = -1;
        int minDistance = 2147483647; // INT_MAX
        
        auto* current = candidateNodes.getHead();
        while (current != nullptr) {
            int nodeId = current->data;
            int distance = getShortestDistance(location, nodeId);
            
            if (distance != -1 && distance < minDistance) {
                minDistance = distance;
                closestNode = nodeId;
            }
            
            current = current->next;
        }
        
        return closestNode;
    }

    // Check if two locations are directly connected
    bool areDirectlyConnected(int node1, int node2) {
        if (!cityMap) return false;
        return cityMap->hasEdge(node1, node2);
    }

    // Get all neighbors of a location
    LinkedList<int> getNeighbors(int nodeId) {
        if (!cityMap) return LinkedList<int>();
        return cityMap->getNeighbors(nodeId);
    }

    // Print the city map
    void printMap() {
        if (cityMap) {
            cityMap->printGraph();
        } else {
            cout << "City map not initialized.\n";
        }
    }
    
    // Get the graph (for direct access if needed)
    Graph* getGraph() {
        return cityMap;
    }
    
    // Set the graph
    void setGraph(Graph* graph) {
        if (cityMap && ownsGraph) {
            delete cityMap;
        }
        cityMap = graph;
        ownsGraph = false;
    }
    
    // Get edge weight between two nodes
    int getEdgeWeight(int from, int to) {
        if (!cityMap) return -1;
        return cityMap->getEdgeWeight(from, to);
    }
    
    // Validate if a path exists between two nodes
    bool pathExists(int startNode, int endNode) {
        if (!cityMap) return false;
        
        LinkedList<int> path = getShortestPath(startNode, endNode);
        return !path.isEmpty();
    }
    
    // Calculate total distance of a given path
    int calculatePathDistance(const LinkedList<int>& path) {
        if (path.isEmpty()) return 0;
        
        int totalDistance = 0;
        auto* current = path.getHead();
        auto* next = (current != nullptr) ? current->next : nullptr;
        
        while (current != nullptr && next != nullptr) {
            int weight = cityMap->getEdgeWeight(current->data, next->data);
            if (weight == -1) return -1; // Invalid path
            totalDistance += weight;
            current = next;
            next = next->next;
        }
        
        return totalDistance;
    }
    
    // Get alternative route (second shortest) - simplified version
    LinkedList<int> getAlternativeRoute(int startNode, int endNode) {
        if (!cityMap) return LinkedList<int>();
        
        // Get the shortest path
        LinkedList<int> shortestPath = getShortestPath(startNode, endNode);
        if (shortestPath.isEmpty() || shortestPath.getSize() < 2) {
            return LinkedList<int>();
        }
        
        // Simple alternative: remove the first edge of shortest path and find new path
        auto* head = shortestPath.getHead();
        if (head && head->next) {
            int firstNode = head->data;
            int secondNode = head->next->data;
            
            // Temporarily remove this edge
            int weight = cityMap->getEdgeWeight(firstNode, secondNode);
            if (weight != -1) {
                cityMap->removeEdge(firstNode, secondNode);
                
                // Find alternative path
                LinkedList<int> alternativePath = getShortestPath(startNode, endNode);
                
                // Restore the edge
                cityMap->addEdge(firstNode, secondNode, weight);
                
                return alternativePath;
            }
        }
        
        return LinkedList<int>();
    }
    
    // Print a path nicely
    static void printPath(const LinkedList<int>& path) {
        if (path.isEmpty()) {
            cout << "No path found.\n";
            return;
        }
        
        cout << "Path: ";
        auto* current = path.getHead();
        while (current != nullptr) {
            cout << current->data;
            if (current->next != nullptr) {
                cout << " -> ";
            }
            current = current->next;
        }
        cout << "\n";
    }
    
    // Print path with distances
    void printPathWithDistances(const LinkedList<int>& path) {
        if (path.isEmpty()) {
            cout << "No path found.\n";
            return;
        }
        
        cout << "Path with distances:\n";
        auto* current = path.getHead();
        auto* next = (current != nullptr) ? current->next : nullptr;
        int totalDistance = 0;
        
        while (current != nullptr && next != nullptr) {
            int weight = cityMap->getEdgeWeight(current->data, next->data);
            cout << current->data << " --(" << weight << "m)--> ";
            totalDistance += weight;
            
            current = next;
            next = next->next;
        }
        
        if (current != nullptr) {
            cout << current->data << " (Destination)\n";
        }
        cout << "Total Distance: " << totalDistance << " meters\n";
    }
    
    // Calculate route from rider to pickup and then to delivery
    void calculateFullRoute(int riderLocation, int pickupLocation, int deliveryLocation) {
        cout << "\n=== Full Route Calculation ===\n";
        
        // Route 1: Rider to Pickup
        cout << "\n1. Rider to Pickup Location:\n";
        LinkedList<int> riderToPickup = getShortestPath(riderLocation, pickupLocation);
        if (riderToPickup.isEmpty()) {
            cout << "   No path from rider to pickup!\n";
            return;
        }
        printPath(riderToPickup);
        int distanceToPickup = calculatePathDistance(riderToPickup);
        cout << "   Distance: " << distanceToPickup << " meters\n";
        
        // Route 2: Pickup to Delivery
        cout << "\n2. Pickup to Delivery Location:\n";
        LinkedList<int> pickupToDelivery = getShortestPath(pickupLocation, deliveryLocation);
        if (pickupToDelivery.isEmpty()) {
            cout << "   No path from pickup to delivery!\n";
            return;
        }
        printPath(pickupToDelivery);
        int distanceToDelivery = calculatePathDistance(pickupToDelivery);
        cout << "   Distance: " << distanceToDelivery << " meters\n";
        
        // Total
        cout << "\n=== Summary ===\n";
        cout << "Total Distance: " << (distanceToPickup + distanceToDelivery) << " meters\n";
    }
    
    // Calculate route with time estimation
    void calculateRouteWithTime(int riderLocation, int pickupLocation, 
                               int deliveryLocation, const string& vehicleType) {
        cout << "\n=== Route with Time Estimation ===\n";
        
        // Route 1: Rider to Pickup
        int distanceToPickup = getShortestDistance(riderLocation, pickupLocation);
        if (distanceToPickup == -1) {
            cout << "No path from rider to pickup!\n";
            return;
        }
        
        int timeToPickup = estimateDeliveryTime(distanceToPickup, vehicleType);
        cout << "Rider to Pickup: " << distanceToPickup << "m (~" << timeToPickup << " min)\n";
        
        // Route 2: Pickup to Delivery
        int distanceToDelivery = getShortestDistance(pickupLocation, deliveryLocation);
        if (distanceToDelivery == -1) {
            cout << "No path from pickup to delivery!\n";
            return;
        }
        
        int timeToDelivery = estimateDeliveryTime(distanceToDelivery, vehicleType);
        cout << "Pickup to Delivery: " << distanceToDelivery << "m (~" << timeToDelivery << " min)\n";
        
        cout << "\nTotal: " << (distanceToPickup + distanceToDelivery) << "m (~" 
             << (timeToPickup + timeToDelivery) << " min)\n";
    }
    
    // Create a sample city map for testing
    void createSampleCity(int numNodes = 10) {
        if (!cityMap) return;
        
        // Add locations (0 to numNodes-1)
        for (int i = 0; i < numNodes; i++) {
            addLocation(i);
        }
        
        // Add roads with distances in meters
        if (numNodes >= 10) {
            addRoad(0, 1, 500);   // Residential area to downtown
            addRoad(0, 2, 800);   // Residential area to mall
            addRoad(1, 3, 600);   // Downtown to business district
            addRoad(2, 3, 400);   // Mall to business district
            addRoad(3, 4, 700);   // Business district to university
            addRoad(4, 5, 300);   // University to stadium
            addRoad(5, 6, 900);   // Stadium to airport
            addRoad(6, 7, 1200);  // Airport to industrial area
            addRoad(7, 8, 550);   // Industrial area to port
            addRoad(8, 9, 450);   // Port to shopping district
            addRoad(9, 0, 1000);  // Shopping district to residential area (loop)
            addRoad(1, 5, 1100);  // Downtown to stadium (shortcut)
            addRoad(3, 7, 1300);  // Business district to industrial area
            addRoad(2, 5, 850);   // Mall to stadium
            addRoad(4, 8, 1150);  // University to port
        } else {
            // Smaller graph for testing
            for (int i = 0; i < numNodes - 1; i++) {
                addRoad(i, i + 1, 500 + (i * 100));
            }
            if (numNodes > 3) {
                addRoad(0, numNodes - 1, 1000); // Create a loop
                addRoad(1, numNodes - 2, 800);  // Add a shortcut
            }
        }
        
        cout << "Sample city map created with " << numNodes << " locations.\n";
    }
    int getNumLocations() const {
    if (!cityMap) return 0;
    
    // Count nodes by checking which ones exist
    int count = 0;
    for (int i = 0; i < 500; i++) { // Assuming max 500 nodes
        if (cityMap->hasEdge(i, i) || cityMap->getNeighbors(i).getSize() > 0) {
            count++;
        }
    }
    return count;
}

    // Check if graph is initialized
    bool isInitialized() const {
        return cityMap != nullptr;
    }
    
    // Get distance between two directly connected nodes
    int getDirectDistance(int from, int to) {
        if (!cityMap) return -1;
        return cityMap->getEdgeWeight(from, to);
    }
};

#endif // ROUTING_SERVICE_H