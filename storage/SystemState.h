#pragma once
#ifndef SYSTEMSTATE_H
#define SYSTEMSTATE_H

#include <iostream>
#include <memory>
#include "../models/User.h"
#include "../models/Restaurant.h"
#include "../models/MenuItem.h"
#include "../models/Order.h"
#include "../models/Rider.h"

#include "../dataStructures/BTree.h"
#include "../dataStructures/HashTable.h"
#include "../dataStructures/Graph.h"
#include "../dataStructures/PriorityQueue.h"
#include "../dataStructures/Queue.h"
#include "../dataStructures/LinkedList.h"
                
#include "../services/UserService.h"
#include "../services/OrderService.h"
#include "../services/RiderService.h"
#include "../services/RoutingService.h"
#include "../services/RestaurantService.h"

using namespace std;

class SystemState {
private:
    // ----- Core Data Structures -----
    PersistentBTree<UserData> users;
    PersistentBTree<Rider> riders;
    PersistentBTree<Order> orders;
    PersistentBTree<Restaurant> restaurants;
    
    HashTable<MenuItem> menuItemsCache;
    Graph* cityGraph;
    
    // Queues for order processing
    Queue<Order> pendingOrders;
    Queue<Order> preparingOrders;
    Queue<Order> readyOrders;
    
    // For rider assignment
    PriorityQueue<Rider*> availableRiders;

    // ----- Service Managers -----
    UserService* userService;
    OrderService* orderService;
    RiderService* riderService;
    RestaurantService* restaurantService;
    RoutingService* routingService;
    
    int nextOrderId;
    int nextUserId;
    int nextRiderId;
    int nextRestaurantId;
    int nextMenuItemId;

public:
    SystemState() 
        : users("users.dat",
                [](const UserData& a, const UserData& b) { return a.id < b.id; },
                [](const UserData& a, const UserData& b) { return a.id == b.id; }),
          riders("riders.dat",
                 [](const Rider& a, const Rider& b) { return a.id < b.id; },
                 [](const Rider& a, const Rider& b) { return a.id == b.id; }),
          orders("orders.dat",
                 [](const Order& a, const Order& b) { return a.id < b.id; },
                 [](const Order& a, const Order& b) { return a.id == b.id; }),
        restaurants("restaurants.dat",
            [](const Restaurant& a, const Restaurant& b) { return a.restaurantId < b.restaurantId; },
            [](const Restaurant& a, const Restaurant& b) { return a.restaurantId == b.restaurantId; }),
          cityGraph(new Graph(100)),
          nextOrderId(1000),
          nextUserId(100),
          nextRiderId(200),
          nextRestaurantId(300),
          nextMenuItemId(1000)
    {
        // Initialize services
        userService = new UserService(&users);
        riderService = new RiderService(&riders, &availableRiders);
        orderService = new OrderService(&orders, &pendingOrders, &preparingOrders, &readyOrders);
        restaurantService = new RestaurantService(&restaurants);
        routingService = new RoutingService(cityGraph);
        
        initializeSampleData();
    }
    
    ~SystemState() {
        delete userService;
        delete riderService;
        delete orderService;
        delete restaurantService;
        delete routingService;
        delete cityGraph;
    }
    
