// client_main.cpp
#include "Client.h"
#include <iostream>
#include <string>
#include <vector>
#include <limits>

using namespace std;

QuickBiteClient client;
void clearScreen();
void pauseScreen();
void showMainMenu();
void loginMenu();
void registerMenu();
void browseRestaurants();
void viewRestaurantMenu(const string& restaurantData);
void placeOrderMenu(int restaurantId, const vector<string>& menuItems);
void viewMyOrders();
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
    cout << "║       CUSTOMER APPLICATION            ║\n";
    cout << "╚═══════════════════════════════════════╝\n";
    cout << "1. Login\n";
    cout << "2. Register\n";
    cout << "3. Exit\n";
    cout << "========================================\n";
}

void showCustomerMenu() {
    cout << "\n╔═══════════════════════════════════════╗\n";
    cout << "║         CUSTOMER DASHBOARD            ║\n";
    cout << "╚═══════════════════════════════════════╝\n";
    cout << "Welcome, " << client.getUserName() << "!\n";
    cout << "========================================\n";
    cout << "1. Browse Restaurants\n";
    cout << "2. View My Orders\n";
    cout << "3. Logout\n";
    cout << "========================================\n";
}

void loginMenu() {
    clearScreen();
    cout << "========================================\n";
    cout << "              LOGIN                     \n";
    cout << "========================================\n";
    
    string email, password;
    
    cout << "Email: ";
    getline(cin, email);
    
    cout << "Password: ";
    getline(cin, password);
    
    cout << "\nConnecting to server...\n";
    
    if (client.login(email, password)) {
        cout << "✓ Login successful!\n";
        pauseScreen();
    } else {
        cout << "✗ Login failed! Invalid credentials.\n";
        pauseScreen();
    }
}

void registerMenu() {
    clearScreen();
    cout << "========================================\n";
    cout << "            REGISTRATION                \n";
    cout << "========================================\n";
    
    string name, email, password, phone, address;
    
    cout << "Name: ";
    getline(cin, name);
    
    cout << "Email: ";
    getline(cin, email);
    
    cout << "Password: ";
    getline(cin, password);
    
    cout << "Phone: ";
    getline(cin, phone);
    
    cout << "Address: ";
    getline(cin, address);
    
    cout << "\nRegistering...\n";
    
    if (client.registerUser(name, email, password, phone, address)) {
        cout << "✓ Registration successful! Please login.\n";
    } else {
        cout << "✗ Registration failed!\n";
    }
    
    pauseScreen();
}

void browseRestaurants() {
    clearScreen();
    cout << "========================================\n";
    cout << "         AVAILABLE RESTAURANTS         \n";
    cout << "========================================\n";
    
    vector<string> restaurants = client.getRestaurants();
    
    if (restaurants.empty()) {
        cout << "No restaurants available.\n";
        pauseScreen();
        return;
    }
    
    for (size_t i = 0; i < restaurants.size(); i++) {
        // Parse: id;name;cuisine;address;rating;deliveryTime
        string restaurant = restaurants[i];
        
        size_t pos1 = restaurant.find(';');
        size_t pos2 = restaurant.find(';', pos1 + 1);
        size_t pos3 = restaurant.find(';', pos2 + 1);
        size_t pos4 = restaurant.find(';', pos3 + 1);
        size_t pos5 = restaurant.find(';', pos4 + 1);
        
        string id = restaurant.substr(0, pos1);
        string name = restaurant.substr(pos1 + 1, pos2 - pos1 - 1);
        string cuisine = restaurant.substr(pos2 + 1, pos3 - pos2 - 1);
        string address = restaurant.substr(pos3 + 1, pos4 - pos3 - 1);
        string rating = restaurant.substr(pos4 + 1, pos5 - pos4 - 1);
        string deliveryTime = restaurant.substr(pos5 + 1);
        
        cout << i + 1 << ". " << name << " (" << cuisine << ")\n";
        cout << "   Rating: " << rating << "/5\n";
        cout << "   Delivery: " << deliveryTime << " mins\n";
        cout << "   Address: " << address << "\n";
        cout << "----------------------------------------\n";
    }
    
    cout << "\nSelect restaurant (0 to go back): ";
    int choice;
    cin >> choice;
    cin.ignore();
    
    if (choice > 0 && choice <= restaurants.size()) {
        viewRestaurantMenu(restaurants[choice - 1]);
    }
}

