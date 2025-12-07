#pragma once
#ifndef MENUITEM_H
#define MENUITEM_H

#include <string>
#include <iostream>
using namespace std;

struct MenuItem {
    int id;         // unique item ID
    string name;    // item name
    string description;
    double price;   // item price
    int stock;      // available stock
    string category;
    int restaurantId; // which restaurant this belongs to

    // Default constructor
    MenuItem() : id(0), name(""), description(""), price(0.0), stock(0), category(""), restaurantId(0) {}
    
    // Constructor with all parameters
    MenuItem(int i, const string& n, const string& desc, double p, int s, 
             const string& cat = "", int rid = 0)
        : id(i), name(n), description(desc), price(p), stock(s), category(cat), restaurantId(rid) {}
    
    // Simplified constructor for backward compatibility (4 parameters)
    MenuItem(int i, const string& n, double p, int s)
        : id(i), name(n), description(""), price(p), stock(s), category(""), restaurantId(0) {}
    
    // Constructor with name, price, stock only (for quick creation)
    MenuItem(const string& n, double p, int s)
        : id(0), name(n), description(""), price(p), stock(s), category(""), restaurantId(0) {}
    
    // Required for BTree operations
    bool operator<(const MenuItem& other) const {
        if (restaurantId != other.restaurantId)
            return restaurantId < other.restaurantId;
        return id < other.id;
    }
    
    bool operator==(const MenuItem& other) const {
        return id == other.id && restaurantId == other.restaurantId;
    }
    
    bool operator>(const MenuItem& other) const {
        return !(*this < other) && !(*this == other);
    }
    
    void reduceStock(int quantity) {
        if (stock >= quantity) {
            stock -= quantity;
        }
    }
    
    void increaseStock(int quantity) {
        stock += quantity;
    }
    
    string toString() const {
        return "ID: " + to_string(id) + " - " + name + " - $" + to_string(price) + 
               " (Stock: " + to_string(stock) + ")";
    }
    
    // Display item information
    void display() const {
        cout << "ID: " << id << "\n";
        cout << "Name: " << name << "\n";
        if (!description.empty()) {
            cout << "Description: " << description << "\n";
        }
        cout << "Price: $" << price << "\n";
        cout << "Stock: " << stock << "\n";
        if (!category.empty()) {
            cout << "Category: " << category << "\n";
        }
        if (restaurantId > 0) {
            cout << "Restaurant ID: " << restaurantId << "\n";
        }
    }
    
    // Getters
    int getId() const { return id; }
    string getName() const { return name; }
    double getPrice() const { return price; }
    int getStock() const { return stock; }
    string getCategory() const { return category; }
    int getRestaurantId() const { return restaurantId; }
    
    // Setters
    void setId(int newId) { id = newId; }
    void setName(const string& newName) { name = newName; }
    void setPrice(double newPrice) { price = newPrice; }
    void setStock(int newStock) { stock = newStock; }
    void setCategory(const string& newCategory) { category = newCategory; }
    void setRestaurantId(int newRestaurantId) { restaurantId = newRestaurantId; }
    void setDescription(const string& newDesc) { description = newDesc; }
};

#endif