    void initializeSampleData() {
    // Initialize city graph with sample locations
    routingService->addRoad(0, 1, 500);   // 500 meters
    routingService->addRoad(0, 2, 800);
    routingService->addRoad(1, 3, 600);
    routingService->addRoad(2, 3, 400);
    routingService->addRoad(3, 4, 700);
    
    // Add sample users
    userService->addUser(nextUserId++, "John Doe", "john@email.com", 
                        "1234567890", "customer", "123 Main St");
    userService->addUser(nextUserId++, "Jane Smith", "jane@email.com", 
                        "0987654321", "customer", "456 Oak Ave");
    userService->addUser(nextUserId++, "Mike Rider", "mike@email.com", 
                        "5551234567", "rider", "789 Pine Rd");
    
    // Add sample restaurants
    Restaurant r1(nextRestaurantId++, "Pizza Palace", "123 Main St", 
                 "555-1111", "Italian", 0);
    int restaurantId1 = r1.getRestaurantId();  // Assuming getRestaurantId() exists
    restaurantService->addRestaurant(r1);
    
    Restaurant r2(nextRestaurantId++, "Burger Barn", "456 Oak Ave", 
                 "555-2222", "American", 2);
    int restaurantId2 = r2.getRestaurantId();
    restaurantService->addRestaurant(r2);
    
    // Add sample menu items - FIXED: Added restaurantId as last parameter
    MenuItem pizza1(nextMenuItemId++, "Margherita Pizza", 
                   "Classic tomato and cheese", 12.99, 50, "main", restaurantId1);
    restaurantService->addMenuItem(restaurantId1, pizza1);
    
    MenuItem pizza2(nextMenuItemId++, "Pepperoni Pizza", 
                   "Pepperoni with extra cheese", 14.99, 30, "main", restaurantId1);
    restaurantService->addMenuItem(restaurantId1, pizza2);
    
    MenuItem burger1(nextMenuItemId++, "Cheeseburger", 
                    "Beef patty with cheese", 8.99, 30, "main", restaurantId2);
    restaurantService->addMenuItem(restaurantId2, burger1);
    
    MenuItem fries(nextMenuItemId++, "French Fries", 
                  "Crispy golden fries", 3.99, 100, "side", restaurantId2);
    restaurantService->addMenuItem(restaurantId2, fries);
    
    // Add sample riders
    Rider rider1(nextRiderId++, "Mike Rider", "5551234567", "motorcycle", 1);
    riderService->addRider(rider1);
    
    Rider rider2(nextRiderId++, "Sarah Driver", "5559876543", "bike", 3);
    riderService->addRider(rider2);
    
    Rider rider3(nextRiderId++, "Tom Biker", "5551112233", "bike", 4);
    riderService->addRider(rider3);
    
    cout << "System initialized with sample data.\n";
    cout << "  Users: " << (nextUserId - 100) << "\n";
    cout << "  Restaurants: " << (nextRestaurantId - 300) << "\n";
    cout << "  Riders: " << (nextRiderId - 200) << "\n";
    cout << "  Menu Items: " << (nextMenuItemId - 1000) << "\n";
}

    // ----- User Management -----
    bool registerUser(const string& name, const string& email, const string& phone, 
                      const string& role = "customer", const string& address = "") {
        bool success = userService->addUser(nextUserId++, name, email, phone, role, address);
        if (success) {
            cout << "User registered successfully. ID: " << (nextUserId - 1) << "\n";
        } else {
            cout << "Failed to register user.\n";
        }
        return success;
    }
    
    bool registerUserWithId(int id, const string& name, const string& email, const string& phone,
                           const string& role = "customer", const string& address = "") {
        bool success = userService->addUser(id, name, email, phone, role, address);
        if (success && id >= nextUserId) {
            nextUserId = id + 1;
        }
        return success;
    }
    
    UserData* getUser(int userId) {
        return userService->getUser(userId);
    }
    
    bool updateUser(int userId, const string& name = "", const string& email = "",
                   const string& phone = "", const string& address = "") {
        UserData updates(userId, name, email, phone, "", address);
        return userService->updateUser(userId, updates);
    }
    
    bool removeUser(int userId) {
        return userService->removeUser(userId);
    }

    // ----- Restaurant Management -----
    bool addRestaurant(const string& name, const string& address, const string& phone,
                       const string& cuisine, int locationNode) {
        Restaurant r(nextRestaurantId++, name, address, phone, cuisine, locationNode);
        bool success = restaurantService->addRestaurant(r);
        if (success) {
            cout << "Restaurant added successfully. ID: " << (nextRestaurantId - 1) << "\n";
        }
        return success;
    }
    
    Restaurant* getRestaurant(int restaurantId) {
        return restaurantService->getRestaurant(restaurantId);
    }
    
    bool updateRestaurant(int restaurantId, const string& name = "", const string& address = "",
                      const string& phone = "", const string& cuisine = "") {
    Restaurant* r = getRestaurant(restaurantId);
    if (!r) return false;
    
    if (!name.empty()) r->setName(name);
    if (!address.empty()) r->setAddress(address);
    if (!phone.empty()) r->setPhone(phone);
    if (!cuisine.empty()) r->setCuisine(cuisine);
    
    return true;
}
    
    bool removeRestaurant(int restaurantId) {
        return restaurantService->removeRestaurant(restaurantId);
    }
    
