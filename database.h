#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include "models/User.h"
#include "models/Restaurant.h"
#include "models/Order.h"
#include "models/Rider.h"
#include "models/MenuItem.h"

using namespace std;
vector<Restaurant> loadAllRestaurants();
    bool saveRestaurant(const Restaurant& restaurant);
    bool saveAllRestaurants(const vector<Restaurant>& restaurants);
    bool deleteRestaurant(int restaurantId);  // ONLY ONE DECLARATION HERE
    
    // MenuItem methods
    vector<MenuItem> loadAllMenuItems();
    bool saveMenuItem(const MenuItem& item);
    bool saveAllMenuItems(const vector<MenuItem>& items);
    bool deleteMenuItem(int itemId);  // ONLY ONE DECLARATION
class Database {
private:
    const string USER_FILE = "users.dat";
    const string RESTAURANT_FILE = "restaurants.dat";
    const string ORDER_FILE = "orders.dat";
    const string RIDER_FILE = "riders.dat";
    const string MENU_ITEM_FILE = "menu_items.dat";
    
    // FIXED: Safe file operations with validation
    template<typename T>
    bool safeWriteRecord(ofstream& file, const T& record) {
        if (!file.is_open()) {
            cerr << "Error: File not open for writing\n";
            return false;
        }
        
        file.write(reinterpret_cast<const char*>(&record), sizeof(T));
        
        if (file.fail()) {
            cerr << "Error: Failed to write record\n";
            return false;
        }
        
        file.flush(); // Force write to disk
        return true;
    }
    
    template<typename T>
    bool safeReadRecord(ifstream& file, T& record) {
        if (!file.is_open()) {
            cerr << "Error: File not open for reading\n";
            return false;
        }
        
        file.read(reinterpret_cast<char*>(&record), sizeof(T));
        
        return !file.fail();
    }
    
    // FIXED: Validate file structure before reading
    template<typename T>
    bool validateFileStructure(const string& filename, size_t& recordCount) {
        ifstream file(filename, ios::binary);
        if (!file) {
            recordCount = 0;
            return true; // Empty file is valid
        }
        
        file.seekg(0, ios::end);
        size_t fileSize = file.tellg();
        file.close();
        
        cout << "DEBUG: File " << filename << " size: " << fileSize << " bytes\n";
        cout << "DEBUG: sizeof(T): " << sizeof(T) << " bytes\n";
        
        if (fileSize == 0) {
            recordCount = 0;
            return true;
        }
        
        if (fileSize % sizeof(T) != 0) {
            cout << "DEBUG: Expected records: " << (fileSize / sizeof(T)) << "\n";
            cout << "WARNING: File " << filename 
                 << " appears corrupted! Size doesn't align with record size.\n";
            recordCount = 0;
            return false;
        }
        
        recordCount = fileSize / sizeof(T);
        cout << "DEBUG: Successfully validated " << recordCount 
             << " records in " << filename << "\n";
        return true;
    }

public:
    Database() {
        cout << "Database initialized\n";
    }
    // Add these methods to your Database class:

// bool deleteRestaurant(int restaurantId) {
//     vector<Restaurant> allRestaurants = loadAllRestaurants();
//     vector<Restaurant> updatedRestaurants;
    
//     for (const auto& r : allRestaurants) {
//         if (r.getRestaurantId() != restaurantId) {
//             updatedRestaurants.push_back(r);
//         }
//     }
    
//     return saveAllRestaurants(updatedRestaurants);
// }

bool deleteMenuItem(int itemId) {
    vector<MenuItem> allItems = loadAllMenuItems();
    vector<MenuItem> updatedItems;
    
    for (const auto& item : allItems) {
        if (item.id != itemId) {
            updatedItems.push_back(item);
        }
    }
    
    return saveAllMenuItems(updatedItems);
}

bool deleteRider(int riderId) {
    vector<Rider> allRiders = loadAllRiders();
    vector<Rider> updatedRiders;
    
    for (const auto& rider : allRiders) {
        if (rider.getId() != riderId) {
            updatedRiders.push_back(rider);
        }
    }
    
    return saveAllRiders(updatedRiders);
}

bool updateUser(const UserData& user) {
    vector<UserData> allUsers = loadAllUsers();
    
    for (auto& u : allUsers) {
        if (u.id == user.id) {
            u = user;
            return saveAllUsers(allUsers);
        }
    }
    
    return false; // User not found
}
    // ===== USER OPERATIONS =====
    bool saveUser(const UserData& user) {
        ofstream file(USER_FILE, ios::binary | ios::app);
        if (!file) {
            cerr << "Error: Could not open " << USER_FILE << "\n";
            return false;
        }
        
        bool success = safeWriteRecord(file, user);
        file.close();
        return success;
    }
    
