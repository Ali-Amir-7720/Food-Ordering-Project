// #pragma once
// #ifndef RESTAURANT_H
// #define RESTAURANT_H

// #include <iostream>
// #include <string>
// #include <vector>
// #include <cstring>
// #include <algorithm>
// #include "MenuItem.h"
// #include "../dataStructures/HashTable.h"

// using namespace std;

// class Restaurant {
// public:
//     int restaurantId;
//     char name[100];
//     char address[200];
//     char phone[20];
//     char cuisine[50];
//     int locationNode;
//     double rating;
//     int deliveryTime;
//     vector<int> menuItemIds;  
    
//     // Serialized menu items
//     int menuItemCount;
//     MenuItem menuItems[50];  // Fixed array for menu items
    
//     // Additional members needed for RestaurantService
//     vector<int> menultemIds;      // List of menu item IDs
//     vector<MenuItem> popularItems; // Popular items list
//     vector<bool> menuItemLookup;  // Quick lookup for menu items by ID
    
//     Restaurant() : restaurantId(-1), locationNode(0), rating(0.0), 
//                   deliveryTime(0), menuItemCount(0) {
//         memset(name, 0, sizeof(name));
//         memset(address, 0, sizeof(address));
//         memset(phone, 0, sizeof(phone));
//         memset(cuisine, 0, sizeof(cuisine));
        
//         // Initialize new members
//         menultemIds.clear();
//         popularItems.clear();
//         menuItemLookup.clear();
//     }
    
//     Restaurant(int id, const string& n, const string& addr, 
//                const string& ph, const string& cui, int loc,
//                double rat = 0.0, int delTime = 0)
//         : restaurantId(id), locationNode(loc), rating(rat), 
//           deliveryTime(delTime), menuItemCount(0) {
        
//         strncpy(name, n.c_str(), sizeof(name) - 1);
//         strncpy(address, addr.c_str(), sizeof(address) - 1);
//         strncpy(phone, ph.c_str(), sizeof(phone) - 1);
//         strncpy(cuisine, cui.c_str(), sizeof(cuisine) - 1);
        
//         name[sizeof(name)-1] = '\0';
//         address[sizeof(address)-1] = '\0';
//         phone[sizeof(phone)-1] = '\0';
//         cuisine[sizeof(cuisine)-1] = '\0';
        
//         // Initialize new members
//         menultemIds.clear();
//         popularItems.clear();
//         menuItemLookup.clear();
//     }
    
//     // Copy constructor
//     Restaurant(const Restaurant& other) {
//         restaurantId = other.restaurantId;
//         strncpy(name, other.name, sizeof(name));
//         strncpy(address, other.address, sizeof(address));
//         strncpy(phone, other.phone, sizeof(phone));
//         strncpy(cuisine, other.cuisine, sizeof(cuisine));
//         locationNode = other.locationNode;
//         rating = other.rating;
//         deliveryTime = other.deliveryTime;
//         menuItemCount = other.menuItemCount;
        
//         // Copy menu items array
//         for (int i = 0; i < menuItemCount; i++) {
//             menuItems[i] = other.menuItems[i];
//         }
        
//         // Copy new members
//         menultemIds = other.menultemIds;
//         popularItems = other.popularItems;
//         menuItemLookup = other.menuItemLookup;
//     }
    
//     // Assignment operator
//     Restaurant& operator=(const Restaurant& other) {
//         if (this != &other) {
//             restaurantId = other.restaurantId;
//             strncpy(name, other.name, sizeof(name));
//             strncpy(address, other.address, sizeof(address));
//             strncpy(phone, other.phone, sizeof(phone));
//             strncpy(cuisine, other.cuisine, sizeof(cuisine));
//             locationNode = other.locationNode;
//             rating = other.rating;
//             deliveryTime = other.deliveryTime;
//             menuItemCount = other.menuItemCount;
            
