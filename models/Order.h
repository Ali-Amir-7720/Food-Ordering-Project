#pragma once
#ifndef ORDER_H
#define ORDER_H

#include <iostream>
#include <string>
#include <cstring>
#include <ctime>
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

class Order {
public:
    int id;
    int customerId;
    int restaurantId;  
    int riderId;
    char deliveryAddress[200];
    int deliveryLocation;
    int statusInt;  
    double totalAmount;
    time_t orderTime;
    time_t estimatedDelivery;
    
    struct OrderItem {
        int restaurantId;  
        int itemId;
        char itemName[100];
        int quantity;
        double price;
        
        OrderItem() : restaurantId(0), itemId(0), quantity(0), price(0.0) {
            memset(itemName, 0, sizeof(itemName));
        }
        
        OrderItem(int restId, int id, const string& name, int qty, double pr)
            : restaurantId(restId), itemId(id), quantity(qty), price(pr) {
            strncpy(itemName, name.c_str(), sizeof(itemName) - 1);
            itemName[sizeof(itemName) - 1] = '\0';
        }
    };
    
    OrderItem items[20]; 
    int itemCount;
    
    bool isMultiRestaurantOrder;
    int restaurantIds[5];     
    int restaurantCount;
    double restaurantSubtotals[5]; 
    Order() : id(0), customerId(0), restaurantId(0), riderId(-1),
              deliveryLocation(0), statusInt(0), totalAmount(0.0),
              orderTime(time(0)), estimatedDelivery(0), itemCount(0),
              isMultiRestaurantOrder(false), restaurantCount(0) {
        memset(deliveryAddress, 0, sizeof(deliveryAddress));
        memset(restaurantIds, 0, sizeof(restaurantIds));
        memset(restaurantSubtotals, 0, sizeof(restaurantSubtotals));
    }
    
    Order(int _id, int _customerId, int _restaurantId, 
          const string& _address, int _riderId, int _deliveryLoc)
        : id(_id), customerId(_customerId), restaurantId(_restaurantId),
          riderId(_riderId), deliveryLocation(_deliveryLoc), 
          statusInt(0), totalAmount(0.0), 
          orderTime(time(0)), estimatedDelivery(0), itemCount(0),
          isMultiRestaurantOrder(false), restaurantCount(1) {
        
        strncpy(deliveryAddress, _address.c_str(), sizeof(deliveryAddress) - 1);
        deliveryAddress[sizeof(deliveryAddress) - 1] = '\0';
        
        restaurantIds[0] = _restaurantId;
        restaurantSubtotals[0] = 0.0;
    }
    
    Order(int _id, int _customerId, const string& _address, int _deliveryLoc)
        : id(_id), customerId(_customerId), restaurantId(0),
          riderId(-1), deliveryLocation(_deliveryLoc), 
          statusInt(0), totalAmount(0.0), 
          orderTime(time(0)), estimatedDelivery(0), itemCount(0),
          isMultiRestaurantOrder(true), restaurantCount(0) {
        
        strncpy(deliveryAddress, _address.c_str(), sizeof(deliveryAddress) - 1);
        deliveryAddress[sizeof(deliveryAddress) - 1] = '\0';
        
        memset(restaurantIds, 0, sizeof(restaurantIds));
        memset(restaurantSubtotals, 0, sizeof(restaurantSubtotals));
    }
    int getCustomerId() const { return customerId; }
    int getOrderId() const { return id; }
    int getRestaurant() const { return restaurantId; }
    double getTotalAmount() const { return totalAmount; }
    int getRiderID() const { return riderId; }
    string getDeliveryAddress() const { return string(deliveryAddress); }
    
    OrderStatus getStatusEnum() const {
        return static_cast<OrderStatus>(statusInt);
    }
    
