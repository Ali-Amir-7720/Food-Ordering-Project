
#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "Database.h"
#include "models/User.h"
#include "models/Restaurant.h"
#include "models/Order.h"
#include "models/Rider.h"
#include "models/MenuItem.h"
#include "dataStructures/HashTable.h"
#include "services/CityGraph.h"
#include <iostream>

using namespace std;

class DatabaseManager {
private:
    Database db;
    HashTable<Rider> ridersHashTable;
    
    void createSampleMenuItems(int restaurantId, const string& restaurantType) {
        if (restaurantType == "Pizza Palace") {
            db.saveMenuItem(MenuItem(1001, "Margherita Pizza", "Classic tomato and cheese", 
                                    8.99, 20, "Pizza", restaurantId));
            db.saveMenuItem(MenuItem(1002, "Pepperoni Pizza", "Pepperoni and cheese", 
                                    9.99, 15, "Pizza", restaurantId));
            db.saveMenuItem(MenuItem(1003, "Garlic Bread", "Fresh baked garlic bread", 
                                    4.99, 30, "Appetizer", restaurantId));
            db.saveMenuItem(MenuItem(1004, "Soft Drink", "Choice of soda", 
                                    1.99, 50, "Beverage", restaurantId));
        } else if (restaurantType == "Burger Barn") {
            db.saveMenuItem(MenuItem(2001, "Cheeseburger", "Beef patty with cheese", 
                                    5.99, 25, "Burger", restaurantId));
            db.saveMenuItem(MenuItem(2002, "Double Burger", "Double beef patty", 
                                    7.99, 20, "Burger", restaurantId));
            db.saveMenuItem(MenuItem(2003, "French Fries", "Crispy golden fries", 
                                    2.99, 40, "Side", restaurantId));
            db.saveMenuItem(MenuItem(2004, "Milkshake", "Chocolate or vanilla", 
                                    3.99, 30, "Beverage", restaurantId));
        } else if (restaurantType == "Sushi Spot") {
            db.saveMenuItem(MenuItem(3001, "California Roll", "Crab, avocado, cucumber", 
                                    6.99, 25, "Sushi", restaurantId));
            db.saveMenuItem(MenuItem(3002, "Salmon Sashimi", "Fresh salmon slices", 
                                    8.99, 20, "Sashimi", restaurantId));
            db.saveMenuItem(MenuItem(3003, "Miso Soup", "Traditional Japanese soup", 
                                    2.99, 35, "Soup", restaurantId));
            db.saveMenuItem(MenuItem(3004, "Green Tea", "Hot green tea", 
                                    1.50, 50, "Beverage", restaurantId));
        }
    }
    
    void createSampleUsers() {
        UserData admin(0, "Admin", "admin@quickbite.com", "000-0000", 
                      "admin123", "admin", "System Address");
        UserData user1(1, "John Doe", "john@example.com", "555-0100", 
                      "password123", "customer", "123 Customer St");
        UserData user2(2, "Alice Smith", "alice@example.com", "555-0200", 
                      "password456", "customer", "456 Customer Ave");
        
        db.saveUser(admin);
        db.saveUser(user1);
        db.saveUser(user2);
    }
    
    void createSampleRestaurants() {
        Restaurant r1(1, "Pizza Palace", "123 Main St", "555-0101", 
                     "Italian", 101, 4.5, 30);
        Restaurant r2(2, "Burger Barn", "456 Oak Ave", "555-0102", 
                     "American", 102, 4.2, 25);
        Restaurant r3(3, "Sushi Spot", "789 Pine Rd", "555-0103", 
                     "Japanese", 103, 4.7, 35);
        
        db.saveRestaurant(r1);
        db.saveRestaurant(r2);
        db.saveRestaurant(r3);
        
        createSampleMenuItems(1, "Pizza Palace");
        createSampleMenuItems(2, "Burger Barn");
        createSampleMenuItems(3, "Sushi Spot");
    }
    
    void createSampleRiders() {
        Rider rider1(1, "John Rider", "Active");
        Rider rider2(2, "Jane Delivery", "Active");
        
        db.saveRider(rider1);
        db.saveRider(rider2);
        
        ridersHashTable.insertItem(1, rider1);
        ridersHashTable.insertItem(2, rider2);
    }
    
