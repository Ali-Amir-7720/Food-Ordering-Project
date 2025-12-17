#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include "models/User.h"
#include "models/Restaurant.h"
#include "models/Order.h"
#include "models/Rider.h"
#include "models/MenuItem.h"

using namespace std;

class Database {
private:
    const string USERS_FILE = "users.dat";
    const string RESTAURANTS_FILE = "restaurants.dat";
    const string ORDERS_FILE = "orders.dat";
    const string RIDERS_FILE = "riders.dat";
    const string MENU_ITEMS_FILE = "menu_items.dat";
    
    template<typename T>
    bool writeToFile(const string& filename, const T& data, bool append = false) {
        try {
            ios::openmode mode = ios::binary;
            if (append) mode |= ios::app;
            
            ofstream file(filename, mode);
            if (!file.is_open()) {
                cout << "Error: Cannot open " << filename << " for writing\n";
                return false;
            }
            
            file.write(reinterpret_cast<const char*>(&data), sizeof(T));
            if (!file.good()) {
                cout << "Error: Failed to write data to " << filename << "\n";
                file.close();
                return false;
            }
            
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "Exception in writeToFile: " << e.what() << endl;
            return false;
        }
    }
    
template<typename T>
vector<T> readAllFromFile(const string& filename) {
    vector<T> items;
    try {
        ifstream file(filename, ios::binary);
        if (!file.is_open()) {
            cout << "DEBUG: File " << filename << " doesn't exist (OK for first run)\n";
            return items; 
        }
        
        file.seekg(0, ios::end);
        long fileSize = file.tellg();
        file.seekg(0, ios::beg);
        
        cout << "DEBUG: File " << filename << " size: " << fileSize << " bytes\n";
        cout << "DEBUG: sizeof(T): " << sizeof(T) << " bytes\n";
        
        long expectedRecords = fileSize / sizeof(T);
        cout << "DEBUG: Expected records: " << expectedRecords << "\n";
        
        if (fileSize % sizeof(T) != 0) {
            cout << "WARNING: File " << filename << " appears corrupted! "
                 << "Size doesn't align with record size.\n";
            file.close();
            return items;
        }
        const long MAX_RECORDS = 10000;
        if (expectedRecords > MAX_RECORDS) {
            cout << "WARNING: File appears corrupted (too many records)\n";
            file.close();
            return items;
        }
        
        T item;
        long count = 0;
        while (file.read(reinterpret_cast<char*>(&item), sizeof(T)) && count < expectedRecords) {
            items.push_back(item);
            count++;
            
            if (count % 100 == 0) {
                cout << "DEBUG: Read " << count << " records...\n";
            }
        }
        
        cout << "DEBUG: Successfully read " << count << " records from " << filename << "\n";
        file.close();
        
    } catch (const exception& e) {
        cout << "EXCEPTION reading from " << filename << ": " << e.what() << endl;
    }
    return items;
}
    template<typename T>
    bool saveAllToFile(const string& filename, const vector<T>& items) {
        try {
            ofstream file(filename, ios::binary);
            if (!file.is_open()) {
                cout << "Error: Cannot open " << filename << "\n";
                return false;
            }
            
            for (const auto& item : items) {
                file.write(reinterpret_cast<const char*>(&item), sizeof(T));
                if (!file.good()) {
                    cout << "Error: Failed to write to " << filename << "\n";
                    file.close();
                    return false;
                }
            }
            
            file.close();
            return true;
        } catch (const exception& e) {
            cout << "Exception in saveAllToFile: " << e.what() << endl;
            return false;
        }
    }

public:
    Database() {}
    bool saveUser(const UserData& user) {
        return writeToFile(USERS_FILE, user, true);
    }
    
    vector<UserData> loadAllUsers() {
        return readAllFromFile<UserData>(USERS_FILE);
    }
    
    bool saveAllUsers(const vector<UserData>& users) {
        return saveAllToFile(USERS_FILE, users);
    }
    
    bool updateUser(const UserData& updatedUser) {
        vector<UserData> users = loadAllUsers();
        
        for (auto& user : users) {
            if (user.id == updatedUser.id) {
                user = updatedUser;
                return saveAllUsers(users);
            }
        }
        return false;
    }
    
