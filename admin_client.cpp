// Enhanced admin_client.cpp - All Features Implemented
#include "Client.h"
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <sstream>
#include <map>
#include <iomanip>
#include <ctime>
using namespace std;

QuickBiteClient client;

// Function prototypes
void adminLogin();
void manageRestaurants();
void manageMenuItems();
void viewAllOrders();
void updateOrderStatus();
void manageRiders();
void assignRiderToOrder();
void manageUsers();
void viewUserStatistics();
void viewCityMap();
void viewSystemStatistics();
void addRestaurant();
void removeRestaurant();
void addMenuItem();
void removeMenuItem();
void addRider();
void removeRider();
void changeUserRole();

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseScreen() {
    cout << "\nPress Enter to continue...";
    cin.get();
}

void showMainMenu() {
    cout << "╔═══════════════════════════════════════╗\n";
    cout << "║     QUICKBITE DELIVERY SYSTEM         ║\n";
    cout << "║       ADMIN APPLICATION               ║\n";
    cout << "╚═══════════════════════════════════════╝\n";
    cout << "1. Login as Admin\n";
    cout << "2. Exit\n";
    cout << "========================================\n";
}

void showAdminMenu() {
    cout << "\n╔═══════════════════════════════════════╗\n";
    cout << "║         ADMIN DASHBOARD               ║\n";
    cout << "╚═══════════════════════════════════════╝\n";
    cout << "Welcome, " << client.getUserName() << "! 👨‍💼\n";
    cout << "========================================\n";
    cout << "1.  Manage Restaurants\n";
    cout << "2.  Manage Menu Items\n";
    cout << "3.  View All Orders\n";
    cout << "4.  Update Order Status\n";
    cout << "5.  Manage Riders\n";
    cout << "6.  Assign Rider to Order\n";
    cout << "7.  Manage Users\n";
    cout << "8.  View User Statistics\n";
    cout << "9.  City Map Information\n";
    cout << "10. System Statistics\n";
    cout << "11. Logout\n";
    cout << "========================================\n";
}

void adminLogin() {
    clearScreen();
    cout << "========================================\n";
    cout << "            ADMIN LOGIN                 \n";
    cout << "========================================\n";
    
    string email, password;
    
    cout << "Email: ";
    getline(cin, email);
    
    cout << "Password: ";
    getline(cin, password);
    
    cout << "\nConnecting to server...\n";
    
    if (client.login(email, password)) {
        if (client.getUserRole() == "admin") {
            cout << "✓ Admin login successful!\n";
            pauseScreen();
        } else {
            cout << "✗ Access denied! Not an admin account.\n";
            client.logout();
            pauseScreen();
        }
    } else {
        cout << "✗ Login failed! Invalid credentials.\n";
        pauseScreen();
    }
}

// === RESTAURANT MANAGEMENT FUNCTIONS ===

void addRestaurant() {
    clearScreen();
    cout << "========================================\n";
    cout << "         ADD NEW RESTAURANT            \n";
    cout << "========================================\n";
    
    string name, cuisine, address, deliveryTimeStr;
    double rating;
    
    cout << "Restaurant Name: ";
    cin.ignore();
    getline(cin, name);
    
    cout << "Cuisine Type: ";
    getline(cin, cuisine);
    
    cout << "Address: ";
    getline(cin, address);
    
    cout << "Rating (0-5): ";
    cin >> rating;
    
    cout << "Delivery Time (minutes): ";
    cin >> deliveryTimeStr;
    
    // Prepare data for server
    string data = name + "|" + cuisine + "|" + address + "|" + 
                  to_string(rating) + "|" + deliveryTimeStr;
    
    // Send command to server (you'll need to add this command to your server)
    string response = client.sendCommand("ADD_RESTAURANT", data);
    
    if (response.substr(0, 7) == "SUCCESS") {
        cout << "\n✓ Restaurant added successfully!\n";
        cout << "Restaurant ID: " << response.substr(8) << "\n";
    } else {
        cout << "\n✗ Failed to add restaurant: " << response << "\n";
    }
    
    pauseScreen();
}

void removeRestaurant() {
    clearScreen();
    cout << "========================================\n";
    cout << "         REMOVE RESTAURANT             \n";
    cout << "========================================\n";
    
    // First show all restaurants
    vector<string> restaurants = client.getRestaurants();
    
    if (restaurants.empty()) {
        cout << "\n✗ No restaurants available.\n";
        pauseScreen();
        return;
    }
    
    cout << "\nAvailable Restaurants:\n";
    cout << "----------------------------------------\n";
    
    for (size_t i = 0; i < restaurants.size(); i++) {
        string restaurant = restaurants[i];
        size_t pos1 = restaurant.find(';');
        if (pos1 != string::npos) {
            string id = restaurant.substr(0, pos1);
            string name = restaurant.substr(pos1 + 1);
            size_t pos2 = name.find(';');
            if (pos2 != string::npos) {
                name = name.substr(0, pos2);
            }
            cout << "  [" << id << "] " << name << "\n";
        }
    }
    
    int restaurantId;
    cout << "\nEnter Restaurant ID to remove: ";
    cin >> restaurantId;
    
    // Send command to server (you'll need to add this command to your server)
    string response = client.sendCommand("REMOVE_RESTAURANT", to_string(restaurantId));
    
    if (response.substr(0, 7) == "SUCCESS") {
        cout << "\n✓ Restaurant removed successfully!\n";
    } else {
        cout << "\n✗ Failed to remove restaurant: " << response << "\n";
    }
    
    pauseScreen();
}