    // ----- Menu Item Management -----
    bool addMenuItem(int restaurantId, const string& name, const string& description,
                 double price, int stock, const string& category) {
    // Add restaurantId as the last parameter
    MenuItem item(nextMenuItemId++, name, description, price, stock, category, restaurantId);
    bool success = restaurantService->addMenuItem(restaurantId, item);
    if (success) {
        cout << "Menu item added successfully. ID: " << (nextMenuItemId - 1) << "\n";
        // Cache the menu item
        menuItemsCache.insertItem(item.id, item);
    }
    return success;
}
    
    MenuItem* getMenuItem(int restaurantId, int itemId) {
        Restaurant* r = getRestaurant(restaurantId);
        if (!r) return nullptr;
        return r->getMenuItem(itemId);
    }
    
    bool updateMenuItemStock(int restaurantId, int itemId, int newStock) {
        return restaurantService->updateMenuItemStock(restaurantId, itemId, newStock);
    }
    
    bool updateMenuItemPrice(int restaurantId, int itemId, double newPrice) {
        return restaurantService->updateMenuItemPrice(restaurantId, itemId, newPrice);
    }
    
    bool removeMenuItem(int restaurantId, int itemId) {
        return restaurantService->removeMenuItem(restaurantId, itemId);
    }

    // ----- Order Management -----
    int placeOrder(int customerId, int restaurantId, const LinkedList<pair<int, int>>& items,
                   const string& deliveryAddress, int deliveryLocation) {
        // Create order
        Order order(nextOrderId++, customerId, restaurantId, deliveryAddress, -1, deliveryLocation);
        
        // Add items to order and check stock
        Restaurant* restaurant = getRestaurant(restaurantId);
        if (!restaurant) {
            cout << "Restaurant not found.\n";
            return -1;
        }
        
        bool allItemsAvailable = true;
        auto* currentItem = items.getHead();
        while (currentItem != nullptr) {
            int itemId = currentItem->data.first;
            int quantity = currentItem->data.second;
            
            MenuItem* menuItem = restaurant->getMenuItem(itemId);
            if (!menuItem) {
                cout << "Item " << itemId << " not found in restaurant menu.\n";
                allItemsAvailable = false;
                break;
            }
            
            if (menuItem->stock < quantity) {
                cout << "Insufficient stock for item " << itemId 
                     << ". Available: " << menuItem->stock << ", Requested: " << quantity << "\n";
                allItemsAvailable = false;
                break;
            }
            
            order.addItem(itemId, menuItem->name, quantity, menuItem->price);
            currentItem = currentItem->next;
        }
        
        if (!allItemsAvailable) {
            cout << "Order failed: items unavailable.\n";
            return -1;
        }
        
        // Update restaurant stock
        currentItem = items.getHead();
        while (currentItem != nullptr) {
            int itemId = currentItem->data.first;
            int quantity = currentItem->data.second;
            restaurant->reduceMenuItemStock(itemId, quantity);
            currentItem = currentItem->next;
        }
        
        // Add to order processing pipeline
        orderService->addOrder(order);
        pendingOrders.enqueue(order);
        
        cout << "Order #" << order.id << " placed successfully. Total: $" << order.totalAmount << "\n";
        return order.id;
    }
    
    // Overload for vector input (for convenience)
    int placeOrder(int customerId, int restaurantId, const vector<pair<int, int>>& itemsVec,
                   const string& deliveryAddress, int deliveryLocation) {
        LinkedList<pair<int, int>> items;
        for (const auto& item : itemsVec) {
            items.insertAtEnd(item);
        }
        return placeOrder(customerId, restaurantId, items, deliveryAddress, deliveryLocation);
    }
    
    Order* getOrder(int orderId) {
        return orderService->getOrder(orderId);
    }
    