    bool deleteUser(int userId) {
        vector<UserData> users = loadAllUsers();
        vector<UserData> updatedUsers;
        bool found = false;
        
        for (const auto& user : users) {
            if (user.id != userId) {
                updatedUsers.push_back(user);
            } else {
                found = true;
            }
        }
        
        return found ? saveAllUsers(updatedUsers) : false;
    }
    bool saveRestaurant(const Restaurant& restaurant) {
        return writeToFile(RESTAURANTS_FILE, restaurant, true);
    }
    
    vector<Restaurant> loadAllRestaurants() {
        return readAllFromFile<Restaurant>(RESTAURANTS_FILE);
    }
    
    bool saveAllRestaurants(const vector<Restaurant>& restaurants) {
        return saveAllToFile(RESTAURANTS_FILE, restaurants);
    }
    
    bool updateRestaurant(const Restaurant& updatedRestaurant) {
        vector<Restaurant> restaurants = loadAllRestaurants();
        
        for (auto& restaurant : restaurants) {
            if (restaurant.getRestaurantId() == updatedRestaurant.getRestaurantId()) {
                restaurant = updatedRestaurant;
                return saveAllRestaurants(restaurants);
            }
        }
        return false;
    }
    
    bool deleteRestaurant(int restaurantId) {
        vector<Restaurant> restaurants = loadAllRestaurants();
        vector<Restaurant> updatedRestaurants;
        bool found = false;
        
        for (const auto& restaurant : restaurants) {
            if (restaurant.getRestaurantId() != restaurantId) {
                updatedRestaurants.push_back(restaurant);
            } else {
                found = true;
            }
        }
        
        if (found) {
            vector<MenuItem> menuItems = loadAllMenuItems();
            vector<MenuItem> updatedMenuItems;
            
            for (const auto& item : menuItems) {
                if (item.restaurantId != restaurantId) {
                    updatedMenuItems.push_back(item);
                }
            }
            
            saveAllMenuItems(updatedMenuItems);
            return saveAllRestaurants(updatedRestaurants);
        }
        
        return false;
    }
    bool saveOrder(const Order& order) {
        return writeToFile(ORDERS_FILE, order, true);
    }
    
    vector<Order> loadAllOrders() {
        return readAllFromFile<Order>(ORDERS_FILE);
    }
    
    bool saveAllOrders(const vector<Order>& orders) {
        return saveAllToFile(ORDERS_FILE, orders);
    }
    
    bool updateOrder(const Order& updatedOrder) {
        vector<Order> orders = loadAllOrders();
        
        for (auto& order : orders) {
            if (order.getOrderId() == updatedOrder.getOrderId()) {
                order = updatedOrder;
                return saveAllOrders(orders);
            }
        }
        return false;
    }
    
    bool deleteOrder(int orderId) {
        vector<Order> orders = loadAllOrders();
        vector<Order> updatedOrders;
        bool found = false;
        
        for (const auto& order : orders) {
            if (order.getOrderId() != orderId) {
                updatedOrders.push_back(order);
            } else {
                found = true;
            }
        }
        
        return found ? saveAllOrders(updatedOrders) : false;
    }
    bool saveRider(const Rider& rider) {
        return writeToFile(RIDERS_FILE, rider, true);
    }
    
    vector<Rider> loadAllRiders() {
        return readAllFromFile<Rider>(RIDERS_FILE);
    }
    
    bool saveAllRiders(const vector<Rider>& riders) {
        return saveAllToFile(RIDERS_FILE, riders);
    }
    
    bool updateRider(const Rider& updatedRider) {
        vector<Rider> riders = loadAllRiders();
        
        for (auto& rider : riders) {
            if (rider.getId() == updatedRider.getId()) {
                rider = updatedRider;
                return saveAllRiders(riders);
            }
        }
        return false;
    }
    
    bool deleteRider(int riderId) {
        vector<Rider> riders = loadAllRiders();
        vector<Rider> updatedRiders;
        bool found = false;
        
        for (const auto& rider : riders) {
            if (rider.getId() != riderId) {
                updatedRiders.push_back(rider);
            } else {
                found = true;
            }
        }
        
        return found ? saveAllRiders(updatedRiders) : false;
    }
    bool saveMenuItem(const MenuItem& item) {
        return writeToFile(MENU_ITEMS_FILE, item, true);
    }
    
    vector<MenuItem> loadAllMenuItems() {
        return readAllFromFile<MenuItem>(MENU_ITEMS_FILE);
    }
    
    bool saveAllMenuItems(const vector<MenuItem>& items) {
        return saveAllToFile(MENU_ITEMS_FILE, items);
    }
    
