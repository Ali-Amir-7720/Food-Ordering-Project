// Enhanced rider_client.cpp - All Features Implemented
#include "Client.h"
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <sstream>
#include <map>
using namespace std;

QuickBiteClient client;

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseScreen() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

void showMainMenu() {
    cout << "╔═══════════════════════════════════════╗\n";
    cout << "║     QUICKBITE DELIVERY SYSTEM         ║\n";
    cout << "║       RIDER APPLICATION               ║\n";
    cout << "╚═══════════════════════════════════════╝\n";
    cout << "1. Login as Rider\n";
    cout << "2. Register as Rider\n";
    cout << "3. Exit\n";
    cout << "========================================\n";
}

void showRiderMenu() {
    cout << "\n╔═══════════════════════════════════════╗\n";
    cout << "║         RIDER DASHBOARD               ║\n";
    cout << "╚═══════════════════════════════════════╝\n";
    cout << "Welcome, " << client.getUserName() << "! 🏍️\n";
    cout << "========================================\n";
    cout << "1. View Available Orders\n";
    cout << "2. View My Assigned Orders\n";
    cout << "3. Accept Order\n";
    cout << "4. Update Delivery Status\n";
    cout << "5. View Delivery Route\n";
    cout << "6. Update Availability Status\n";
    cout << "7. View My Statistics\n";
    cout << "8. Logout\n";
    cout << "========================================\n";
}

void riderLogin() {
    clearScreen();
    cout << "========================================\n";
    cout << "            RIDER LOGIN                 \n";
    cout << "========================================\n";
    
    string email, password;
    
    cout << "Email: ";
    getline(cin, email);
    
    cout << "Password: ";
    getline(cin, password);
    
    cout << "\nConnecting to server...\n";
    
    if (client.login(email, password)) {
        string userRole = client.getUserRole();
        
        if (userRole == "rider") {
            cout << "✓ Rider login successful!\n";
            cout << "Welcome, " << client.getUserName() << "!\n";
            cout << "Rider ID: " << client.getUserId() << "\n";
            pauseScreen();
        } else {
            cout << "✗ Access denied! Not a rider account.\n";
            cout << "Your account type is: " << userRole << "\n";
            client.logout();
            pauseScreen();
        }
    } else {
        cout << "✗ Login failed! Invalid credentials.\n";
        pauseScreen();
    }
}

void riderRegister() {
    clearScreen();
    cout << "========================================\n";
    cout << "          RIDER REGISTRATION            \n";
    cout << "========================================\n";
    
    string name, email, password, phone, address;
    
    cout << "Full Name: ";
    getline(cin, name);
    
    cout << "Email: ";
    getline(cin, email);
    
    cout << "Password: ";
    getline(cin, password);
    
    cout << "Phone Number: ";
    getline(cin, phone);
    
    cout << "Address: ";
    getline(cin, address);
    
    cout << "\nRegistering as rider...\n";
    
    if (client.registerUser(name, email, password, phone, address)) {
        cout << "✓ Registration successful!\n";
        cout << "\n⚠️  IMPORTANT:\n";
        cout << "Your account is currently registered as 'customer'.\n";
        cout << "Please contact admin to activate your rider account.\n\n";
        cout << "Login credentials:\n";
        cout << "  Email: " << email << "\n";
        cout << "  Password: " << password << "\n";
    } else {
        cout << "✗ Registration failed!\n";
    }
    
    pauseScreen();
}

void viewAvailableOrders() {
    clearScreen();
    cout << "========================================\n";
    cout << "        AVAILABLE ORDERS                \n";
    cout << "========================================\n";
    
    string response = client.sendCommand("GET_AVAILABLE_ORDERS");
    
    if (response.substr(0, 7) == "SUCCESS") {
        string data = response.substr(8);
        
        if (data.empty()) {
            cout << "\n✓ No available orders at the moment.\n";
            cout << "Check back later!\n";
        } else {
            cout << "\n📦 Available Orders:\n\n";
            
            vector<string> orders;
            size_t start = 0, end;
            
            while ((end = data.find('|', start)) != string::npos) {
                orders.push_back(data.substr(start, end - start));
                start = end + 1;
            }
            if (start < data.length()) {
                orders.push_back(data.substr(start));
            }
            
            for (const auto& orderStr : orders) {
                vector<string> parts;
                size_t pos = 0, next;
                
                while ((next = orderStr.find(';', pos)) != string::npos) {
                    parts.push_back(orderStr.substr(pos, next - pos));
                    pos = next + 1;
                }
                parts.push_back(orderStr.substr(pos));
                
                if (parts.size() >= 5) {
                    cout << "┌─────────────────────────────────────┐\n";
                    cout << "│ Order #" << parts[0] << "\n";
                    cout << "├─────────────────────────────────────┤\n";
                    cout << "│ Restaurant: " << parts[1] << "\n";
                    cout << "│ Destination: " << parts[2] << "\n";
                    cout << "│ Amount: $" << parts[3] << "\n";
                    cout << "│ Distance: " << parts[4] << " km\n";
                    cout << "└─────────────────────────────────────┘\n\n";
                }
            }
        }
    } else {
        cout << "\n✗ Failed to retrieve available orders.\n";
    }
    
    pauseScreen();
}

