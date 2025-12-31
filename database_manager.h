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
#include "storage/SystemState.h"
#include "services/UserService.h"
#include <iostream>

using namespace std;

// Forward declaration
class SystemState;

class DatabaseManager {
private:
    Database db;
    HashTable<Rider> ridersHashTable;
    SystemState* systemState;
    UserManager userManager;
    
    void createSampleMenuItems(int restaurantId, const string& restaurantName) {
    cout << "Creating menu items for " << restaurantName << " (ID: " << restaurantId << ")\n";
    
    vector<MenuItem> menuItems;
    
    if (restaurantName == "Pizza Palace") {
        menuItems = {
            MenuItem(1000 + restaurantId * 10 + 1, "Margherita Pizza", "Classic tomato and cheese", 
                    8.99, 20, "Pizza", restaurantId),
            MenuItem(1000 + restaurantId * 10 + 2, "Pepperoni Pizza", "Pepperoni and cheese", 
                    9.99, 15, "Pizza", restaurantId),
            MenuItem(1000 + restaurantId * 10 + 3, "Garlic Bread", "Fresh baked garlic bread", 
                    4.99, 30, "Appetizer", restaurantId),
            MenuItem(1000 + restaurantId * 10 + 4, "Soft Drink", "Choice of soda", 
                    1.99, 50, "Beverage", restaurantId)
        };
    } else if (restaurantName == "Burger Barn") {
        menuItems = {
            MenuItem(2000 + restaurantId * 10 + 1, "Cheeseburger", "Beef patty with cheese", 
                    5.99, 25, "Burger", restaurantId),
            MenuItem(2000 + restaurantId * 10 + 2, "Double Burger", "Double beef patty", 
                    7.99, 20, "Burger", restaurantId),
            MenuItem(2000 + restaurantId * 10 + 3, "French Fries", "Crispy golden fries", 
                    2.99, 40, "Side", restaurantId),
            MenuItem(2000 + restaurantId * 10 + 4, "Milkshake", "Chocolate or vanilla", 
                    3.99, 30, "Beverage", restaurantId)
        };
    } else if (restaurantName == "Sushi Spot") {
        menuItems = {
            MenuItem(3000 + restaurantId * 10 + 1, "California Roll", "Crab, avocado, cucumber", 
                    6.99, 25, "Sushi", restaurantId),
            MenuItem(3000 + restaurantId * 10 + 2, "Salmon Sashimi", "Fresh salmon slices", 
                    8.99, 20, "Sashimi", restaurantId),
            MenuItem(3000 + restaurantId * 10 + 3, "Miso Soup", "Traditional Japanese soup", 
                    2.99, 35, "Soup", restaurantId),
            MenuItem(3000 + restaurantId * 10 + 4, "Green Tea", "Hot green tea", 
                    1.50, 50, "Beverage", restaurantId)
        };
    }
    
    // Save each menu item
    for (const auto& item : menuItems) {
        bool saved = db.saveMenuItem(item);
        cout << "  " << (saved ? "✓" : "✗") << " Saved: " << item.getName() 
             << " (ID: " << item.id << ")\n";
    }
    
    cout << "  → Created " << menuItems.size() << " menu items\n";
}
    /*UserData(int i, const string& n, const string& e, const string& p, 
             const string& r = "customer", const string& a = "", 
             const string& pass = "")*/
    void createSampleUsers() {
        UserData admin(0, "admin", "admin@gmail.com", "000-0000", 
                      "admin", "System Address", "admin123");
        UserData user1(1, "Ali Amir", "ali@gmail.com", "555-0100", 
                     "customer", "123 Customer St", "itu123");
        UserData user2(2, "Haider ", "haider@gmail.com", "555-0200", 
                       "customer", "456 Customer Ave","password456");
        
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
    cout << "\n========================================\n";
    cout << "  CREATING SAMPLE RIDERS               \n";
    cout << "========================================\n";
    
    // RIDER 1: Ali Khan
    int riderId1 = 10;
    // Check UserData constructor signature! It might be:
    // UserData(id, name, email, phone, role, password, address)
    UserData riderUser1(riderId1, "Ali Khan", "ali.khan@gmail.com", 
                       "0300-1111111", "rider", "Karachi Station", "rider123");
    db.saveUser(riderUser1);
    
    Rider rider1(riderId1, "Ali Khan", "ali.khan@gmail.com", "rider123",
                 "0300-1111111", "Bike", 4.8);
    rider1.setStatus("Active");
    rider1.completedDeliveries = 45;
    db.saveRider(rider1);
    ridersHashTable.insertItem(riderId1, rider1);
    
    cout << "✓ Created Rider #1:\n";
    cout << "  ID: " << riderId1 << "\n";
    cout << "  Name: Ali Khan\n";
    cout << "  Email: ali.khan@gmail.com\n";
    cout << "  Password: rider123\n";
    cout << "  Role: rider\n";  // Added role
    cout << "  Status: Active\n";
    cout << "  Rating: 4.8/5\n";
    cout << "  Vehicle: Bike\n";
    cout << "  Saved to: users.dat AND riders.dat\n\n";
    
    // RIDER 2: Fatima Ahmed
    int riderId2 = 11;
    // NOTE: Swapped password and role positions!
    UserData riderUser2(riderId2, "Fatima Ahmed", "fatima.ahmed@gmail.com", 
                       "0300-2222222", "rider", "Lahore Hub", "rider456");
    db.saveUser(riderUser2);
    
    Rider rider2(riderId2, "Fatima Ahmed", "fatima.ahmed@gmail.com", "rider456",
                 "0300-2222222", "Motorcycle", 4.9);
    rider2.setStatus("Active");
    rider2.completedDeliveries = 52;
    db.saveRider(rider2);
    ridersHashTable.insertItem(riderId2, rider2);
    
    cout << "✓ Created Rider #2:\n";
    cout << "  ID: " << riderId2 << "\n";
    cout << "  Name: Fatima Ahmed\n";
    cout << "  Email: fatima.ahmed@gmail.com\n";
    cout << "  Password: rider456\n";
    cout << "  Role: rider\n";  
    cout << "  Status: Active\n";
    cout << "  Rating: 4.9/5\n";
    cout << "  Vehicle: Motorcycle\n";
    cout << "  Saved to: users.dat AND riders.dat\n\n";
    
    // RIDER 3: Mike Wilson
    int riderId3 = 12;
    // NOTE: Swapped password and role positions!
    UserData riderUser3(riderId3, "Mike Wilson", "mike.wilson@gmail.com", 
                       "0300-3333333", "rider", "Islamabad center", "rider789");
    db.saveUser(riderUser3);
    
    Rider rider3(riderId3, "Mike Wilson", "mike.wilson@gmail.com", "rider789",
                 "0300-3333333", "Car", 4.7);
    rider3.setStatus("Active");
    rider3.completedDeliveries = 38;
    db.saveRider(rider3);
    ridersHashTable.insertItem(riderId3, rider3);
    
    cout << "✓ Created Rider #3:\n";
    cout << "  ID: " << riderId3 << "\n";
    cout << "  Name: Mike Wilson\n";
    cout << "  Email: mike.wilson@gmail.com\n";
    cout << "  Password: rider789\n";
    cout << "  Role: rider\n";  // Added role
    cout << "  Status: Active\n";
    cout << "  Rating: 4.7/5\n";
    cout << "  Vehicle: Car\n";
    cout << "  Saved to: users.dat AND riders.dat\n";
    
    cout << "========================================\n";
    cout << "  ✓ 3 RIDERS CREATED SUCCESSFULLY      \n";
    cout << "========================================\n\n";
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
    
    // Helper methods for SystemState export
    vector<Restaurant> exportSystemStateRestaurants();
    vector<Order> exportSystemStateOrders();
    vector<Rider> exportSystemStateRiders();
    
public:
   public:
    DatabaseManager() : ridersHashTable(10), systemState(nullptr), userManager() {}
    
    // Link SystemState
    void setSystemState(SystemState* state) {
        systemState = state;
        cout << "DEBUG: SystemState linked to DatabaseManager\n";
    }
    
    // PUBLIC: Database access methods
    Database& getDatabase() { return db; }
    HashTable<Rider>& getRidersHashTable() { return ridersHashTable; }
    
    // PUBLIC: Menu item operations
    void saveMenuItem(const MenuItem& item) {
        db.saveMenuItem(item);
    }
    
    vector<MenuItem> getMenuItemsByRestaurant(int restaurantId) {
        return db.loadMenuItemsByRestaurant(restaurantId);
    }
    
    bool updateMenuItem(const MenuItem& item) {
        return db.updateMenuItem(item);
    }
    
    // PUBLIC: Restaurant operations
    void updateRestaurant(const Restaurant& restaurant) {
        db.updateRestaurant(restaurant);
    }
    
    // PUBLIC: Order operations
    void updateOrder(const Order& order) {
        db.updateOrder(order);
    }
    UserManager& getUserManager() {
        return userManager;
    }
    
    // Const version
    const UserManager& getUserManager() const {
        return userManager;
    }

    // PUBLIC: Rider operations
    void updateRider(const Rider& rider) {
        db.updateRider(rider);
        ridersHashTable.insertItem(rider.getId(), rider);
    }
    
    // PUBLIC: Database management
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
    
    // PUBLIC: Initialization
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
    // Check if menu items file exists and has data
    bool menuItemsEmpty = false;
    {
        ifstream menuFile("menu_items.dat", ios::binary);
        if (!menuFile) {
            menuItemsEmpty = true;
        } else {
            menuFile.seekg(0, ios::end);
            menuItemsEmpty = (menuFile.tellg() <= 8); // Less than signature + count
            menuFile.close();
        }
    }
    
    bool usersEmpty = db.loadAllUsers().empty();
    bool restaurantsEmpty = db.loadAllRestaurants().empty();
    bool ridersEmpty = db.loadAllRiders().empty();
    
    if (usersEmpty || restaurantsEmpty || ridersEmpty || menuItemsEmpty) {
        cout << "Initializing empty database with sample data...\n";
        
        // Always recreate these
        if (usersEmpty) {
            cout << "\n--- Creating sample users ---\n";
            createSampleUsers();
        }
        
        if (restaurantsEmpty) {
            cout << "\n--- Creating sample restaurants ---\n";
            createSampleRestaurants();
        }
        
        if (ridersEmpty) {
            cout << "\n--- Creating sample riders ---\n";
            createSampleRiders();
        }
        
        if (menuItemsEmpty) {
            cout << "\n--- Creating sample menu items ---\n";
            // Force recreate menu items
            vector<Restaurant> restaurants = db.loadAllRestaurants();
            for (const auto& restaurant : restaurants) {
                createSampleMenuItems(restaurant.getRestaurantId(), restaurant.getName());
            }
        }
        
        initializeSampleCityMap(cityGraph);
        
        cout << "\n✓ Sample data created for empty database.\n";
    } else {
        cout << "✓ Database already contains data. Skipping initialization.\n";
    }
}
    