    bool saveAllUsers(const vector<UserData>& users) {
        ofstream file(USER_FILE, ios::binary | ios::trunc);
        if (!file) {
            cerr << "Error: Could not open " << USER_FILE << "\n";
            return false;
        }
        
        for (const auto& user : users) {
            if (!safeWriteRecord(file, user)) {
                file.close();
                return false;
            }
        }
        
        file.close();
        return true;
    }
    
    vector<UserData> loadAllUsers() {
        vector<UserData> users;
        size_t recordCount;
        
        if (!validateFileStructure<UserData>(USER_FILE, recordCount)) {
            return users; // Return empty vector if corrupted
        }
        
        ifstream file(USER_FILE, ios::binary);
        if (!file) {
            return users;
        }
        
        UserData temp;
        while (safeReadRecord(file, temp)) {
            users.push_back(temp);
        }
        
        file.close();
        cout << "DEBUG: Successfully read " << users.size() 
             << " records from " << USER_FILE << "\n";
        return users;
    }
    
    // ===== RESTAURANT OPERATIONS =====
    bool saveRestaurant(const Restaurant& restaurant) {
        // First, load all existing restaurants
        vector<Restaurant> restaurants = loadAllRestaurants();
        
        // Check if restaurant already exists
        bool found = false;
        for (auto& r : restaurants) {
            if (r.getRestaurantId() == restaurant.getRestaurantId()) {
                r = restaurant; // Update existing
                found = true;
                break;
            }
        }
        
        if (!found) {
            restaurants.push_back(restaurant); // Add new
        }
        
        // Save all restaurants
        return saveAllRestaurants(restaurants);
    }
    
    bool saveAllRestaurants(const vector<Restaurant>& restaurants) {
        ofstream file(RESTAURANT_FILE, ios::binary | ios::trunc);
        if (!file) {
            cerr << "Error: Could not open " << RESTAURANT_FILE << "\n";
            return false;
        }
        
        for (const auto& restaurant : restaurants) {
            if (!safeWriteRecord(file, restaurant)) {
                file.close();
                return false;
            }
        }
        
        file.close();
        return true;
    }
    
    vector<Restaurant> loadAllRestaurants() {
        vector<Restaurant> restaurants;
        size_t recordCount;
        
        if (!validateFileStructure<Restaurant>(RESTAURANT_FILE, recordCount)) {
            cout << "⚠️  Restaurant file corrupted. Returning empty vector.\n";
            return restaurants;
        }
        
        ifstream file(RESTAURANT_FILE, ios::binary);
        if (!file) {
            return restaurants;
        }
        
        Restaurant temp;
        while (safeReadRecord(file, temp)) {
            restaurants.push_back(temp);
        }
        
        file.close();
        return restaurants;
    }
    
    bool updateRestaurant(const Restaurant& restaurant) {
        return saveRestaurant(restaurant); // Uses the safe update method
    }
    
    bool deleteRestaurant(int restaurantId) {
        vector<Restaurant> restaurants = loadAllRestaurants();
        
        auto it = restaurants.begin();
        while (it != restaurants.end()) {
            if (it->getRestaurantId() == restaurantId) {
                it = restaurants.erase(it);
                return saveAllRestaurants(restaurants);
            } else {
                ++it;
            }
        }
        
        return false;
    }
    
    // ===== ORDER OPERATIONS =====
    bool saveOrder(const Order& order) {
        vector<Order> orders = loadAllOrders();
        
        bool found = false;
        for (auto& o : orders) {
            if (o.getOrderId() == order.getOrderId()) {
                o = order;
                found = true;
                break;
            }
        }
        
        if (!found) {
            orders.push_back(order);
        }
        
        return saveAllOrders(orders);
    }
    
