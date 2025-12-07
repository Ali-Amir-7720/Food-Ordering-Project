#pragma once
#ifndef CUSTOMER_H
#define CUSTOMER_H
#include <iostream>
#include <string>
#include <vector>
#include "Order.h"
#include "MenuItem.h"
using namespace std;

class Customer {
public:
    int customerID;
    string name;
    string address;
    string phone;
    vector<int> orderHistory;        // store order IDs
    vector<int> favoriteItems;       // store MenuItem IDs
    vector<int> favoriteRestaurants; // store Restaurant IDs

    Customer() : customerID(-1), name(""), address(""), phone("") {}

    Customer(int id, const string& n, const string& addr, const string& ph)
        : customerID(id), name(n), address(addr), phone(ph) {}

    void placeOrder(int orderID) {
        orderHistory.push_back(orderID);
    }

    void addFavoriteItem(int itemID) {
        favoriteItems.push_back(itemID);
    }

    void addFavoriteRestaurant(int restaurantID) {
        favoriteRestaurants.push_back(restaurantID);
    }

    void removeFavoriteItem(int itemID) {
        for (size_t i = 0; i < favoriteItems.size(); ++i) {
            if (favoriteItems[i] == itemID) {
                favoriteItems.erase(favoriteItems.begin() + i);
                return;
            }
        }
    }

    void removeFavoriteRestaurant(int restaurantID) {
        for (size_t i = 0; i < favoriteRestaurants.size(); ++i) {
            if (favoriteRestaurants[i] == restaurantID) {
                favoriteRestaurants.erase(favoriteRestaurants.begin() + i);
                return;
            }
        }
    }

    void printCustomer() const {
        cout << "Customer ID: " << customerID << ", Name: " << name
             << ", Address: " << address << ", Phone: " << phone << endl;

        cout << "Order History: ";
        if (orderHistory.empty()) cout << "None";
        else for (int id : orderHistory) cout << id << " ";
        cout << "\nFavorite Items: ";
        if (favoriteItems.empty()) cout << "None";
        else for (int id : favoriteItems) cout << id << " ";
        cout << "\nFavorite Restaurants: ";
        if (favoriteRestaurants.empty()) cout << "None";
        else for (int id : favoriteRestaurants) cout << id << " ";
        cout << endl;
    }
};
#endif