void manageRestaurants() {
    while (true) {
        clearScreen();
        cout << "========================================\n";
        cout << "         MANAGE RESTAURANTS            \n";
        cout << "========================================\n";
        cout << "1. View All Restaurants\n";
        cout << "2. View Restaurant Details\n";
        cout << "3. Add New Restaurant\n";
        cout << "4. Remove Restaurant\n";
        cout << "5. Back to Admin Menu\n";
        cout << "========================================\n";
        
        int choice;
        cout << "\nEnter choice (1-5): ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore();
        
        switch (choice) {
            case 1: {
                clearScreen();
                cout << "========================================\n";
                cout << "         ALL RESTAURANTS               \n";
                cout << "========================================\n";
                
                vector<string> restaurants = client.getRestaurants();
                
                if (restaurants.empty()) {
                    cout << "\n✗ No restaurants available.\n";
                } else {
                    cout << "\nTotal Restaurants: " << restaurants.size() << "\n\n";
                    
                    for (size_t i = 0; i < restaurants.size(); i++) {
                        string restaurant = restaurants[i];
                        
                        size_t pos1 = restaurant.find(';');
                        size_t pos2 = restaurant.find(';', pos1 + 1);
                        size_t pos3 = restaurant.find(';', pos2 + 1);
                        size_t pos4 = restaurant.find(';', pos3 + 1);
                        size_t pos5 = restaurant.find(';', pos4 + 1);
                        
                        if (pos1 != string::npos && pos5 != string::npos) {
                            string id = restaurant.substr(0, pos1);
                            string name = restaurant.substr(pos1 + 1, pos2 - pos1 - 1);
                            string cuisine = restaurant.substr(pos2 + 1, pos3 - pos2 - 1);
                            string address = restaurant.substr(pos3 + 1, pos4 - pos3 - 1);
                            string rating = restaurant.substr(pos4 + 1, pos5 - pos4 - 1);
                            string deliveryTime = restaurant.substr(pos5 + 1);
                            
                            cout << "┌─────────────────────────────────────┐\n";
                            cout << "│ [" << id << "] " << name << "\n";
                            cout << "├─────────────────────────────────────┤\n";
                            cout << "│ Cuisine: " << cuisine << "\n";
                            cout << "│ Address: " << address << "\n";
                            cout << "│ Rating: " << rating << "/5 ⭐\n";
                            cout << "│ Delivery: " << deliveryTime << " mins\n";
                            cout << "└─────────────────────────────────────┘\n\n";
                        }
                    }
                }
                pauseScreen();
                break;
            }
            case 2: {
                int restaurantId;
                cout << "\nEnter Restaurant ID: ";
                cin >> restaurantId;
                cin.ignore();
                
                vector<string> menuItems = client.getMenu(restaurantId);
                
                clearScreen();
                cout << "========================================\n";
                cout << "      RESTAURANT MENU ITEMS            \n";
                cout << "========================================\n";
                
                if (menuItems.empty()) {
                    cout << "\n✗ No menu items found.\n";
                } else {
                    cout << "\nTotal Items: " << menuItems.size() << "\n\n";
                    
                    for (const auto& item : menuItems) {
                        size_t pos1 = item.find(';');
                        size_t pos2 = item.find(';', pos1 + 1);
                        size_t pos3 = item.find(';', pos2 + 1);
                        size_t pos4 = item.find(';', pos3 + 1);
                        size_t pos5 = item.find(';', pos4 + 1);
                        
                        if (pos1 != string::npos && pos5 != string::npos) {
                            string id = item.substr(0, pos1);
                            string name = item.substr(pos1 + 1, pos2 - pos1 - 1);
                            string description = item.substr(pos2 + 1, pos3 - pos2 - 1);
                            string price = item.substr(pos3 + 1, pos4 - pos3 - 1);
                            string stock = item.substr(pos4 + 1, pos5 - pos4 - 1);
                            string category = item.substr(pos5 + 1);
                            
                            cout << "┌─────────────────────────────────────┐\n";
                            cout << "│ [" << id << "] " << name << "\n";
                            cout << "├─────────────────────────────────────┤\n";
                            cout << "│ " << description << "\n";
                            cout << "│ Price: $" << price << " | Stock: " << stock << "\n";
                            cout << "│ Category: " << category << "\n";
                            cout << "└─────────────────────────────────────┘\n\n";
                        }
                    }
                }
                pauseScreen();
                break;
            }
            case 3:
                addRestaurant();
                break;
            case 4:
                removeRestaurant();
                break;
            case 5:
                return;
        }
    }
}

// === MENU ITEM MANAGEMENT FUNCTIONS ===

void addMenuItem() {
    clearScreen();
    cout << "========================================\n";
    cout << "         ADD MENU ITEM                 \n";
    cout << "========================================\n";
    
    int restaurantId;
    string name, description, category, priceStr, stockStr;
    double price;
    int stock;
    
    cout << "Restaurant ID: ";
    cin >> restaurantId;
    cin.ignore();
    
    cout << "Item Name: ";
    getline(cin, name);
    
    cout << "Description: ";
    getline(cin, description);
    
    cout << "Category: ";
    getline(cin, category);
    
    cout << "Price: $";
    getline(cin, priceStr);
    price = stod(priceStr);
    
    cout << "Stock Quantity: ";
    getline(cin, stockStr);
    stock = stoi(stockStr);
    
    // Prepare data for server
    string data = to_string(restaurantId) + "|" + name + "|" + description + "|" +
                  to_string(price) + "|" + to_string(stock) + "|" + category;
    
    // Send command to server
    string response = client.sendCommand("ADD_MENU_ITEM", data);
    
    if (response.substr(0, 7) == "SUCCESS") {
        cout << "\n✓ Menu item added successfully!\n";
        cout << "Item ID: " << response.substr(8) << "\n";
    } else {
        cout << "\n✗ Failed to add menu item: " << response << "\n";
    }
    
    pauseScreen();
}