void viewMyAssignedOrders() {
    clearScreen();
    cout << "========================================\n";
    cout << "       MY ASSIGNED ORDERS              \n";
    cout << "========================================\n";
    
    string data = to_string(client.getUserId());
    string response = client.sendCommand("GET_RIDER_ORDERS", data);
    
    if (response.substr(0, 7) == "SUCCESS") {
        string ordersData = response.substr(8);
        
        if (ordersData.empty()) {
            cout << "\n✓ No assigned orders at the moment.\n";
        } else {
            cout << "\n📦 Your Assigned Orders:\n\n";
            
            vector<string> orders;
            size_t start = 0, end;
            
            while ((end = ordersData.find('|', start)) != string::npos) {
                orders.push_back(ordersData.substr(start, end - start));
                start = end + 1;
            }
            if (start < ordersData.length()) {
                orders.push_back(ordersData.substr(start));
            }
            
            for (const auto& orderStr : orders) {
                vector<string> parts;
                size_t pos = 0, next;
                
                while ((next = orderStr.find(';', pos)) != string::npos) {
                    parts.push_back(orderStr.substr(pos, next - pos));
                    pos = next + 1;
                }
                parts.push_back(orderStr.substr(pos));
                
                if (parts.size() >= 6) {
                    string statusIcon = "🟡";
                    if (parts[4] == "Delivered") statusIcon = "🟢";
                    if (parts[4] == "Cancelled") statusIcon = "🔴";
                    
                    cout << "┌─────────────────────────────────────┐\n";
                    cout << "│ Order #" << parts[0] << " " << statusIcon << "\n";
                    cout << "├─────────────────────────────────────┤\n";
                    cout << "│ Restaurant: " << parts[1] << "\n";
                    cout << "│ Customer: " << parts[2] << "\n";
                    cout << "│ Address: " << parts[3] << "\n";
                    cout << "│ Status: " << parts[4] << "\n";
                    cout << "│ Amount: $" << parts[5] << "\n";
                    cout << "└─────────────────────────────────────┘\n\n";
                }
            }
        }
    } else {
        cout << "\n✗ Failed to retrieve your orders.\n";
    }
    
    pauseScreen();
}

void acceptOrder() {
    clearScreen();
    cout << "========================================\n";
    cout << "           ACCEPT ORDER                \n";
    cout << "========================================\n";
    
    int orderId;
    
    cout << "\nEnter Order ID to accept: ";
    cin >> orderId;
    cin.ignore();
    
    cout << "\nAccepting order " << orderId << "...\n";
    
    string data = to_string(orderId) + "|" + to_string(client.getUserId());
    string response = client.sendCommand("ASSIGN_RIDER", data);
    
    if (response.substr(0, 7) == "SUCCESS") {
        cout << "✓ Order accepted successfully!\n";
        cout << "\n📍 Next Steps:\n";
        cout << "  1. Navigate to restaurant\n";
        cout << "  2. Pick up the order\n";
        cout << "  3. Update status to 'Dispatched'\n";
        cout << "  4. Deliver to customer\n";
        cout << "  5. Update status to 'Delivered'\n";
    } else {
        cout << "✗ Failed to accept order!\n";
    }
    
    pauseScreen();
}