    void loadAllData(vector<Restaurant>& restaurants, 
                 vector<Order>& orders) {  // Remove UserManager parameter
    
    cout << "Loading data from database...\n";
    
    loadUsersIntoManager(userManager);  
    loadRestaurantsWithMenus(restaurants);
    orders = db.loadAllOrders();
    loadRidersIntoHashTable();
    
    cout << "✓ Data loaded successfully:\n";
    cout << "  - Users: " << userManager.getTotalUsers() << "\n";  // Use member variable
    cout << "  - Restaurants: " << restaurants.size() << "\n";
    cout << "  - Orders: " << orders.size() << "\n";
    cout << "  - Riders: " << ridersHashTable.getSize() << "\n";
}
   void saveAllData(const vector<Restaurant>& restaurants,
                 const vector<Order>& orders,
                 const UserManager& userManager) {
    
    cout << "\n========================================\n";
    cout << "     SAVING DATA TO DATABASE...        \n";
    cout << "========================================\n";
    
    bool allSaved = true;
    int savedCount = 0;
    
    // 1. Save users from UserManager (always do this first)
    try {
        cout << "DEBUG: Extracting users from UserManager...\n";
        vector<UserData> users = userManager.getAllUsersAsVector();
        cout << "DEBUG: Found " << users.size() << " users to save\n";
        
        if (db.saveAllUsers(users)) {
            cout << "✓ Saved " << users.size() << " users\n";
            savedCount++;
        } else {
            cout << "✗ Failed to save users\n";
            allSaved = false;
        }
    } catch (const exception& e) {
        cout << "✗ Error saving users: " << e.what() << "\n";
        allSaved = false;
    }
    
    // 2. Save restaurants (even if empty vector is passed)
    try {
        vector<Restaurant> allRestaurants = restaurants;
        
        // Add restaurants from SystemState if available
        if (systemState != nullptr) {
            cout << "DEBUG: Checking SystemState for additional restaurants...\n";
            try {
                vector<Restaurant> stateRestaurants = exportSystemStateRestaurants();
                cout << "DEBUG: Found " << stateRestaurants.size() << " restaurants in SystemState\n";
                
                for (const auto& r : stateRestaurants) {
                    bool exists = false;
                    for (const auto& existing : allRestaurants) {
                        if (existing.getRestaurantId() == r.getRestaurantId()) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        allRestaurants.push_back(r);
                    }
                }
            } catch (const exception& e) {
                cout << "⚠ Could not get restaurants from SystemState: " << e.what() << "\n";
            }
        }
        
        cout << "DEBUG: Saving " << allRestaurants.size() << " restaurants...\n";
        if (db.saveAllRestaurants(allRestaurants)) {
            cout << "✓ Saved " << allRestaurants.size() << " restaurants\n";
            savedCount++;
        } else {
            cout << "✗ Failed to save restaurants\n";
            allSaved = false;
        }
    } catch (const exception& e) {
        cout << "✗ Error saving restaurants: " << e.what() << "\n";
        allSaved = false;
    }
    
   // 3. Save menu items (load existing ones first)
try {
    cout << "DEBUG: Collecting and saving menu items...\n";
    vector<MenuItem> allMenuItems = db.loadAllMenuItems();
    
    // DEBUG: Show what was loaded
    cout << "DEBUG: Loaded " << allMenuItems.size() << " menu items from database\n";
    for (const auto& item : allMenuItems) {
        cout << "  Item: ID=" << item.id << ", Restaurant=" << item.restaurantId 
             << ", Name=" << item.getName() << ", Price=" << item.price << "\n";
    }
    
    // If no menu items in DB but we have restaurants, create sample items
    if (allMenuItems.empty()) {
        cout << "DEBUG: No menu items found, checking restaurants...\n";
        vector<Restaurant> existingRestaurants = db.loadAllRestaurants();
        cout << "DEBUG: Found " << existingRestaurants.size() << " restaurants\n";
        
        for (const auto& restaurant : existingRestaurants) {
            cout << "DEBUG: Creating menu items for " << restaurant.getName() 
                 << " (ID: " << restaurant.getRestaurantId() << ")\n";
            createSampleMenuItems(restaurant.getRestaurantId(), restaurant.getName());
        }
        
        // Reload after creating
        allMenuItems = db.loadAllMenuItems();
        cout << "DEBUG: Now have " << allMenuItems.size() << " menu items\n";
    }
    
    if (db.saveAllMenuItems(allMenuItems)) {
        cout << "✓ Saved " << allMenuItems.size() << " menu items\n";
        savedCount++;
    } else {
        cout << "✗ Failed to save menu items\n";
        allSaved = false;
    }
} catch (const exception& e) {
    cout << "✗ Error saving menu items: " << e.what() << "\n";
    allSaved = false;
}
    
    // 4. Save orders (even if empty)
    try {
        vector<Order> allOrders = orders;
        
        // Add orders from SystemState if available
        if (systemState != nullptr) {
            cout << "DEBUG: Checking SystemState for additional orders...\n";
            try {
                vector<Order> stateOrders = exportSystemStateOrders();
                cout << "DEBUG: Found " << stateOrders.size() << " orders in SystemState\n";
                
                for (const auto& o : stateOrders) {
                    bool exists = false;
                    for (const auto& existing : allOrders) {
                        if (existing.getOrderId() == o.getOrderId()) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        allOrders.push_back(o);
                    }
                }
            } catch (const exception& e) {
                cout << "⚠ Could not get orders from SystemState: " << e.what() << "\n";
            }
        }
        
        cout << "DEBUG: Saving " << allOrders.size() << " orders...\n";
        if (db.saveAllOrders(allOrders)) {
            cout << "✓ Saved " << allOrders.size() << " orders\n";
            savedCount++;
        } else {
            cout << "✗ Failed to save orders\n";
            allSaved = false;
        }
    } catch (const exception& e) {
        cout << "✗ Error saving orders: " << e.what() << "\n";
        allSaved = false;
    }
    
    // 5. Save riders (from hash table AND SystemState)
    try {
        vector<Rider> allRiders;
        
        // Get riders from SystemState if available
        if (systemState != nullptr) {
            cout << "DEBUG: Checking SystemState for riders...\n";
            try {
                vector<Rider> stateRiders = exportSystemStateRiders();
                cout << "DEBUG: Found " << stateRiders.size() << " riders in SystemState\n";
                allRiders.insert(allRiders.end(), stateRiders.begin(), stateRiders.end());
            } catch (const exception& e) {
                cout << "⚠ Could not get riders from SystemState: " << e.what() << "\n";
            }
        }
        
        // Add riders from hash table
        cout << "DEBUG: Collecting riders from hash table...\n";
        ridersHashTable.traverse([&](int id, Rider& rider) {
            // Check if rider already exists in the vector
            bool exists = false;
            for (const auto& existing : allRiders) {
                if (existing.getId() == rider.getId()) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                allRiders.push_back(rider);
            }
        });
        
        // If no riders found, create sample riders
        if (allRiders.empty()) {
            cout << "DEBUG: No riders found, creating sample riders...\n";
            createSampleRiders();
            
            // Re-collect from hash table after creating
            ridersHashTable.traverse([&](int id, Rider& rider) {
                allRiders.push_back(rider);
            });
        }
        
        cout << "DEBUG: Saving " << allRiders.size() << " riders...\n";
        if (db.saveAllRiders(allRiders)) {
            cout << "✓ Saved " << allRiders.size() << " riders\n";
            savedCount++;
        } else {
            cout << "✗ Failed to save riders\n";
            allSaved = false;
        }
    } catch (const exception& e) {
        cout << "✗ Error saving riders: " << e.what() << "\n";
        allSaved = false;
    }
    
    // 6. Create a backup after saving
    try {
        cout << "DEBUG: Creating backup of database...\n";
        backupDatabase();
        cout << "✓ Database backup created\n";
    } catch (const exception& e) {
        cout << "⚠ Could not create backup: " << e.what() << "\n";
    }
    
    // 7. Final summary
    cout << "\n========================================\n";
    if (allSaved) {
        cout << "       DATA SAVED SUCCESSFULLY!         \n";
    } else {
        cout << "       DATA SAVED WITH WARNINGS         \n";
    }
    cout << "========================================\n";
    
    // Print final statistics
    printStatistics();
    
    cout << "========================================\n";
    if (savedCount == 5) {
        cout << "✓ All 5 data categories saved successfully\n";
    } else {
        cout << "⚠ Only " << savedCount << "/5 data categories saved completely\n";
    }
    cout << "========================================\n";
}
};

#endif // DATABASE_MANAGER_H