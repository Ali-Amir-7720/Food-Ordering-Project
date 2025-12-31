#pragma once
#ifndef ORDERSERVICE_H
#define ORDERSERVICE_H
#include <iostream>
#include <vector>
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
          preparingOrders(nullptr), readyOrders(nullptr) {
        loadOrdersFromPersistent();
    }
    
    OrderService(PersistentBTree<Order>* persistentOrders, 
                 Queue<Order>* pending, Queue<Order>* preparing, Queue<Order>* ready)
        : orders(persistentOrders), pendingOrders(pending), 
          preparingOrders(preparing), readyOrders(ready) {
        loadOrdersFromPersistent();
    }

    // Load orders from persistent storage to cache
    void loadOrdersFromPersistent() {
        if (orders && !orders->isEmpty()) {
            vector<Order> allOrders = orders->getAllKeys();
            for (const auto& order : allOrders) {
                orderCache.insertItem(order.id, order);
            }
            cout << "Loaded " << allOrders.size() << " orders from persistent storage.\n";
        }
    }

    bool addOrder(const Order& o) {
        if (orderCache.searchTable(o.id) != nullptr) {
            cout << "Order with ID " << o.id << " already exists.\n";
            return false;
        }
        
        // Add to cache
        orderCache.insertItem(o.id, o);
        
        // Add to persistent storage
        if (orders) {
            orders->insert(o);
        }
        
        // Add to appropriate queue based on status
        OrderStatus status = o.getStatusEnum();
        if (pendingOrders && status == OrderStatus::Pending) {
            pendingOrders->enqueue(o);
        } else if (preparingOrders && status == OrderStatus::Preparing) {
            preparingOrders->enqueue(o);
        } else if (readyOrders && status == OrderStatus::ReadyForPickup) {
            readyOrders->enqueue(o);
        }
        
        cout << "Order #" << o.id << " added successfully.\n";
        return true;
    }

    bool removeOrder(int orderId) {
        if (orderCache.searchTable(orderId) == nullptr) {
            cout << "Order #" << orderId << " not found.\n";
            return false;
        }
        
        // Remove from cache
        orderCache.removeItem(orderId);
        
        // Remove from persistent storage
        if (orders) {
            Order dummy;
            dummy.id = orderId;
            orders->remove(dummy);
        }
        
        cout << "Order #" << orderId << " removed.\n";
        return true;
    }

    Order* getOrder(int orderId) {
        return orderCache.searchTable(orderId);
    }
    
    const Order* getOrder(int orderId) const {
        return orderCache.searchTable(orderId);
    }

    bool updateOrderStatus(int orderId, OrderStatus newStatus) {
        Order* o = orderCache.searchTable(orderId);
        if (!o) {
            cout << "Order #" << orderId << " not found.\n";
            return false;
        }
        
        OrderStatus oldStatus = o->getStatusEnum();
        o->updateStatus(newStatus);
        
        // Update persistent storage
        if (orders) {
            Order oldOrder;
            oldOrder.id = orderId;
            orders->remove(oldOrder);
            orders->insert(*o);
        }
        
        cout << "Order #" << orderId << " status updated to " << o->getStatus() << "\n";
        return true;
    }
    
    // Overload for string status
    bool updateOrderStatus(int orderId, const string& statusStr) {
        // Convert string to OrderStatus
        OrderStatus newStatus = OrderStatus::Pending;
        
        if (statusStr == "pending") newStatus = OrderStatus::Pending;
        else if (statusStr == "confirmed") newStatus = OrderStatus::Confirmed;
        else if (statusStr == "preparing") newStatus = OrderStatus::Preparing;
        else if (statusStr == "ready") newStatus = OrderStatus::ReadyForPickup;
        else if (statusStr == "dispatched") newStatus = OrderStatus::Dispatched;
        else if (statusStr == "in_transit") newStatus = OrderStatus::InTransit;
        else if (statusStr == "delivered") newStatus = OrderStatus::Delivered;
        else if (statusStr == "cancelled") newStatus = OrderStatus::Cancelled;
        else {
            cout << "Invalid status string: " << statusStr << "\n";
            return false;
        }
        
        return updateOrderStatus(orderId, newStatus);
    }

    bool assignRiderToOrder(int orderId, int riderId) {
        Order* o = orderCache.searchTable(orderId);
        if (!o) {
            cout << "Order #" << orderId << " not found.\n";
            return false;
        }
        
        o->assignRider(riderId);
        
        // Update persistent storage
        if (orders) {
            Order oldOrder;
            oldOrder.id = orderId;
            orders->remove(oldOrder);
            orders->insert(*o);
        }
        
        cout << "Rider #" << riderId << " assigned to Order #" << orderId << "\n";
        return true;
    }

    void printAllOrders() const {
        cout << "\n=== All Orders ===\n";
        if (orderCache.isEmpty()) {
            cout << "No orders found.\n";
            return;
        }
        
        int count = 0;
        orderCache.traverse([&count](int id, const Order& o) {
            count++;
            cout << "Order #" << id 
                 << " | Customer: " << o.customerId
                 << " | Restaurant: " << (o.isMultiRestaurantOrder ? "Multiple" : to_string(o.restaurantId))
                 << " | Rider: " << (o.riderId == -1 ? "None" : to_string(o.riderId))
                 << " | Status: " << o.getStatus()
                 << " | Total: $" << o.totalAmount << endl;
        });
        
        cout << "Total orders: " << count << "\n\n";
    }
    
    void printOrderDetails(int orderId) const {
        const Order* o = getOrder(orderId);
        if (!o) {
            cout << "Order #" << orderId << " not found.\n";
            return;
        }
        
        o->printOrder();
    }

    LinkedList<Order> getOrdersByCustomer(int customerId) const {
        LinkedList<Order> result;
        orderCache.traverse([&](int id, const Order& o) {
            if (o.customerId == customerId) {
                result.push_back(o);
            }
        });
        return result;
    }

    LinkedList<Order> getOrdersByRestaurant(int restaurantId) const {
        LinkedList<Order> result;
        orderCache.traverse([&](int id, const Order& o) {
            if (o.isMultiRestaurantOrder) {
                // Check if this restaurant is part of the multi-restaurant order
                for (int i = 0; i < o.restaurantCount; i++) {
                    if (o.restaurantIds[i] == restaurantId) {
                        result.push_back(o);
                        break;
                    }
                }
            } else if (o.restaurantId == restaurantId) {
                result.push_back(o);
            }
        });
        return result;
    }

    LinkedList<Order> getOrdersByRider(int riderId) const {
        LinkedList<Order> result;
        orderCache.traverse([&](int id, const Order& o) {
            if (o.riderId == riderId) {
                result.push_back(o);
            }
        });
        return result;
    }

    LinkedList<Order> getOrdersByStatus(OrderStatus status) const {
        LinkedList<Order> result;
        orderCache.traverse([&](int id, const Order& o) {
            if (o.getStatusEnum() == status) {
                result.push_back(o);
            }
        });
        return result;
    }

    LinkedList<Order> getPendingOrders() const {
        return getOrdersByStatus(OrderStatus::Pending);
    }
    
    LinkedList<Order> getPreparingOrders() const {
        return getOrdersByStatus(OrderStatus::Preparing);
    }
    
    LinkedList<Order> getReadyOrders() const {
        return getOrdersByStatus(OrderStatus::ReadyForPickup);
    }
    
    LinkedList<Order> getDispatchedOrders() const {
        return getOrdersByStatus(OrderStatus::Dispatched);
    }
    
    LinkedList<Order> getDeliveredOrders() const {
        return getOrdersByStatus(OrderStatus::Delivered);
    }

    // Get order statistics
    void printOrderStatistics() const {
        int totalOrders = 0;
        int pendingCount = 0;
        int confirmedCount = 0;
        int preparingCount = 0;
        int readyCount = 0;
        int dispatchedCount = 0;
        int deliveredCount = 0;
        int cancelledCount = 0;
        double totalRevenue = 0.0;
        
        orderCache.traverse([&](int id, const Order& o) {
            totalOrders++;
            totalRevenue += o.totalAmount;
            
            switch(o.getStatusEnum()) {
                case OrderStatus::Pending: pendingCount++; break;
                case OrderStatus::Confirmed: confirmedCount++; break;
                case OrderStatus::Preparing: preparingCount++; break;
                case OrderStatus::ReadyForPickup: readyCount++; break;
                case OrderStatus::Dispatched: dispatchedCount++; break;
                case OrderStatus::InTransit: dispatchedCount++; break;
                case OrderStatus::Delivered: deliveredCount++; break;
                case OrderStatus::Cancelled: cancelledCount++; break;
            }
        });
        
        cout << "\n=== Order Statistics ===\n";
        cout << "Total Orders: " << totalOrders << "\n";
        cout << "Pending: " << pendingCount << "\n";
        cout << "Confirmed: " << confirmedCount << "\n";
        cout << "Preparing: " << preparingCount << "\n";
        cout << "Ready: " << readyCount << "\n";
        cout << "Dispatched/In Transit: " << dispatchedCount << "\n";
        cout << "Delivered: " << deliveredCount << "\n";
        cout << "Cancelled: " << cancelledCount << "\n";
        cout << "Total Revenue: $" << totalRevenue << "\n";
        cout << "Average Order Value: $" 
             << (totalOrders > 0 ? totalRevenue / totalOrders : 0.0) << "\n\n";
    }

    // Get count of orders
    int getTotalOrderCount() const {
        int count = 0;
        orderCache.traverse([&count](int id, const Order& o) {
            count++;
        });
        return count;
    }
    
    int getOrderCountByCustomer(int customerId) const {
        return getOrdersByCustomer(customerId).getSize();
    }
    
    int getOrderCountByRestaurant(int restaurantId) const {
        return getOrdersByRestaurant(restaurantId).getSize();
    }
    
    int getOrderCountByRider(int riderId) const {
        return getOrdersByRider(riderId).getSize();
    }

    // Cancel order
    bool cancelOrder(int orderId) {
        Order* o = orderCache.searchTable(orderId);
        if (!o) {
            cout << "Order #" << orderId << " not found.\n";
            return false;
        }
        
        if (o->isDelivered()) {
            cout << "Cannot cancel delivered order.\n";
            return false;
        }
        
        if (o->isCancelled()) {
            cout << "Order already cancelled.\n";
            return false;
        }
        
        return updateOrderStatus(orderId, OrderStatus::Cancelled);
    }

    // Mark order as delivered
    bool markOrderDelivered(int orderId) {
        Order* o = orderCache.searchTable(orderId);
        if (!o) {
            cout << "Order #" << orderId << " not found.\n";
            return false;
        }
        
        if (o->riderId == -1) {
            cout << "Cannot mark as delivered: No rider assigned.\n";
            return false;
        }
        
        return updateOrderStatus(orderId, OrderStatus::Delivered);
    }

    // Get multi-restaurant orders
    LinkedList<Order> getMultiRestaurantOrders() const {
        LinkedList<Order> result;
        orderCache.traverse([&](int id, const Order& o) {
            if (o.isMultiRestaurantOrder) {
                result.push_back(o);
            }
        });
        return result;
    }

    // Clear all orders (for testing)
    void clearAllOrders() {
        orderCache.clear();
        if (orders) {
            orders->clear();
        }
        cout << "All orders cleared.\n";
    }
};

#endif