void removeMenuItem() {
    clearScreen();
    cout << "========================================\n";
    cout << "         REMOVE MENU ITEM              \n";
    cout << "========================================\n";
    
    int restaurantId, itemId;
    
    cout << "Restaurant ID: ";
    cin >> restaurantId;
    cin.ignore();
    
    // Show menu items for this restaurant
    vector<string> menuItems = client.getMenu(restaurantId);
    
    if (menuItems.empty()) {
        cout << "\n✗ No menu items found for this restaurant.\n";
        pauseScreen();
        return;
    }
    
    cout << "\nAvailable Menu Items:\n";
    cout << "----------------------------------------\n";
    
    for (const auto& item : menuItems) {
        size_t pos1 = item.find(';');
        size_t pos2 = item.find(';', pos1 + 1);
        
        if (pos1 != string::npos && pos2 != string::npos) {
            string id = item.substr(0, pos1);
            string name = item.substr(pos1 + 1, pos2 - pos1 - 1);
            cout << "  [" << id << "] " << name << "\n";
        }
    }
    
    cout << "\nEnter Menu Item ID to remove: ";
    cin >> itemId;
    
    // Prepare data for server
    string data = to_string(restaurantId) + "|" + to_string(itemId);
    
    // Send command to server
    string response = client.sendCommand("REMOVE_MENU_ITEM", data);
    
    if (response.substr(0, 7) == "SUCCESS") {
        cout << "\n✓ Menu item removed successfully!\n";
    } else {
        cout << "\n✗ Failed to remove menu item: " << response << "\n";
    }
    
    pauseScreen();
}

void manageMenuItems() {
    while (true) {
        clearScreen();
        cout << "========================================\n";
        cout << "         MANAGE MENU ITEMS             \n";
        cout << "========================================\n";
        cout << "1. View Restaurant Menu\n";
        cout << "2. Add Menu Item\n";
        cout << "3. Remove Menu Item\n";
        cout << "4. Back to Admin Menu\n";
        cout << "========================================\n";
        
        int choice;
        cout << "\nEnter choice (1-4): ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore();
        
        switch (choice) {
            case 1: {
                int restaurantId;
                cout << "\nEnter Restaurant ID: ";
                cin >> restaurantId;
                cin.ignore();
                
                vector<string> menuItems = client.getMenu(restaurantId);
                
                clearScreen();
                cout << "========================================\n";
                cout << "         MENU ITEMS                    \n";
                cout << "========================================\n";
                
                if (menuItems.empty()) {
                    cout << "\n✗ No menu items found for this restaurant.\n";
                } else {
                    cout << "\nTotal Items: " << menuItems.size() << "\n\n";
                    
                    for (const auto& item : menuItems) {
                        size_t pos1 = item.find(';');
                        size_t pos2 = item.find(';', pos1 + 1);
                        size_t pos3 = item.find(';', pos2 + 1);
                        size_t pos4 = item.find(';', pos3 + 1);
                        size_t pos5 = item.find(';', pos4 + 1);
                        
                        if (pos1 != string::npos && pos5 != string::npos) {
                            string id = item.substr(0, pos1);
                            string name = item.substr(pos1 + 1, pos2 - pos1 - 1);
                            string description = item.substr(pos2 + 1, pos3 - pos2 - 1);
                            string price = item.substr(pos3 + 1, pos4 - pos3 - 1);
                            string stock = item.substr(pos4 + 1, pos5 - pos4 - 1);
                            string category = item.substr(pos5 + 1);
                            
                            cout << "┌─────────────────────────────────────┐\n";
                            cout << "│ [" << id << "] " << name << "\n";
                            cout << "├─────────────────────────────────────┤\n";
                            cout << "│ " << description << "\n";
                            cout << "│ Price: $" << price << " | Stock: " << stock << "\n";
                            cout << "│ Category: " << category << "\n";
                            cout << "└─────────────────────────────────────┘\n\n";
                        }
                    }
                }
                pauseScreen();
                break;
            }
            case 2:
                addMenuItem();
                break;
            case 3:
                removeMenuItem();
                break;
            case 4:
                return;
        }
    }
}

// === RIDER MANAGEMENT FUNCTIONS ===

void addRider() {
    clearScreen();
    cout << "========================================\n";
    cout << "           ADD NEW RIDER               \n";
    cout << "========================================\n";
    
    string name, email, phone, password, address, vehicle;
    
    cout << "Rider Name: ";
    cin.ignore();
    getline(cin, name);
    
    cout << "Email: ";
    getline(cin, email);
    
    cout << "Phone: ";
    getline(cin, phone);
    
    cout << "Password: ";
    getline(cin, password);
    
    cout << "Address: ";
    getline(cin, address);
    
    cout << "Vehicle Type (Bike/Motorcycle/Car): ";
    getline(cin, vehicle);
    
    // Prepare data for server
    string data = name + "|" + email + "|" + phone + "|" + password + "|" +
                  address + "|" + vehicle;
    
    // Send command to server
    string response = client.sendCommand("ADD_RIDER", data);
    
    if (response.substr(0, 7) == "SUCCESS") {
        cout << "\n✓ Rider added successfully!\n";
        cout << "Rider ID: " << response.substr(8) << "\n";
        cout << "\nLogin Credentials:\n";
        cout << "  Email: " << email << "\n";
        cout << "  Password: " << password << "\n";
    } else {
        cout << "\n✗ Failed to add rider: " << response << "\n";
    }
    
    pauseScreen();
}

