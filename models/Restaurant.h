#pragma once
#ifndef RESTAURANT_H
#define RESTAURANT_H
#include <iostream>
#include <string>
#include "../dataStructures/HashTable.h"
#include "MenuItem.h"
using namespace std;

struct RestaurantInfo {
    int id;
    string name;
    string address;
    string phone;
    string cuisine; // "Italian", "Chinese", etc.
    int locationNode; // Graph node ID
    double rating;  // Added rating field
    int deliveryTime; // Added deliveryTime field
    
    RestaurantInfo() : id(0), name(""), address(""), phone(""), cuisine(""), 
                      locationNode(0), rating(0.0), deliveryTime(0) {}
    
    RestaurantInfo(int _id, const string& _name, const string& _addr, 
                   const string& _phone, const string& _cuisine, int _loc,
                   double _rating = 0.0, int _deliveryTime = 0)
        : id(_id), name(_name), address(_addr), phone(_phone), 
          cuisine(_cuisine), locationNode(_loc), rating(_rating), 
          deliveryTime(_deliveryTime) {}
    
    int getRestaurantId() const { return id; }
    string getName() const { return name; }
    string getCuisine() const { return cuisine; }
    string getAddress() const { return address; }
    string getPhone() const { return phone; }
    double getRating() const { return rating; }
    int getDeliveryTime() const { return deliveryTime; }
    
    // Setters (optional but useful)
    void setRating(double newRating) { rating = newRating; }
    void setDeliveryTime(int time) { deliveryTime = time; }
};

class Restaurant {
public:
    RestaurantInfo info;
    HashTable<MenuItem> menu; // key = menu item ID
    
    Restaurant() = default;
    
    Restaurant(int id, const string& name, const string& addr, 
               const string& phone, const string& cuisine, int location,
               double rating = 0.0, int deliveryTime = 0)
        : info(id, name, addr, phone, cuisine, location, rating, deliveryTime) {}
    
    // Getter methods for Restaurant class
    int getRestaurantId() const { return info.id; }
    string getName() const { return info.name; }
    string getCuisine() const { return info.cuisine; }
    string getAddress() const { return info.address; }
    string getPhone() const { return info.phone; }
    double getRating() const { return info.rating; }
    int getDeliveryTime() const { return info.deliveryTime; }
    int getLocationNode() const { return info.locationNode; }
    
    // Convenience method to get all info
    RestaurantInfo getInfo() const { return info; }
    
    bool addMenuItem(const MenuItem& item) {
        if (menu.searchTable(item.id) != nullptr) return false;
        menu.insertItem(item.id, item);
        return true;
    }
    
    bool removeMenuItem(int itemId) {
        return menu.removeItem(itemId);
    }
    
    MenuItem* getMenuItem(int itemId) {
        return menu.searchTable(itemId);
    }
    
    bool reduceMenuItemStock(int itemId, int quantity) {
        MenuItem* item = getMenuItem(itemId);
        if (!item || item->stock < quantity) return false;
        item->stock -= quantity;
        return true;
    }
    
    bool updateMenuItem(int itemId, const string& newName, double newPrice, int newStock) {
        MenuItem* item = getMenuItem(itemId);
        if (!item) return false;
        item->name = newName;
        item->price = newPrice;
        item->stock = newStock;
        return true;
    }
    
    void printMenu() const {
        cout << "Menu for " << info.name << ":\n";
        menu.traverse([](int id, const MenuItem& item) {
            cout << "  " << item.id << ": " << item.name 
                 << " - $" << item.price 
                 << " (Stock: " << item.stock << ")\n";
        });
    }
    
    // Method to display restaurant information
    void displayInfo() const {
        cout << "Restaurant ID: " << info.id << "\n";
        cout << "Name: " << info.name << "\n";
        cout << "Cuisine: " << info.cuisine << "\n";
        cout << "Address: " << info.address << "\n";
        cout << "Phone: " << info.phone << "\n";
        cout << "Rating: " << info.rating << "/5\n";
        cout << "Delivery Time: " << info.deliveryTime << " minutes\n";
        cout << "Location Node: " << info.locationNode << "\n";
    }
    
    friend ostream& operator<<(ostream& os, const Restaurant& r) {
        os << r.info.name << " (" << r.info.cuisine << ")";
        return os;
    }
};

#endif