    vector<MenuItem> loadMenuItemsByRestaurant(int restaurantId) {
        vector<MenuItem> allItems = loadAllMenuItems();
        vector<MenuItem> filteredItems;
        
        for (const auto& item : allItems) {
            if (item.restaurantId == restaurantId) {
                filteredItems.push_back(item);
            }
        }
        
        return filteredItems;
    }
    
    bool updateMenuItem(const MenuItem& updatedItem) {
        vector<MenuItem> items = loadAllMenuItems();
        
        for (auto& item : items) {
            if (item.id == updatedItem.id) {
                item = updatedItem;
                return saveAllMenuItems(items);
            }
        }
        return false;
    }
    
    bool deleteMenuItem(int itemId) {
        vector<MenuItem> items = loadAllMenuItems();
        vector<MenuItem> updatedItems;
        bool found = false;
        
        for (const auto& item : items) {
            if (item.id != itemId) {
                updatedItems.push_back(item);
            } else {
                found = true;
            }
        }
        
        return found ? saveAllMenuItems(updatedItems) : false;
    }
    
    // ========== DATABASE MANAGEMENT ==========
    void clearAllData() {
        try {
            remove(USERS_FILE.c_str());
            remove(RESTAURANTS_FILE.c_str());
            remove(ORDERS_FILE.c_str());
            remove(RIDERS_FILE.c_str());
            remove(MENU_ITEMS_FILE.c_str());
            cout << "✓ All database files cleared\n";
        } catch (const exception& e) {
            cout << "Exception in clearAllData: " << e.what() << endl;
        }
    }
    
    void printDatabaseStats() {
        cout << "\n╔════════════════════════════════╗\n";
        cout << "║    DATABASE STATISTICS         ║\n";
        cout << "╠════════════════════════════════╣\n";
        cout << "║ Users:        " << loadAllUsers().size() << " records\n";
        cout << "║ Restaurants:  " << loadAllRestaurants().size() << " records\n";
        cout << "║ Orders:       " << loadAllOrders().size() << " records\n";
        cout << "║ Riders:       " << loadAllRiders().size() << " records\n";
        cout << "║ Menu Items:   " << loadAllMenuItems().size() << " records\n";
        cout << "╚════════════════════════════════╝\n";
    }
    
    void debugFileSizes() {
        cout << "\n╔════════════════════════════════╗\n";
        cout << "║      DATABASE FILE SIZES       ║\n";
        cout << "╠════════════════════════════════╣\n";
        
        auto getFileSize = [](const string& filename) -> long {
            ifstream file(filename, ios::binary | ios::ate);
            if (!file.is_open()) return -1;
            return file.tellg();
        };
        
        auto printSize = [&](const string& name, const string& file) {
            long size = getFileSize(file);
            cout << "║ " << name;
            for (size_t i = name.length(); i < 18; i++) cout << " ";
            if (size == -1) {
                cout << "[Not Found]\n";
            } else {
                cout << size << " bytes\n";
            }
        };
        
        printSize("users.dat:", USERS_FILE);
        printSize("restaurants.dat:", RESTAURANTS_FILE);
        printSize("orders.dat:", ORDERS_FILE);
        printSize("riders.dat:", RIDERS_FILE);
        printSize("menu_items.dat:", MENU_ITEMS_FILE);
        
        cout << "╚════════════════════════════════╝\n";
    }
    
    bool isDatabaseEmpty() {
        return loadAllUsers().empty() && 
               loadAllRestaurants().empty() && 
               loadAllOrders().empty() && 
               loadAllRiders().empty() && 
               loadAllMenuItems().empty();
    }
    
    bool verifyDataIntegrity() {
        cout << "\nVerifying database integrity...\n";
        bool hasErrors = false;
        
        // Check for orphaned menu items
        vector<MenuItem> menuItems = loadAllMenuItems();
        vector<Restaurant> restaurants = loadAllRestaurants();
        
        for (const auto& item : menuItems) {
            bool found = false;
            for (const auto& restaurant : restaurants) {
                if (item.restaurantId == restaurant.getRestaurantId()) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                cout << "⚠ Warning: Menu item " << item.id 
                     << " has invalid restaurant ID " << item.restaurantId << "\n";
                hasErrors = true;
            }
        }
        
        if (!hasErrors) {
            cout << "✓ Database integrity check passed\n";
        }
        
        return !hasErrors;
    }
};

#endif // DATABASE_H