//             // Copy menu items array
//             for (int i = 0; i < menuItemCount; i++) {
//                 menuItems[i] = other.menuItems[i];
//             }
            
//             // Copy new members
//             menultemIds = other.menultemIds;
//             popularItems = other.popularItems;
//             menuItemLookup = other.menuItemLookup;
//         }
//         return *this;
//     }
    
//     // Methods needed for RestaurantService.cpp
//     vector<int>& getMenultemIds() { return menultemIds; }
//     const vector<int>& getMenultemIds() const { return menultemIds; }
    
//     vector<MenuItem>& getPopularItems() { return popularItems; }
//     const vector<MenuItem>& getPopularItems() const { return popularItems; }
    
//     // Check if restaurant has a specific menu item
//     bool hasMenultem(int itemId) const {
//         // First check in the fixed array
//         for (int i = 0; i < menuItemCount; i++) {
//             if (menuItems[i].id == itemId) {
//                 return true;
//             }
//         }
        
//         // Then check in the vector (if using that approach)
//         return find(menultemIds.begin(), menultemIds.end(), itemId) != menultemIds.end();
//     }
    
//     // NEW: Added to check if restaurant has menu item (alternative name)
//     bool hasMenuItem(int itemId) const {
//         return hasMenultem(itemId);
//     }
    
//     // Add a menu item ID to the list
//     void addMenultemId(int itemId) {
//         if (!hasMenultem(itemId)) {
//             menultemIds.push_back(itemId);
//             // Update lookup if needed
//             if (menuItemLookup.size() <= itemId) {
//                 menuItemLookup.resize(itemId + 1, false);
//             }
//             menuItemLookup[itemId] = true;
//         }
//     }
    
//     // Remove a menu item ID from the list
//     void removeMenuItemId(int itemId) {
//         auto it = find(menultemIds.begin(), menultemIds.end(), itemId);
//         if (it != menultemIds.end()) {
//             menultemIds.erase(it);
//             if (itemId < menuItemLookup.size()) {
//                 menuItemLookup[itemId] = false;
//             }
//         }
//     }
    
//     // Add a popular item
//     void addPopularItem(const MenuItem& item) {
//         // Check if already in popular items
//         for (const auto& popular : popularItems) {
//             if (popular.id == item.id) {
//                 return; // Already exists
//             }
//         }
//         popularItems.push_back(item);
//     }
    
//     // Remove a popular item
//     void removePopularItem(int itemId) {
//         auto it = remove_if(popularItems.begin(), popularItems.end(),
//             [itemId](const MenuItem& item) { return item.id == itemId; });
//         popularItems.erase(it, popularItems.end());
//     }
    
//     // Clear all popular items
//     void clearPopularItems() {
//         popularItems.clear();
//     }
    
//     // Get popular item by index
//     MenuItem* getPopularItem(int index) {
//         if (index >= 0 && index < popularItems.size()) {
//             return &popularItems[index];
//         }
//         return nullptr;
//     }
    
//     const MenuItem* getPopularItem(int index) const {
//         if (index >= 0 && index < popularItems.size()) {
//             return &popularItems[index];
//         }
//         return nullptr;
//     }
    
//     // Get number of popular items
//     int getPopularItemsCount() const {
//         return popularItems.size();
//     }
    
//     // Get number of menu item IDs
//     int getMenultemIdsCount() const {
//         return menultemIds.size();
//     }
    
//     // Serialization method
//     void serialize(char* buffer) const {
//         int offset = 0;
        
//         // Serialize basic fields
//         memcpy(buffer + offset, &restaurantId, sizeof(restaurantId));
//         offset += sizeof(restaurantId);
        
//         memcpy(buffer + offset, name, sizeof(name));
//         offset += sizeof(name);
        
//         memcpy(buffer + offset, address, sizeof(address));
//         offset += sizeof(address);
        
//         memcpy(buffer + offset, phone, sizeof(phone));
//         offset += sizeof(phone);
        