    string getStatus() const {
        switch(static_cast<OrderStatus>(statusInt)) {
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
    
    string getStatusAsString() const {
        return getStatus();
    }
    
    bool isMultiRestaurant() const {
        return isMultiRestaurantOrder;
    }
    void addItem(int restId, int itemId, const string& itemName, int quantity, double price) {
        if (itemCount >= 20) return;
        
        items[itemCount] = OrderItem(restId, itemId, itemName, quantity, price);
        totalAmount += quantity * price;
        itemCount++;
        bool found = false;
        for (int i = 0; i < restaurantCount; i++) {
            if (restaurantIds[i] == restId) {
                restaurantSubtotals[i] += quantity * price;
                found = true;
                break;
            }
        }
        
        if (!found && restaurantCount < 5) {
            restaurantIds[restaurantCount] = restId;
            restaurantSubtotals[restaurantCount] = quantity * price;
            restaurantCount++;
            if (restaurantId == 0) {
                restaurantId = restId;
            }
            if (restaurantCount > 1) {
                isMultiRestaurantOrder = true;
            }
        }
    }
    void addItem(int itemId, const string& itemName, int quantity, double price) {
        addItem(restaurantId, itemId, itemName, quantity, price);
    }
    void assignRider(int _riderId) {
        riderId = _riderId;
        updateStatus(OrderStatus::Dispatched);
    }
    void updateStatus(OrderStatus newStatus) {
        statusInt = static_cast<int>(newStatus);
        if (newStatus == OrderStatus::Dispatched) {
            estimatedDelivery = time(0) + 1800;  
        }
    }
    
    void updateOverallStatus(OrderStatus newStatus) {
        updateStatus(newStatus);
    }
    
    bool isDelivered() const {
        return static_cast<OrderStatus>(statusInt) == OrderStatus::Delivered;
    }
    
    bool isCancelled() const {
        return static_cast<OrderStatus>(statusInt) == OrderStatus::Cancelled;
    }
    void printOrder() const {
        cout << "\n=== Order #" << id << " ===\n";
        cout << "Customer ID: " << customerId << "\n";
        
        if (isMultiRestaurantOrder) {
            cout << "Type: MULTI-RESTAURANT ORDER\n";
            cout << "Restaurants (" << restaurantCount << "): ";
            for (int i = 0; i < restaurantCount; i++) {
                cout << restaurantIds[i];
                if (i < restaurantCount - 1) cout << ", ";
            }
            cout << "\n";
        } else {
            cout << "Restaurant ID: " << restaurantId << "\n";
        }
        
        cout << "Status: " << getStatus() << "\n";
        cout << "Delivery Address: " << deliveryAddress << "\n";
        
        if (riderId >= 0) {
            cout << "Rider ID: " << riderId << "\n";
        } else {
            cout << "Rider: Not assigned\n";
        }
        
        cout << "\nItems:\n";
        
        if (isMultiRestaurantOrder) {
            for (int r = 0; r < restaurantCount; r++) {
                cout << "\n  Restaurant " << restaurantIds[r] 
                     << " (Subtotal: $" << restaurantSubtotals[r] << "):\n";
                
                for (int i = 0; i < itemCount; i++) {
                    if (items[i].restaurantId == restaurantIds[r]) {
                        cout << "    " << items[i].itemName 
                             << " x" << items[i].quantity 
                             << " @ $" << items[i].price 
                             << " = $" << (items[i].quantity * items[i].price) << "\n";
                    }
                }
            }
        } else {
            for (int i = 0; i < itemCount; i++) {
                cout << "  " << items[i].itemName 
                     << " x" << items[i].quantity 
                     << " @ $" << items[i].price 
                     << " = $" << (items[i].quantity * items[i].price) << "\n";
            }
        }
        
        cout << "\nTotal Amount: $" << totalAmount << "\n";
        char timeStr[100];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&orderTime));
        cout << "Order Time: " << timeStr << "\n";
        
        if (estimatedDelivery > 0) {
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&estimatedDelivery));
            cout << "Estimated Delivery: " << timeStr << "\n";
        }
        cout << "==================\n";
    }
    bool operator<(const Order& other) const {
        return id < other.id;
    }
    
    bool operator==(const Order& other) const {
        return id == other.id;
    }
    
    friend ostream& operator<<(ostream& os, const Order& o) {
        os << "Order #" << o.id << " ($" << o.totalAmount << ")";
        if (o.isMultiRestaurantOrder) {
            os << " [Multi-Restaurant]";
        }
        return os;
    }
    struct RestaurantOrderPart {
        int restaurantId;
        double restaurantSubtotal;
        
        RestaurantOrderPart() : restaurantId(0), restaurantSubtotal(0.0) {}
        RestaurantOrderPart(int id, OrderStatus status, int time, double subtotal)
            : restaurantId(id), restaurantSubtotal(subtotal) {}
    };
    const RestaurantOrderPart* getRestaurantParts() const {
        static RestaurantOrderPart tempParts[5];
        for (int i = 0; i < restaurantCount; i++) {
            tempParts[i].restaurantId = restaurantIds[i];
            tempParts[i].restaurantSubtotal = restaurantSubtotals[i];
        }
        return tempParts;
    }
    
    int getRestaurantPartsCount() const {
        return restaurantCount;
    }
};

#endif // ORDER_H