    bool saveAllOrders(const vector<Order>& orders) {
        ofstream file(ORDER_FILE, ios::binary | ios::trunc);
        if (!file) {
            cerr << "Error: Could not open " << ORDER_FILE << "\n";
            return false;
        }
        
        for (const auto& order : orders) {
            if (!safeWriteRecord(file, order)) {
                file.close();
                return false;
            }
        }
        
        file.close();
        return true;
    }
    
    vector<Order> loadAllOrders() {
        vector<Order> orders;
        size_t recordCount;
        
        if (!validateFileStructure<Order>(ORDER_FILE, recordCount)) {
            cout << "⚠️  Order file corrupted. Returning empty vector.\n";
            return orders;
        }
        
        ifstream file(ORDER_FILE, ios::binary);
        if (!file) {
            return orders;
        }
        
        Order temp;
        while (safeReadRecord(file, temp)) {
            orders.push_back(temp);
        }
        
        file.close();
        return orders;
    }
    
    bool updateOrder(const Order& order) {
        return saveOrder(order);
    }
    
    // ===== RIDER OPERATIONS =====
    bool saveRider(const Rider& rider) {
        vector<Rider> riders = loadAllRiders();
        
        bool found = false;
        for (auto& r : riders) {
            if (r.getId() == rider.getId()) {
                r = rider;
                found = true;
                break;
            }
        }
        
        if (!found) {
            riders.push_back(rider);
        }
        
        return saveAllRiders(riders);
    }
    
    bool saveAllRiders(const vector<Rider>& riders) {
        ofstream file(RIDER_FILE, ios::binary | ios::trunc);
        if (!file) {
            cerr << "Error: Could not open " << RIDER_FILE << "\n";
            return false;
        }
        
        for (const auto& rider : riders) {
            if (!safeWriteRecord(file, rider)) {
                file.close();
                return false;
            }
        }
        
        file.close();
        return true;
    }
    
    vector<Rider> loadAllRiders() {
        vector<Rider> riders;
        size_t recordCount;
        
        if (!validateFileStructure<Rider>(RIDER_FILE, recordCount)) {
            return riders;
        }
        
        ifstream file(RIDER_FILE, ios::binary);
        if (!file) {
            return riders;
        }
        
        Rider temp;
        while (safeReadRecord(file, temp)) {
            riders.push_back(temp);
        }
        
        file.close();
        return riders;
    }
    