//         memcpy(buffer + offset, cuisine, sizeof(cuisine));
//         offset += sizeof(cuisine);
        
//         memcpy(buffer + offset, &locationNode, sizeof(locationNode));
//         offset += sizeof(locationNode);
        
//         memcpy(buffer + offset, &rating, sizeof(rating));
//         offset += sizeof(rating);
        
//         memcpy(buffer + offset, &deliveryTime, sizeof(deliveryTime));
//         offset += sizeof(deliveryTime);
        
//         // Serialize menu item count and array
//         memcpy(buffer + offset, &menuItemCount, sizeof(menuItemCount));
//         offset += sizeof(menuItemCount);
        
//         // Serialize menu items array
//         for (int i = 0; i < menuItemCount; i++) {
//             menuItems[i].serialize(buffer + offset);
//             offset += sizeof(MenuItem);
//         }
        
//         // Serialize menultemIds vector
//         int menultemIdsSize = menultemIds.size();
//         memcpy(buffer + offset, &menultemIdsSize, sizeof(menultemIdsSize));
//         offset += sizeof(menultemIdsSize);
        
//         for (int id : menultemIds) {
//             memcpy(buffer + offset, &id, sizeof(id));
//             offset += sizeof(id);
//         }
        
//         // Serialize popularItems vector
//         int popularItemsSize = popularItems.size();
//         memcpy(buffer + offset, &popularItemsSize, sizeof(popularItemsSize));
//         offset += sizeof(popularItemsSize);
        
//         for (const MenuItem& item : popularItems) {
//             item.serialize(buffer + offset);
//             offset += sizeof(MenuItem);
//         }
//     }
    
//     // Deserialization method
//     void deserialize(const char* buffer) {
//         int offset = 0;
        
//         // Deserialize basic fields
//         memcpy(&restaurantId, buffer + offset, sizeof(restaurantId));
//         offset += sizeof(restaurantId);
        
//         memcpy(name, buffer + offset, sizeof(name));
//         offset += sizeof(name);
        
//         memcpy(address, buffer + offset, sizeof(address));
//         offset += sizeof(address);
        
//         memcpy(phone, buffer + offset, sizeof(phone));
//         offset += sizeof(phone);
        
//         memcpy(cuisine, buffer + offset, sizeof(cuisine));
//         offset += sizeof(cuisine);
        
//         memcpy(&locationNode, buffer + offset, sizeof(locationNode));
//         offset += sizeof(locationNode);
        
//         memcpy(&rating, buffer + offset, sizeof(rating));
//         offset += sizeof(rating);
        
//         memcpy(&deliveryTime, buffer + offset, sizeof(deliveryTime));
//         offset += sizeof(deliveryTime);
        
//         // Deserialize menu item count and array
//         memcpy(&menuItemCount, buffer + offset, sizeof(menuItemCount));
//         offset += sizeof(menuItemCount);
        
//         for (int i = 0; i < menuItemCount; i++) {
//             menuItems[i].deserialize(buffer + offset);
//             offset += sizeof(MenuItem);
//         }
        
//         // Deserialize menultemIds vector
//         int menultemIdsSize;
//         memcpy(&menultemIdsSize, buffer + offset, sizeof(menultemIdsSize));
//         offset += sizeof(menultemIdsSize);
        
//         menultemIds.resize(menultemIdsSize);
//         for (int i = 0; i < menultemIdsSize; i++) {
//             memcpy(&menultemIds[i], buffer + offset, sizeof(int));
//             offset += sizeof(int);
//         }
        
//         // Deserialize popularItems vector
//         int popularItemsSize;
//         memcpy(&popularItemsSize, buffer + offset, sizeof(popularItemsSize));
//         offset += sizeof(popularItemsSize);
        
//         popularItems.resize(popularItemsSize);
//         for (int i = 0; i < popularItemsSize; i++) {
//             popularItems[i].deserialize(buffer + offset);
//             offset += sizeof(MenuItem);
//         }
//     }
    