    void loadUsersIntoManager(UserManager& userManager) {
        vector<UserData> users = db.loadAllUsers();
        for (const auto& user : users) {
            userManager.registerUser(user.id, user.name, user.email, 
                                   user.phone, user.password, 
                                   user.role, user.address);
        }
    }
    
    void loadRestaurantsWithMenus(vector<Restaurant>& restaurants) {
        vector<Restaurant> rawRestaurants = db.loadAllRestaurants();
        vector<MenuItem> allMenuItems = db.loadAllMenuItems();
        
        restaurants.clear();
        for (const auto& restaurant : rawRestaurants) {
            Restaurant newRestaurant(
                restaurant.getRestaurantId(),
                restaurant.getName(),
                restaurant.getAddress(),
                restaurant.getPhone(),
                restaurant.getCuisine(),
                restaurant.getLocationNode(),
                restaurant.getRating(),
                restaurant.getDeliveryTime()
            );
            
            for (const auto& item : allMenuItems) {
                if (item.restaurantId == restaurant.getRestaurantId()) {
                    newRestaurant.addMenuItemId(item.id);
                }
            }
            
            restaurants.push_back(newRestaurant);
        }
    }
    
    void loadRidersIntoHashTable() {
        vector<Rider> riders = db.loadAllRiders();
        ridersHashTable.clear();
        for (const auto& rider : riders) {
            ridersHashTable.insertItem(rider.getId(), rider);
        }
    }
    
    void saveRidersFromHashTable() {
        vector<Rider> riders;
        ridersHashTable.traverse([&](int id, Rider& rider) {
            riders.push_back(rider);
        });
        
        if (!db.saveAllRiders(riders)) {
            cout << "Error: Failed to save riders\n";
        }
    }
    
public:
    DatabaseManager() : ridersHashTable(10) {}
    void initializeSampleCityMap(CityGraph& cityGraph) {
        cout << "Initializing city map with sample locations...\n";

        cityGraph.addLocation(101, "Pizza Palace Area", "restaurant");
        cityGraph.addLocation(102, "Burger Barn Area", "restaurant");
        cityGraph.addLocation(103, "Sushi Spot Area", "restaurant");
        cityGraph.addLocation(201, "Customer District A", "home");
        cityGraph.addLocation(202, "Customer District B", "home");
        cityGraph.addLocation(203, "Customer District C", "home");
        cityGraph.addLocation(301, "Rider Station 1", "rider");
        cityGraph.addLocation(302, "Rider Station 2", "rider");
        cityGraph.addLocation(400, "City Center", "landmark");
        cityGraph.addRoad(101, 400, 1000);  
        cityGraph.addRoad(102, 400, 1200);  
        cityGraph.addRoad(103, 400, 800);   
        cityGraph.addRoad(400, 201, 1500);  
        cityGraph.addRoad(400, 202, 1800);  
        cityGraph.addRoad(400, 203, 2000);  
        cityGraph.addRoad(301, 400, 500);   
        cityGraph.addRoad(302, 400, 600);   
        cityGraph.addRoad(101, 201, 2000); 
        cityGraph.addRoad(102, 202, 1900);  
        cityGraph.addRoad(103, 203, 2200); 
        cityGraph.addRoad(201, 202, 1000); 
        cityGraph.addRoad(202, 203, 1100); 
        cout << "✓ City map initialized with 9 locations and 13 roads\n";
    }
    
    void initializeSampleData(CityGraph& cityGraph) {
        bool usersEmpty = db.loadAllUsers().empty();
        bool restaurantsEmpty = db.loadAllRestaurants().empty();
        bool ridersEmpty = db.loadAllRiders().empty();
        
        if (usersEmpty && restaurantsEmpty && ridersEmpty) {
            cout << "Initializing empty database with sample data...\n";
            
            createSampleUsers();
            createSampleRestaurants();
            createSampleRiders();
            initializeSampleCityMap(cityGraph);
            
            cout << "✓ Sample data created for empty database.\n";
        } else {
            cout << "✓ Database already contains data. Skipping initialization.\n";
        }
    }
    