void removeRider() {
    clearScreen();
    cout << "========================================\n";
    cout << "           REMOVE RIDER                \n";
    cout << "========================================\n";
    
    // Show all riders
    vector<string> riders = client.getRiders();
    
    if (riders.empty()) {
        cout << "\n✗ No riders in the system.\n";
        pauseScreen();
        return;
    }
    
    cout << "\nAvailable Riders:\n";
    cout << "----------------------------------------\n";
    
    for (const auto& rider : riders) {
        size_t pos1 = rider.find(';');
        size_t pos2 = rider.find(';', pos1 + 1);
        
        if (pos1 != string::npos && pos2 != string::npos) {
            string id = rider.substr(0, pos1);
            string name = rider.substr(pos1 + 1, pos2 - pos1 - 1);
            cout << "  [" << id << "] " << name << "\n";
        }
    }
    
    int riderId;
    cout << "\nEnter Rider ID to remove: ";
    cin >> riderId;
    
    // Send command to server
    string response = client.sendCommand("REMOVE_RIDER", to_string(riderId));
    
    if (response.substr(0, 7) == "SUCCESS") {
        cout << "\n✓ Rider removed successfully!\n";
    } else {
        cout << "\n✗ Failed to remove rider: " << response << "\n";
    }
    
    pauseScreen();
}

void manageRiders() {
    while (true) {
        clearScreen();
        cout << "========================================\n";
        cout << "           MANAGE RIDERS               \n";
        cout << "========================================\n";
        cout << "1. View All Riders\n";
        cout << "2. View Rider Statistics\n";
        cout << "3. Update Rider Status\n";
        cout << "4. Add New Rider\n";
        cout << "5. Remove Rider\n";
        cout << "6. Back to Admin Menu\n";
        cout << "========================================\n";
        
        int choice;
        cout << "\nEnter choice (1-6): ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore();
        
        switch (choice) {
            case 1: {
                clearScreen();
                cout << "========================================\n";
                cout << "           ALL RIDERS                  \n";
                cout << "========================================\n";
                
                vector<string> riders = client.getRiders();
                
                if (riders.empty()) {
                    cout << "\n✗ No riders in the system.\n";
                } else {
                    cout << "\nTotal Riders: " << riders.size() << "\n\n";
                    
                    int activeCount = 0;
                    int busyCount = 0;
                    int offlineCount = 0;
                    
                    for (const auto& rider : riders) {
                        size_t pos1 = rider.find(';');
                        size_t pos2 = rider.find(';', pos1 + 1);
                        
                        if (pos1 != string::npos && pos2 != string::npos) {
                            string id = rider.substr(0, pos1);
                            string name = rider.substr(pos1 + 1, pos2 - pos1 - 1);
                            string status = rider.substr(pos2 + 1);
                            
                            if (status == "Active") activeCount++;
                            else if (status == "Busy") busyCount++;
                            else offlineCount++;
                            
                            string statusIcon = "🔴";
                            if (status == "Active") statusIcon = "🟢";
                            else if (status == "Busy") statusIcon = "🟡";
                            
                            cout << "┌─────────────────────────────────────┐\n";
                            cout << "│ [" << id << "] " << name << "\n";
                            cout << "├─────────────────────────────────────┤\n";
                            cout << "│ Status: " << statusIcon << " " << status << "\n";
                            cout << "└─────────────────────────────────────┘\n\n";
                        }
                    }
                    
                    cout << "========================================\n";
                    cout << "Summary:\n";
                    cout << "  Active: " << activeCount;
                    cout << " | Busy: " << busyCount;
                    cout << " | Offline: " << offlineCount << "\n";
                    cout << "========================================\n";
                }
                pauseScreen();
                break;
            }
            case 2: {
                int riderId;
                cout << "\nEnter Rider ID: ";
                cin >> riderId;
                cin.ignore();
                
                string data = to_string(riderId);
                string response = client.sendCommand("GET_RIDER_STATS", data);
                
                clearScreen();
                cout << "========================================\n";
                cout << "         RIDER STATISTICS              \n";
                cout << "========================================\n";
                
                if (response.substr(0, 7) == "SUCCESS") {
                    string statsData = response.substr(8);
                    
                    vector<string> stats;
                    size_t start = 0, end;
                    
                    while ((end = statsData.find(';', start)) != string::npos) {
                        stats.push_back(statsData.substr(start, end - start));
                        start = end + 1;
                    }
                    if (start < statsData.length()) {
                        stats.push_back(statsData.substr(start));
                    }
                    
                    if (stats.size() >= 6) {
                        cout << "\n┌─────────────────────────────────────┐\n";
                        cout << "│ Rider ID: " << riderId << "\n";
                        cout << "├─────────────────────────────────────┤\n";
                        cout << "│ Total Deliveries: " << stats[0] << "\n";
                        cout << "│ Today's Deliveries: " << stats[1] << "\n";
                        cout << "│ Success Rate: " << stats[2] << "%\n";
                        cout << "│ Average Rating: " << stats[3] << "/5 ⭐\n";
                        cout << "│ On-time Rate: " << stats[4] << "%\n";
                        cout << "│ Total Earnings: $" << stats[5] << "\n";
                        cout << "└─────────────────────────────────────┘\n";
                    }
                } else {
                    cout << "\n✗ Failed to retrieve rider statistics.\n";
                }
                
                pauseScreen();
                break;
            }
            case 3: {
                int riderId;
                string status;
                
                cout << "\nEnter Rider ID: ";
                cin >> riderId;
                cin.ignore();
                
                cout << "New Status (Active/Busy/Offline): ";
                getline(cin, status);
                
                string data = to_string(riderId) + "|" + status;
                string response = client.sendCommand("UPDATE_RIDER_STATUS", data);
                
                if (response.substr(0, 7) == "SUCCESS") {
                    cout << "\n✓ Rider status updated!\n";
                } else {
                    cout << "\n✗ Failed to update rider status.\n";
                }
                
                pauseScreen();
                break;
            }
            case 4:
                addRider();
                break;
            case 5:
                removeRider();
                break;
            case 6:
                return;
        }
    }
}