void updateDeliveryStatus() {
    clearScreen();
    cout << "========================================\n";
    cout << "       UPDATE DELIVERY STATUS          \n";
    cout << "========================================\n";
    
    int orderId;
    
    cout << "\nEnter Order ID: ";
    cin >> orderId;
    cin.ignore();
    
    cout << "\nSelect new status:\n";
    cout << "  1. Preparing (Picked up from restaurant)\n";
    cout << "  2. Dispatched (Started delivery)\n";
    cout << "  3. In Transit (On the way)\n";
    cout << "  4. Delivered (Successfully delivered)\n";
    cout << "  5. Cancelled (Unable to deliver)\n";
    
    int statusChoice;
    cout << "\nEnter choice (1-5): ";
    cin >> statusChoice;
    cin.ignore();
    
    string status;
    switch (statusChoice) {
        case 1: status = "Preparing"; break;
        case 2: status = "Dispatched"; break;
        case 3: status = "In Transit"; break;
        case 4: status = "Delivered"; break;
        case 5: status = "Cancelled"; break;
        default:
            cout << "Invalid choice!\n";
            pauseScreen();
            return;
    }
    
    cout << "\nUpdating order " << orderId << " to '" << status << "'...\n";
    
    string data = to_string(orderId) + "|" + status;
    string response = client.sendCommand("UPDATE_ORDER_STATUS", data);
    
    if (response.substr(0, 7) == "SUCCESS") {
        cout << "✓ Status updated successfully!\n";
        
        if (statusChoice == 4) {
            cout << "\n🎉 Delivery completed!\n";
            cout << "Great job! Your earnings have been updated.\n";
        }
    } else {
        cout << "✗ Failed to update status!\n";
    }
    
    pauseScreen();
}

void viewDeliveryRoute() {
    clearScreen();
    cout << "========================================\n";
    cout << "         DELIVERY ROUTE                \n";
    cout << "========================================\n";
    
    // Get city map
    string mapResponse = client.sendCommand("GET_CITY_MAP");
    
    if (mapResponse.substr(0, 7) != "SUCCESS") {
        cout << "\n✗ Failed to retrieve city map.\n";
        pauseScreen();
        return;
    }
    
    string mapData = mapResponse.substr(8);
    
    // Parse and display locations
    map<int, string> locations;
    size_t start = 0, end;
    
    while ((end = mapData.find('|', start)) != string::npos) {
        string location = mapData.substr(start, end - start);
        size_t semi = location.find(';');
        
        if (semi != string::npos) {
            int id = stoi(location.substr(0, semi));
            string name = location.substr(semi + 1);
            locations[id] = name;
        }
        
        start = end + 1;
    }
    
    if (start < mapData.length()) {
        string location = mapData.substr(start);
        size_t semi = location.find(';');
        
        if (semi != string::npos) {
            int id = stoi(location.substr(0, semi));
            string name = location.substr(semi + 1);
            locations[id] = name;
        }
    }
    
    cout << "\n📍 City Map Locations:\n\n";
    int count = 1;
    for (const auto& loc : locations) {
        cout << "  " << count++ << ". [" << loc.first << "] " << loc.second << "\n";
    }
    
    // Get order for route calculation
    cout << "\n" << string(40, '=') << "\n";
    cout << "Enter Order ID for route calculation: ";
    int orderId;
    cin >> orderId;
    cin.ignore();
    
    cout << "Enter your current location ID: ";
    int riderLoc;
    cin >> riderLoc;
    cin.ignore();
    
    // Request route from server
    string routeData = to_string(orderId) + "|" + to_string(riderLoc);
    string routeResponse = client.sendCommand("GET_DELIVERY_ROUTE", routeData);
    
    if (routeResponse.substr(0, 7) == "SUCCESS") {
        cout << "\n🗺️  Calculated Delivery Route:\n\n";
        
        // Parse route: SUCCESS|TO_RESTAURANT|1,2,3|TO_CUSTOMER|3,4,5
        string routeInfo = routeResponse.substr(8);
        
        size_t toRestPos = routeInfo.find("TO_RESTAURANT|");
        size_t toCustomerPos = routeInfo.find("|TO_CUSTOMER|");
        
        if (toRestPos != string::npos && toCustomerPos != string::npos) {
            // Extract routes
            string route1Str = routeInfo.substr(toRestPos + 14, toCustomerPos - toRestPos - 14);
            string route2Str = routeInfo.substr(toCustomerPos + 13);
            
            // Display route to restaurant
            cout << "══════════════════════════════════════\n";
            cout << "  Step 1: Your Location → Restaurant\n";
            cout << "══════════════════════════════════════\n";
            
            stringstream ss1(route1Str);
            string node;
            int step = 1;
            
            while (getline(ss1, node, ',')) {
                int nodeId = stoi(node);
                cout << "  " << step++ << ". ";
                if (locations.find(nodeId) != locations.end()) {
                    cout << locations[nodeId] << " [" << nodeId << "]\n";
                } else {
                    cout << "Location " << nodeId << "\n";
                }
            }
            
            cout << "\n══════════════════════════════════════\n";
            cout << "  Step 2: Restaurant → Customer\n";
            cout << "══════════════════════════════════════\n";
            
            stringstream ss2(route2Str);
            step = 1;
            
            while (getline(ss2, node, ',')) {
                int nodeId = stoi(node);
                cout << "  " << step++ << ". ";
                if (locations.find(nodeId) != locations.end()) {
                    cout << locations[nodeId] << " [" << nodeId << "]\n";
                } else {
                    cout << "Location " << nodeId << "\n";
                }
            }
            
            cout << "\n✓ Route calculated using Dijkstra's algorithm\n";
        }
    } else {
        cout << "\n✗ Failed to calculate route.\n";
    }
    
    pauseScreen();
}

