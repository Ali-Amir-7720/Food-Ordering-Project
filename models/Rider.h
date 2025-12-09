#pragma once
#ifndef RIDER_H
#define RIDER_H
#include <iostream>
#include <string>
#include <utility>
using namespace std;

class Rider {
public:
    int id;
    string name;
    string phone;
    string vehicle; // "bike", "motorcycle", "car"
    int location;   // Graph node ID
    string status;  // "available", "busy", "offline"
    double rating;
    int completedDeliveries;
    
    // Add this constructor to your Rider class in Rider.h
Rider(int _id, const string& _name, const string& _status)
    : id(_id), name(_name), phone(""), vehicle("bike"), location(0), status(_status) {}
    Rider() : id(0), name(""), phone(""), vehicle(""), location(0), 
              status("available"), rating(0.0), completedDeliveries(0) {}
    
    Rider(int _id, const string& _name, const string& _phone, 
          const string& _vehicle, int _location)
        : id(_id), name(_name), phone(_phone), vehicle(_vehicle), 
          location(_location), status("available"), rating(0.0), completedDeliveries(0) {}
    
    void assignOrder(int orderId) {
        status = "busy";
        // In real implementation, track current order
    }
    string getStatus() const {
        return status;
    }
    string getName() const {
        return name;
    }
    void completeDelivery(bool success) {
        completedDeliveries++;
        if (success) {
            rating = (rating * (completedDeliveries - 1) + 5.0) / completedDeliveries;
        }
        status = "available";
    }
    
    void updateLocation(int newLocation) {
        location = newLocation;
    }
    
    double calculatePriority(int pickupLocation, int deliveryLocation) const {
        // Simple priority calculation based on distance to pickup
        // In real system, use actual distance calculation
        double distanceToPickup = abs(location - pickupLocation);
        return 100.0 / (distanceToPickup + 1); // Closer riders get higher priority
    }
    
    friend ostream& operator<<(ostream& os, const Rider& rider) {
        os << "Rider{ID: " << rider.id 
           << ", Name: " << rider.name 
           << ", Status: " << rider.status 
           << ", Location: " << rider.location 
           << ", Vehicle: " << rider.vehicle 
           << ", Rating: " << rider.rating << "}";
        return os;
    }
};

#endif