// === USER MANAGEMENT FUNCTIONS ===

void changeUserRole() {
    clearScreen();
    cout << "========================================\n";
    cout << "         CHANGE USER ROLE              \n";
    cout << "========================================\n";
    
    // First get all users
    string response = client.sendCommand("GET_ALL_USERS");
    
    if (response.substr(0, 7) != "SUCCESS") {
        cout << "\n✗ Failed to retrieve users.\n";
        pauseScreen();
        return;
    }
    
    string data = response.substr(8);
    if (data.empty()) {
        cout << "\n✗ No users in system.\n";
        pauseScreen();
        return;
    }
    
    vector<string> users;
    size_t start = 0, end;
    
    while ((end = data.find('|', start)) != string::npos) {
        users.push_back(data.substr(start, end - start));
        start = end + 1;
    }
    if (start < data.length()) {
        users.push_back(data.substr(start));
    }
    
    cout << "\nAvailable Users:\n";
    cout << "----------------------------------------\n";
    
    for (const auto& user : users) {
        vector<string> parts;
        size_t pos = 0, next;
        
        while ((next = user.find(';', pos)) != string::npos) {
            parts.push_back(user.substr(pos, next - pos));
            pos = next + 1;
        }
        parts.push_back(user.substr(pos));
        
        if (parts.size() >= 5) {
            cout << "  [" << parts[0] << "] " << parts[1] 
                 << " (" << parts[2] << ", Current Role: " << parts[3] << ")\n";
        }
    }
    
    int userId;
    string newRole;
    
    cout << "\nEnter User ID: ";
    cin >> userId;
    cin.ignore();
    
    cout << "\nAvailable Roles:\n";
    cout << "  1. Customer\n";
    cout << "  2. Rider\n";
    cout << "  3. Restaurant Owner\n";
    cout << "  4. Admin\n";
    
    int roleChoice;
    cout << "\nSelect new role (1-4): ";
    cin >> roleChoice;
    cin.ignore();
    
    switch (roleChoice) {
        case 1: newRole = "customer"; break;
        case 2: newRole = "rider"; break;
        case 3: newRole = "restaurant_owner"; break;
        case 4: newRole = "admin"; break;
        default:
            cout << "\n✗ Invalid role choice!\n";
            pauseScreen();
            return;
    }
    
    // Prepare data for server
    string changeData = to_string(userId) + "|" + newRole;
    
    // Send command to server
    string changeResponse = client.sendCommand("CHANGE_USER_ROLE", changeData);
    
    if (changeResponse.substr(0, 7) == "SUCCESS") {
        cout << "\n✓ User role changed successfully!\n";
        cout << "User ID " << userId << " is now a " << newRole << "\n";
    } else {
        cout << "\n✗ Failed to change user role: " << changeResponse << "\n";
    }
    
    pauseScreen();
}

void manageUsers() {
    while (true) {
        clearScreen();
        cout << "========================================\n";
        cout << "           USER MANAGEMENT             \n";
        cout << "========================================\n";
        cout << "1. View All Users\n";
        cout << "2. Change User Role\n";
        cout << "3. View User Statistics\n";
        cout << "4. Back to Admin Menu\n";
        cout << "========================================\n";
        
        int choice;
        cout << "\nEnter choice (1-4): ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore();
        
        switch (choice) {
            case 1: {
                clearScreen();
                cout << "========================================\n";
                cout << "           ALL USERS                   \n";
                cout << "========================================\n";
                
                string response = client.sendCommand("GET_ALL_USERS");
                
                if (response.substr(0, 7) == "SUCCESS") {
                    string data = response.substr(8);
                    
                    if (data.empty()) {
                        cout << "\n✗ No users in system.\n";
                    } else {
                        vector<string> users;
                        size_t start = 0, end;
                        
                        while ((end = data.find('|', start)) != string::npos) {
                            users.push_back(data.substr(start, end - start));
                            start = end + 1;
                        }
                        if (start < data.length()) {
                            users.push_back(data.substr(start));
                        }
                        
                        cout << "\nTotal Users: " << users.size() << "\n\n";
                        
                        int customerCount = 0, riderCount = 0, adminCount = 0, ownerCount = 0;
                        
                        for (const auto& user : users) {
                            vector<string> parts;
                            size_t pos = 0, next;
                            
                            while ((next = user.find(';', pos)) != string::npos) {
                                parts.push_back(user.substr(pos, next - pos));
                                pos = next + 1;
                            }
                            parts.push_back(user.substr(pos));
                            
                            if (parts.size() >= 5) {
                                if (parts[3] == "customer") customerCount++;
                                else if (parts[3] == "rider") riderCount++;
                                else if (parts[3] == "admin") adminCount++;
                                else if (parts[3] == "restaurant_owner") ownerCount++;
                                
                                string roleIcon = "👤";
                                if (parts[3] == "rider") roleIcon = "🏍️";
                                else if (parts[3] == "admin") roleIcon = "👨‍💼";
                                else if (parts[3] == "restaurant_owner") roleIcon = "👨‍🍳";
                                
                                cout << "┌─────────────────────────────────────┐\n";
                                cout << "│ [" << parts[0] << "] " << parts[1] << "\n";
                                cout << "├─────────────────────────────────────┤\n";
                                cout << "│ " << roleIcon << " Role: " << parts[3] << "\n";
                                cout << "│ 📧 Email: " << parts[2] << "\n";
                                cout << "│ 📱 Phone: " << parts[4] << "\n";
                                cout << "└─────────────────────────────────────┘\n\n";
                            }
                        }
                        
                        cout << "========================================\n";
                        cout << "Summary:\n";
                        cout << "  Customers: " << customerCount;
                        cout << " | Riders: " << riderCount;
                        cout << " | Restaurant Owners: " << ownerCount;
                        cout << " | Admins: " << adminCount << "\n";
                        cout << "========================================\n";
                    }
                } else {
                    cout << "\n✗ Failed to retrieve users.\n";
                }
                
                pauseScreen();
                break;
            }
            case 2:
                changeUserRole();
                break;
            case 3:
                viewUserStatistics();
                break;
            case 4:
                return;
        }
    }
}