void updateAvailability() {
    clearScreen();
    cout << "========================================\n";
    cout << "       UPDATE AVAILABILITY             \n";
    cout << "========================================\n";
    
    cout << "\nSelect new status:\n";
    cout << "  1. Active (Available for orders)\n";
    cout << "  2. Busy (Currently on delivery)\n";
    cout << "  3. Offline (Not accepting orders)\n";
    
    int choice;
    cout << "\nEnter choice (1-3): ";
    cin >> choice;
    cin.ignore();
    
    string status;
    
    switch (choice) {
        case 1: status = "Active"; break;
        case 2: status = "Busy"; break;
        case 3: status = "Offline"; break;
        default:
            cout << "Invalid choice!\n";
            pauseScreen();
            return;
    }
    
    cout << "\nUpdating status to " << status << "...\n";
    
    string data = to_string(client.getUserId()) + "|" + status;
    string response = client.sendCommand("UPDATE_RIDER_STATUS", data);
    
    if (response.substr(0, 7) == "SUCCESS") {
        cout << "✓ Status updated successfully!\n";
    } else {
        cout << "✗ Failed to update status.\n";
    }
    
    pauseScreen();
}

void viewMyStatistics() {
    clearScreen();
    cout << "========================================\n";
    cout << "         MY STATISTICS                 \n";
    cout << "========================================\n";
    
    string data = to_string(client.getUserId());
    string response = client.sendCommand("GET_RIDER_STATS", data);
    
    if (response.substr(0, 7) == "SUCCESS") {
        string statsData = response.substr(8);
        
        // Parse: totalDeliveries;todayDeliveries;successRate;avgRating;onTimeRate;earnings
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
            cout << "│ Performance Overview\n";
            cout << "├─────────────────────────────────────┤\n";
            cout << "│ Total Deliveries: " << stats[0] << "\n";
            cout << "│ Completed Today: " << stats[1] << "\n";
            cout << "│ Success Rate: " << stats[2] << "%\n";
            
            double rating = stod(stats[3]);
            cout << "│ Average Rating: ";
            for (int i = 0; i < 5; i++) {
                if (i < (int)rating) cout << "⭐";
                else cout << "☆";
            }
            cout << " (" << stats[3] << "/5)\n";
            
            cout << "│ On-time Rate: " << stats[4] << "%\n";
            cout << "│ Total Earnings: $" << stats[5] << "\n";
            cout << "└─────────────────────────────────────┘\n";
        }
    } else {
        cout << "\n⚠️  No statistics available yet.\n";
        cout << "Complete some deliveries to see your stats!\n";
    }
    
    pauseScreen();
}

int main() {
    string serverIP;
    int serverPort;
    
    cout << "╔═══════════════════════════════════════╗\n";
    cout << "║     QUICKBITE RIDER CLIENT            ║\n";
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
            cout << "\nEnter choice (1-3): ";
            
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            cin.ignore();
            
            switch (choice) {
                case 1:
                    riderLogin();
                    break;
                case 2:
                    riderRegister();
                    break;
                case 3:
                    cout << "\nGoodbye!\n";
                    return 0;
            }
        } else {
            showRiderMenu();
            
            int choice;
            cout << "\nEnter choice (1-8): ";
            
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            cin.ignore();
            
            switch (choice) {
                case 1:
                    viewAvailableOrders();
                    break;
                case 2:
                    viewMyAssignedOrders();
                    break;
                case 3:
                    acceptOrder();
                    break;
                case 4:
                    updateDeliveryStatus();
                    break;
                case 5:
                    viewDeliveryRoute();
                    break;
                case 6:
                    updateAvailability();
                    break;
                case 7:
                    viewMyStatistics();
                    break;
                case 8:
                    cout << "\nLogging out...\n";
                    client.logout();
                    cout << "✓ Logged out successfully!\n";
                    pauseScreen();
                    break;
                default:
                    cout << "\nInvalid choice!\n";
                    pauseScreen();
            }
        }
    }
    
    return 0;
}