void viewRestaurantMenu(const string& restaurantData) {
    // Extract restaurant ID
    size_t pos = restaurantData.find(';');
    int restaurantId = stoi(restaurantData.substr(0, pos));
    
    clearScreen();
    cout << "========================================\n";
    cout << "           RESTAURANT MENU              \n";
    cout << "========================================\n";
    
    vector<string> menuItems = client.getMenu(restaurantId);
    
    if (menuItems.empty()) {
        cout << "No menu items available.\n";
        pauseScreen();
        return;
    }
    
    for (size_t i = 0; i < menuItems.size(); i++) {
        // Parse: id;name;description;price;stock;category
        string item = menuItems[i];
        
        size_t p1 = item.find(';');
        size_t p2 = item.find(';', p1 + 1);
        size_t p3 = item.find(';', p2 + 1);
        size_t p4 = item.find(';', p3 + 1);
        size_t p5 = item.find(';', p4 + 1);
        
        string id = item.substr(0, p1);
        string name = item.substr(p1 + 1, p2 - p1 - 1);
        string description = item.substr(p2 + 1, p3 - p2 - 1);
        string price = item.substr(p3 + 1, p4 - p3 - 1);
        string stock = item.substr(p4 + 1, p5 - p4 - 1);
        string category = item.substr(p5 + 1);
        
        cout << i + 1 << ". " << name << " - $" << price << "\n";
        cout << "   " << description << "\n";
        cout << "   Category: " << category << " | Stock: " << stock << "\n";
        cout << "----------------------------------------\n";
    }
    
    cout << "\nWould you like to place an order? (y/n): ";
    char choice;
    cin >> choice;
    cin.ignore();
    
    if (tolower(choice) == 'y') {
        placeOrderMenu(restaurantId, menuItems);
    }
}

void placeOrderMenu(int restaurantId, const vector<string>& menuItems) {
    vector<pair<int, int>> orderItems; // itemId, quantity
    
    cout << "\nEnter items to order (item number, quantity)\n";
    cout << "Enter 0 to finish:\n\n";
    
    while (true) {
        int itemNum, quantity;
        
        cout << "Item number: ";
        cin >> itemNum;
        
        if (itemNum == 0) break;
        
        if (itemNum < 1 || itemNum > menuItems.size()) {
            cout << "Invalid item number!\n";
            continue;
        }
        
        cout << "Quantity: ";
        cin >> quantity;
        cin.ignore();
        
        if (quantity <= 0) {
            cout << "Invalid quantity!\n";
            continue;
        }
        
        // Extract item ID from menu item string
        string item = menuItems[itemNum - 1];
        size_t pos = item.find(';');
        int itemId = stoi(item.substr(0, pos));
        
        orderItems.push_back({itemId, quantity});
        
        cout << "✓ Added to order\n\n";
    }
    
    if (orderItems.empty()) {
        cout << "No items selected.\n";
        pauseScreen();
        return;
    }
    
    cout << "\nPlacing order...\n";
    
    int orderId = client.placeOrder(restaurantId, orderItems);
    
    if (orderId > 0) {
        cout << "✓ Order placed successfully!\n";
        cout << "Order ID: " << orderId << "\n";
    } else {
        cout << "✗ Order failed!\n";
    }
    
    pauseScreen();
}

void viewMyOrders() {
    clearScreen();
    cout << "========================================\n";
    cout << "            MY ORDERS                   \n";
    cout << "========================================\n";
    
    vector<string> orders = client.getOrders();
    
    if (orders.empty()) {
        cout << "You have no orders.\n";
        pauseScreen();
        return;
    }
    
    for (size_t i = 0; i < orders.size(); i++) {
        // Parse: orderId;restaurantId;totalAmount;status
        string order = orders[i];
        
        size_t p1 = order.find(';');
        size_t p2 = order.find(';', p1 + 1);
        size_t p3 = order.find(';', p2 + 1);
        
        string orderId = order.substr(0, p1);
        string restaurantId = order.substr(p1 + 1, p2 - p1 - 1);
        string totalAmount = order.substr(p2 + 1, p3 - p2 - 1);
        string status = order.substr(p3 + 1);
        
        cout << i + 1 << ". Order #" << orderId << "\n";
        cout << "   Restaurant ID: " << restaurantId << "\n";
        cout << "   Total: $" << totalAmount << "\n";
        cout << "   Status: " << status << "\n";
        cout << "----------------------------------------\n";
    }
    
    pauseScreen();
}

int main() {
    string serverIP;
    int serverPort;
    
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
        cerr << "Failed to connect to server!\n";
        return 1;
    }
    
    while (true) {
        clearScreen();
        
        if (client.getUserId() == -1) {
            // Not logged in
            showMainMenu();
            
            int choice;
            cout << "\nEnter choice (1-3): ";
            
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            cin.ignore();
            
            switch (choice) {
                case 1:
                    loginMenu();
                    break;
                case 2:
                    registerMenu();
                    break;
                case 3:
                    cout << "Goodbye!\n";
                    return 0;
            }
        } else {
            // Logged in
            showCustomerMenu();
            
            int choice;
            cout << "\nEnter choice (1-3): ";
            
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            cin.ignore();
            
            switch (choice) {
                case 1:
                    browseRestaurants();
                    break;
                case 2:
                    viewMyOrders();
                    break;
                case 3:
                    cout << "\nLogging out...\n";
                    client.logout();
                    cout << "✓ Logged out successfully!\n";
                    pauseScreen();
                    break;
            }
        }
    }
    
    return 0;
}