//     // Getter methods
//     int getRestaurantId() const { return restaurantId; }
//     string getName() const { return string(name); }
//     string getAddress() const { return string(address); }
//     string getPhone() const { return string(phone); }
//     string getCuisine() const { return string(cuisine); }
//     int getLocationNode() const { return locationNode; }
    
//     // ALIAS for getLocationNode (for compatibility)
//     int getLocation() const { return locationNode; }
    
//     double getRating() const { return rating; }
//     int getDeliveryTime() const { return deliveryTime; }
    
//     // Setters
//     void setName(const string& n) {
//         strncpy(name, n.c_str(), sizeof(name) - 1);
//         name[sizeof(name) - 1] = '\0';
//     }
    
//     void setAddress(const string& a) {
//         strncpy(address, a.c_str(), sizeof(address) - 1);
//         address[sizeof(address) - 1] = '\0';
//     }
    
//     void setPhone(const string& p) {
//         strncpy(phone, p.c_str(), sizeof(phone) - 1);
//         phone[sizeof(phone) - 1] = '\0';
//     }
    
//     void setCuisine(const string& c) {
//         strncpy(cuisine, c.c_str(), sizeof(cuisine) - 1);
//         cuisine[sizeof(cuisine) - 1] = '\0';
//     }
    
//     void setRating(double r) { rating = r; }
//     void setDeliveryTime(int dt) { deliveryTime = dt; }
    
//     // Menu management methods for the fixed array
//     bool addMenuItem(const MenuItem& item) {
//         if (menuItemCount >= 50) return false;  // Array full
        
//         // Check for duplicate ID
//         for (int i = 0; i < menuItemCount; i++) {
//             if (menuItems[i].id == item.id) return false;
//         }
        
//         menuItems[menuItemCount] = item;
//         menuItemCount++;
        
//         // Also add to the vector for consistency
//         addMenultemId(item.id);
//         return true;
//     }
    
//     // SINGLE removeMenuItem method
//     bool removeMenuItem(int itemId) {
//         bool removedFromArray = false;
        
//         // Remove from fixed array
//         for (int i = 0; i < menuItemCount; i++) {
//             if (menuItems[i].id == itemId) {
//                 // Shift remaining items left
//                 for (int j = i; j < menuItemCount - 1; j++) {
//                     menuItems[j] = menuItems[j + 1];
//                 }
//                 menuItemCount--;
//                 removedFromArray = true;
//                 break;
//             }
//         }
        
//         // Remove from vector
//         removeMenuItemId(itemId);
        
//         return removedFromArray;
//     }
    
//     MenuItem* getMenuItem(int itemId) {
//         for (int i = 0; i < menuItemCount; i++) {
//             if (menuItems[i].id == itemId) {
//                 return &menuItems[i];
//             }
//         }
//         return nullptr;
//     }
    
//     // NEW: Added increaseMenuItemStock method
//     bool increaseMenuItemStock(int itemId, int quantity) {
//         MenuItem* item = getMenuItem(itemId);
//         if (!item) return false;
//         item->stock += quantity;
//         return true;
//     }
    
//     bool reduceMenuItemStock(int itemId, int quantity) {
//         MenuItem* item = getMenuItem(itemId);
//         if (!item || item->stock < quantity) return false;
//         item->stock -= quantity;
//         return true;
//     }
    
//     bool updateMenuItem(int itemId, const string& newName, double newPrice, int newStock) {
//         MenuItem* item = getMenuItem(itemId);
//         if (!item) return false;
        
//         strncpy(item->name, newName.c_str(), sizeof(item->name) - 1);
//         item->name[sizeof(item->name)-1] = '\0';
//         item->price = newPrice;
//         item->stock = newStock;
//         return true;
//     }
    
//     // Print menu using menuItemsBTree if available
//     void printMenu() const {
//         cout << "Menu for " << name << ":\n";
        