    void loadAllData(vector<Restaurant>& restaurants, 
                     vector<Order>& orders,
                     UserManager& userManager) {
        
        cout << "Loading data from database...\n";
        
        loadUsersIntoManager(userManager);
        loadRestaurantsWithMenus(restaurants);
        orders = db.loadAllOrders();
        loadRidersIntoHashTable();
        
        cout << "✓ Data loaded successfully:\n";
        cout << "  - Users: " << userManager.getTotalUsers() << "\n";
        cout << "  - Restaurants: " << restaurants.size() << "\n";
        cout << "  - Orders: " << orders.size() << "\n";
        cout << "  - Riders: " << ridersHashTable.getSize() << "\n";
    }
    
    void saveAllData(const vector<Restaurant>& restaurants,
                     const vector<Order>& orders,
                     const UserManager& userManager) {
        
        cout << "Saving all data to database...\n";
        
        if (!db.saveAllRestaurants(restaurants)) {
            cout << "Error: Failed to save restaurants\n";
            return;
        }
        
        if (!db.saveAllOrders(orders)) {
            cout << "Error: Failed to save orders\n";
            return;
        }
        
        saveRidersFromHashTable();
        
        vector<UserData> users = userManager.getAllUsersAsVector();
        if (!db.saveAllUsers(users)) {
            cout << "Error: Failed to save users\n";
            return;
        }
        
        vector<MenuItem> allMenuItems = db.loadAllMenuItems();
        if (!db.saveAllMenuItems(allMenuItems)) {
            cout << "Error: Failed to save menu items\n";
            return;
        }
        
        cout << "✓ All data saved to database.\n";
    }
    
    Database& getDatabase() { return db; }
    HashTable<Rider>& getRidersHashTable() { return ridersHashTable; }
    
    void saveMenuItem(const MenuItem& item) {
        db.saveMenuItem(item);
    }
    
    vector<MenuItem> getMenuItemsByRestaurant(int restaurantId) {
        return db.loadMenuItemsByRestaurant(restaurantId);
    }
    
    bool updateMenuItem(const MenuItem& item) {
        return db.updateMenuItem(item);
    }
    
    void updateRestaurant(const Restaurant& restaurant) {
        db.updateRestaurant(restaurant);
    }
    
    void updateOrder(const Order& order) {
        db.updateOrder(order);
    }
    
    void updateRider(const Rider& rider) {
        db.updateRider(rider);
        ridersHashTable.insertItem(rider.getId(), rider);
    }
    
    void backupDatabase() {
        cout << "Creating database backup...\n";
        
        #ifdef _WIN32
            system("copy users.dat users_backup.dat >nul 2>&1");
            system("copy restaurants.dat restaurants_backup.dat >nul 2>&1");
            system("copy orders.dat orders_backup.dat >nul 2>&1");
            system("copy riders.dat riders_backup.dat >nul 2>&1");
            system("copy menu_items.dat menu_items_backup.dat >nul 2>&1");
        #else
            system("cp users.dat users_backup.dat 2>/dev/null");
            system("cp restaurants.dat restaurants_backup.dat 2>/dev/null");
            system("cp orders.dat orders_backup.dat 2>/dev/null");
            system("cp riders.dat riders_backup.dat 2>/dev/null");
            system("cp menu_items.dat menu_items_backup.dat 2>/dev/null");
        #endif
        
        cout << "✓ Backup created successfully!\n";
    }
    
    void restoreDatabase() {
        cout << "Restoring database from backup...\n";
        
        #ifdef _WIN32
            system("copy users_backup.dat users.dat >nul 2>&1");
            system("copy restaurants_backup.dat restaurants.dat >nul 2>&1");
            system("copy orders_backup.dat orders.dat >nul 2>&1");
            system("copy riders_backup.dat riders.dat >nul 2>&1");
            system("copy menu_items_backup.dat menu_items.dat >nul 2>&1");
        #else
            system("cp users_backup.dat users.dat 2>/dev/null");
            system("cp restaurants_backup.dat restaurants.dat 2>/dev/null");
            system("cp orders_backup.dat orders.dat 2>/dev/null");
            system("cp riders_backup.dat riders.dat 2>/dev/null");
            system("cp menu_items_backup.dat menu_items.dat 2>/dev/null");
        #endif
        
        cout << "✓ Database restored from backup!\n";
    }
    
    void clearAllData() {
        db.clearAllData();
        ridersHashTable.clear();
        cout << "✓ All data cleared from database.\n";
    }
    
    void printStatistics() {
        db.printDatabaseStats();
    }
};

#endif // DATABASE_MANAGER_H