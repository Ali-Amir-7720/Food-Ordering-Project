#pragma once
#ifndef ORDERSERVICE_H
#define ORDERSERVICE_H
#include <iostream>
#include "../models/Order.h"
#include "../dataStructures/HashTable.h"
#include "../dataStructures/BTree.h"
#include "../dataStructures/LinkedList.h"
#include "../dataStructures/Queue.h"
using namespace std;

class OrderService {
private:
    PersistentBTree<Order>* orders;
    HashTable<Order> orderCache;
    Queue<Order>* pendingOrders;
    Queue<Order>* preparingOrders;
    Queue<Order>* readyOrders;

public:
    OrderService() : orders(nullptr), pendingOrders(nullptr), 
                     preparingOrders(nullptr), readyOrders(nullptr) {}
    
    OrderService(PersistentBTree<Order>* persistentOrders) 
        : orders(persistentOrders), pendingOrders(nullptr), 
          preparingOrders(nullptr), readyOrders(nullptr) {}
    
    OrderService(PersistentBTree<Order>* persistentOrders, 
                 Queue<Order>* pending, Queue<Order>* preparing, Queue<Order>* ready)
        : orders(persistentOrders), pendingOrders(pending), 
          preparingOrders(preparing), readyOrders(ready) {}

    bool addOrder(const Order& o) {
        if (orderCache.searchTable(o.id) != nullptr) return false;
        
        // Add to cache
        orderCache.insertItem(o.id, o);
        
        // Add to persistent storage
        if (orders) {
            orders->insert(o);
        }
        
        // Add to appropriate queue
        if (pendingOrders) {
            pendingOrders->enqueue(o);
        }
        
        return true;
    }

    bool removeOrder(int orderId) {
        if (orderCache.searchTable(orderId) == nullptr) return false;
        
        // Remove from cache
        orderCache.removeItem(orderId);
        
        // Remove from persistent storage
        if (orders) {
            Order dummy;
            dummy.id = orderId;
            orders->remove(dummy);
        }
        
        return true;
    }

    Order* getOrder(int orderId) {
        return orderCache.searchTable(orderId);
    }

    void updateOrderStatus(int orderId, const string& statusStr) {
        Order* o = orderCache.searchTable(orderId);
        if (!o) return;
        
        // Convert string to OrderStatus
        OrderStatus newStatus = OrderStatus::Pending;
        if (statusStr == "confirmed") newStatus = OrderStatus::Confirmed;
        else if (statusStr == "preparing") newStatus = OrderStatus::Preparing;
        else if (statusStr == "ready") newStatus = OrderStatus::ReadyForPickup;
        else if (statusStr == "dispatched") newStatus = OrderStatus::Dispatched;
        else if (statusStr == "delivered") newStatus = OrderStatus::Delivered;
        else if (statusStr == "cancelled") newStatus = OrderStatus::Cancelled;
        
        OrderStatus oldStatus = o->status;
        o->updateStatus(newStatus);
        
        // Move between queues if applicable
        if (pendingOrders && preparingOrders && readyOrders) {
            // This is simplified - in real implementation, you'd need to 
            // find and remove from old queue, add to new queue
        }
        
        // Update persistent storage
        if (orders) {
            orders->remove(*o);
            orders->insert(*o);
        }
    }

    void printAllOrders() {
        cout << "=== All Orders ===\n";
        orderCache.traverse([](int id, Order& o) {
            cout << "Order #" << id 
                 << " | Customer: " << o.customerId
                 << " | Restaurant: " << o.restaurantId
                 << " | Rider: " << (o.riderId == -1 ? "None" : to_string(o.riderId))
                 << " | Status: ";
            
            switch(o.status) {
                case OrderStatus::Pending: cout << "Pending"; break;
                case OrderStatus::Confirmed: cout << "Confirmed"; break;
                case OrderStatus::Preparing: cout << "Preparing"; break;
                case OrderStatus::ReadyForPickup: cout << "Ready"; break;
                case OrderStatus::Dispatched: cout << "Dispatched"; break;
                case OrderStatus::Delivered: cout << "Delivered"; break;
                case OrderStatus::Cancelled: cout << "Cancelled"; break;
            }
            
            cout << " | Total: $" << o.totalAmount << endl;
        });
    }

    LinkedList<Order> getOrdersByCustomer(int customerId) {
        LinkedList<Order> result;
        orderCache.traverse([&](int id, Order& o) {
            if (o.customerId == customerId)
                result.push_back(o);
        });
        return result;
    }

    LinkedList<Order> getOrdersByRider(int riderId) {
        LinkedList<Order> result;
        orderCache.traverse([&](int id, Order& o) {
            if (o.riderId == riderId)
                result.push_back(o);
        });
        return result;
    }

    LinkedList<Order> getPendingOrders() {
        LinkedList<Order> result;
        if (pendingOrders) {
            // Create a copy to traverse (in real implementation, queue should support traversal)
        }
        return result;
    }
};

#endif