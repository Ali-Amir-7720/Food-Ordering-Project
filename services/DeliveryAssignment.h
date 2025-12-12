#pragma once
#ifndef DELIVERY_ASSIGNMENT_H
#define DELIVERY_ASSIGNMENT_H

#include "CityGraph.h"
#include "models/Rider.h"
#include "models/Order.h"
#include "models/Restaurant.h"
#include "dataStructures/LinkedList.h"
#include <climits>

struct DeliveryRoute {
    int riderId;
    int restaurantLocation;
    int customerLocation;
    LinkedList<int> pathToRestaurant;
    LinkedList<int> pathToCustomer;
    LinkedList<int> completePath;  // For multi-restaurant
    int totalDistance;
    int estimatedTime;  // minutes
    
    DeliveryRoute() : riderId(-1), restaurantLocation(0), customerLocation(0), 
                     totalDistance(0), estimatedTime(0) {}
};

class DeliveryAssignmentSystem {
private:
    CityGraph* cityGraph;
    
    // Calculate estimated time based on distance and vehicle type
    int calculateDeliveryTime(int distance, const string& vehicleType) {
        // Average speeds: bike=300m/min, motorcycle=600m/min, car=800m/min
        int speed;
        if (vehicleType == "bike") speed = 300;
        else if (vehicleType == "motorcycle") speed = 600;
        else if (vehicleType == "car") speed = 800;
        else speed = 400; // default
        
        int travelTime = distance / speed;
        int pickupTime = 5;  // 5 minutes to collect from restaurant
        int deliveryTime = 2; // 2 minutes to deliver
        
        return travelTime + pickupTime + deliveryTime;
    }
    
public:
    DeliveryAssignmentSystem(CityGraph* graph) : cityGraph(graph) {}
    
    // Find best rider for a single restaurant order
    DeliveryRoute assignRiderToOrder(const Order& order, 
                                     const LinkedList<Rider>& availableRiders,
                                     int restaurantLocation) {
        DeliveryRoute bestRoute;
        int minTotalDistance = INT_MAX;
        
        auto* riderNode = availableRiders.getHead();
        while (riderNode != nullptr) {
            const Rider& rider = riderNode->data;
            
            if (strcmp(rider.status, "available") != 0 && 
                strcmp(rider.status, "Active") != 0) {
                riderNode = riderNode->next;
                continue;
            }
            
            // Calculate path from rider to restaurant
            auto toRestaurant = cityGraph->findShortestPath(rider.location, restaurantLocation);
            
            // Calculate path from restaurant to customer
            auto toCustomer = cityGraph->findShortestPath(restaurantLocation, order.deliveryLocation);
            
            if (toRestaurant.first.isEmpty() || toCustomer.first.isEmpty()) {
                riderNode = riderNode->next;
                continue; // No valid path
            }
            
            int totalDist = toRestaurant.second + toCustomer.second;
            
            if (totalDist < minTotalDistance) {
                minTotalDistance = totalDist;
                
                bestRoute.riderId = rider.id;
                bestRoute.restaurantLocation = restaurantLocation;
                bestRoute.customerLocation = order.deliveryLocation;
                bestRoute.pathToRestaurant = toRestaurant.first;
                bestRoute.pathToCustomer = toCustomer.first;
                bestRoute.totalDistance = totalDist;
                bestRoute.estimatedTime = calculateDeliveryTime(totalDist, rider.getVehicle());
            }
            
            riderNode = riderNode->next;
        }
        
        return bestRoute;
    }
    
