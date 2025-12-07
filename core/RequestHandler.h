#pragma once
#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "../storage/SystemState.h"
#include "../models/MenuItem.h"
#include "../models/Order.h"

using namespace std;

class RequestHandler {
private:
    SystemState* systemState;
    
    vector<string> split(const string& str, char delimiter) {
        vector<string> tokens;
        string token;
        stringstream ss(str);
        
        while (getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

public:
    RequestHandler(SystemState* state) : systemState(state) {}
    
    string handleRequest(const string& request) {
        vector<string> tokens = split(request, '|');
        if (tokens.empty()) return "ERROR|Invalid request format";
        
        string command = tokens[0];
        
        if (command == "GET_MENU") {
            return handleGetMenu(tokens);
        }
        else if (command == "PLACE_ORDER") {
            return handlePlaceOrder(tokens);
        }
        else if (command == "GET_ORDER_STATUS") {
            return handleGetOrderStatus(tokens);
        }
        else if (command == "GET_RESTAURANTS") {
            return handleGetRestaurants();
        }
        else if (command == "REGISTER_USER") {
            return handleRegisterUser(tokens);
        }
        else if (command == "REGISTER_RIDER") {
            return handleRegisterRider(tokens);
        }
        else if (command == "UPDATE_RIDER_LOCATION") {
            return handleUpdateRiderLocation(tokens);
        }
        else if (command == "GET_ROUTE") {
            return handleGetRoute(tokens);
        }
        else if (command == "LOGIN") {
            return handleLogin(tokens);
        }
        else {
            return "ERROR|Unknown command: " + command;
        }
    }

private:
    string handleGetMenu(const vector<string>& tokens) {
        if (tokens.size() < 2) return "ERROR|Missing restaurant ID";
        
        try {
            int restaurantID = stoi(tokens[1]);
            Restaurant* restaurant = systemState->getRestaurant(restaurantID);
            if (!restaurant) return "ERROR|Restaurant not found";
            
            string result = "MENU|" + to_string(restaurantID) + "|";
            result += "1:Pizza:$12.99:10,2:Pasta:$9.99:5,3:Salad:$7.99:8";
            return result;
        } catch (...) {
            return "ERROR|Invalid restaurant ID";
        }
    }
    
    string handlePlaceOrder(const vector<string>& tokens) {
        if (tokens.size() < 6) return "ERROR|Missing parameters";
        
        try {
            int customerID = stoi(tokens[1]);
            int restaurantID = stoi(tokens[2]);
            string deliveryAddress = tokens[3];
            int deliveryLocation = stoi(tokens[4]);
            
            vector<pair<int, int>> items;
            string itemsStr = tokens[5];
            vector<string> itemPairs = split(itemsStr, ',');
            
            for (const auto& pairStr : itemPairs) {
                vector<string> itemInfo = split(pairStr, ':');
                if (itemInfo.size() == 2) {
                    int itemId = stoi(itemInfo[0]);
                    int quantity = stoi(itemInfo[1]);
                    items.push_back(make_pair(itemId, quantity));
                }
            }
            
            int orderId = systemState->placeOrder(customerID, restaurantID, items, 
                                                  deliveryAddress, deliveryLocation);
            
            if (orderId == -1) {
                return "ERROR|Failed to place order";
            }
            
            return "ORDER_PLACED|" + to_string(orderId);
        } catch (...) {
            return "ERROR|Invalid parameters";
        }
    }
    
    string handleGetOrderStatus(const vector<string>& tokens) {
        if (tokens.size() < 2) return "ERROR|Missing order ID";
        
        try {
            int orderID = stoi(tokens[1]);
            Order* order = systemState->getOrder(orderID);
            if (!order) return "ERROR|Order not found";
            
            string statusStr;
            switch (order->status) {
                case OrderStatus::Confirmed: statusStr = "Confirmed"; break;
                case OrderStatus::Preparing: statusStr = "Preparing"; break;
                case OrderStatus::ReadyForPickup: statusStr = "ReadyForPickup"; break;
                case OrderStatus::Dispatched: statusStr = "Dispatched"; break;
                case OrderStatus::InTransit: statusStr = "InTransit"; break;
                case OrderStatus::Delivered: statusStr = "Delivered"; break;
                case OrderStatus::Cancelled: statusStr = "Cancelled"; break;
                default: statusStr = "Unknown"; break;
            }
            
            return "ORDER_STATUS|" + to_string(orderID) + "|" + statusStr + "|" + 
                   to_string(order->getRiderID()) + "|" + to_string(order->getTotalPrice());
        } catch (...) {
            return "ERROR|Invalid order ID";
        }
    }
    
    string handleGetRestaurants() {
        return "RESTAURANTS|300:Pizza Palace:Italian:Open|301:Burger Barn:American:Open";
    }
    
    string handleRegisterUser(const vector<string>& tokens) {
        if (tokens.size() < 5) return "ERROR|Missing user details";
        
        string name = tokens[1];
        string email = tokens[2];
        string phone = tokens[3];
        string role = tokens.size() > 4 ? tokens[4] : "customer";
        string address = tokens.size() > 5 ? tokens[5] : "";
        
        bool success = systemState->registerUser(name, email, phone, role, address);
        
        if (success) {
            return "USER_REGISTERED|Success";
        } else {
            return "ERROR|Failed to register user";
        }
    }
    
    string handleRegisterRider(const vector<string>& tokens) {
        if (tokens.size() < 5) return "ERROR|Missing rider details";
        
        string name = tokens[1];
        string phone = tokens[2];
        string vehicle = tokens[3];
        int location = stoi(tokens[4]);
        
        bool success = systemState->registerRider(name, phone, vehicle, location);
        
        if (success) {
            return "RIDER_REGISTERED|Success";
        } else {
            return "ERROR|Failed to register rider";
        }
    }
    
    string handleUpdateRiderLocation(const vector<string>& tokens) {
        if (tokens.size() < 3) return "ERROR|Missing parameters";
        
        try {
            int riderID = stoi(tokens[1]);
            int location = stoi(tokens[2]);
            
            bool success = systemState->updateRiderLocation(riderID, location);
            
            if (success) {
                return "LOCATION_UPDATED|Success";
            } else {
                return "ERROR|Rider not found";
            }
        } catch (...) {
            return "ERROR|Invalid parameters";
        }
    }
    
    string handleGetRoute(const vector<string>& tokens) {
        if (tokens.size() < 3) return "ERROR|Missing locations";
        
        try {
            int start = stoi(tokens[1]);
            int end = stoi(tokens[2]);
            
            vector<int> route = systemState->getDeliveryRoute(start, end);
            int distance = systemState->getDeliveryDistance(start, end);
            
            if (route.empty()) {
                return "ROUTE|No route found";
            }
            
            string routeStr = "ROUTE|Distance:" + to_string(distance) + "|Path:";
            for (size_t i = 0; i < route.size(); i++) {
                routeStr += to_string(route[i]);
                if (i < route.size() - 1) routeStr += "->";
            }
            
            return routeStr;
        } catch (...) {
            return "ERROR|Invalid locations";
        }
    }
    
    string handleLogin(const vector<string>& tokens) {
        if (tokens.size() < 3) return "ERROR|Missing credentials";
        
        string email = tokens[1];
        string role = tokens[2];
        
        return "LOGIN_SUCCESS|Welcome|Role:" + role;
    }
};
#endif