// === EXISTING FUNCTIONS (keep as is) ===

void viewAllOrders() {
    clearScreen();
    cout << "========================================\n";
    cout << "           ALL ORDERS                  \n";
    cout << "========================================\n";
    
    string response = client.sendCommand("GET_ALL_ORDERS");
    
    if (response.substr(0, 7) == "SUCCESS") {
        string data = response.substr(8);
        
        if (data.empty()) {
            cout << "\n✓ No orders in the system.\n";
        } else {
            cout << "\n📦 System Orders:\n\n";
            
            vector<string> orders;
            size_t start = 0, end;
            
            while ((end = data.find('|', start)) != string::npos) {
                orders.push_back(data.substr(start, end - start));
                start = end + 1;
            }
            if (start < data.length()) {
                orders.push_back(data.substr(start));
            }
            
            cout << "Total Orders: " << orders.size() << "\n\n";
            
            for (size_t i = 0; i < orders.size(); i++) {
                string order = orders[i];
                
                vector<string> parts;
                size_t pos = 0, next;
                
                while ((next = order.find(';', pos)) != string::npos) {
                    parts.push_back(order.substr(pos, next - pos));
                    pos = next + 1;
                }
                parts.push_back(order.substr(pos));
                
                if (parts.size() >= 6) {
                    string statusIcon = "🟡";
                    if (parts[4] == "Delivered") statusIcon = "🟢";
                    if (parts[4] == "Cancelled") statusIcon = "🔴";
                    
                    cout << "┌─────────────────────────────────────┐\n";
                    cout << "│ Order #" << parts[0] << " " << statusIcon << "\n";
                    cout << "├─────────────────────────────────────┤\n";
                    cout << "│ Customer: " << parts[1] << "\n";
                    cout << "│ Restaurant: " << parts[2] << "\n";
                    cout << "│ Rider: " << parts[3] << "\n";
                    cout << "│ Status: " << parts[4] << "\n";
                    cout << "│ Amount: $" << parts[5] << "\n";
                    cout << "└─────────────────────────────────────┘\n\n";
                }
            }
        }
    } else {
        cout << "\n✗ Failed to retrieve orders.\n";
    }
    
    pauseScreen();
}

void updateOrderStatus() {
    clearScreen();
    cout << "========================================\n";
    cout << "         UPDATE ORDER STATUS           \n";
    cout << "========================================\n";
    
    int orderId;
    cout << "Enter Order ID: ";
    cin >> orderId;
    cin.ignore();
    
    cout << "\nAvailable Statuses:\n";
    cout << "  1. Pending\n";
    cout << "  2. Confirmed\n";
    cout << "  3. Preparing\n";
    cout << "  4. Ready for Pickup\n";
    cout << "  5. Dispatched\n";
    cout << "  6. In Transit\n";
    cout << "  7. Delivered\n";
    cout << "  8. Cancelled\n";
    
    int statusChoice;
    cout << "\nSelect status (1-8): ";
    cin >> statusChoice;
    cin.ignore();
    
    string status;
    switch (statusChoice) {
        case 1: status = "Pending"; break;
        case 2: status = "Confirmed"; break;
        case 3: status = "Preparing"; break;
        case 4: status = "Ready for Pickup"; break;
        case 5: status = "Dispatched"; break;
        case 6: status = "In Transit"; break;
        case 7: status = "Delivered"; break;
        case 8: status = "Cancelled"; break;
        default:
            cout << "\n✗ Invalid choice!\n";
            pauseScreen();
            return;
    }
    
    string data = to_string(orderId) + "|" + status;
    string response = client.sendCommand("UPDATE_ORDER_STATUS", data);
    
    if (response.substr(0, 7) == "SUCCESS") {
        cout << "\n✓ Order status updated successfully!\n";
    } else {
        cout << "\n✗ Failed to update order status.\n";
    }
    
    pauseScreen();
}

