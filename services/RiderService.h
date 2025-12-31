#pragma once
#ifndef RIDERSERVICE_H
#define RIDERSERVICE_H
#include <iostream>
#include <vector>
#include <cstring>
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

    int calculatePriority(Rider* rider) {
        // Lower number = higher priority
        // Base priority on rating (higher rating = lower priority number)
        int basePriority = 100 - (int)(rider->rating * 10);
        return max(1, basePriority);
    }
    
    // Helper to remove rider from priority queue
    void removeFromAvailableQueue(Rider* rider) {
        if (!availableRiders || !rider) return;
        
        // Get all elements from queue
        vector<pair<Rider*, int>> tempItems;
        
        while (!availableRiders->isEmpty()) {
            Rider* current = availableRiders->peek();
            int priority = availableRiders->peekPriority();
            availableRiders->dequeue();
            
            if (current && current->id != rider->id) {
                tempItems.push_back(make_pair(current, priority));
            }
        }
        
        // Rebuild queue without the removed rider
        for (const auto& item : tempItems) {
            availableRiders->enqueue(item.first, item.second);
        }
    }

public:
    RiderService() : availableRiders(nullptr), persistentRiders(nullptr) {}
    
    RiderService(PersistentBTree<Rider>* persistent, PriorityQueue<Rider*>* riderQueue = nullptr) 
        : persistentRiders(persistent), availableRiders(riderQueue) {
        // Load riders from persistent storage to cache
        if (persistentRiders && !persistentRiders->isEmpty()) {
            loadRidersFromPersistent();
        }
    }
    
    ~RiderService() {
        // Note: We don't delete availableRiders or persistentRiders
        // as they are externally managed pointers
    }

    // Load all riders from persistent storage into memory
    void loadRidersFromPersistent() {
        if (!persistentRiders) return;
        
        // Get all riders from B-tree
        vector<Rider> allRiders = persistentRiders->getAllKeys();
        
        for (const Rider& r : allRiders) {
            riders.insertItem(r.id, r);
            
            if (availableRiders && strcmp(r.status, "available") == 0) {
                Rider* riderPtr = riders.searchTable(r.id);
                if (riderPtr) {
                    int priority = calculatePriority(riderPtr);
                    availableRiders->enqueue(riderPtr, priority);
                }
            }
        }
        
        cout << "Loaded " << allRiders.size() << " riders from persistent storage.\n";
    }

    bool addRider(const Rider& r) {
        if (riders.searchTable(r.id) != nullptr) {
            cout << "ERROR: Rider with ID " << r.id << " already exists.\n";
            return false;
        }
        
        riders.insertItem(r.id, r);
        
        if (persistentRiders) {
            persistentRiders->insert(r);
        }
        
        if (availableRiders && strcmp(r.status, "available") == 0) {
            Rider* riderPtr = riders.searchTable(r.id);
            if (riderPtr) {
                int priority = calculatePriority(riderPtr);
                availableRiders->enqueue(riderPtr, priority);
            }
        }
        
        cout << "Rider " << r.name << " (ID: " << r.id << ") added successfully.\n";
        return true;
    }

    bool removeRider(int riderId) {
        Rider* rider = riders.searchTable(riderId);
        if (rider == nullptr) {
            cout << "ERROR: Rider with ID " << riderId << " not found.\n";
            return false;
        }
        
        // Remove from available riders if present
        if (availableRiders && strcmp(rider->status, "available") == 0) {
            removeFromAvailableQueue(rider);
        }
        
        // Remove from persistent storage
        if (persistentRiders) {
            persistentRiders->remove(*rider);
        }
        
        // Remove from hash table
        riders.removeItem(riderId);
        
        cout << "Rider ID " << riderId << " removed successfully.\n";
        return true;
    }

    Rider* getRider(int riderId) {
        return riders.searchTable(riderId);
    }
    
    const Rider* getRider(int riderId) const {
        return riders.searchTable(riderId);
    }

    bool updateRiderLocation(int riderId, int newLocation) {
        Rider* r = riders.searchTable(riderId);
        if (!r) {
            cout << "ERROR: Rider with ID " << riderId << " not found.\n";
            return false;
        }
        
        int oldLocation = r->location;
        r->location = newLocation;
        
        // Update priority if in available riders
        if (availableRiders && strcmp(r->status, "available") == 0) {
            // Remove and re-add with new priority
            removeFromAvailableQueue(r);
            int priority = calculatePriority(r);
            availableRiders->enqueue(r, priority);
        }
        
        // Update persistent storage
        if (persistentRiders) {
            Rider temp = *r;
            temp.location = oldLocation; // Create temp with old location for removal
            persistentRiders->remove(temp);
            persistentRiders->insert(*r);
        }
        
        cout << "Rider ID " << riderId << " location updated from " 
             << oldLocation << " to " << newLocation << ".\n";
        return true;
    }
    Rider* findRiderByEmail(const string& email) {
        vector<Rider> allRiders = getAllRiders(); 
        
        for (Rider& rider : allRiders) {
            if (rider.getEmail() == email) {
                return &rider;
            }
        }
        return nullptr;
    }
    bool updateRiderStatus(int riderId, const string& status) {
        Rider* r = riders.searchTable(riderId);
        if (!r) {
            cout << "ERROR: Rider with ID " << riderId << " not found.\n";
            return false;
        }
        
        char oldStatus[20];
        strcpy(oldStatus, r->status);
        
        if (strcmp(oldStatus, status.c_str()) == 0) {
            cout << "Rider ID " << riderId << " already has status: " << status << "\n";
            return true;
        }
        
        strncpy(r->status, status.c_str(), sizeof(r->status) - 1);
        r->status[sizeof(r->status) - 1] = '\0';
        
        // Update available riders queue
        if (availableRiders) {
            if (status == "available" && strcmp(oldStatus, "available") != 0) {
                // Add to available queue
                int priority = calculatePriority(r);
                availableRiders->enqueue(r, priority);
            } else if (status != "available" && strcmp(oldStatus, "available") == 0) {
                // Remove from available queue
                removeFromAvailableQueue(r);
            }
        }
        
        // Update persistent storage
        if (persistentRiders) {
            Rider temp = *r;
            strcpy(temp.status, oldStatus); // Create temp with old status for removal
            persistentRiders->remove(temp);
            persistentRiders->insert(*r);
        }
        
        cout << "Rider ID " << riderId << " status updated from '" 
             << oldStatus << "' to '" << status << "'.\n";
        return true;
    }

    LinkedList<Rider> getAvailableRiders() {
        LinkedList<Rider> available;
        riders.traverse([&](int id, Rider& r) {
            if (strcmp(r.status, "available") == 0) {
                available.push_back(r);
            }
        });
        return available;
    }

    Rider* findBestRider(int pickupLocation, int deliveryLocation) {
        if (!availableRiders || availableRiders->isEmpty()) {
            // Fallback: find best available rider manually
            LinkedList<Rider> available = getAvailableRiders();
            if (available.isEmpty()) {
                cout << "No available riders found.\n";
                return nullptr;
            }
            
            // Find rider with best priority score
            Rider* best = nullptr;
            double bestScore = -1;
            
            auto* node = available.getHead();
            while (node != nullptr) {
                double score = node->data.calculatePriority(pickupLocation, deliveryLocation);
                if (score > bestScore) {
                    bestScore = score;
                    // Get pointer from hash table
                    best = riders.searchTable(node->data.id);
                }
                node = node->next;
            }
            
            if (best) {
                cout << "Best rider (fallback): " << best->name 
                     << " (ID: " << best->id << ") with score: " << bestScore << "\n";
            }
            return best;
        }
        
        // Get highest priority rider from queue
        Rider* best = availableRiders->peek();
        if (best) {
            cout << "Best rider from queue: " << best->name 
                 << " (ID: " << best->id 
                 << ", Priority: " << availableRiders->peekPriority() << ")\n";
        }
        return best;
    }
    
    // Assign rider to an order
    bool assignRiderToOrder(int riderId, int orderId) {
        Rider* rider = getRider(riderId);
        if (!rider) {
            cout << "ERROR: Rider with ID " << riderId << " not found.\n";
            return false;
        }
        
        if (strcmp(rider->status, "available") != 0) {
            cout << "ERROR: Rider ID " << riderId << " is not available (Status: " 
                 << rider->status << ").\n";
            return false;
        }
        
        // Update status to busy
        cout << "Assigning Rider " << rider->name << " (ID: " << riderId 
             << ") to Order " << orderId << ".\n";
        return updateRiderStatus(riderId, "busy");
    }
    
    // Complete delivery and make rider available
    bool completeDelivery(int riderId, int newLocation) {
        Rider* rider = getRider(riderId);
        if (!rider) {
            cout << "ERROR: Rider with ID " << riderId << " not found.\n";
            return false;
        }
        
        cout << "Completing delivery for Rider " << rider->name << " (ID: " << riderId << ").\n";
        
        // Update location
        if (!updateRiderLocation(riderId, newLocation)) {
            return false;
        }
        
        // Make available again
        return updateRiderStatus(riderId, "available");
    }

    void printAllRiders() {
        cout << "\n=== All Riders ===\n";
        if (riders.isEmpty()) {
            cout << "No riders in the system.\n\n";
            return;
        }
        
        int count = 0;
        riders.traverse([&count](int id, Rider& r) {
            count++;
            cout << "ID: " << id
                 << " | Name: " << r.name
                 << " | Status: " << r.status
                 << " | Location: " << r.location
                 << " | Vehicle: " << r.vehicle
                 << " | Rating: " << r.rating << endl;
        });
        cout << "Total riders: " << count << "\n\n";
    }
    
    void printAvailableRiders() {
        cout << "\n=== Available Riders ===\n";
        LinkedList<Rider> available = getAvailableRiders();
        if (available.isEmpty()) {
            cout << "No available riders.\n\n";
            return;
        }
        
        auto* node = available.getHead();
        int count = 0;
        while (node != nullptr) {
            count++;
            const Rider& r = node->data;
            cout << "ID: " << r.id
                 << " | Name: " << r.name
                 << " | Location: " << r.location
                 << " | Vehicle: " << r.vehicle
                 << " | Rating: " << r.rating << endl;
            node = node->next;
        }
        cout << "Total available: " << count << "\n\n";
    }
    
    // Get rider count
    int getRiderCount() const {
        return riders.getSize();
    }
    
    // Get available rider count
    int getAvailableRiderCount() const {
        int count = 0;
        riders.traverse([&count](int id, const Rider& r) {
            if (strcmp(r.status, "available") == 0) count++;
        });
        return count;
    }
    
    // Find riders by vehicle type
    LinkedList<Rider> findRidersByVehicle(const string& vehicle) {
        LinkedList<Rider> result;
        riders.traverse([&](int id, Rider& r) {
            if (strcmp(r.vehicle, vehicle.c_str()) == 0) {
                result.push_back(r);
            }
        });
        return result;
    }
    
    // Find riders above certain rating
    LinkedList<Rider> findRidersByMinRating(double minRating) {
        LinkedList<Rider> result;
        riders.traverse([&](int id, Rider& r) {
            if (r.rating >= minRating) {
                result.push_back(r);
            }
        });
        return result;
    }
    
    // Update rider rating
    bool updateRiderRating(int riderId, double newRating) {
        Rider* r = riders.searchTable(riderId);
        if (!r) {
            cout << "ERROR: Rider with ID " << riderId << " not found.\n";
            return false;
        }
        
        if (newRating < 0.0 || newRating > 5.0) {
            cout << "ERROR: Invalid rating. Must be between 0.0 and 5.0.\n";
            return false;
        }
        
        double oldRating = r->rating;
        r->rating = newRating;
        
        // Update priority if in available queue
        if (availableRiders && strcmp(r->status, "available") == 0) {
            removeFromAvailableQueue(r);
            int priority = calculatePriority(r);
            availableRiders->enqueue(r, priority);
        }
        
        // Update persistent storage
        if (persistentRiders) {
            Rider temp = *r;
            temp.rating = oldRating;
            persistentRiders->remove(temp);
            persistentRiders->insert(*r);
        }
        
        cout << "Rider ID " << riderId << " rating updated from " 
             << oldRating << " to " << newRating << ".\n";
        return true;
    }
    
    // Update rider vehicle
    bool updateRiderVehicle(int riderId, const string& newVehicle) {
        Rider* r = riders.searchTable(riderId);
        if (!r) {
            cout << "ERROR: Rider with ID " << riderId << " not found.\n";
            return false;
        }
        
        char oldVehicle[20];
        strcpy(oldVehicle, r->vehicle);
        
        strncpy(r->vehicle, newVehicle.c_str(), sizeof(r->vehicle) - 1);
        r->vehicle[sizeof(r->vehicle) - 1] = '\0';
        
        // Update persistent storage
        if (persistentRiders) {
            Rider temp = *r;
            strcpy(temp.vehicle, oldVehicle);
            persistentRiders->remove(temp);
            persistentRiders->insert(*r);
        }
        
        cout << "Rider ID " << riderId << " vehicle updated from '" 
             << oldVehicle << "' to '" << newVehicle << "'.\n";
        return true;
    }
    
    // Get statistics
    void printStatistics() {
        cout << "\n=== Rider Service Statistics ===\n";
        cout << "Total Riders: " << getRiderCount() << "\n";
        cout << "Available Riders: " << getAvailableRiderCount() << "\n";
        
        int busyCount = 0;
        int offlineCount = 0;
        riders.traverse([&](int id, const Rider& r) {
            if (strcmp(r.status, "busy") == 0) busyCount++;
            else if (strcmp(r.status, "offline") == 0) offlineCount++;
        });
        
        cout << "Busy Riders: " << busyCount << "\n";
        cout << "Offline Riders: " << offlineCount << "\n\n";
    }
    
    // Search rider by name
    Rider* findRiderByName(const string& name) {
        Rider* found = nullptr;
        riders.traverse([&](int id, Rider& r) {
            if (strcmp(r.name, name.c_str()) == 0) {
                found = &r;
            }
        });
        return found;
    }
    
    // Get all riders as vector
    vector<Rider> getAllRiders() {
        vector<Rider> allRiders;
        riders.traverse([&](int id, Rider& r) {
            allRiders.push_back(r);
        });
        return allRiders;
    }
    
    // Clear all riders
    void clearAllRiders() {
        riders.clear();
        if (availableRiders) {
            availableRiders->clear();
        }
        if (persistentRiders) {
            persistentRiders->clear();
        }
        cout << "All riders cleared from the system.\n";
    }
    
    // Friend operator for output
    friend ostream& operator<<(ostream& os, RiderService& service) {
        os << "\n=== Rider Service Summary ===\n";
        os << "Total Riders: " << service.getRiderCount() << "\n";
        os << "Available Riders: " << service.getAvailableRiderCount() << "\n\n";
        
        if (service.riders.isEmpty()) {
            os << "No riders in the system.\n";
            return os;
        }
        
        os << "Rider Details:\n";
        service.riders.traverse([&](int id, Rider& r) {
            os << "  ID: " << id 
               << " | Name: " << r.name
               << " | Status: " << r.status
               << " | Location: " << r.location
               << " | Vehicle: " << r.vehicle
               << " | Rating: " << r.rating << "\n";
        });
        os << "\n";
        
        return os;
    }
};

#endif