//         // Show menu items from the fixed array
//         for (int i = 0; i < menuItemCount; i++) {
//             cout << "  " << menuItems[i].id << ": " << menuItems[i].name 
//                  << " - Rs. " << menuItems[i].price 
//                  << " (Stock: " << menuItems[i].stock << ")\n";
//         }
        
//         // Also show any items in the menuItemIds vector
//         if (menuItemIds.size() > menuItemCount) {
//             cout << "\nAdditional menu items (IDs):\n";
//             for (size_t i = menuItemCount; i < menuItemIds.size(); i++) {
//                 cout << "  Item ID: " << menuItemIds[i] << " (details in BTree)\n";
//             }
//         }
//     }
    
//     bool addMenuItem(int itemId, const string& itemName) {
//         if (menuItemCount >= 50) return false;
        
//         // Check for duplicate
//         for (int i = 0; i < menuItemCount; i++) {
//             if (menuItems[i].id == itemId) return false;
//         }
        
//         // Create a new MenuItem and add to array
//         MenuItem newItem(itemId, itemName, "", 0.0, 0, "", 0);
//         menuItems[menuItemCount] = newItem;
//         menuItemCount++;
        
//         // Also add to vector
//         menuItemIds.push_back(itemId);
//         return true;
//     }
    
//     void displayInfo() const {
//         cout << "Restaurant ID: " << restaurantId << "\n";
//         cout << "Name: " << name << "\n";
//         cout << "Cuisine: " << cuisine << "\n";
//         cout << "Address: " << address << "\n";
//         cout << "Phone: " << phone << "\n";
//         cout << "Rating: " << rating << "/5\n";
//         cout << "Delivery Time: " << deliveryTime << " minutes\n";
//         cout << "Location Node: " << locationNode << "\n";
//         cout << "Menu Items: " << menuItemCount << "\n";
//         cout << "Menu Item IDs: " << menultemIds.size() << "\n";
//         cout << "Popular Items: " << popularItems.size() << "\n";
//     }
    
//     // Comparison operators for BTree
//     bool operator<(const Restaurant& other) const {
//         return restaurantId < other.restaurantId;
//     }
    
//     bool operator==(const Restaurant& other) const {
//         return restaurantId == other.restaurantId;
//     }
    
//     friend ostream& operator<<(ostream& os, const Restaurant& r) {
//         os << r.name << " (" << r.cuisine << ")";
//         return os;
//     }
// };

// #endif
#pragma once
#ifndef RESTAURANT_H
#define RESTAURANT_H

#include <iostream>
#include <string>
#include <cstring>
#include "MenuItem.h"

using namespace std;

class Restaurant {
public:
    // Basic restaurant data (POD types for serialization)
    int restaurantId;
    char name[100];
    char address[200];
    char phone[20];
    char cuisine[50];
    int locationNode;
    double rating;
    int deliveryTime;
    
    // Menu item references (stored separately in database)
    int menuItemCount;  // Number of menu items
    int menuItemIds[100];  // Array of menu item IDs (max 100 items)
    
    // Constructor
    Restaurant() : restaurantId(-1), locationNode(0), rating(0.0), 
                  deliveryTime(0), menuItemCount(0) {
        memset(name, 0, sizeof(name));
        memset(address, 0, sizeof(address));
        memset(phone, 0, sizeof(phone));
        memset(cuisine, 0, sizeof(cuisine));
        memset(menuItemIds, 0, sizeof(menuItemIds));
    }
    