    bool updateOrderStatus(int orderId, const string& status) {
        Order* order = getOrder(orderId);
        if (!order) {
            cout << "Order not found.\n";
            return false;
        }
        
        orderService->updateOrderStatus(orderId, status);
        
        // Move through processing pipeline
        if (status == "confirmed") {
            // Move from pending to preparing
            if (!pendingOrders.isEmpty()) {
                Order o = pendingOrders.dequeue();
                preparingOrders.enqueue(o);
                cout << "Order #" << orderId << " moved to preparing.\n";
            }
        } else if (status == "ready") {
            // Move from preparing to ready
            if (!preparingOrders.isEmpty()) {
                Order o = preparingOrders.dequeue();
                readyOrders.enqueue(o);
                cout << "Order #" << orderId << " is ready for pickup.\n";
            }
        } else if (status == "dispatched") {
            // Assign rider and start delivery
            assignRiderToOrder(order);
        } else if (status == "delivered") {
            // Complete the order
            if (order->riderId != -1) {
                Rider* rider = riderService->getRider(order->riderId);
                if (rider) {
                    rider->completeDelivery(true);
                    riderService->updateRiderStatus(order->riderId, "available");
                    // Add rider back to available queue
                    availableRiders.enqueue(rider, calculateRiderPriority(rider));
                }
            }
            cout << "Order #" << orderId << " delivered successfully.\n";
        } else if (status == "cancelled") {
            // Restore stock if order was preparing or ready
            Restaurant* restaurant = getRestaurant(order->restaurantId);
            if (restaurant) {
                // Restore stock for all items
                for (const auto& item : order->items) {
                    MenuItem* menuItem = restaurant->getMenuItem(item.itemId);
                    if (menuItem) {
                        menuItem->stock += item.quantity;
                    }
                }
            }
            cout << "Order #" << orderId << " cancelled.\n";
        }
        
        return true;
    }
    
    bool cancelOrder(int orderId) {
        return updateOrderStatus(orderId, "cancelled");
    }
    
    LinkedList<Order> getOrdersByCustomer(int customerId) {
        return orderService->getOrdersByCustomer(customerId);
    }
    
    LinkedList<Order> getOrdersByRider(int riderId) {
        return orderService->getOrdersByRider(riderId);
    }

    // ----- Rider Management -----
    bool registerRider(const string& name, const string& phone, const string& vehicle, int location) {
        Rider rider(nextRiderId++, name, phone, vehicle, location);
        bool success = riderService->addRider(rider);
        if (success) {
            cout << "Rider registered successfully. ID: " << (nextRiderId - 1) << "\n";
            // Add to available riders queue
            Rider* riderPtr = riderService->getRider(rider.id);
            if (riderPtr) {
                availableRiders.enqueue(riderPtr, calculateRiderPriority(riderPtr));
            }
        }
        return success;
    }
    
    Rider* getRider(int riderId) {
        return riderService->getRider(riderId);
    }
    
    bool updateRiderLocation(int riderId, int location) {
        bool success = riderService->updateRiderLocation(riderId, location);
        if (success) {
            Rider* rider = getRider(riderId);
            if (rider && rider->status == "available") {
                // Update priority in queue
                availableRiders.updatePriority(rider, calculateRiderPriority(rider));
            }
        }
        return success;
    }
    
    bool updateRiderStatus(int riderId, const string& status) {
        return riderService->updateRiderStatus(riderId, status);
    }
    
    LinkedList<Rider> getAvailableRiders() {
        return riderService->getAvailableRiders();
    }
    
    Rider* findBestRiderForOrder(int pickupLocation, int deliveryLocation) {
        return riderService->findBestRider(pickupLocation, deliveryLocation);
    }

    // ----- Route Planning -----
    vector<int> getDeliveryRoute(int startLocation, int endLocation) {
    LinkedList<int> linkedPath = routingService->getShortestPath(startLocation, endLocation);
    vector<int> path;
    
    // Method 1: If LinkedList has isEmpty() and removeFront()
    while (!linkedPath.isEmpty()) {
        path.push_back(linkedPath.removeFront());
    }
    
    // OR Method 2: If LinkedList has an iterator
    // for (auto it = linkedPath.begin(); it != linkedPath.end(); ++it) {
    //     path.push_back(*it);
    // }
    
    // OR Method 3: If LinkedList has toArray() or similar
    // int* arr = linkedPath.toArray();
    // for (int i = 0; i < arraySize; i++) {
    //     path.push_back(arr[i]);
    // }
    // delete[] arr;
    
    return path;
}
    int getDeliveryDistance(int startLocation, int endLocation) {
        return routingService->getShortestDistance(startLocation, endLocation);
    }
    
    int estimateDeliveryTime(int distance, const string& vehicleType) {
        return routingService->estimateDeliveryTime(distance, vehicleType);
    }
    
    void addCityLocation(int nodeId) {
        routingService->addLocation(nodeId);
    }
    
    void addCityRoad(int from, int to, int distance) {
        routingService->addRoad(from, to, distance);
    }
    