void assignRiderToOrder() {
    clearScreen();
    cout << "========================================\n";
    cout << "       ASSIGN RIDER TO ORDER           \n";
    cout << "========================================\n";
    
    int orderId, riderId;
    
    cout << "Enter Order ID: ";
    cin >> orderId;
    cin.ignore();
    
    cout << "\nAvailable Active Riders:\n";
    cout << "----------------------------------------\n";
    
    vector<string> riders = client.getRiders();
    
    bool hasActiveRiders = false;
    for (const auto& rider : riders) {
        size_t pos1 = rider.find(';');
        size_t pos2 = rider.find(';', pos1 + 1);
        
        if (pos1 != string::npos && pos2 != string::npos) {
            string id = rider.substr(0, pos1);
            string name = rider.substr(pos1 + 1, pos2 - pos1 - 1);
            string status = rider.substr(pos2 + 1);
            
            if (status == "Active") {
                cout << "  [" << id << "] " << name << "\n";
                hasActiveRiders = true;
            }
        }
    }
    
    if (!hasActiveRiders) {
        cout << "  No active riders available.\n";
        pauseScreen();
        return;
    }
    
    cout << "\nEnter Rider ID: ";
    cin >> riderId;
    cin.ignore();
    
    if (client.assignRider(orderId, riderId)) {
        cout << "\n✓ Rider assigned successfully!\n";
    } else {
        cout << "\n✗ Failed to assign rider!\n";
    }
    
    pauseScreen();
}

void viewUserStatistics() {
    clearScreen();
    cout << "========================================\n";
    cout << "         USER STATISTICS               \n";
    cout << "========================================\n";
    
    string response = client.sendCommand("GET_SYSTEM_STATS");
    
    if (response.substr(0, 7) == "SUCCESS") {
        string data = response.substr(8);
        
        vector<string> stats;
        size_t start = 0, end;
        
        while ((end = data.find(';', start)) != string::npos) {
            stats.push_back(data.substr(start, end - start));
            start = end + 1;
        }
        if (start < data.length()) {
            stats.push_back(data.substr(start));
        }
        
        if (stats.size() >= 9) {
            cout << "\n┌─────────────────────────────────────┐\n";
            cout << "│ User Demographics\n";
            cout << "├─────────────────────────────────────┤\n";
            cout << "│ Total Users: " << stats[0] << "\n";
            cout << "│   Customers: " << stats[1] << "\n";
            cout << "│   Riders: " << stats[2] << "\n";
            cout << "│   Admins: " << stats[3] << "\n";
            cout << "└─────────────────────────────────────┘\n\n";
            
            cout << "┌─────────────────────────────────────┐\n";
            cout << "│ Business Metrics\n";
            cout << "├─────────────────────────────────────┤\n";
            cout << "│ Active Restaurants: " << stats[4] << "\n";
            cout << "│ Total Orders: " << stats[5] << "\n";
            cout << "│   Pending: " << stats[6] << "\n";
            cout << "│   Completed: " << stats[7] << "\n";
            cout << "│ Total Revenue: $" << stats[8] << "\n";
            cout << "└─────────────────────────────────────┘\n";
        }
    } else {
        cout << "\n✗ Failed to retrieve statistics.\n";
    }
    
    pauseScreen();
}

void viewCityMap() {
    clearScreen();
    cout << "========================================\n";
    cout << "         CITY MAP INFORMATION          \n";
    cout << "========================================\n";
    
    string response = client.sendCommand("GET_CITY_MAP");
    
    if (response.substr(0, 7) == "SUCCESS") {
        string data = response.substr(8);
        
        if (data.empty()) {
            cout << "\n✗ No city map data available.\n";
        } else {
            cout << "\n📍 City Locations:\n\n";
            
            vector<string> locations;
            size_t start = 0, end;
            
            while ((end = data.find('|', start)) != string::npos) {
                locations.push_back(data.substr(start, end - start));
                start = end + 1;
            }
            if (start < data.length()) {
                locations.push_back(data.substr(start));
            }
            
            int count = 1;
            for (const auto& location : locations) {
                size_t semi = location.find(';');
                
                if (semi != string::npos) {
                    string id = location.substr(0, semi);
                    string name = location.substr(semi + 1);
                    
                    cout << "  " << count++ << ". [" << id << "] " << name << "\n";
                }
            }
            
            cout << "\nTotal Locations: " << locations.size() << "\n";
        }
    } else {
        cout << "\n✗ Failed to retrieve city map data.\n";
    }
    
    pauseScreen();
}