    // Parameterized constructor
    Restaurant(int id, const string& n, const string& addr, 
               const string& ph, const string& cui, int loc,
               double rat = 0.0, int delTime = 0)
        : restaurantId(id), locationNode(loc), rating(rat), 
          deliveryTime(delTime), menuItemCount(0) {
        
        strncpy(name, n.c_str(), sizeof(name) - 1);
        strncpy(address, addr.c_str(), sizeof(address) - 1);
        strncpy(phone, ph.c_str(), sizeof(phone) - 1);
        strncpy(cuisine, cui.c_str(), sizeof(cuisine) - 1);
        
        name[sizeof(name)-1] = '\0';
        address[sizeof(address)-1] = '\0';
        phone[sizeof(phone)-1] = '\0';
        cuisine[sizeof(cuisine)-1] = '\0';
        
        memset(menuItemIds, 0, sizeof(menuItemIds));
    }
    
    // Menu item management
    bool addMenuItemId(int itemId) {
        if (menuItemCount >= 100) return false;
        
        // Check for duplicates
        for (int i = 0; i < menuItemCount; i++) {
            if (menuItemIds[i] == itemId) return false;
        }
        
        menuItemIds[menuItemCount] = itemId;
        menuItemCount++;
        return true;
    }
    
    bool removeMenuItemId(int itemId) {
        for (int i = 0; i < menuItemCount; i++) {
            if (menuItemIds[i] == itemId) {
                // Shift remaining items left
                for (int j = i; j < menuItemCount - 1; j++) {
                    menuItemIds[j] = menuItemIds[j + 1];
                }
                menuItemCount--;
                return true;
            }
        }
        return false;
    }
    
    bool hasMenuItemId(int itemId) const {
        for (int i = 0; i < menuItemCount; i++) {
            if (menuItemIds[i] == itemId) return true;
        }
        return false;
    }
    
    // Getters
    int getRestaurantId() const { return restaurantId; }
    string getName() const { return string(name); }
    string getAddress() const { return string(address); }
    string getPhone() const { return string(phone); }
    string getCuisine() const { return string(cuisine); }
    int getLocationNode() const { return locationNode; }
    int getLocation() const { return locationNode; }  // Alias
    double getRating() const { return rating; }
    int getDeliveryTime() const { return deliveryTime; }
    int getMenuItemCount() const { return menuItemCount; }
    
    // Setters
    void setName(const string& n) {
        strncpy(name, n.c_str(), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
    }
    
    void setAddress(const string& a) {
        strncpy(address, a.c_str(), sizeof(address) - 1);
        address[sizeof(address) - 1] = '\0';
    }
    
    void setPhone(const string& p) {
        strncpy(phone, p.c_str(), sizeof(phone) - 1);
        phone[sizeof(phone) - 1] = '\0';
    }
    
    void setCuisine(const string& c) {
        strncpy(cuisine, c.c_str(), sizeof(cuisine) - 1);
        cuisine[sizeof(cuisine) - 1] = '\0';
    }
    
    void setRating(double r) { rating = r; }
    void setDeliveryTime(int dt) { deliveryTime = dt; }
    
    // Print methods
    void printMenu() const {
        cout << "Menu Item IDs for " << name << ":\n";
        for (int i = 0; i < menuItemCount; i++) {
            cout << "  Item ID: " << menuItemIds[i] << "\n";
        }
    }
    
    void displayInfo() const {
        cout << "Restaurant ID: " << restaurantId << "\n";
        cout << "Name: " << name << "\n";
        cout << "Cuisine: " << cuisine << "\n";
        cout << "Address: " << address << "\n";
        cout << "Phone: " << phone << "\n";
        cout << "Rating: " << rating << "/5\n";
        cout << "Delivery Time: " << deliveryTime << " minutes\n";
        cout << "Location Node: " << locationNode << "\n";
        cout << "Menu Items: " << menuItemCount << "\n";
    }
    
    // Comparison operators for BTree
    bool operator<(const Restaurant& other) const {
        return restaurantId < other.restaurantId;
    }
    
    bool operator==(const Restaurant& other) const {
        return restaurantId == other.restaurantId;
    }
    
    // Size calculation
    static size_t getSize() {
        return sizeof(Restaurant);
    }
};

#endif // RESTAURANT_H