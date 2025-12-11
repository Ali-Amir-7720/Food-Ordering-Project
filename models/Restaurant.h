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
    int restaurantId;
    char name[100];
    char address[200];
    char phone[20];
    char cuisine[50];
    int locationNode;
    double rating;
    int deliveryTime;
    
    int menuItemCount;  
    int menuItemIds[100];  
    Restaurant() : restaurantId(-1), locationNode(0), rating(0.0), 
                  deliveryTime(0), menuItemCount(0) {
        memset(name, 0, sizeof(name));
        memset(address, 0, sizeof(address));
        memset(phone, 0, sizeof(phone));
        memset(cuisine, 0, sizeof(cuisine));
        memset(menuItemIds, 0, sizeof(menuItemIds));
    }
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
    bool addMenuItemId(int itemId) {
        if (menuItemCount >= 100) return false;
        
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
    int getRestaurantId() const { return restaurantId; }
    string getName() const { return string(name); }
    string getAddress() const { return string(address); }
    string getPhone() const { return string(phone); }
    string getCuisine() const { return string(cuisine); }
    int getLocationNode() const { return locationNode; }
    int getLocation() const { return locationNode; } 
    double getRating() const { return rating; }
    int getDeliveryTime() const { return deliveryTime; }
    int getMenuItemCount() const { return menuItemCount; }
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
    bool operator<(const Restaurant& other) const {
        return restaurantId < other.restaurantId;
    }
    
    bool operator==(const Restaurant& other) const {
        return restaurantId == other.restaurantId;
    }
    static size_t getSize() {
        return sizeof(Restaurant);
    }
};

#endif // RESTAURANT_H