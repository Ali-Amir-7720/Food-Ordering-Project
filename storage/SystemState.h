#pragma once
#ifndef SYSTEMSTATE_H
#define SYSTEMSTATE_H

#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <climits>
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

#include "../Database.h"
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
    
    Database* database;
    
    int nextOrderId;
    int nextUserId;
    int nextRiderId;
    int nextRestaurantId;
    int nextMenuItemId;

    // Helper method to get all restaurants from BTree
    vector<Restaurant> getAllRestaurantsFromBTree() {
        vector<Restaurant> restaurantList;
        
        // Iterate through all possible restaurant IDs
        for (int id = 300; id < nextRestaurantId; id++) {
            Restaurant* r = restaurantService->getRestaurant(id);
            if (r) {
                restaurantList.push_back(*r);
            }
        }
        
        return restaurantList;
    }

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
            [](const Restaurant& a, const Restaurant& b) { return a.getRestaurantId() < b.getRestaurantId(); },
            [](const Restaurant& a, const Restaurant& b) { return a.getRestaurantId() == b.getRestaurantId(); }),
          cityGraph(new Graph(100)),
          database(new Database()),
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
        
        // Load existing data from database
        loadFromDatabase();
        
        // Only initialize sample data if empty
        if (restaurants.isEmpty()) {
            initializeSampleData();
        }
    }
    
    ~SystemState() {
        // Save before destroying
        saveToDatabase();
        
        delete userService;
        delete riderService;
        delete orderService;
        delete restaurantService;
        delete routingService;
        delete cityGraph;
        delete database;
    }
    
    // Database Integration
    void loadFromDatabase() {
        cout << "Loading restaurants from database...\n";
        vector<Restaurant> dbRestaurants = database->loadAllRestaurants();
        
        for (const auto& restaurant : dbRestaurants) {
            restaurants.insert(restaurant);
            if (restaurant.getRestaurantId() >= nextRestaurantId) {
                nextRestaurantId = restaurant.getRestaurantId() + 1;
            }
        }
        
        cout << "Loaded " << dbRestaurants.size() << " restaurants from database.\n";
        
        // Load menu items
        vector<MenuItem> dbMenuItems = database->loadAllMenuItems();
        for (const auto& item : dbMenuItems) {
            menuItemsCache.insertItem(item.id, item);
            if (item.id >= nextMenuItemId) {
                nextMenuItemId = item.id + 1;
            }
        }
        cout << "Loaded " << dbMenuItems.size() << " menu items from database.\n";
    }
    
    void saveToDatabase() {
        cout << "Saving restaurants to database...\n";
        
        // Get all restaurants using helper method
        vector<Restaurant> restaurantList = getAllRestaurantsFromBTree();
        
        // Save to database
        if (database->saveAllRestaurants(restaurantList)) {
            cout << "✓ Saved " << restaurantList.size() << " restaurants\n";
        } else {
            cout << "✗ Failed to save restaurants\n";
        }
        
        // Save all menu items from database (they're already there from individual saves)
        vector<MenuItem> menuItemsList = database->loadAllMenuItems();
        
        if (!menuItemsList.empty()) {
            cout << "✓ " << menuItemsList.size() << " menu items already in database\n";
        }
    }
    
    void initializeSampleData() {
        // Initialize city graph with sample locations
        routingService->addRoad(0, 1, 500);
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
        int restaurantId1 = r1.getRestaurantId();
        restaurantService->addRestaurant(r1);
        database->saveRestaurant(r1);
        
        Restaurant r2(nextRestaurantId++, "Burger Barn", "456 Oak Ave", 
                     "555-2222", "American", 2);
        int restaurantId2 = r2.getRestaurantId();
        restaurantService->addRestaurant(r2);
        database->saveRestaurant(r2);
        
        // Add sample menu items
        MenuItem pizza1(nextMenuItemId++, "Margherita Pizza", 
                       "Classic tomato and cheese", 12.99, 50, "main", restaurantId1);
        restaurantService->addMenuItem(restaurantId1, pizza1);
        database->saveMenuItem(pizza1);
        
        MenuItem pizza2(nextMenuItemId++, "Pepperoni Pizza", 
                       "Pepperoni with extra cheese", 14.99, 30, "main", restaurantId1);
        restaurantService->addMenuItem(restaurantId1, pizza2);
        database->saveMenuItem(pizza2);
        
        MenuItem burger1(nextMenuItemId++, "Cheeseburger", 
                        "Beef patty with cheese", 8.99, 30, "main", restaurantId2);
        restaurantService->addMenuItem(restaurantId2, burger1);
        database->saveMenuItem(burger1);
        
        MenuItem fries(nextMenuItemId++, "French Fries", 
                      "Crispy golden fries", 3.99, 100, "side", restaurantId2);
        restaurantService->addMenuItem(restaurantId2, fries);
        database->saveMenuItem(fries);
        
        
       // Create sample riders with email and password
int nextRiderId = 1;

Rider rider1(nextRiderId++, "Mike Rider", "mike@quickbite.com", "rider123", 
              "5551234567", "motorcycle", 1);
riderService->addRider(rider1);
database->saveRider(rider1);

Rider rider2(nextRiderId++, "Sarah Driver", "sarah@quickbite.com", "delivery456", 
              "5559876543", "bike", 3);
riderService->addRider(rider2);
database->saveRider(rider2);

Rider rider3(nextRiderId++, "Tom Biker", "tom@quickbite.com", "bike789", 
              "5551112233", "bike", 4);
riderService->addRider(rider3);
database->saveRider(rider3);
        
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
            database->saveRestaurant(r);
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
        
        database->updateRestaurant(*r);
        
        return true;
    }
    
    bool removeRestaurant(int restaurantId) {
        bool success = restaurantService->removeRestaurant(restaurantId);
        if (success) {
            database->deleteRestaurant(restaurantId);
        }
        return success;
    }
    
    // ----- Menu Item Management -----
    bool addMenuItem(int restaurantId, const string& name, const string& description,
                 double price, int stock, const string& category) {
        MenuItem item(nextMenuItemId++, name, description, price, stock, category, restaurantId);
        bool success = restaurantService->addMenuItem(restaurantId, item);
        if (success) {
            cout << "Menu item added successfully. ID: " << (nextMenuItemId - 1) << "\n";
            menuItemsCache.insertItem(item.id, item);
            database->saveMenuItem(item);
        }
        return success;
    }
    
    MenuItem* getMenuItem(int restaurantId, int itemId) {
        MenuItem* item = menuItemsCache.getItem(itemId);
        
        if (!item) return nullptr;
        
        if (item->restaurantId == restaurantId) {
            return item;
        }
        
        return nullptr;
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
    // For backward compatibility - LinkedList version
    int placeOrder(int customerId, int restaurantId, const LinkedList<pair<int, int>>& items,
                   const string& deliveryAddress, int deliveryLocation) {
        vector<pair<int, int>> itemsVec;
        auto* currentItem = items.getHead();
        while (currentItem != nullptr) {
            itemsVec.push_back(currentItem->data);
            currentItem = currentItem->next;
        }
        
        map<int, vector<pair<int, int>>> restaurantItems;
        restaurantItems[restaurantId] = itemsVec;
        
        return placeMultiRestaurantOrder(customerId, restaurantItems, deliveryAddress, deliveryLocation);
    }
    
    // Overload for vector input (for convenience)
    int placeOrder(int customerId, int restaurantId, const vector<pair<int, int>>& itemsVec,
                   const string& deliveryAddress, int deliveryLocation) {
        map<int, vector<pair<int, int>>> restaurantItems;
        restaurantItems[restaurantId] = itemsVec;
        
        return placeMultiRestaurantOrder(customerId, restaurantItems, deliveryAddress, deliveryLocation);
    }
    
    // Multi-restaurant order placement
    int placeMultiRestaurantOrder(int customerId, 
                                  const map<int, vector<pair<int, int>>>& restaurantItems,
                                  const string& deliveryAddress, 
                                  int deliveryLocation) {
        UserData* customer = userService->getUser(customerId);
        if (!customer) {
            cout << "Customer not found.\n";
            return -1;
        }
        
        // Create order with the multi-restaurant constructor
        Order order(nextOrderId++, customerId, deliveryAddress, deliveryLocation);
        
        double totalAmount = 0.0;
        bool allItemsAvailable = true;
        int primaryRestaurantId = -1;
        
        // Process items for each restaurant
        for (const auto& restaurantEntry : restaurantItems) {
            int restaurantId = restaurantEntry.first;
            const vector<pair<int, int>>& items = restaurantEntry.second;
            
            Restaurant* restaurant = getRestaurant(restaurantId);
            if (!restaurant) {
                cout << "Restaurant " << restaurantId << " not found.\n";
                allItemsAvailable = false;
                break;
            }
            
            double restaurantSubtotal = 0.0;
            
            // Check stock and add items
            for (const auto& itemPair : items) {
                int itemId = itemPair.first;
                int quantity = itemPair.second;
                
                MenuItem* menuItem = menuItemsCache.getItem(itemId);
                if (!menuItem) {
                    cout << "Item " << itemId << " not found.\n";
                    allItemsAvailable = false;
                    break;
                }
                
                if (menuItem->restaurantId != restaurantId) {
                    cout << "Item " << itemId << " doesn't belong to restaurant " << restaurantId << "\n";
                    allItemsAvailable = false;
                    break;
                }
                
                if (menuItem->stock < quantity) {
                    cout << "Insufficient stock for item " << itemId 
                         << " in restaurant " << restaurantId 
                         << ". Available: " << menuItem->stock << ", Requested: " << quantity << "\n";
                    allItemsAvailable = false;
                    break;
                }
                
                order.addItem(restaurantId, itemId, menuItem->name, quantity, menuItem->price);
                restaurantSubtotal += quantity * menuItem->price;
                
                menuItem->stock -= quantity;
                
                if (!restaurant->hasMenuItemId(itemId)) {
                    restaurant->addMenuItemId(itemId);
                }
            }
            
            if (!allItemsAvailable) break;
            
            totalAmount += restaurantSubtotal;
            
            // Set primary restaurant if not set yet
            if (primaryRestaurantId == -1) {
                primaryRestaurantId = restaurantId;
            }
        }
        
        if (!allItemsAvailable) {
            cout << "Order failed: items unavailable.\n";
            
            for (const auto& restaurantEntry : restaurantItems) {
                const vector<pair<int, int>>& items = restaurantEntry.second;
                
                for (const auto& itemPair : items) {
                    int itemId = itemPair.first;
                    int quantity = itemPair.second;
                    
                    MenuItem* menuItem = menuItemsCache.getItem(itemId);
                    if (menuItem) {
                        menuItem->stock += quantity;
                    }
                }
            }
            
            return -1;
        }
        
        order.totalAmount = totalAmount;
        
        // Set the primary restaurant ID (first restaurant in the order)
        if (primaryRestaurantId != -1) {
            order.restaurantId = primaryRestaurantId;
        }
        
        // Add to order processing pipeline
        orderService->addOrder(order);
        pendingOrders.enqueue(order);
        
        cout << "Order #" << order.id << " placed successfully.\n";
        if (order.isMultiRestaurant()) {
            cout << "  Type: Multi-Restaurant\n";
            cout << "  Restaurants: " << order.restaurantCount << "\n";
        } else {
            cout << "  Type: Single Restaurant\n";
            cout << "  Restaurant: " << order.restaurantId << "\n";
        }
        cout << "  Total: $" << order.totalAmount << "\n";
        
        return order.id;
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
        
        OrderStatus orderStatus;
        if (status == "pending") orderStatus = OrderStatus::Pending;
        else if (status == "confirmed") orderStatus = OrderStatus::Confirmed;
        else if (status == "preparing") orderStatus = OrderStatus::Preparing;
        else if (status == "ready") orderStatus = OrderStatus::ReadyForPickup;
        else if (status == "dispatched") orderStatus = OrderStatus::Dispatched;
        else if (status == "in_transit") orderStatus = OrderStatus::InTransit;
        else if (status == "delivered") orderStatus = OrderStatus::Delivered;
        else if (status == "cancelled") orderStatus = OrderStatus::Cancelled;
        else {
            cout << "Invalid status: " << status << "\n";
            return false;
        }
        
        order->updateStatus(orderStatus);
        
        // Move through processing pipeline
        if (status == "confirmed") {
            if (!pendingOrders.isEmpty()) {
                Order o = pendingOrders.dequeue();
                preparingOrders.enqueue(o);
                cout << "Order #" << orderId << " moved to preparing.\n";
            }
        } else if (status == "ready") {
            if (!preparingOrders.isEmpty()) {
                Order o = preparingOrders.dequeue();
                readyOrders.enqueue(o);
                cout << "Order #" << orderId << " is ready for pickup.\n";
            }
        } else if (status == "dispatched") {
            assignRiderToOrder(order);
        } else if (status == "delivered") {
            if (order->riderId != -1) {
                Rider* rider = riderService->getRider(order->riderId);
                if (rider) {
                    rider->completeDelivery(true);
                    riderService->updateRiderStatus(order->riderId, "available");
                    availableRiders.enqueue(rider, calculateRiderPriority(rider));
                }
            }
            cout << "Order #" << orderId << " delivered successfully.\n";
        } else if (status == "cancelled") {
            // Restore stock
            for (int i = 0; i < order->itemCount; i++) {
                int itemId = order->items[i].itemId;
                int quantity = order->items[i].quantity;
                
                MenuItem* menuItem = menuItemsCache.getItem(itemId);
                if (menuItem) {
                    menuItem->stock += quantity;
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
    bool registerRider(const string& name, const string& email, const string& password, 
                   const string& phone, const string& vehicle, int location) {
    
    // Validate inputs
    if (email.empty() || password.empty()) {
        cout << "Error: Email and password are required\n";
        return false;
    }
    
    // Check if email already exists
    if (riderService->findRiderByEmail(email) != nullptr) {
        cout << "Error: Email already registered\n";
        return false;
    }
    
    // Create rider with email and password
    Rider rider(nextRiderId++, name, email, password, phone, vehicle, location);
    rider.setStatus("Active");  // Default status
    
    bool success = riderService->addRider(rider);
    
    if (success) {
        // Save to database
        database->saveRider(rider);
        
        cout << "✓ Rider registered successfully!\n";
        cout << "  Rider ID: " << (nextRiderId - 1) << "\n";
        cout << "  Name: " << name << "\n";
        cout << "  Email: " << email << "\n";
        cout << "  Status: Active\n";
        
        // Add to available riders queue if active
        Rider* riderPtr = riderService->getRider(rider.id);
        if (riderPtr) {
            availableRiders.enqueue(riderPtr, calculateRiderPriority(riderPtr));
            cout << "  Added to available riders queue\n";
        }
        
        return true;
    } else {
        cout << "✗ Failed to register rider\n";
        return false;
    }
}
    
    Rider* getRider(int riderId) {
        return riderService->getRider(riderId);
    }
    
    bool updateRiderLocation(int riderId, int location) {
        bool success = riderService->updateRiderLocation(riderId, location);
        if (success) {
            Rider* rider = getRider(riderId);
            if (rider && rider->status == "available") {
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
        
        while (!linkedPath.isEmpty()) {
            path.push_back(linkedPath.removeFront());
        }
        
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
        
        Rider* bestRider = riderService->findBestRider(
            order->deliveryLocation, 
            order->deliveryLocation
        );
        
        if (bestRider) {
            order->assignRider(bestRider->id);
            riderService->updateRiderStatus(bestRider->id, "busy");
            
            availableRiders.remove(bestRider);
            
            vector<int> route = getDeliveryRoute(
                bestRider->location,
                order->deliveryLocation
            );

            if (!route.empty()) {
                int distance = getDeliveryDistance(
                    bestRider->location,
                    order->deliveryLocation
                );
                
                int estimatedTime = estimateDeliveryTime(distance, bestRider->vehicle);
                cout << "Rider " << bestRider->name << " (ID: " << bestRider->id 
                     << ") assigned to Order #" << order->id 
                     << ". Estimated delivery time: " << estimatedTime << " minutes.\n";
                
                cout << "Route: ";
                for (size_t i = 0; i < route.size(); i++) {
                    cout << route[i];
                    if (i < route.size() - 1) {
                        cout << " -> ";
                    }
                }
                cout << " -> Destination\n";
                
                // For multi-restaurant orders, show restaurant list
                if (order->isMultiRestaurant()) {
                    cout << "Restaurants in Order #" << order->id << ": ";
                    for (int i = 0; i < order->restaurantCount; i++) {
                        Restaurant* restaurant = getRestaurant(order->restaurantIds[i]);
                        if (restaurant) {
                            cout << restaurant->getName();
                        } else {
                            cout << "Restaurant " << order->restaurantIds[i];
                        }
                        if (i < order->restaurantCount - 1) {
                            cout << " → ";
                        }
                    }
                    cout << "\n";
                }
            }
        } else {
            cout << "No available riders for Order #" << order->id << "\n";
        }
    }
    
    int calculateRiderPriority(Rider* rider) {
        if (!rider) return 1000;
        
        int priority = 100 - (int)(rider->rating * 10);
        priority -= rider->completedDeliveries / 10;
        
        return max(1, priority);
    }
    
    // Helper to get restaurant IDs from an order
    vector<int> getOrderRestaurantIds(const Order& order) {
        vector<int> restaurantIds;
        if (order.isMultiRestaurant()) {
            for (int i = 0; i < order.restaurantCount; i++) {
                restaurantIds.push_back(order.restaurantIds[i]);
            }
        } else {
            restaurantIds.push_back(order.restaurantId);
        }
        return restaurantIds;
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
        Queue<Order> tempQueue = pendingOrders;
        while (!tempQueue.isEmpty()) {
            Order o = tempQueue.dequeue();
            cout << "Order #" << o.id << " - Customer: " << o.customerId 
                 << " - Total: $" << o.totalAmount << "\n";
        }
    }
    
    void printPreparingOrders() {
        cout << "=== Preparing Orders (" << preparingOrders.size() << ") ===\n";
        Queue<Order> tempQueue = preparingOrders;
        while (!tempQueue.isEmpty()) {
            Order o = tempQueue.dequeue();
            cout << "Order #" << o.id << " - Customer: " << o.customerId 
                 << " - Restaurants: " << (o.isMultiRestaurant() ? o.restaurantCount : 1) << "\n";
        }
    }
    
    void printReadyOrders() {
        cout << "=== Ready Orders (" << readyOrders.size() << ") ===\n";
        Queue<Order> tempQueue = readyOrders;
        while (!tempQueue.isEmpty()) {
            Order o = tempQueue.dequeue();
            cout << "Order #" << o.id << " - Customer: " << o.customerId 
                 << " - Ready for pickup\n";
        }
    }
    
    void printAvailableRidersQueue() {
        cout << "=== Available Riders Queue (" << availableRiders.size() << ") ===\n";
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
        while (!pendingOrders.isEmpty()) pendingOrders.dequeue();
        while (!preparingOrders.isEmpty()) preparingOrders.dequeue();
        while (!readyOrders.isEmpty()) readyOrders.dequeue();
        
        while (!availableRiders.isEmpty()) availableRiders.dequeue();
        
        cout << "System data cleared (except persistent storage).\n";
    }
    
    void resetSystem() {
        clearAllData();
        
        nextOrderId = 1000;
        nextUserId = 100;
        nextRiderId = 200;
        nextRestaurantId = 300;
        nextMenuItemId = 1000;
        
        initializeSampleData();
        
        cout << "System reset to initial state.\n";
    }
    
    // ----- Export Methods for DatabaseManager Integration -----
    vector<Restaurant> exportRestaurants() {
        cout << "DEBUG: Exporting restaurants from SystemState...\n";
        vector<Restaurant> restaurantList = getAllRestaurantsFromBTree();
        cout << "DEBUG: Found " << restaurantList.size() << " restaurants in SystemState\n";
        return restaurantList;
    }
    
    vector<Order> exportOrders() {
        cout << "DEBUG: Exporting orders from SystemState...\n";
        vector<Order> orderList;
        
        // Collect from all order IDs
        for (int id = 1000; id < nextOrderId; id++) {
            Order* order = orderService->getOrder(id);
            if (order) {
                orderList.push_back(*order);
            }
        }
        
        cout << "DEBUG: Found " << orderList.size() << " orders\n";
        return orderList;
    }
    
    vector<Rider> exportRiders() {
        cout << "DEBUG: Exporting riders from SystemState...\n";
        vector<Rider> riderList;
        
        // Collect from all rider IDs
        for (int id = 200; id < nextRiderId; id++) {
            Rider* rider = riderService->getRider(id);
            if (rider) {
                riderList.push_back(*rider);
            }
        }
        
        cout << "DEBUG: Found " << riderList.size() << " riders\n";
        return riderList;
    }
    
    vector<MenuItem> exportMenuItems() {
        cout << "DEBUG: Exporting menu items from SystemState...\n";
        vector<MenuItem> menuItemsList = database->loadAllMenuItems();
        cout << "DEBUG: Found " << menuItemsList.size() << " menu items\n";
        return menuItemsList;
    }
};

#endif // SYSTEMSTATE_H