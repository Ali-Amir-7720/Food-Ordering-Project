#pragma once
#ifndef ORDER_H
#define ORDER_H
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include "../dataStructures/LinkedList.h"
#include "MenuItem.h"
using namespace std;

enum class OrderStatus {
    Pending,
    Confirmed,
    Preparing,
    ReadyForPickup,
    Dispatched,
    InTransit,
    Delivered,
    Cancelled
};

struct OrderItem {
    int itemId;
    string itemName;
    int quantity;
    double price;
    
    OrderItem(int id, const string& name, int qty, double pr)
        : itemId(id), itemName(name), quantity(qty), price(pr) {}
    
    double getTotal() const { return quantity * price; }
};

class Order {
public:
    int id;
    int customerId;
    int restaurantId;
    int riderId;
    string deliveryAddress;
    int deliveryLocation; // Graph node ID
    OrderStatus status;
    vector<OrderItem> items;
    double totalAmount;
    time_t orderTime;
    time_t estimatedDelivery;
    

    int getCustomerId() const {
        return customerId;
    }
    int getOrderId() const {
        return id;
    }
    int getRestaurant() const {
        return restaurantId;
    }
    int getTotalAmount() const {
        return totalAmount;
    }
    string getStatus() const {
        switch(status) {
            case OrderStatus::Pending: return "Pending";
            case OrderStatus::Confirmed: return "Confirmed";
            case OrderStatus::Preparing: return "Preparing";
            case OrderStatus::ReadyForPickup: return "Ready for Pickup";
            case OrderStatus::Dispatched: return "Dispatched";
            case OrderStatus::InTransit: return "In Transit";
            case OrderStatus::Delivered: return "Delivered";
            case OrderStatus::Cancelled: return "Cancelled";
            default: return "Unknown";
        }
    }
    Order() : id(0), customerId(0), restaurantId(0), riderId(-1),
              deliveryLocation(0), status(OrderStatus::Pending),
              totalAmount(0.0), orderTime(time(0)) {}
    
    Order(int _id, int _customerId, int _restaurantId, 
          const string& _address, int _riderId, int _deliveryLoc)
        : id(_id), customerId(_customerId), restaurantId(_restaurantId),
          riderId(_riderId), deliveryAddress(_address), 
          deliveryLocation(_deliveryLoc), status(OrderStatus::Pending),
          totalAmount(0.0), orderTime(time(0)) {}
    
    void addItem(int itemId, const string& itemName, int quantity, double price) {
        items.emplace_back(itemId, itemName, quantity, price);
        totalAmount += quantity * price;
    }
    
    void addItem(int itemId, int quantity, double price) {
        items.emplace_back(itemId, "Item " + to_string(itemId), quantity, price);
        totalAmount += quantity * price;
    }
    
    void assignRider(int _riderId) {
        riderId = _riderId;
        updateStatus(OrderStatus::Dispatched);
    }
    
    void updateStatus(OrderStatus newStatus) {
        status = newStatus;
        if (newStatus == OrderStatus::Dispatched) {
            estimatedDelivery = time(0) + 1800; // 30 minutes from now
        }
    }
    
    bool isDelivered() const {
        return status == OrderStatus::Delivered;
    }
    
    bool isCancelled() const {
        return status == OrderStatus::Cancelled;
    }
    
    void printOrder() const {
        cout << "Order #" << id << "\n";
        cout << "Customer: " << customerId << "\n";
        cout << "Restaurant: " << restaurantId << "\n";
        cout << "Status: ";
        switch(status) {
            case OrderStatus::Pending: cout << "Pending"; break;
            case OrderStatus::Confirmed: cout << "Confirmed"; break;
            case OrderStatus::Preparing: cout << "Preparing"; break;
            case OrderStatus::ReadyForPickup: cout << "Ready for Pickup"; break;
            case OrderStatus::Dispatched: cout << "Dispatched"; break;
            case OrderStatus::InTransit: cout << "In Transit"; break;
            case OrderStatus::Delivered: cout << "Delivered"; break;
            case OrderStatus::Cancelled: cout << "Cancelled"; break;
        }
        cout << "\n";
        cout << "Items:\n";
        for (const auto& item : items) {
            cout << "  " << item.itemName << " x" << item.quantity 
                 << " @ $" << item.price << " = $" << item.getTotal() << "\n";
        }
        cout << "Total: $" << totalAmount << "\n";
    }
    
    friend ostream& operator<<(ostream& os, const Order& o) {
        os << "Order #" << o.id << " ($" << o.totalAmount << ")";
        return os;
    }
    int getRiderID()const {
        return riderId;
    }
    int getTotalPrice()const{
        return totalAmount;
    }
};

#endif