void viewSystemStatistics() {
    clearScreen();
    cout << "========================================\n";
    cout << "       SYSTEM STATISTICS               \n";
    cout << "========================================\n";
    
    string response = client.sendCommand("GET_SYSTEM_STATS");
    
    if (response.substr(0, 7) == "SUCCESS") {
        string data = response.substr(8);
        
        if (data.empty()) {
            cout << "\n✗ No system statistics available.\n";
        } else {
            vector<string> stats;
            size_t start = 0, end;
            
            while ((end = data.find(';', start)) != string::npos) {
                stats.push_back(data.substr(start, end - start));
                start = end + 1;
            }
            if (start < data.length()) {
                stats.push_back(data.substr(start));
            }
            
            if (stats.size() >= 9) {
                cout << "\n📊 SYSTEM OVERVIEW\n";
                cout << "════════════════════════════════════════\n\n";
                
                cout << "👥 USER STATISTICS\n";
                cout << "┌─────────────────────────────────────┐\n";
                cout << "│ Total Users: " << stats[0] << "\n";
                cout << "│ ─────────────────────────────────── │\n";
                cout << "│ Customers: " << stats[1] << "\n";
                cout << "│ Riders: " << stats[2] << "\n";
                cout << "│ Admins: " << stats[3] << "\n";
                cout << "└─────────────────────────────────────┘\n\n";
                
                cout << "🏪 RESTAURANT STATISTICS\n";
                cout << "┌─────────────────────────────────────┐\n";
                cout << "│ Total Restaurants: " << stats[4] << "\n";
                cout << "└─────────────────────────────────────┘\n\n";
                
                cout << "📦 ORDER STATISTICS\n";
                cout << "┌─────────────────────────────────────┐\n";
                cout << "│ Total Orders: " << stats[5] << "\n";
                cout << "│ ─────────────────────────────────── │\n";
                cout << "│ Pending Orders: " << stats[6] << "\n";
                cout << "│ Completed Orders: " << stats[7] << "\n";
                
                int totalOrders = stoi(stats[5]);
                int completedOrders = stoi(stats[7]);
                int pendingOrders = stoi(stats[6]);
                
                double completionRate = totalOrders > 0 ? (completedOrders * 100.0 / totalOrders) : 0;
                double pendingRate = totalOrders > 0 ? (pendingOrders * 100.0 / totalOrders) : 0;
                
                cout << "│ ─────────────────────────────────── │\n";
                cout << "│ Completion Rate: " << fixed << setprecision(1) << completionRate << "%\n";
                cout << "│ Pending Rate: " << fixed << setprecision(1) << pendingRate << "%\n";
                cout << "└─────────────────────────────────────┘\n\n";
                
                cout << "💰 FINANCIAL STATISTICS\n";
                cout << "┌─────────────────────────────────────┐\n";
                cout << "│ Total Revenue: $" << stats[8] << "\n";
                
                if (completedOrders > 0) {
                    double avgOrderValue = stod(stats[8]) / completedOrders;
                    cout << "│ ─────────────────────────────────── │\n";
                    cout << "│ Avg Order Value: $" << fixed << setprecision(2) << avgOrderValue << "\n";
                }
                cout << "└─────────────────────────────────────┘\n\n";
                
                cout << "🩺 PLATFORM HEALTH\n";
                cout << "┌─────────────────────────────────────┐\n";
                
                double riderToCustomerRatio = stoi(stats[2]) > 0 ? (stod(stats[1]) / stod(stats[2])) : 0;
                double ordersPerRestaurant = stoi(stats[4]) > 0 ? (stod(stats[5]) / stod(stats[4])) : 0;
                
                cout << "│ Rider/Customer Ratio: 1:" << fixed << setprecision(1) << riderToCustomerRatio << "\n";
                cout << "│ Orders/Restaurant: " << fixed << setprecision(1) << ordersPerRestaurant << "\n";
                
                cout << "│ ─────────────────────────────────── │\n";
                
                if (riderToCustomerRatio <= 10) {
                    cout << "│ Rider Coverage: 🟢 Excellent\n";
                } else if (riderToCustomerRatio <= 20) {
                    cout << "│ Rider Coverage: 🟡 Good\n";
                } else {
                    cout << "│ Rider Coverage: 🔴 Needs Attention\n";
                }
                
                if (completionRate >= 80) {
                    cout << "│ Order Fulfillment: 🟢 Excellent\n";
                } else if (completionRate >= 60) {
                    cout << "│ Order Fulfillment: 🟡 Good\n";
                } else {
                    cout << "│ Order Fulfillment: 🔴 Needs Attention\n";
                }
                
                cout << "└─────────────────────────────────────┘\n";
            } else {
                cout << "\n✗ Incomplete statistics data received.\n";
            }
        }
    } else {
        cout << "\n✗ Failed to retrieve system statistics.\n";
    }
    
    pauseScreen();
}

int main() {
    string serverIP;
    int serverPort;
    
    cout << "╔═══════════════════════════════════════╗\n";
    cout << "║     QUICKBITE ADMIN CLIENT            ║\n";
    cout << "╚═══════════════════════════════════════╝\n\n";
    
    cout << "Enter server IP (default 127.0.0.1): ";
    getline(cin, serverIP);
    if (serverIP.empty()) serverIP = "127.0.0.1";
    
    cout << "Enter server port (default 8080): ";
    string portStr;
    getline(cin, portStr);
    serverPort = portStr.empty() ? 8080 : stoi(portStr);
    
    client = QuickBiteClient(serverIP, serverPort);
    
    cout << "\nConnecting to server...\n";
    if (!client.connectToServer()) {
        cerr << "✗ Failed to connect to server!\n";
        pauseScreen();
        return 1;
    }
    
    cout << "✓ Connected successfully!\n";
    pauseScreen();
    
    while (true) {
        clearScreen();
        
        if (client.getUserId() == -1) {
            showMainMenu();
            
            int choice;
            cout << "\nEnter choice (1-2): ";
            
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            cin.ignore();
            
            switch (choice) {
                case 1:
                    adminLogin();
                    break;
                case 2:
                    cout << "\nGoodbye!\n";
                    return 0;
            }
        } else {
            showAdminMenu();
            
            int choice;
            cout << "\nEnter choice (1-11): ";
            
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            cin.ignore();
            
            switch (choice) {
                case 1:
                    manageRestaurants();
                    break;
                case 2:
                    manageMenuItems();
                    break;
                case 3:
                    viewAllOrders();
                    break;
                case 4:
                    updateOrderStatus();
                    break;
                case 5:
                    manageRiders();
                    break;
                case 6:
                    assignRiderToOrder();
                    break;
                case 7:
                    manageUsers();
                    break;
                case 8:
                    viewUserStatistics();
                    break;
                case 9:
                    viewCityMap();
                    break;
                case 10:
                    viewSystemStatistics();
                    break;
                case 11:
                    cout << "\nLogging out...\n";
                    client.logout();
                    cout << "✓ Logged out successfully!\n";
                    pauseScreen();
                    break;
                default:
                    cout << "\n✗ Invalid choice!\n";
                    pauseScreen();
            }
        }
    }
    
    return 0;
}