    bool updateRider(const Rider& rider) {
        return saveRider(rider);
    }
    bool saveMenuItem(const MenuItem& item) {
    // Load all existing items
    vector<MenuItem> items = loadAllMenuItems();
    
    // Check if item already exists
    bool found = false;
    for (auto& existing : items) {
        if (existing.id == item.id) {
            existing = item;  // Update existing
            found = true;
            break;
        }
    }
    
    // If not found, add as new
    if (!found) {
        items.push_back(item);
    }
    
    // Save all items back
    return saveAllMenuItems(items);
}
    bool saveAllMenuItems(const vector<MenuItem>& items) {
    ofstream file(MENU_ITEM_FILE, ios::binary);
    if (!file) {
        return false;
    }
    
    // Write header: "MENU" signature and count
    const char signature[] = "MENU";
    file.write(signature, 4);
    
    uint32_t count = static_cast<uint32_t>(items.size());
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));
    
    // Write each item
    for (const auto& item : items) {
        // Write ID
        int id = item.id;
        file.write(reinterpret_cast<const char*>(&id), sizeof(id));
        
        // Write restaurant ID
        int restaurantId = item.restaurantId;
        file.write(reinterpret_cast<const char*>(&restaurantId), sizeof(restaurantId));
        
        // Write name (with length prefix)
        string name = item.getName();
        uint32_t nameLen = static_cast<uint32_t>(name.length());
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
        file.write(name.c_str(), nameLen);
        
        // Write description
        string desc = item.getDescription();
        uint32_t descLen = static_cast<uint32_t>(desc.length());
        file.write(reinterpret_cast<const char*>(&descLen), sizeof(descLen));
        file.write(desc.c_str(), descLen);
        
        // Write price
        double price = item.price;
        file.write(reinterpret_cast<const char*>(&price), sizeof(price));
        
        // Write stock
        int stock = item.stock;
        file.write(reinterpret_cast<const char*>(&stock), sizeof(stock));
        
        // Write category
        string category = item.getCategory();
        uint32_t catLen = static_cast<uint32_t>(category.length());
        file.write(reinterpret_cast<const char*>(&catLen), sizeof(catLen));
        file.write(category.c_str(), catLen);
    }
    
    file.close();
    cout << "✓ Saved " << items.size() << " menu items to binary file" << endl;
    return true;
}
    
    vector<MenuItem> loadAllMenuItems() {
    vector<MenuItem> items;
    
    cout << "\n=== DEBUG loadAllMenuItems START ===\n";
    
    // Try binary file first
    ifstream binFile("menu_items.dat", ios::binary);
    if (binFile) {
        cout << "Found menu_items.dat\n";
        
        // Check file size
        binFile.seekg(0, ios::end);
        size_t fileSize = binFile.tellg();
        binFile.seekg(0, ios::beg);
        
        cout << "File size: " << fileSize << " bytes\n";
        
        if (fileSize < 8) {
            cout << "File too small (<8 bytes), treating as empty\n";
            binFile.close();
            cout << "=== DEBUG loadAllMenuItems END (empty) ===\n";
            return items;
        }
        
        // Read signature
        char signature[5];
        binFile.read(signature, 4);
        signature[4] = '\0';
        
        cout << "Signature: " << signature << "\n";
        
        if (string(signature) != "MENU") {
            cout << "Invalid signature '" << signature << "', expected 'MENU'\n";
            binFile.close();
            cout << "=== DEBUG loadAllMenuItems END (invalid) ===\n";
            return items;
        }
        
        // Read count
        uint32_t count;
        binFile.read(reinterpret_cast<char*>(&count), sizeof(count));
        cout << "Item count in file: " << count << "\n";
        
        // Calculate expected size
        // Header: 4 (signature) + 4 (count) = 8 bytes
        // Each item: ID(4) + RestaurantID(4) + NameLen(4) + Name + DescLen(4) + Desc + Price(8) + Stock(4) + CatLen(4) + Category
        size_t minSize = 8 + count * (4 + 4 + 4 + 0 + 4 + 0 + 8 + 4 + 4 + 0);
        
        if (fileSize < minSize) {
            cout << "File size " << fileSize << " < minimum expected " << minSize << "\n";
            binFile.close();
            cout << "=== DEBUG loadAllMenuItems END (size mismatch) ===\n";
            return items;
        }
        
        // Read items
        for (uint32_t i = 0; i < count; i++) {
            try {
                cout << "Reading item " << (i+1) << "/" << count << "...\n";
                
                // Read ID
                int id;
                binFile.read(reinterpret_cast<char*>(&id), sizeof(id));
                
                // Read restaurant ID
                int restaurantId;
                binFile.read(reinterpret_cast<char*>(&restaurantId), sizeof(restaurantId));
                
                // Read name length and name
                uint32_t nameLen;
                binFile.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
                string name(nameLen, '\0');
                binFile.read(&name[0], nameLen);
                
                // Read description length and description
                uint32_t descLen;
                binFile.read(reinterpret_cast<char*>(&descLen), sizeof(descLen));
                string description(descLen, '\0');
                binFile.read(&description[0], descLen);
                
                // Read price
                double price;
                binFile.read(reinterpret_cast<char*>(&price), sizeof(price));
                
                // Read stock
                int stock;
                binFile.read(reinterpret_cast<char*>(&stock), sizeof(stock));
                
                // Read category length and category
                uint32_t catLen;
                binFile.read(reinterpret_cast<char*>(&catLen), sizeof(catLen));
                string category(catLen, '\0');
                binFile.read(&category[0], catLen);
                
                // Create menu item
                MenuItem item(id, name, description, price, stock, category, restaurantId);
                items.push_back(item);
                
                cout << "  ✓ Loaded: ID=" << id << ", Name=" << name 
                     << ", Restaurant=" << restaurantId << ", Price=$" << price << "\n";
                    
            } catch (const exception& e) {
                cout << "✗ Error reading item " << i << ": " << e.what() << "\n";
                break;
            }
        }
        
        binFile.close();
        
        if (!items.empty()) {
            cout << "✓ Successfully loaded " << items.size() << "/" << count << " menu items from binary\n";
            cout << "=== DEBUG loadAllMenuItems END (success) ===\n";
            return items;
        }
    } else {
        cout << "Binary file not found\n";
    }
    
    // Fallback to text file
    cout << "Trying text file fallback...\n";
    ifstream textFile("menu_items.txt");
    if (textFile) {
        cout << "Found menu_items.txt\n";
        
        string line;
        size_t loaded = 0;
        while (getline(textFile, line)) {
            if (line.empty()) continue;
            
            vector<string> parts;
            size_t start = 0, end;
            while ((end = line.find('|', start)) != string::npos) {
                parts.push_back(line.substr(start, end - start));
                start = end + 1;
            }
            parts.push_back(line.substr(start));
            
            if (parts.size() >= 7) {
                try {
                    int id = stoi(parts[0]);
                    int restaurantId = stoi(parts[1]);
                    string name = parts[2];
                    string description = parts[3];
                    double price = stod(parts[4]);
                    int stock = stoi(parts[5]);
                    string category = parts[6];
                    
                    MenuItem item(id, name, description, price, stock, category, restaurantId);
                    items.push_back(item);
                    loaded++;
                    
                    cout << "  ✓ From text: ID=" << id << ", Name=" << name << "\n";
                } catch (const exception& e) {
                    cout << "✗ Error parsing line: " << e.what() << "\n";
                }
            }
        }
        
        textFile.close();
        
        if (loaded > 0) {
            cout << "✓ Loaded " << loaded << " items from text file\n";
            
            // Save to binary for next time
            saveAllMenuItems(items);
            
            cout << "=== DEBUG loadAllMenuItems END (text fallback) ===\n";
            return items;
        }
    }
    
    cout << "⚠ No menu items found in any format\n";
    cout << "=== DEBUG loadAllMenuItems END (empty) ===\n";
    return items;
}
    vector<MenuItem> loadMenuItemsByRestaurant(int restaurantId) {
    vector<MenuItem> allItems = loadAllMenuItems();
    vector<MenuItem> restaurantItems;
    
    cout << "DEBUG: Looking for menu items for restaurant " << restaurantId << endl;
    cout << "DEBUG: Total items loaded: " << allItems.size() << endl;
    
    for (const auto& item : allItems) {
        cout << "DEBUG: Item ID=" << item.id << ", RestaurantID=" << item.restaurantId 
             << ", Name=" << item.getName() << endl;
        
        if (item.restaurantId == restaurantId) {
            restaurantItems.push_back(item);
        }
    }
    
    cout << "DEBUG: Found " << restaurantItems.size() << " items for restaurant " << restaurantId << endl;
    return restaurantItems;
}
    
    bool updateMenuItem(const MenuItem& item) {
        return saveMenuItem(item);
    }
    
    // ===== UTILITY OPERATIONS =====
    void clearAllData() {
        ofstream(USER_FILE, ios::binary | ios::trunc).close();
        ofstream(RESTAURANT_FILE, ios::binary | ios::trunc).close();
        ofstream(ORDER_FILE, ios::binary | ios::trunc).close();
        ofstream(RIDER_FILE, ios::binary | ios::trunc).close();
        ofstream(MENU_ITEM_FILE, ios::binary | ios::trunc).close();
    }
    
    void printDatabaseStats() {
        cout << "\n========================================\n";
        cout << "    DATABASE STATISTICS         \n";
        cout << "========================================\n";
        
        cout << "• Users:        " << loadAllUsers().size() << " records\n";
        cout << "• Restaurants:  " << loadAllRestaurants().size() << " records\n";
        cout << "• Orders:       " << loadAllOrders().size() << " records\n";
        cout << "• Riders:       " << loadAllRiders().size() << " records\n";
        cout << "• Menu Items:   " << loadAllMenuItems().size() << " records\n";
        
        cout << "========================================\n";
    }
};

#endif // DATABASE_H