    // For multi-restaurant orders
    DeliveryRoute assignRiderToMultiRestaurantOrder(const Order& order,
                                                    const LinkedList<Rider>& availableRiders,
                                                    const LinkedList<int>& restaurantLocations) {
        DeliveryRoute bestRoute;
        int minTotalDistance = INT_MAX;
        
        auto* riderNode = availableRiders.getHead();
        while (riderNode != nullptr) {
            const Rider& rider = riderNode->data;
            
            if (strcmp(rider.status, "available") != 0 && 
                strcmp(rider.status, "Active") != 0) {
                riderNode = riderNode->next;
                continue;
            }
            
            // Calculate total path: rider -> restaurants -> customer
            int totalDist = 0;
            LinkedList<int> completePath;
            int currentLoc = rider.location;
            bool pathValid = true;
            
            // Visit each restaurant
            auto* restNode = restaurantLocations.getHead();
            while (restNode != nullptr) {
                int restLoc = restNode->data;
                auto segment = cityGraph->findShortestPath(currentLoc, restLoc);
                
                if (segment.first.isEmpty()) {
                    pathValid = false;
                    break;
                }
                
                totalDist += segment.second;
                
                // Add path segment
                auto* pathNode = segment.first.getHead();
                while (pathNode != nullptr) {
                    // Skip duplicate nodes
                    if (completePath.isEmpty() || 
                        completePath.getTail()->data != pathNode->data) {
                        completePath.insertAtEnd(pathNode->data);
                    }
                    pathNode = pathNode->next;
                }
                
                currentLoc = restLoc;
                restNode = restNode->next;
            }
            
            if (!pathValid) {
                riderNode = riderNode->next;
                continue;
            }
            
            // Finally, go to customer
            auto toCustomer = cityGraph->findShortestPath(currentLoc, order.deliveryLocation);
            if (toCustomer.first.isEmpty()) {
                riderNode = riderNode->next;
                continue;
            }
            
            totalDist += toCustomer.second;
            
            // Add final segment
            auto* pathNode = toCustomer.first.getHead();
            bool skipFirst = true;
            while (pathNode != nullptr) {
                if (skipFirst) {
                    skipFirst = false;
                    pathNode = pathNode->next;
                    continue;
                }
                completePath.insertAtEnd(pathNode->data);
                pathNode = pathNode->next;
            }
            
            if (totalDist < minTotalDistance) {
                minTotalDistance = totalDist;
                
                bestRoute.riderId = rider.id;
                bestRoute.customerLocation = order.deliveryLocation;
                bestRoute.completePath = completePath;
                bestRoute.totalDistance = totalDist;
                bestRoute.estimatedTime = calculateDeliveryTime(totalDist, rider.getVehicle());
            }
            
            riderNode = riderNode->next;
        }
        
        return bestRoute;
    }
    
    // Print delivery route details
    void printDeliveryRoute(const DeliveryRoute& route, 
                           const Rider& rider,
                           bool isMultiRestaurant = false) {
        cout << "\n╔════════════════════════════════════════╗\n";
        cout << "║    🚀 DELIVERY ROUTE ASSIGNED         ║\n";
        cout << "╚════════════════════════════════════════╝\n";
        
        cout << "\n👤 Rider: " << rider.getName() << " (ID: " << rider.getId() << ")\n";
        cout << "🚗 Vehicle: " << rider.getVehicle() << "\n";
        cout << "📍 Current Location: " << cityGraph->getLocationName(rider.location) 
             << " [" << rider.location << "]\n";
        
        cout << "\n📊 Delivery Details:\n";
        cout << "   Total Distance: " << route.totalDistance << " meters\n";
        cout << "   Estimated Time: " << route.estimatedTime << " minutes\n";
        
        if (isMultiRestaurant) {
            cout << "\n=== 🛣️ COMPLETE ROUTE (Multi-Restaurant) ===\n";
            cityGraph->printPathDetails(route.completePath, route.totalDistance);
        } else {
            cout << "\n=== 🛣️ ROUTE TO RESTAURANT ===\n";
            int distToRestaurant = 0;
            auto* node = route.pathToRestaurant.getHead();
            while (node != nullptr && node->next != nullptr) {
                distToRestaurant += cityGraph->getDirectDistance(node->data, node->next->data);
                node = node->next;
            }
            cityGraph->printPathDetails(route.pathToRestaurant, distToRestaurant);
            
            cout << "\n=== 🛣️ ROUTE TO CUSTOMER ===\n";
            int distToCustomer = 0;
            node = route.pathToCustomer.getHead();
            while (node != nullptr && node->next != nullptr) {
                distToCustomer += cityGraph->getDirectDistance(node->data, node->next->data);
                node = node->next;
            }
            cityGraph->printPathDetails(route.pathToCustomer, distToCustomer);
        }
        
        cout << "\n✅ Rider has been notified and is en route!\n";
    }
};

#endif // DELIVERY_ASSIGNMEN