// ============= Restaurant.h (COMPLETE VERSION) =============
#pragma once
#ifndef RESTAURANT_H
#define RESTAURANT_H
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include "../dataStructures/HashTable.h"
#include "MenuItem.h"
using namespace std;

class Restaurant {
private:
    int restaurantId;
    char name[100];
    char address[200];
    char phone[20];
    char cuisine[50];
    int locationNode;
    double rating;
    int deliveryTime;
    MenuItem menuItems[50];
    int menuItemCount;
    
    // CRITICAL: Don't use vector in serialized class!
    // Use fixed array instead
    int menuItemIdsArray[50];  // Fixed size array
    int menuItemIdsCount;      // Track how many IDs we have

public:
    Restaurant() : restaurantId(0), locationNode(0), rating(0.0), 
                   deliveryTime(0), menuItemCount(0), menuItemIdsCount(0) {
        memset(name, 0, sizeof(name));
        memset(address, 0, sizeof(address));
        memset(phone, 0, sizeof(phone));
        memset(cuisine, 0, sizeof(cuisine));
        memset(menuItemIdsArray, 0, sizeof(menuItemIdsArray));
    }
    
    Restaurant(int id, const string& n, const string& addr, const string& ph,
               const string& cuis, int location, double rat = 4.0, int delTime = 30)
        : restaurantId(id), locationNode(location), rating(rat), 
          deliveryTime(delTime), menuItemCount(0), menuItemIdsCount(0) {
        strncpy(name, n.c_str(), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        
        strncpy(address, addr.c_str(), sizeof(address) - 1);
        address[sizeof(address) - 1] = '\0';
        
        strncpy(phone, ph.c_str(), sizeof(phone) - 1);
        phone[sizeof(phone) - 1] = '\0';
        
        strncpy(cuisine, cuis.c_str(), sizeof(cuisine) - 1);
        cuisine[sizeof(cuisine) - 1] = '\0';
        
        memset(menuItemIdsArray, 0, sizeof(menuItemIdsArray));
    }
    
    // Constructor without phone (for compatibility with existing code)
    Restaurant(int id, const string& n, const string& cuis, const string& addr,
               double rat, int delTime)
        : restaurantId(id), rating(rat), deliveryTime(delTime),
          menuItemCount(0), menuItemIdsCount(0), locationNode(0) {
        strncpy(name, n.c_str(), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        
        strncpy(cuisine, cuis.c_str(), sizeof(cuisine) - 1);
        cuisine[sizeof(cuisine) - 1] = '\0';
        
        strncpy(address, addr.c_str(), sizeof(address) - 1);
        address[sizeof(address) - 1] = '\0';
        
        memset(phone, 0, sizeof(phone));
        memset(menuItemIdsArray, 0, sizeof(menuItemIdsArray));
    }
    
    // Copy constructor
    Restaurant(const Restaurant& other) 
        : restaurantId(other.restaurantId),
          locationNode(other.locationNode),
          rating(other.rating),
          deliveryTime(other.deliveryTime),
          menuItemCount(other.menuItemCount),
          menuItemIdsCount(other.menuItemIdsCount) {
        
        memcpy(name, other.name, sizeof(name));
        memcpy(address, other.address, sizeof(address));
        memcpy(phone, other.phone, sizeof(phone));
        memcpy(cuisine, other.cuisine, sizeof(cuisine));
        memcpy(menuItems, other.menuItems, sizeof(menuItems));
        memcpy(menuItemIdsArray, other.menuItemIdsArray, sizeof(menuItemIdsArray));
    }
    
    // Assignment operator
    Restaurant& operator=(const Restaurant& other) {
        if (this != &other) {
            restaurantId = other.restaurantId;
            locationNode = other.locationNode;
            rating = other.rating;
            deliveryTime = other.deliveryTime;
            menuItemCount = other.menuItemCount;
            menuItemIdsCount = other.menuItemIdsCount;
            
            memcpy(name, other.name, sizeof(name));
            memcpy(address, other.address, sizeof(address));
            memcpy(phone, other.phone, sizeof(phone));
            memcpy(cuisine, other.cuisine, sizeof(cuisine));
            memcpy(menuItems, other.menuItems, sizeof(menuItems));
            memcpy(menuItemIdsArray, other.menuItemIdsArray, sizeof(menuItemIdsArray));
        }
        return *this;
    }
    
    // Getters
    int getRestaurantId() const { return restaurantId; }
    string getName() const { return string(name); }
    string getAddress() const { return string(address); }
    string getPhone() const { return string(phone); }
    string getCuisine() const { return string(cuisine); }
    int getLocationNode() const { return locationNode; }
    double getRating() const { return rating; }
    int getDeliveryTime() const { return deliveryTime; }
    int getMenuItemCount() const { return menuItemCount; }
    int getMenuItemIdsCount() const { return menuItemIdsCount; }
    
    // Setters
    void setName(const string& n) {
        strncpy(name, n.c_str(), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
    }
    
    void setAddress(const string& addr) {
        strncpy(address, addr.c_str(), sizeof(address) - 1);
        address[sizeof(address) - 1] = '\0';
    }
    
    void setPhone(const string& ph) {
        strncpy(phone, ph.c_str(), sizeof(phone) - 1);
        phone[sizeof(phone) - 1] = '\0';
    }
    
    void setCuisine(const string& cuis) {
        strncpy(cuisine, cuis.c_str(), sizeof(cuisine) - 1);
        cuisine[sizeof(cuisine) - 1] = '\0';
    }
    
    void setRating(double rat) { rating = rat; }
    void setDeliveryTime(int time) { deliveryTime = time; }
    
    // Menu item management
    bool addMenuItem(const MenuItem& item) {
        if (hasMenuItem(item.id)) return false;
        
        if (menuItemCount < 50) {
            menuItems[menuItemCount] = item;
            menuItemCount++;
        }
        
        return addMenuItemId(item.id);
    }
    
    bool addMenuItemId(int itemId) {
        // Check if already exists
        if (hasMenuItemId(itemId)) return false;
        
        // Check if we have space
        if (menuItemIdsCount >= 50) return false;
        
        // Add the ID
        menuItemIdsArray[menuItemIdsCount] = itemId;
        menuItemIdsCount++;
        return true;
    }
    
    bool hasMenuItemId(int itemId) const {
        for (int i = 0; i < menuItemIdsCount; i++) {
            if (menuItemIdsArray[i] == itemId) {
                return true;
            }
        }
        return false;
    }
    
    bool hasMenuItem(int itemId) const {
        for (int i = 0; i < menuItemCount; i++) {
            if (menuItems[i].id == itemId) {
                return true;
            }
        }
        return false;
    }
    
    MenuItem* getMenuItem(int itemId) {
        for (int i = 0; i < menuItemCount; i++) {
            if (menuItems[i].id == itemId) {
                return &menuItems[i];
            }
        }
        return nullptr;
    }
    
    // FIXED: Remove menu item ID from both arrays
    void removeMenuItemId(int itemId) {
        // Remove from menuItemIdsArray
        for (int i = 0; i < menuItemIdsCount; i++) {
            if (menuItemIdsArray[i] == itemId) {
                // Shift all elements after i left by one position
                for (int j = i; j < menuItemIdsCount - 1; j++) {
                    menuItemIdsArray[j] = menuItemIdsArray[j + 1];
                }
                menuItemIdsArray[menuItemIdsCount - 1] = 0;
                menuItemIdsCount--;
                
                // Also remove from menuItems array if present
                for (int k = 0; k < menuItemCount; k++) {
                    if (menuItems[k].id == itemId) {
                        // Shift all elements after k left by one position
                        for (int l = k; l < menuItemCount - 1; l++) {
                            menuItems[l] = menuItems[l + 1];
                        }
                        // Clear the last element
                        memset(&menuItems[menuItemCount - 1], 0, sizeof(MenuItem));
                        menuItemCount--;
                        break;
                    }
                }
                return;
            }
        }
    }
    
    void printMenu() const {
        if (menuItemCount == 0) {
            cout << "No menu items available.\n";
            return;
        }
        
        for (int i = 0; i < menuItemCount; i++) {
            cout << "  [" << menuItems[i].id << "] " << menuItems[i].name << "\n";
            cout << "      " << menuItems[i].description << "\n";
            cout << "      Price: $" << menuItems[i].price << "\n";
            cout << "      Stock: " << menuItems[i].stock << "\n";
            cout << "      Category: " << menuItems[i].category << "\n\n";
        }
    }
    
    void printDetails() const {
        cout << "Restaurant ID: " << restaurantId << "\n";
        cout << "Name: " << name << "\n";
        cout << "Address: " << address << "\n";
        cout << "Phone: " << phone << "\n";
        cout << "Cuisine: " << cuisine << "\n";
        cout << "Location Node: " << locationNode << "\n";
        cout << "Rating: " << rating << "/5\n";
        cout << "Delivery Time: " << deliveryTime << " minutes\n";
        cout << "Menu Items: " << menuItemCount << "\n";
        cout << "Menu Item IDs: " << menuItemIdsCount << "\n";
    }
    
    // Get all menu item IDs as vector (for compatibility)
    vector<int> getMenuItemIds() const {
        vector<int> ids;
        for (int i = 0; i < menuItemIdsCount; i++) {
            ids.push_back(menuItemIdsArray[i]);
        }
        return ids;
    }
    
    // Get menu items as vector
    vector<MenuItem> getMenuItems() const {
        vector<MenuItem> items;
        for (int i = 0; i < menuItemCount; i++) {
            items.push_back(menuItems[i]);
        }
        return items;
    }
    
    // Clear all menu items
    void clearMenu() {
        menuItemCount = 0;
        menuItemIdsCount = 0;
        memset(menuItems, 0, sizeof(menuItems));
        memset(menuItemIdsArray, 0, sizeof(menuItemIdsArray));
    }
     void setLocationNode(int node) { locationNode = node; }
    
    
  
    
};
#endif