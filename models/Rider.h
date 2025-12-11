#pragma once
#ifndef RIDER_H
#define RIDER_H
#include <iostream>
#include <string>
#include <cstring>
using namespace std;

class Rider {
public:
    int id;
    char name[100];
    char phone[20];
    char vehicle[20];  // "bike", "motorcycle", "car"
    int location;      // Graph node ID
    char status[20];   // "available", "busy", "offline"
    double rating;
    int completedDeliveries;
    
    // Default constructor
    Rider() : id(0), location(0), rating(0.0), completedDeliveries(0) {
        memset(name, 0, sizeof(name));
        memset(phone, 0, sizeof(phone));
        memset(vehicle, 0, sizeof(vehicle));
        memset(status, 0, sizeof(status));
        strcpy(status, "available");
        strcpy(vehicle, "bike");
    }
    
    // Simple constructor for initialization
    Rider(int _id, const string& _name, const string& _status)
        : id(_id), location(0), rating(0.0), completedDeliveries(0) {
        
        strncpy(name, _name.c_str(), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        
        memset(phone, 0, sizeof(phone));
        
        strcpy(vehicle, "bike");
        
        strncpy(status, _status.c_str(), sizeof(status) - 1);
        status[sizeof(status) - 1] = '\0';
    }
    
    // Full constructor
    Rider(int _id, const string& _name, const string& _phone, 
          const string& _vehicle, int _location)
        : id(_id), location(_location), rating(0.0), completedDeliveries(0) {
        
        strncpy(name, _name.c_str(), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        
        strncpy(phone, _phone.c_str(), sizeof(phone) - 1);
        phone[sizeof(phone) - 1] = '\0';
        
        strncpy(vehicle, _vehicle.c_str(), sizeof(vehicle) - 1);
        vehicle[sizeof(vehicle) - 1] = '\0';
        
        strcpy(status, "available");
    }
    
    // Helper methods
    string getName() const { return string(name); }
    string getStatus() const { return string(status); }
    string getPhone() const { return string(phone); }
    string getVehicle() const { return string(vehicle); }
    int getId() const { return id; }
    int getLocation() const { return location; }
    double getRating() const { return rating; }
    
    void assignOrder(int orderId) {
        strcpy(status, "busy");
    }
    
    void completeDelivery(bool success) {
        completedDeliveries++;
        if (success) {
            rating = (rating * (completedDeliveries - 1) + 5.0) / completedDeliveries;
        }
        strcpy(status, "available");
    }
    
    void updateLocation(int newLocation) {
        location = newLocation;
    }
    
    void setStatus(const string& s) {
        strncpy(status, s.c_str(), sizeof(status) - 1);
        status[sizeof(status) - 1] = '\0';
    }
    
    double calculatePriority(int pickupLocation, int deliveryLocation) const {
        double distanceToPickup = abs(location - pickupLocation);
        return 100.0 / (distanceToPickup + 1);
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

#endif // RIDER_H
