#pragma once
#ifndef RIDERSERVICE_H
#define RIDERSERVICE_H
#include <iostream>
#include "../models/Rider.h"
#include "../dataStructures/HashTable.h"
#include "../dataStructures/LinkedList.h"
#include "../dataStructures/PriorityQueue.h"
#include "../dataStructures/BTree.h"
using namespace std;

class RiderService {
private:
    HashTable<Rider> riders;
    PriorityQueue<Rider*>* availableRiders;
    PersistentBTree<Rider>* persistentRiders;

public:
    RiderService() : availableRiders(nullptr), persistentRiders(nullptr) {}
    
    RiderService(PersistentBTree<Rider>* persistent, PriorityQueue<Rider*>* riderQueue = nullptr) 
        : persistentRiders(persistent), availableRiders(riderQueue) {
        // Load riders from persistent storage to cache
        if (persistentRiders) {
            // This would need a traversal method in PersistentBTree
        }
    }

    bool addRider(const Rider& r) {
        if (riders.searchTable(r.id) != nullptr) return false;
        
        riders.insertItem(r.id, r);
        
        if (persistentRiders) {
            persistentRiders->insert(r);
        }
        
        if (availableRiders && r.status == "available") {
            Rider* riderPtr = riders.searchTable(r.id);
            if (riderPtr) {
                int priority = calculatePriority(riderPtr);
                availableRiders->enqueue(riderPtr, priority);
            }
        }
        return true;
    }

    bool removeRider(int riderId) {
        if (riders.searchTable(riderId) == nullptr) return false;
        
        // Remove from available riders if present
        if (availableRiders) {
            Rider* rider = riders.searchTable(riderId);
            // Would need to search and remove from priority queue
            // For simplicity, we'll clear and rebuild if needed
        }
        
        riders.removeItem(riderId);
        
        if (persistentRiders) {
            Rider dummy(riderId, "", "", "", 0);
            persistentRiders->remove(dummy);
        }
        return true;
    }

    Rider* getRider(int riderId) {
        return riders.searchTable(riderId);
    }

    bool updateRiderLocation(int riderId, int newLocation) {
        Rider* r = riders.searchTable(riderId);
        if (!r) return false;
        r->location = newLocation;
        
        // Update priority if in available riders
        if (availableRiders && r->status == "available") {
            int priority = calculatePriority(r);
            availableRiders->updatePriority(r, priority);
        }
        
        // Update persistent storage
        if (persistentRiders) {
            persistentRiders->remove(*r);
            persistentRiders->insert(*r);
        }
        
        return true;
    }

    bool updateRiderStatus(int riderId, const string& status) {
        Rider* r = riders.searchTable(riderId);
        if (!r) return false;
        
        string oldStatus = r->status;
        r->status = status;
        
        // Update available riders queue
        if (availableRiders) {
            if (status == "available" && oldStatus != "available") {
                int priority = calculatePriority(r);
                availableRiders->enqueue(r, priority);
            } else if (status != "available" && oldStatus == "available") {
                // Remove from available riders (simplified)
                // In real implementation, would need to find and remove
            }
        }
        
        if (persistentRiders) {
            persistentRiders->remove(*r);
            persistentRiders->insert(*r);
        }
        
        return true;
    }

    LinkedList<Rider> getAvailableRiders() {
        LinkedList<Rider> available;
        riders.traverse([&](int id, Rider& r) {
            if (r.status == "available") available.push_back(r);
        });
        return available;
    }

    Rider* findBestRider(int pickupLocation, int deliveryLocation) {
        if (!availableRiders || availableRiders->isEmpty()) {
            // Fallback: find first available rider
            LinkedList<Rider> available = getAvailableRiders();
            if (available.isEmpty()) return nullptr;
            
            // Return rider closest to pickup
            Rider* best = nullptr;
            double bestPriority = -1;
            
            available.traverse([&](const Rider& r) {
                double priority = r.calculatePriority(pickupLocation, deliveryLocation);
                if (priority > bestPriority) {
                    bestPriority = priority;
                    best = const_cast<Rider*>(&r);
                }
            });
            
            return best;
        }
        
        // Get highest priority rider from queue
        return availableRiders->peek();
    }

    void printAllRiders() {
        cout << "=== All Riders ===\n";
        riders.traverse([](int id, Rider& r) {
            cout << "ID: " << id
                 << " | Name: " << r.name
                 << " | Status: " << r.status
                 << " | Location: " << r.location
                 << " | Vehicle: " << r.vehicle
                 << " | Rating: " << r.rating << endl;
        });
    }
     public:
    // ... existing code ...
    
    friend ostream& operator<<(ostream& os, const RiderService& service) {
        os << "=== Rider Service ===\n";
        // os << "Total Riders: " << service.riders.getCount() << "\n";
        
        int availableCount = 0;
        service.riders.traverse([&](int id, const Rider& r) {
            if (r.status == "available") availableCount++;
        });
        os << "Available Riders: " << availableCount << "\n";
        
        service.riders.traverse([&](int id, const Rider& r) {
            os << "  ID: " << id 
               << " | Name: " << r.name
               << " | Status: " << r.status
               << " | Location: " << r.location
               << " | Vehicle: " << r.vehicle
               << " | Rating: " << r.rating << "\n";
        });
        
        return os;
    }
    
private:
    int calculatePriority(Rider* rider) {
        // Lower number = higher priority
        // Base priority on rating (higher rating = lower priority number)
        int basePriority = 100 - (int)(rider->rating * 10);
        return max(1, basePriority); // Ensure positive priority
    }
};

#endif