    // ----- System Operations -----
private:
    void assignRiderToOrder(Order* order) {
        if (!order) return;
        
        // Find the best available rider
        Rider* bestRider = riderService->findBestRider(
            order->deliveryLocation, 
            order->deliveryLocation
        );
        
        if (bestRider) {
            order->assignRider(bestRider->id);
            riderService->updateRiderStatus(bestRider->id, "busy");
            
            // Remove from available riders queue
            availableRiders.remove(bestRider);
            
            // Calculate route and estimated time
            vector<int> route = getDeliveryRoute(
                bestRider->location,
                order->deliveryLocation
            );

            if (!route.empty()) {  // vector uses empty() not isEmpty()
    int distance = getDeliveryDistance(
        bestRider->location,
        order->deliveryLocation
    );
    
    int estimatedTime = estimateDeliveryTime(distance, bestRider->vehicle);
    cout << "Rider " << bestRider->name << " (ID: " << bestRider->id 
         << ") assigned to Order #" << order->id 
         << ". Estimated delivery time: " << estimatedTime << " minutes.\n";
    
    // Print the route - vector doesn't have traverse()
    cout << "Route: ";
    for (size_t i = 0; i < route.size(); i++) {
        cout << route[i];
        if (i < route.size() - 1) {
            cout << " -> ";
        }
    }
    cout << " -> Destination\n";
}
        } else {
            cout << "No available riders for Order #" << order->id << "\n";
        }
    }
    
    int calculateRiderPriority(Rider* rider) {
        if (!rider) return 1000;
        
        // Lower number = higher priority
        // Factors: rating (higher is better), completed deliveries (more is better)
        int priority = 100 - (int)(rider->rating * 10);
        priority -= rider->completedDeliveries / 10; // More deliveries = slightly higher priority
        
        return max(1, priority); // Ensure positive priority
    }

public:
    // ----- Utility Methods -----
    void printAllUsers() {
        cout << "=== All Users ===\n";
        userService->printAllUsers();
    }

    void printAllRestaurants() {
        cout << "=== All Restaurants ===\n";
        restaurantService->printAllRestaurants();
    }
    
    void printRestaurantMenu(int restaurantId) {
    Restaurant* r = getRestaurant(restaurantId);
    if (r) {
        cout << "=== Menu for " << r->getName() << " ===\n";
        r->printMenu();
    } else {
        cout << "Restaurant not found.\n";
    }
}

    void printAllOrders() {
        cout << "=== All Orders ===\n";
        orderService->printAllOrders();
    }
    
    void printOrderDetails(int orderId) {
        Order* order = getOrder(orderId);
        if (order) {
            order->printOrder();
        } else {
            cout << "Order not found.\n";
        }
    }
    
    void printAllRiders() {
        cout << "=== All Riders ===\n";
        riderService->printAllRiders();
    }
    
    void printCityMap() {
        cout << "=== City Map ===\n";
        routingService->printMap();
    }
    
    void printPendingOrders() {
        cout << "=== Pending Orders (" << pendingOrders.size() << ") ===\n";
        pendingOrders.traverse([](const Order& o) {
            cout << "Order #" << o.id << " - Customer: " << o.customerId 
                 << " - Total: $" << o.totalAmount << "\n";
        });
    }
    
    void printPreparingOrders() {
        cout << "=== Preparing Orders (" << preparingOrders.size() << ") ===\n";
        preparingOrders.traverse([](const Order& o) {
            cout << "Order #" << o.id << " - Customer: " << o.customerId 
                 << " - Restaurant: " << o.restaurantId << "\n";
        });
    }
    
    void printReadyOrders() {
        cout << "=== Ready Orders (" << readyOrders.size() << ") ===\n";
        readyOrders.traverse([](const Order& o) {
            cout << "Order #" << o.id << " - Customer: " << o.customerId 
                 << " - Ready for pickup at Restaurant: " << o.restaurantId << "\n";
        });
    }
    
    void printAvailableRidersQueue() {
        cout << "=== Available Riders Queue (" << availableRiders.size() << ") ===\n";
        // Note: PriorityQueue doesn't have traverse, so we need to copy
        PriorityQueue<Rider*> tempQueue = availableRiders;
        int count = 1;
        while (!tempQueue.isEmpty()) {
            Rider* rider = tempQueue.dequeue();
            cout << count++ << ". " << rider->name << " (ID: " << rider->id 
                 << ") - Vehicle: " << rider->vehicle 
                 << " - Location: " << rider->location 
                 << " - Rating: " << rider->rating << "\n";
        }
    }
    
    // ----- Get Statistics -----
    int getTotalUsers() const {
        return nextUserId - 100;
    }
    
    int getTotalRestaurants() const {
        return nextRestaurantId - 300;
    }
    
    int getTotalOrders() const {
        return nextOrderId - 1000;
    }
    
    int getTotalRiders() const {
        return nextRiderId - 200;
    }
    
    int getActiveOrders() const {
        return pendingOrders.size() + preparingOrders.size() + readyOrders.size();
    }
    
    int getAvailableRidersCount() const {
        return availableRiders.size();
    }
    
    int getPendingOrdersCount() const {
        return pendingOrders.size();
    }
    
    int getPreparingOrdersCount() const {
        return preparingOrders.size();
    }
    
    int getReadyOrdersCount() const {
        return readyOrders.size();
    }
    
    int getCityLocationsCount() const {
        return routingService->getNumLocations();
    }
    
    // ----- Dashboard -----
    void showDashboard() {
        cout << "\n" << string(50, '=') << "\n";
        cout << "          FOOD DELIVERY SYSTEM DASHBOARD\n";
        cout << string(50, '=') << "\n";
        cout << "Users:          " << getTotalUsers() << "\n";
        cout << "Restaurants:    " << getTotalRestaurants() << "\n";
        cout << "Riders:         " << getTotalRiders() << "\n";
        cout << "Total Orders:   " << getTotalOrders() << "\n";
        cout << string(50, '-') << "\n";
        cout << "Active Orders:  " << getActiveOrders() << "\n";
        cout << "  - Pending:    " << getPendingOrdersCount() << "\n";
        cout << "  - Preparing:  " << getPreparingOrdersCount() << "\n";
        cout << "  - Ready:      " << getReadyOrdersCount() << "\n";
        cout << string(50, '-') << "\n";
        cout << "Available Riders: " << getAvailableRidersCount() << "\n";
        cout << "City Locations:   " << getCityLocationsCount() << "\n";
        cout << string(50, '=') << "\n\n";
    }
    
    // ----- System Control -----
    void clearAllData() {
        // Clear queues
        while (!pendingOrders.isEmpty()) pendingOrders.dequeue();
        while (!preparingOrders.isEmpty()) preparingOrders.dequeue();
        while (!readyOrders.isEmpty()) readyOrders.dequeue();
        
        // Clear priority queue
        while (!availableRiders.isEmpty()) availableRiders.dequeue();
        
        // Clear cache
        // (Would need clear method in HashTable)
        
        cout << "System data cleared (except persistent storage).\n";
    }
    
    void resetSystem() {
        clearAllData();
        
        // Reset IDs
        nextOrderId = 1000;
        nextUserId = 100;
        nextRiderId = 200;
        nextRestaurantId = 300;
        nextMenuItemId = 1000;
        
        // Reinitialize sample data
        initializeSampleData();
        
        cout << "System reset to initial state.\n";
    }
    
    // ----- Test Methods -----
    void runTestScenario() {
        cout << "\n" << string(50, '=') << "\n";
        cout << "        RUNNING TEST SCENARIO\n";
        cout << string(50, '=') << "\n";
        
        // 1. Place an order
        cout << "\n1. Placing a test order...\n";
        LinkedList<pair<int, int>> items;
        items.insertAtEnd(make_pair(1000, 2)); // 2x Margherita Pizza
        items.insertAtEnd(make_pair(1001, 1)); // 1x Pepperoni Pizza
        
        int orderId = placeOrder(100, 300, items, "123 Test St", 4);
        
        if (orderId != -1) {
            // 2. Confirm the order
            cout << "\n2. Confirming order...\n";
            updateOrderStatus(orderId, "confirmed");
            
            // 3. Mark as ready
            cout << "\n3. Marking order as ready...\n";
            updateOrderStatus(orderId, "ready");
            
            // 4. Dispatch with rider
            cout << "\n4. Dispatching order...\n";
            updateOrderStatus(orderId, "dispatched");
            
            // 5. Mark as delivered
            cout << "\n5. Delivering order...\n";
            updateOrderStatus(orderId, "delivered");
        }
        
        cout << "\nTest scenario completed.\n";
        cout << string(50, '=') << "\n\n";
    }
};

#endif // SYSTEMSTATE_H