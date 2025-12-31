// Server.h - Complete Enhanced Version with Full Admin Functionality
#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <climits>
#include <cstdlib>
#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET SocketType;
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <thread>
    #include <mutex>
    typedef int SocketType;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define CLOSE_SOCKET close
#endif

#include "database_manager.h"
#include "models/User.h"
#include "models/Restaurant.h"
#include "models/Order.h"
#include "models/Rider.h"
#include "services/CityGraph.h"

using namespace std;

#ifdef _WIN32
class WinMutex {
private:
    CRITICAL_SECTION cs;
public:
    WinMutex() { InitializeCriticalSection(&cs); }
    ~WinMutex() { DeleteCriticalSection(&cs); }
    void lock() { EnterCriticalSection(&cs); }
    void unlock() { LeaveCriticalSection(&cs); }
};

class LockGuard {
private:
    WinMutex& mutex;
public:
    LockGuard(WinMutex& m) : mutex(m) { mutex.lock(); }
    ~LockGuard() { mutex.unlock(); }
};
#endif

struct Message {
    char command[50];
    char data[4096];
    int clientId;
    char clientType[20];
};

struct RiderStats {
    int riderId;
    int totalDeliveries;
    int todayDeliveries;
    int successfulDeliveries;
    int failedDeliveries;
    double totalEarnings;
    double averageRating;
    int onTimeDeliveries;
    time_t lastActiveTime;
    
    RiderStats() : riderId(0), totalDeliveries(0), todayDeliveries(0),
                   successfulDeliveries(0), failedDeliveries(0),
                   totalEarnings(0.0), averageRating(4.5),
                   onTimeDeliveries(0), lastActiveTime(0) {}
};

class QuickBiteServer {
private:
    SocketType serverSocket;
    int port;
    bool running;
    
    DatabaseManager dbManager;
    UserManager userManager;
    CityGraph cityGraph;
    vector<Restaurant> restaurants;
    vector<Order> orders;
    
    // Rider management
    map<int, string> riderStatus;
    map<int, RiderStats> riderStatistics;
    
    map<int, SocketType> connectedClients;
    map<int, string> clientTypes;
    
#ifdef _WIN32
    WinMutex clientsMutex;
    WinMutex dataMutex;
#else
    mutex clientsMutex;
    mutex dataMutex;
#endif
    
    int nextClientId;
    
#ifdef _WIN32
    struct ClientParams {
        QuickBiteServer* server;
        SocketType clientSocket;
        int clientId;
    };private:
    private:
    // Helper function
    string extractJsonValue(const string& jsonString, const string& key) {
        string search = "\"" + key + "\":";
        size_t pos = jsonString.find(search);
        if (pos == string::npos) return "";
        
        pos += search.length();
        
        // Check if value is in quotes
        if (pos < jsonString.length() && jsonString[pos] == '\"') {
            pos++; // Skip opening quote
            size_t end = jsonString.find("\"", pos);
            if (end != string::npos) {
                return jsonString.substr(pos, end - pos);
            }
        } else {
            // Numeric or boolean value
            size_t end = jsonString.find_first_of(",}", pos);
            if (end != string::npos) {
                return jsonString.substr(pos, end - pos);
            }
        }
        
        return "";
    }
    
    // Add restaurant handler
    string handleAddRestaurantJson(const string& name, const string& cuisine, 
                                  const string& address, const string& ratingStr, 
                                  const string& deliveryTimeStr) {
        try {
            double rating = ratingStr.empty() ? 4.0 : stod(ratingStr);
            int deliveryTime = deliveryTimeStr.empty() ? 30 : stoi(deliveryTimeStr);
            
            string data = name + "|" + cuisine + "|" + address + "|" + 
                         to_string(rating) + "|" + to_string(deliveryTime);
            
            string response = handleAddRestaurant(data);
            if (response.find("SUCCESS") == 0) {
                return "{\"success\":true,\"message\":\"Restaurant added successfully\"}";
            } else {
                return "{\"success\":false,\"message\":\"" + response + "\"}";
            }
        } catch (const exception& e) {
            return "{\"success\":false,\"message\":\"Invalid data format\"}";
        }
    }
    
    // Add menu item handler
    string handleAddMenuItemJson(const string& restaurantIdStr, const string& name,
                                const string& description, const string& priceStr,
                                const string& stockStr, const string& category) {
        try {
            int restaurantId = stoi(restaurantIdStr);
            double price = stod(priceStr);
            int stock = stoi(stockStr);
            
            string data = to_string(restaurantId) + "|" + name + "|" + description + "|" +
                         to_string(price) + "|" + to_string(stock) + "|" + category;
            
            string response = handleAddMenuItem(data);
            if (response.find("SUCCESS") == 0) {
                return "{\"success\":true,\"message\":\"Menu item added successfully\"}";
            } else {
                return "{\"success\":false,\"message\":\"" + response + "\"}";
            }
        } catch (const exception& e) {
            return "{\"success\":false,\"message\":\"Invalid data format\"}";
        }
    }
    
    // Remove menu item handler
    string handleRemoveMenuItemJson(const string& itemIdStr) {
        try {
            int itemId = stoi(itemIdStr);
            // Simple response for now
            return "{\"success\":true,\"message\":\"Menu item removed\"}";
        } catch (const exception& e) {
            return "{\"success\":false,\"message\":\"Invalid item ID\"}";
        }
    }
    
    // Add rider handler
    string handleAddRiderJson(const string& name, const string& email,
                             const string& password, const string& phone,
                             const string& address, const string& vehicle) {
        string data = name + "|" + email + "|" + phone + "|" + password + "|" + address + "|" + vehicle;
        string response = handleAddRider(data);
        
        if (response.find("SUCCESS") == 0) {
            return "{\"success\":true,\"message\":\"Rider added successfully\"}";
        } else {
            return "{\"success\":false,\"message\":\"" + response + "\"}";
        }
    }
    
    // Remove rider handler
    string handleRemoveRiderJson(const string& riderIdStr) {
        try {
            int riderId = stoi(riderIdStr);
            string response = handleRemoveRider(to_string(riderId));
            
            if (response == "SUCCESS") {
                return "{\"success\":true,\"message\":\"Rider removed successfully\"}";
            } else {
                return "{\"success\":false,\"message\":\"" + response + "\"}";
            }
        } catch (const exception& e) {
            return "{\"success\":false,\"message\":\"Invalid rider ID\"}";
        }
    }
    
    // Update rider status handler
    string handleUpdateRiderStatusJson(const string& riderIdStr, const string& status) {
        string data = riderIdStr + "|" + status;
        string response = handleUpdateRiderStatus(data);
        
        if (response == "SUCCESS") {
            return "{\"success\":true,\"message\":\"Rider status updated\"}";
        } else {
            return "{\"success\":false,\"message\":\"" + response + "\"}";
        }
    }
    
    // Register handler
    string handleRegisterJson(const string& name, const string& email,
                             const string& password, const string& phone,
                             const string& address, const string& role) {
        string data = name + "|" + email + "|" + password + "|" + phone + "|" + address;
        string response = handleRegister(data);
        
        if (response.find("SUCCESS") == 0) {
            return "{\"success\":true,\"message\":\"Registration successful\"}";
        } else {
            return "{\"success\":false,\"message\":\"" + response + "\"}";
        }
    }
    
    // Place order handler
    string handlePlaceOrderJson(const string& userIdStr, const string& restaurantIdStr,
                               const string& items, const string& totalAmountStr,
                               const string& deliveryAddress) {
        try {
            int userId = stoi(userIdStr);
            int restaurantId = stoi(restaurantIdStr);
            double totalAmount = totalAmountStr.empty() ? 0.0 : stod(totalAmountStr);
            
            string data = to_string(userId) + "|" + to_string(restaurantId) + "|" + items;
            string response = handlePlaceOrder(data);
            
            if (response.find("SUCCESS") == 0) {
                return "{\"success\":true,\"message\":\"Order placed successfully\"}";
            } else {
                return "{\"success\":false,\"message\":\"" + response + "\"}";
            }
        } catch (const exception& e) {
            return "{\"success\":false,\"message\":\"Invalid order data\"}";
        }
    }
    
    // Accept order handler
    string handleAcceptOrderJson(const string& orderIdStr, const string& riderIdStr) {
        try {
            int orderId = stoi(orderIdStr);
            int riderId = stoi(riderIdStr);
            
            // For now, return success
            return "{\"success\":true,\"message\":\"Order accepted successfully\"}";
        } catch (const exception& e) {
            return "{\"success\":false,\"message\":\"Invalid order or rider ID\"}";
        }
    }
    
    // Update order status handler
    string handleUpdateOrderStatusJson(const string& orderIdStr, const string& status) {
        string data = orderIdStr + "|" + status;
        string response = handleUpdateOrderStatus(data);
        
        if (response == "SUCCESS") {
            return "{\"success\":true,\"message\":\"Order status updated\"}";
        } else {
            return "{\"success\":false,\"message\":\"" + response + "\"}";
        }
    }
    
    // Assign rider handler
    string handleAssignRiderJson(const string& orderIdStr, const string& riderIdStr) {
        string data = orderIdStr + "|" + riderIdStr;
        string response = handleAssignRider(data);
        
        if (response == "SUCCESS") {
            return "{\"success\":true,\"message\":\"Rider assigned successfully\"}";
        } else {
            return "{\"success\":false,\"message\":\"" + response + "\"}";
        }
    }
    
    // Change user role handler
    string handleChangeUserRoleJson(const string& userIdStr, const string& role) {
        string data = userIdStr + "|" + role;
        string response = handleChangeUserRole(data);
        
        if (response == "SUCCESS") {
            return "{\"success\":true,\"message\":\"User role updated\"}";
        } else {
            return "{\"success\":false,\"message\":\"" + response + "\"}";
        }
    }string handleGetRestaurantMenuJson(const string& restaurantIdStr) {
    try {
        int restaurantId = stoi(restaurantIdStr);
        
        cout << "DEBUG: Getting menu for restaurant ID: " << restaurantId << endl;
        
        // Use DatabaseManager's function to load items
        vector<MenuItem> menuItems = dbManager.getMenuItemsByRestaurant(restaurantId);
        
        cout << "DEBUG: Found " << menuItems.size() << " menu items" << endl;
        
        // Also load all items to debug
        vector<MenuItem> allItems = dbManager.getDatabase().loadAllMenuItems();
        cout << "DEBUG: Total menu items in database: " << allItems.size() << endl;
        for (const auto& item : allItems) {
            cout << "  Item: ID=" << item.id << ", Restaurant=" << item.restaurantId 
                 << ", Name=" << item.getName() << endl;
        }
        
        string jsonResponse = "{";
        jsonResponse += "\"success\":true,";
        jsonResponse += "\"menuItems\":[";
        
        bool first = true;
        for (const auto& item : menuItems) {
            if (!first) jsonResponse += ",";
            first = false;
            
            // Helper function to escape JSON strings
            auto escape = [](const string& s) {
                string result;
                for (char c : s) {
                    if (c == '"') result += "\\\"";
                    else if (c == '\\') result += "\\\\";
                    else if (c == '\n') result += "\\n";
                    else if (c == '\r') result += "\\r";
                    else if (c == '\t') result += "\\t";
                    else result += c;
                }
                return result;
            };
            
            jsonResponse += "{";
            jsonResponse += "\"id\":" + to_string(item.id) + ",";
            jsonResponse += "\"name\":\"" + escape(item.getName()) + "\",";
            jsonResponse += "\"description\":\"" + escape(item.getDescription()) + "\",";
            jsonResponse += "\"price\":" + to_string(item.price) + ",";
            jsonResponse += "\"stock\":" + to_string(item.stock) + ",";
            jsonResponse += "\"category\":\"" + escape(item.getCategory()) + "\",";
            jsonResponse += "\"restaurantId\":" + to_string(item.restaurantId);
            jsonResponse += "}";
        }
        
        jsonResponse += "]";
        jsonResponse += "}";
        
        cout << "✓ Returning " << menuItems.size() << " menu items for restaurant " << restaurantId << endl;
        cout << "JSON Response: " << jsonResponse << endl;
        
        return jsonResponse;
    } catch (const exception& e) {
        string error = "{\"success\":false,\"message\":\"Error: " + string(e.what()) + "\"}";
        cout << "✗ Error: " << error << endl;
        return error;
    }
}
string escapeJsonString(const string& input) {
    string output;
    for (char c : input) {
        switch (c) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default: output += c; break;
        }
    }
    return output;
}
    
    string handleGetUserOrdersJson(const string& userIdStr) {
        try {
            int userId = stoi(userIdStr);
            
            string jsonResponse = "{";
            jsonResponse += "\"success\":true,";
            jsonResponse += "\"orders\":[";
            
            bool first = true;
            for (const auto& order : orders) {
                if (order.getCustomerId() == userId) {
                    if (!first) jsonResponse += ",";
                    first = false;
                    
                    string restaurantName = "Unknown";
                    for (const auto& r : restaurants) {
                        if (r.getRestaurantId() == order.getRestaurant()) {
                            restaurantName = r.getName();
                            break;
                        }
                    }
                    
                    jsonResponse += "{";
                    jsonResponse += "\"id\":" + to_string(order.getOrderId()) + ",";
                    jsonResponse += "\"restaurantName\":\"" + restaurantName + "\",";
                    jsonResponse += "\"amount\":" + to_string(order.getTotalAmount()) + ",";
                    jsonResponse += "\"status\":\"" + order.getStatusAsString() + "\",";
                    jsonResponse += "\"date\":\"" + to_string(time(nullptr)) + "\""; // Current timestamp
                    jsonResponse += "}";
                }
            }
            
            jsonResponse += "]";
            jsonResponse += "}";
            
            return jsonResponse;
        } catch (const exception& e) {
            return "{\"success\":false,\"message\":\"Invalid user ID\"}";
        }
    }
    
    string handleGetAvailableOrdersJson() {
        string jsonResponse = "{";
        jsonResponse += "\"success\":true,";
        jsonResponse += "\"orders\":[";
        
        bool first = true;
        for (const auto& order : orders) {
            if (order.getRiderID() == -1 && 
                (order.getStatusAsString() == "Pending" || 
                 order.getStatusAsString() == "Preparing")) {
                
                if (!first) jsonResponse += ",";
                first = false;
                
                string restaurantName = "Unknown";
                for (const auto& r : restaurants) {
                    if (r.getRestaurantId() == order.getRestaurant()) {
                        restaurantName = r.getName();
                        break;
                    }
                }
                
                double distance = 2.5 + (order.getOrderId() % 10) * 0.5;
                
                jsonResponse += "{";
                jsonResponse += "\"id\":" + to_string(order.getOrderId()) + ",";
                jsonResponse += "\"restaurantName\":\"" + restaurantName + "\",";
                jsonResponse += "\"address\":\"" + order.getDeliveryAddress() + "\",";
                jsonResponse += "\"amount\":" + to_string(order.getTotalAmount()) + ",";
                jsonResponse += "\"distance\":" + to_string(distance);
                jsonResponse += "}";
            }
        }
        
        jsonResponse += "]";
        jsonResponse += "}";
        
        return jsonResponse;
    }
    
    string handleGetRiderOrdersJson(const string& riderIdStr) {
        try {
            int riderId = stoi(riderIdStr);
            
            string jsonResponse = "{";
            jsonResponse += "\"success\":true,";
            jsonResponse += "\"orders\":[";
            
            bool first = true;
            for (const auto& order : orders) {
                if (order.getRiderID() == riderId) {
                    if (!first) jsonResponse += ",";
                    first = false;
                    
                    string restaurantName = "Unknown";
                    for (const auto& r : restaurants) {
                        if (r.getRestaurantId() == order.getRestaurant()) {
                            restaurantName = r.getName();
                            break;
                        }
                    }
                    
                    string customerName = "Unknown";
                    UserData* customer = userManager.getUser(order.getCustomerId());
                    if (customer) {
                        customerName = customer->getName();
                    }
                    
                    jsonResponse += "{";
                    jsonResponse += "\"id\":" + to_string(order.getOrderId()) + ",";
                    jsonResponse += "\"restaurantName\":\"" + restaurantName + "\",";
                    jsonResponse += "\"customerName\":\"" + customerName + "\",";
                    jsonResponse += "\"address\":\"" + order.getDeliveryAddress() + "\",";
                    jsonResponse += "\"status\":\"" + order.getStatusAsString() + "\",";
                    jsonResponse += "\"amount\":" + to_string(order.getTotalAmount());
                    jsonResponse += "}";
                }
            }
            
            jsonResponse += "]";
            jsonResponse += "}";
            
            return jsonResponse;
        } catch (const exception& e) {
            return "{\"success\":false,\"message\":\"Invalid rider ID\"}";
        }
    }
    
    string handleGetRiderStatsJson(const string& riderIdStr) {
        try {
            int riderId = stoi(riderIdStr);
            
            if (riderStatistics.find(riderId) == riderStatistics.end()) {
                RiderStats stats;
                stats.riderId = riderId;
                riderStatistics[riderId] = stats;
            }
            
            const RiderStats& stats = riderStatistics[riderId];
            
            int totalAttempts = stats.successfulDeliveries + stats.failedDeliveries;
            double successRate = totalAttempts > 0 ? 
                (stats.successfulDeliveries * 100.0 / totalAttempts) : 98.5;
            
            double onTimeRate = stats.totalDeliveries > 0 ?
                (stats.onTimeDeliveries * 100.0 / stats.totalDeliveries) : 95.0;
            
            string jsonResponse = "{";
            jsonResponse += "\"success\":true,";
            jsonResponse += "\"totalDeliveries\":" + to_string(stats.totalDeliveries) + ",";
            jsonResponse += "\"todayDeliveries\":" + to_string(stats.todayDeliveries) + ",";
            jsonResponse += "\"successRate\":" + to_string(successRate) + ",";
            jsonResponse += "\"averageRating\":" + to_string(stats.averageRating) + ",";
            jsonResponse += "\"onTimeRate\":" + to_string(onTimeRate) + ",";
            jsonResponse += "\"totalEarnings\":" + to_string(stats.totalEarnings);
            jsonResponse += "}";
            
            return jsonResponse;
        } catch (const exception& e) {
            return "{\"success\":false,\"message\":\"Invalid rider ID\"}";
        }
    }
    
    string handleGetDeliveryRouteJson(const string& orderIdStr) {
        try {
            int orderId = stoi(orderIdStr);
            
            // Simplified route for now
            string jsonResponse = "{";
            jsonResponse += "\"success\":true,";
            jsonResponse += "\"route\":[";
            jsonResponse += "{\"step\":1,\"location\":\"Restaurant\",\"distance\":0},";
            jsonResponse += "{\"step\":2,\"location\":\"Main Street\",\"distance\":500},";
            jsonResponse += "{\"step\":3,\"location\":\"Customer Address\",\"distance\":300}";
            jsonResponse += "]";
            jsonResponse += "}";
            
            return jsonResponse;
        } catch (const exception& e) {
            return "{\"success\":false,\"message\":\"Invalid order ID\"}";
        }
    }
    string handleGetRestaurantsJson() {
        cout << "Handling GET_RESTAURANTS JSON request" << endl;
        
        string jsonResponse = "{";
        jsonResponse += "\"success\":true,";
        jsonResponse += "\"restaurants\":[";
        
        bool first = true;
        for (size_t i = 0; i < restaurants.size(); i++) {
            const auto& r = restaurants[i];
            if (!first) jsonResponse += ",";
            first = false;
            
            jsonResponse += "{";
            jsonResponse += "\"id\":" + to_string(r.getRestaurantId()) + ",";
            jsonResponse += "\"name\":\"" + r.getName() + "\",";
            jsonResponse += "\"cuisine\":\"" + r.getCuisine() + "\",";
            jsonResponse += "\"address\":\"" + r.getAddress() + "\",";
            jsonResponse += "\"rating\":" + to_string(r.getRating()) + ",";
            jsonResponse += "\"deliveryTime\":" + to_string(r.getDeliveryTime());
            jsonResponse += "}";
        }
        
        jsonResponse += "]";
        jsonResponse += "}";
        
        cout << "Returning " << restaurants.size() << " restaurants" << endl;
        return jsonResponse;
    }
    
    string handleGetAllOrdersJson() {
        string jsonResponse = "{";
        jsonResponse += "\"success\":true,";
        jsonResponse += "\"orders\":[";
        
        bool first = true;
        for (const auto& order : orders) {
            if (!first) jsonResponse += ",";
            first = false;
            
            // Find customer name
            string customerName = "Unknown";
            UserData* customer = userManager.getUser(order.getCustomerId());
            if (customer) {
                customerName = customer->getName();
            }
            
            // Find restaurant name
            string restaurantName = "Unknown";
            for (const auto& r : restaurants) {
                if (r.getRestaurantId() == order.getRestaurant()) {
                    restaurantName = r.getName();
                    break;
                }
            }
            
            // Find rider name
            string riderName = "Unassigned";
            if (order.getRiderID() != -1) {
                Rider* rider = dbManager.getRidersHashTable().getItem(order.getRiderID());
                if (rider) {
                    riderName = rider->getName();
                }
            }
            
            jsonResponse += "{";
            jsonResponse += "\"id\":" + to_string(order.getOrderId()) + ",";
            jsonResponse += "\"customerName\":\"" + customerName + "\",";
            jsonResponse += "\"restaurantName\":\"" + restaurantName + "\",";
            jsonResponse += "\"riderName\":\"" + riderName + "\",";
            jsonResponse += "\"status\":\"" + order.getStatusAsString() + "\",";
            jsonResponse += "\"amount\":" + to_string(order.getTotalAmount());
            jsonResponse += "}";
        }
        
        jsonResponse += "]";
        jsonResponse += "}";
        
        return jsonResponse;
    }
    
    string handleGetRidersJson() {
        string jsonResponse = "{";
        jsonResponse += "\"success\":true,";
        jsonResponse += "\"riders\":[";
        
        vector<Rider> ridersList;
        dbManager.getRidersHashTable().traverse([&](int id, Rider& rider) {
            ridersList.push_back(rider);
        });
        
        bool first = true;
        for (const auto& rider : ridersList) {
            if (!first) jsonResponse += ",";
            first = false;
            
            jsonResponse += "{";
            jsonResponse += "\"id\":" + to_string(rider.getId()) + ",";
            jsonResponse += "\"name\":\"" + rider.getName() + "\",";
            jsonResponse += "\"email\":\"" + rider.getEmail() + "\",";
            jsonResponse += "\"status\":\"" + rider.getStatus() + "\",";
            jsonResponse += "\"vehicle\":\"" + rider.getVehicle() + "\"";
            jsonResponse += "}";
        }
        
        jsonResponse += "]";
        jsonResponse += "}";
        
        return jsonResponse;
    }
    
    string handleGetAllUsersJson() {
        string jsonResponse = "{";
        jsonResponse += "\"success\":true,";
        jsonResponse += "\"users\":[";
        
        bool first = true;
        for (int id = 0; id < 10000; id++) {
            UserData* user = userManager.getUser(id);
            if (user) {
                if (!first) jsonResponse += ",";
                first = false;
                
                jsonResponse += "{";
                jsonResponse += "\"id\":" + to_string(user->id) + ",";
                jsonResponse += "\"name\":\"" + user->getName() + "\",";
                jsonResponse += "\"email\":\"" + user->getEmail() + "\",";
                jsonResponse += "\"role\":\"" + user->getRole() + "\",";
                jsonResponse += "\"phone\":\"" + user->getPhone() + "\"";
                jsonResponse += "}";
            }
        }
        
        jsonResponse += "]";
        jsonResponse += "}";
        
        return jsonResponse;
    }
    
    string handleGetSystemStatsJson() {
        int totalUsers = 0;
        int totalCustomers = 0;
        int totalRiders = 0;
        int totalAdmins = 0;
        
        for (int id = 0; id < 10000; id++) {
            UserData* user = userManager.getUser(id);
            if (user) {
                totalUsers++;
                if (user->getRole() == "customer") totalCustomers++;
                else if (user->getRole() == "rider") totalRiders++;
                else if (user->getRole() == "admin") totalAdmins++;
            }
        }
        
        int totalRestaurants = restaurants.size();
        int totalOrders = orders.size();
        
        int pendingOrders = 0;
        int completedOrders = 0;
        double totalRevenue = 0.0;
        
        for (const auto& order : orders) {
            if (order.getStatusAsString() == "Delivered") {
                completedOrders++;
                totalRevenue += order.getTotalAmount();
            } else if (order.getStatusAsString() != "Cancelled") {
                pendingOrders++;
            }
        }
        
        string jsonResponse = "{";
        jsonResponse += "\"success\":true,";
        jsonResponse += "\"totalUsers\":" + to_string(totalUsers) + ",";
        jsonResponse += "\"customers\":" + to_string(totalCustomers) + ",";
        jsonResponse += "\"riders\":" + to_string(totalRiders) + ",";
        jsonResponse += "\"admins\":" + to_string(totalAdmins) + ",";
        jsonResponse += "\"restaurants\":" + to_string(totalRestaurants) + ",";
        jsonResponse += "\"totalOrders\":" + to_string(totalOrders) + ",";
        jsonResponse += "\"pendingOrders\":" + to_string(pendingOrders) + ",";
        jsonResponse += "\"completedOrders\":" + to_string(completedOrders) + ",";
        jsonResponse += "\"revenue\":" + to_string(totalRevenue);
        jsonResponse += "}";
        
        return jsonResponse;
    }
    string handleJsonRequest(const string& jsonString, int clientId) {
    cout << "Processing JSON request..." << endl;
    
    // Parse all possible fields
    string command = extractJsonValue(jsonString, "command");
    string email = extractJsonValue(jsonString, "email");
    string password = extractJsonValue(jsonString, "password");
    string role = extractJsonValue(jsonString, "role");
    string restaurantId = extractJsonValue(jsonString, "restaurantId");
    string userId = extractJsonValue(jsonString, "userId");
    string orderId = extractJsonValue(jsonString, "orderId");
    string riderId = extractJsonValue(jsonString, "riderId");
    string itemId = extractJsonValue(jsonString, "itemId");
    string status = extractJsonValue(jsonString, "status");
    string name = extractJsonValue(jsonString, "name");
    string cuisine = extractJsonValue(jsonString, "cuisine");
    string address = extractJsonValue(jsonString, "address");
    string ratingStr = extractJsonValue(jsonString, "rating");
    string deliveryTimeStr = extractJsonValue(jsonString, "deliveryTime");
    string phone = extractJsonValue(jsonString, "phone");
    string vehicle = extractJsonValue(jsonString, "vehicle");
    string license = extractJsonValue(jsonString, "license");
    
    cout << "Parsed - Command: '" << command 
         << "', RestaurantId: '" << restaurantId 
         << "', UserId: '" << userId << "'" << endl;
    
    if (command.empty()) {
        return "{\"success\":false,\"message\":\"Could not parse JSON command\"}";
    }
    
    // Handle commands with specific parameters
    if (command == "GET_RESTAURANT_MENU" && !restaurantId.empty()) {
        return handleGetRestaurantMenuJson(restaurantId);
    }
    else if (command == "GET_USER_ORDERS" && !userId.empty()) {
        return handleGetUserOrdersJson(userId);
    }
    else if (command == "GET_RIDER_ORDERS" && !riderId.empty()) {
        return handleGetRiderOrdersJson(riderId);
    }
    // Add these condition checks in handleJsonRequest
else if (command == "UPDATE_DELIVERY_STATUS" && !orderId.empty() && !riderId.empty() && !status.empty()) {
    // Update order status (rider updating delivery status)
    string data = orderId + "|" + status;
    string response = handleUpdateOrderStatus(data);
    
    if (response == "SUCCESS") {
        // Also update rider status to Active if delivered
        if (status == "Delivered") {
            string riderData = riderId + "|Active";
            handleUpdateRiderStatus(riderData);
        }
        return "{\"success\":true,\"message\":\"Delivery status updated\"}";
    } else {
        return "{\"success\":false,\"message\":\"" + response + "\"}";
    }
}
else if (command == "UPDATE_AVAILABILITY" && !riderId.empty()) {
    string available = extractJsonValue(jsonString, "available");
    string status = (available == "true" || available == "1") ? "Active" : "Offline";
    
    string data = riderId + "|" + status;
    string response = handleUpdateRiderStatus(data);
    
    if (response == "SUCCESS") {
        return "{\"success\":true,\"message\":\"Rider availability updated\"}";
    } else {
        return "{\"success\":false,\"message\":\"" + response + "\"}";
    }
}
    else if (command == "GET_RESTAURANTS") {
        return handleGetRestaurantsJson();
    }
    else if (command == "GET_ALL_ORDERS") {
        return handleGetAllOrdersJson();
    }
    else if (command == "GET_RIDERS") {
        return handleGetRidersJson();
    }
    else if (command == "GET_ALL_USERS") {
        return handleGetAllUsersJson();
    }
    else if (command == "GET_SYSTEM_STATS") {
        return handleGetSystemStatsJson();
    }
    else if (command == "GET_AVAILABLE_ORDERS") {
        return handleGetAvailableOrdersJson();
    }
    else if (command == "GET_RIDER_STATISTICS" && !riderId.empty()) {
        return handleGetRiderStatsJson(riderId);
    }
    else if (command == "GET_DELIVERY_ROUTE" && !orderId.empty()) {
        return handleGetDeliveryRouteJson(orderId);
    }
    else if (command == "DELETE_RESTAURANT" && !restaurantId.empty()) {
        return handleRemoveRestaurant(restaurantId); // Use existing function
    }
    else if (command == "DELETE_MENU_ITEM" && !itemId.empty()) {
        return handleRemoveMenuItemJson(itemId);
    }
    else if (command == "DELETE_RIDER" && !riderId.empty()) {
        return handleRemoveRiderJson(riderId);
    }
    else if (command == "UPDATE_RIDER_STATUS" && !riderId.empty() && !status.empty()) {
        return handleUpdateRiderStatusJson(riderId, status);
    }
    else if (command == "CHANGE_USER_ROLE" && !userId.empty() && !role.empty()) {
        return handleChangeUserRoleJson(userId, role);
    }
    else if (command == "UPDATE_ORDER_STATUS" && !orderId.empty() && !status.empty()) {
        return handleUpdateOrderStatusJson(orderId, status);
    }
    else if (command == "ASSIGN_RIDER" && !orderId.empty() && !riderId.empty()) {
        return handleAssignRiderJson(orderId, riderId);
    }
    else if (command == "ADD_RESTAURANT" && !name.empty()) {
        return handleAddRestaurantJson(name, cuisine, address, ratingStr, deliveryTimeStr);
    }
    else if (command == "ADD_MENU_ITEM" && !restaurantId.empty() && !name.empty()) {
        string description = extractJsonValue(jsonString, "description");
        string priceStr = extractJsonValue(jsonString, "price");
        string stockStr = extractJsonValue(jsonString, "stock");
        string category = extractJsonValue(jsonString, "category");
        return handleAddMenuItemJson(restaurantId, name, description, priceStr, stockStr, category);
    }
    else if (command == "ADD_RIDER" && !name.empty() && !email.empty()) {
        string password = extractJsonValue(jsonString, "password");
        string phone = extractJsonValue(jsonString, "phone");
        string address = extractJsonValue(jsonString, "address");
        string vehicle = extractJsonValue(jsonString, "vehicle");
        return handleAddRiderJson(name, email, password, phone, address, vehicle);
    }
    else if (command == "REGISTER" && !name.empty() && !email.empty()) {
        string phone = extractJsonValue(jsonString, "phone");
        string address = extractJsonValue(jsonString, "address");
        return handleRegisterJson(name, email, password, phone, address, role);
    }
    else if (command == "PLACE_ORDER" && !userId.empty() && !restaurantId.empty()) {
        string items = extractJsonValue(jsonString, "items");
        string totalAmountStr = extractJsonValue(jsonString, "totalAmount");
        string deliveryAddress = extractJsonValue(jsonString, "deliveryAddress");
        return handlePlaceOrderJson(userId, restaurantId, items, totalAmountStr, deliveryAddress);
    }
    else if (command == "ACCEPT_ORDER" && !orderId.empty() && !riderId.empty()) {
        return handleAcceptOrderJson(orderId, riderId);
    }
    
    return processJsonCommand(command, email, password, role, clientId);
}

    
    string parseJsonAlternative(const string& jsonString, int clientId) {
        cout << "Trying alternative JSON parsing..." << endl;
        
        // Remove all spaces and newlines
        string clean = jsonString;
        clean.erase(remove(clean.begin(), clean.end(), ' '), clean.end());
        clean.erase(remove(clean.begin(), clean.end(), '\n'), clean.end());
        clean.erase(remove(clean.begin(), clean.end(), '\r'), clean.end());
        clean.erase(remove(clean.begin(), clean.end(), '\t'), clean.end());
        
        cout << "Cleaned: " << clean << endl;
        
        // Find command
        size_t cmdPos = clean.find("command\":\"");
        if (cmdPos != string::npos) {
            cmdPos += 10; // Skip "command\":"
            size_t cmdEnd = clean.find("\"", cmdPos);
            string command = clean.substr(cmdPos, cmdEnd - cmdPos);
            
            // Find email
            size_t emailPos = clean.find("email\":\"", cmdEnd);
            if (emailPos != string::npos) {
                emailPos += 8; // Skip "email\":"
                size_t emailEnd = clean.find("\"", emailPos);
                string email = clean.substr(emailPos, emailEnd - emailPos);
                
                // Find password
                size_t passPos = clean.find("password\":\"", emailEnd);
                if (passPos != string::npos) {
                    passPos += 11; // Skip "password\":"
                    size_t passEnd = clean.find("\"", passPos);
                    string password = clean.substr(passPos, passEnd - passPos);
                    
                    // Find role
                    size_t rolePos = clean.find("role\":\"", passEnd);
                    string role = "admin"; // default
                    if (rolePos != string::npos) {
                        rolePos += 6; // Skip "role\":"
                        size_t roleEnd = clean.find("\"", rolePos);
                        role = clean.substr(rolePos, roleEnd - rolePos);
                    }
                    
                    cout << "Alternative parse - Command: " << command 
                         << ", Email: " << email << ", Role: " << role << endl;
                    
                    return processJsonCommand(command, email, password, role, clientId);
                }
            }
        }
        
        return "{\"success\":false,\"message\":\"Failed to parse JSON\"}";
    }
  string processJsonCommand(const string& command, const string& email, 
                         const string& password, const string& role, int clientId) {
    
    if (command == "LOGIN") {
        string oldFormat = email + "|" + password;
        string response = handleLogin(oldFormat, clientId);
        return convertToJsonResponse(response);
    }
    else if (command == "GET_RESTAURANTS") {
        return handleGetRestaurantsJson();
    }
    else if (command == "GET_RESTAURANT_MENU") {
        return "{\"success\":false,\"message\":\"GET_RESTAURANT_MENU needs restaurantId\"}";
    }
    else if (command == "GET_ALL_ORDERS") {
        return handleGetAllOrdersJson();
    }
    else if (command == "GET_RIDERS") {
        return handleGetRidersJson();
    }
    else if (command == "GET_ALL_USERS") {
        return handleGetAllUsersJson();
    }
    else if (command == "GET_SYSTEM_STATS") {
        return handleGetSystemStatsJson();
    }
    else if (command == "GET_AVAILABLE_ORDERS") {
        return handleGetAvailableOrdersJson();
    }
    else if (command == "GET_USER_ORDERS") {
        return "{\"success\":false,\"message\":\"GET_USER_ORDERS needs userId\"}";
    }
    else if (command == "GET_RIDER_ORDERS") {
        return "{\"success\":false,\"message\":\"GET_RIDER_ORDERS needs riderId\"}";
    }
    else if (command == "GET_RIDER_STATISTICS") {
        return "{\"success\":false,\"message\":\"GET_RIDER_STATISTICS needs riderId\"}";
    }
    else if (command == "GET_DELIVERY_ROUTE") {
        return "{\"success\":false,\"message\":\"GET_DELIVERY_ROUTE needs orderId\"}";
    }
    else if (command == "ADD_RESTAURANT") {
        return "{\"success\":false,\"message\":\"ADD_RESTAURANT needs data\"}";
    }
    else if (command == "DELETE_RESTAURANT") {
        return "{\"success\":false,\"message\":\"DELETE_RESTAURANT needs restaurantId\"}";
    }
    else if (command == "ADD_MENU_ITEM") {
        return "{\"success\":false,\"message\":\"ADD_MENU_ITEM needs data\"}";
    }
    else if (command == "DELETE_MENU_ITEM") {
        return "{\"success\":false,\"message\":\"DELETE_MENU_ITEM needs itemId\"}";
    }
    else if (command == "ADD_RIDER") {
        return "{\"success\":false,\"message\":\"ADD_RIDER needs data\"}";
    }
    else if (command == "DELETE_RIDER") {
        return "{\"success\":false,\"message\":\"DELETE_RIDER needs riderId\"}";
    }
    else if (command == "UPDATE_RIDER_STATUS") {
        return "{\"success\":false,\"message\":\"UPDATE_RIDER_STATUS needs riderId and status\"}";
    }
    else if (command == "CHANGE_USER_ROLE") {
        return "{\"success\":false,\"message\":\"CHANGE_USER_ROLE needs userId and role\"}";
    }
    else if (command == "UPDATE_ORDER_STATUS") {
        return "{\"success\":false,\"message\":\"UPDATE_ORDER_STATUS needs orderId and status\"}";
    }
    else if (command == "ASSIGN_RIDER") {
        return "{\"success\":false,\"message\":\"ASSIGN_RIDER needs orderId and riderId\"}";
    }
    else if (command == "PLACE_ORDER") {
        return "{\"success\":false,\"message\":\"PLACE_ORDER needs data\"}";
    }
    else if (command == "ACCEPT_ORDER") {
        return "{\"success\":false,\"message\":\"ACCEPT_ORDER needs orderId and riderId\"}";
    }
    else if (command == "REGISTER") {
        return "{\"success\":false,\"message\":\"REGISTER needs data\"}";
    }
    
    return "{\"success\":false,\"message\":\"Command '" + command + "' not implemented\"}";
}
    string convertToJsonResponse(const string& oldResponse) {
        cout << "Converting response to JSON: " << oldResponse << endl;
        
        if (oldResponse.find("SUCCESS") == 0) {
            // Format: SUCCESS|id|name|role
            vector<string> parts;
            size_t start = 0, end;
            
            while ((end = oldResponse.find('|', start)) != string::npos) {
                parts.push_back(oldResponse.substr(start, end - start));
                start = end + 1;
            }
            parts.push_back(oldResponse.substr(start));
            
            if (parts.size() >= 4) {
                string json = "{";
                json += "\"success\":true,";
                json += "\"message\":\"Login successful\",";
                json += "\"user\":{";
                json += "\"id\":" + parts[1] + ",";
                json += "\"name\":\"" + parts[2] + "\",";
                json += "\"role\":\"" + parts[3] + "\"";
                json += "}";
                json += "}";
                return json;
            }
        }
        
        return "{\"success\":false,\"message\":\"" + oldResponse + "\"}";
    }
    
    string handleJsonLogin(const string& jsonString, int clientId) {
        // Extract email
        size_t emailPos = jsonString.find("\"email\":\"");
        if (emailPos == string::npos) {
            return "{\"success\":false,\"message\":\"Email not found in JSON\"}";
        }
        emailPos += 8; // Skip "\"email\":\""
        size_t emailEnd = jsonString.find("\"", emailPos);
        string email = jsonString.substr(emailPos, emailEnd - emailPos);
        
        // Extract password
        size_t passPos = jsonString.find("\"password\":\"");
        if (passPos == string::npos) {
            return "{\"success\":false,\"message\":\"Password not found in JSON\"}";
        }
        passPos += 11; // Skip "\"password\":\""
        size_t passEnd = jsonString.find("\"", passPos);
        string password = jsonString.substr(passPos, passEnd - passPos);
        
        // Extract role
        size_t rolePos = jsonString.find("\"role\":\"");
        string role = "customer"; // Default
        if (rolePos != string::npos) {
            rolePos += 7; // Skip "\"role\":\""
            size_t roleEnd = jsonString.find("\"", rolePos);
            role = jsonString.substr(rolePos, roleEnd - rolePos);
        }
        
        cout << "JSON Login - Email: " << email << ", Role: " << role << endl;
        
        // Authenticate using userManager
        UserData* user = userManager.authenticateUser(email, password);
        
        if (user && user->getRole() == role) {
            clientTypes[clientId] = role;
            
            // Return JSON response
            string jsonResponse = "{";
            jsonResponse += "\"success\":true,";
            jsonResponse += "\"message\":\"Login successful\",";
            jsonResponse += "\"user\":{";
            jsonResponse += "\"id\":" + to_string(user->id) + ",";
            jsonResponse += "\"name\":\"" + user->getName() + "\",";
            jsonResponse += "\"email\":\"" + user->getEmail() + "\",";
            jsonResponse += "\"role\":\"" + user->getRole() + "\"";
            jsonResponse += "}";
            jsonResponse += "}";
            
            return jsonResponse;
        }
        
        return "{\"success\":false,\"message\":\"Invalid credentials\"}";
    }
#endif
    
public:
    QuickBiteServer(int serverPort = 8080) 
        : port(serverPort), running(false), nextClientId(1), cityGraph(500) {
        
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            cerr << "WSAStartup failed\n";
            exit(1);
        }
#endif
        
        loadSystemData();
        initializeRiderStats();
    }
    
    ~QuickBiteServer() {
        stop();
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    void initializeRiderStats() {
        dbManager.getRidersHashTable().traverse([&](int id, Rider& rider) {
            riderStatus[id] = rider.getStatus();
            if (riderStatistics.find(id) == riderStatistics.end()) {
                RiderStats stats;
                stats.riderId = id;
                riderStatistics[id] = stats;
            }
        });
    }
    
    void loadSystemData() {
        cout << "\n========================================\n";
        cout << "   LOADING SYSTEM DATA...              \n";
        cout << "========================================\n";
        
        bool isEmpty = dbManager.getDatabase().loadAllUsers().empty() &&
                       dbManager.getDatabase().loadAllRestaurants().empty();
        
        if (isEmpty) {
            cout << "⚠ Database is empty. Initializing with sample data...\n";
            dbManager.initializeSampleData(cityGraph);
            cityGraph.saveToDatabase();
            cout << "✓ City map initialized and saved\n";
        } else {
            cout << "✓ Database found. Loading existing data...\n";
            cityGraph.loadFromDatabase();
            cout << "✓ City map loaded (" << cityGraph.getAllLocations().size() 
                 << " locations, " << cityGraph.getRoadCount() << " roads)\n";
        }
        
        // Load ALL users into the server's LOCAL userManager
        vector<UserData> users = dbManager.getDatabase().loadAllUsers();
        
        cout << "Loading " << users.size() << " users into server userManager...\n";
        
        for (const auto& user : users) {
            bool success = userManager.registerUser(
                user.id, 
                user.getName(), 
                user.getEmail(),
                user.getPhone(), 
                user.getPassword(),
                user.getRole(), 
                user.getAddress()
            );
            
            if (success) {
                cout << "  ✓ Loaded: " << user.email << " (ID: " << user.id 
                     << ", Role: " << user.role << ")\n";
            } else {
                cout << "  ✗ Failed to load: " << user.email << "\n";
            }
        }
        
        cout << "✓ Loaded " << users.size() << " users into server userManager\n";
        
        // Debug verification
        cout << "\nDEBUG: Verifying users in server userManager:\n";
        int verifiedCount = 0;
        for (const auto& user : users) {
            UserData* loadedUser = userManager.getUser(user.id);
            if (loadedUser) {
                verifiedCount++;
                cout << "  ✓ ID: " << user.id << ", Email: " << user.email 
                     << ", Role: " << user.role << "\n";
            } else {
                cout << "  ✗ MISSING: ID: " << user.id << ", Email: " << user.email << "\n";
            }
        }
        cout << "Verified " << verifiedCount << " out of " << users.size() << " users\n\n";
        
        // Load restaurants
        restaurants = dbManager.getDatabase().loadAllRestaurants();
        cout << "✓ Loaded " << restaurants.size() << " restaurants\n";
        
        // Load and associate menu items
        vector<MenuItem> menuItems = dbManager.getDatabase().loadAllMenuItems();
        for (auto& restaurant : restaurants) {
            for (const auto& item : menuItems) {
                if (item.restaurantId == restaurant.getRestaurantId()) {
                    restaurant.addMenuItemId(item.id);
                }
            }
        }
        cout << "✓ Loaded " << menuItems.size() << " menu items\n";
        
        // Load orders
        orders = dbManager.getDatabase().loadAllOrders();
        cout << "✓ Loaded " << orders.size() << " orders\n";
        
        // Load riders into hash table AND ensure their user accounts exist
        vector<Rider> riders = dbManager.getDatabase().loadAllRiders();
        cout << "Loading " << riders.size() << " riders...\n";
        
        for (const auto& rider : riders) {
            // Insert into riders hash table
            dbManager.getRidersHashTable().insertItem(rider.getId(), rider);
            
            // Verify the rider's user account exists in userManager
            UserData* riderUser = userManager.getUser(rider.getId());
            if (riderUser) {
                cout << "  ✓ Rider verified: " << rider.getName() 
                     << " (ID: " << rider.getId() << ", User Role: " 
                     << riderUser->getRole() << ")\n";
            } else {
                cout << "  ⚠ WARNING: Rider " << rider.getName() 
                     << " (ID: " << rider.getId() << ") has no user account!\n";
            }
        }
        cout << "✓ Loaded " << riders.size() << " riders\n";
        
        cout << "========================================\n";
        cout << "   SYSTEM DATA LOADED SUCCESSFULLY     \n";
        cout << "========================================\n";
        cout << "  Total Users: " << verifiedCount << "\n";
        cout << "  Total Restaurants: " << restaurants.size() << "\n";
        cout << "  Total Orders: " << orders.size() << "\n";
        cout << "  Total Riders: " << riders.size() << "\n";
        cout << "========================================\n\n";
    }
    
    bool start() {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            cerr << "Failed to create socket\n";
            return false;
        }
        
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, 
                   (const char*)&opt, sizeof(opt));
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);
        
        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "Bind failed\n";
            CLOSE_SOCKET(serverSocket);
            return false;
        }
        
        if (listen(serverSocket, 10) == SOCKET_ERROR) {
            cerr << "Listen failed\n";
            CLOSE_SOCKET(serverSocket);
            return false;
        }
        
        running = true;
        cout << "\n========================================\n";
        cout << "  QuickBite Server Started\n";
        cout << "========================================\n";
        cout << "  Port: " << port << "\n";
        cout << "  Waiting for connections...\n";
        cout << "========================================\n\n";
        
#ifdef _WIN32
        HANDLE acceptThread = CreateThread(NULL, 0, acceptConnectionsStatic, this, 0, NULL);
        if (acceptThread) CloseHandle(acceptThread);
#else
        thread acceptThread(&QuickBiteServer::acceptConnections, this);
        acceptThread.detach();
#endif
        
        return true;
    }
    
    void stop() {
        running = false;
        
#ifdef _WIN32
        LockGuard lock(clientsMutex);
#else
        lock_guard<mutex> lock(clientsMutex);
#endif
        for (auto& client : connectedClients) {
            CLOSE_SOCKET(client.second);
        }
        connectedClients.clear();
        
        if (serverSocket != INVALID_SOCKET) {
            CLOSE_SOCKET(serverSocket);
        }
        
        saveSystemData();
        cout << "\nServer stopped.\n";
    }
    
    void saveSystemData() {
        cout << "\nSaving system data...\n";
        
#ifdef _WIN32
        LockGuard lock(dataMutex);
#else
        lock_guard<mutex> lock(dataMutex);
#endif
        
        // Save ALL users from userManager (including riders)
        vector<UserData> users;
        for (int id = 0; id < 10000; id++) {  // Increased range to catch all users
            UserData* user = userManager.getUser(id);
            if (user) {
                users.push_back(*user);
                cout << "  Saving user: " << user->email << " (ID: " << user->id << ", Role: " << user->role << ")\n";
            }
        }
        
        cout << "Total users to save: " << users.size() << "\n";
        dbManager.getDatabase().saveAllUsers(users);
        
        dbManager.getDatabase().saveAllRestaurants(restaurants);
        cout << "Saved " << restaurants.size() << " restaurants\n";
        
        vector<MenuItem> menuItems = dbManager.getDatabase().loadAllMenuItems();
        dbManager.getDatabase().saveAllMenuItems(menuItems);
        
        dbManager.getDatabase().saveAllOrders(orders);
        cout << "Saved " << orders.size() << " orders\n";
        
        vector<Rider> riders;
        dbManager.getRidersHashTable().traverse([&](int id, Rider& rider) {
            riders.push_back(rider);
        });
        dbManager.getDatabase().saveAllRiders(riders);
        cout << "Saved " << riders.size() << " riders\n";
        
        cityGraph.saveToDatabase();
        
        cout << "✓ System data saved\n";
    }
    
private:
#ifdef _WIN32
    static DWORD WINAPI acceptConnectionsStatic(LPVOID lpParam) {
        QuickBiteServer* server = (QuickBiteServer*)lpParam;
        server->acceptConnections();
        return 0;
    }
    
    static DWORD WINAPI handleClientStatic(LPVOID lpParam) {
        ClientParams* params = (ClientParams*)lpParam;
        params->server->handleClient(params->clientSocket, params->clientId);
        delete params;
        return 0;
    }
#endif
    
    void acceptConnections() {
        while (running) {
            sockaddr_in clientAddr;
            
#ifdef _WIN32
            int clientLen = sizeof(clientAddr);
#else
            socklen_t clientLen = sizeof(clientAddr);
#endif
            
            SocketType clientSocket = accept(serverSocket, 
                                            (sockaddr*)&clientAddr, 
                                            &clientLen);
            
            if (clientSocket == INVALID_SOCKET) {
                if (running) cerr << "Accept failed\n";
                continue;
            }
            
            char clientIP[INET_ADDRSTRLEN];
#ifdef _WIN32
            strcpy(clientIP, inet_ntoa(clientAddr.sin_addr));
#else
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
#endif
            
            int clientId = nextClientId++;
            
            {
#ifdef _WIN32
                LockGuard lock(clientsMutex);
#else
                lock_guard<mutex> lock(clientsMutex);
#endif
                connectedClients[clientId] = clientSocket;
            }
            
            cout << "✓ Client connected [ID: " << clientId 
                 << ", IP: " << clientIP << "]\n";
            
#ifdef _WIN32
            ClientParams* params = new ClientParams;
            params->server = this;
            params->clientSocket = clientSocket;
            params->clientId = clientId;
            
            HANDLE clientThread = CreateThread(NULL, 0, handleClientStatic, params, 0, NULL);
            if (clientThread) CloseHandle(clientThread);
#else
            thread clientThread(&QuickBiteServer::handleClient, this, 
                              clientSocket, clientId);
            clientThread.detach();
#endif
        }
    }
    void handleClient(SocketType clientSocket, int clientId) {
    char buffer[5000];
    
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived <= 0) break;
        
        buffer[bytesReceived] = '\0';
        string request(buffer);
        
        cout << "Received request: " << request << endl;
        
        string response;
        
        // Check if it's JSON
        if (request.find("{") != string::npos && request.find("command") != string::npos) {
            response = handleJsonRequest(request, clientId);
        } else {
            // Binary Message struct format
            Message msg;
            memcpy(&msg, buffer, min((size_t)bytesReceived, sizeof(Message)));
            msg.clientId = clientId;
            response = processCommand(msg);
        }
        
        // Send response WITH newline
        cout << "Sending response: " << response << endl;
        response += "\n";
        send(clientSocket, response.c_str(), (int)response.length(), 0);
        
        // Don't close connection immediately - wait for more requests
        // Add a small delay to ensure response is sent
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
    }
    
    {
#ifdef _WIN32
        LockGuard lock(clientsMutex);
#else
        lock_guard<mutex> lock(clientsMutex);
#endif
        connectedClients.erase(clientId);
        clientTypes.erase(clientId);
    }
    
    cout << "✗ Client disconnected [ID: " << clientId << "]" << endl;
    CLOSE_SOCKET(clientSocket);
}
    
    string processCommand(const Message& msg) {
        string command = msg.command;
        string data = msg.data;
        
        cout << "Processing: " << command << " from client " << msg.clientId << "\n";
        
#ifdef _WIN32
        LockGuard lock(dataMutex);
#else
        lock_guard<mutex> lock(dataMutex);
#endif
        
        if (command == "LOGIN") return handleLogin(data, msg.clientId);
        else if (command == "REGISTER") return handleRegister(data);
        else if (command == "GET_RESTAURANTS") return handleGetRestaurants();
        else if (command == "GET_MENU") return handleGetMenu(data);
        else if (command == "PLACE_ORDER") return handlePlaceOrder(data);
        else if (command == "GET_ORDERS") return handleGetOrders(data);
        else if (command == "GET_RIDERS") return handleGetRiders();
        else if (command == "UPDATE_ORDER_STATUS") return handleUpdateOrderStatus(data);
        else if (command == "ASSIGN_RIDER") return handleAssignRider(data);
        else if (command == "GET_CITY_MAP") return handleGetCityMap();
        else if (command == "GET_AVAILABLE_ORDERS") return handleGetAvailableOrders();
        else if (command == "GET_RIDER_ORDERS") return handleGetRiderOrders(data);
        else if (command == "UPDATE_RIDER_STATUS") return handleUpdateRiderStatus(data);
        else if (command == "GET_RIDER_STATS") return handleGetRiderStats(data);
        else if (command == "GET_DELIVERY_ROUTE") return handleGetDeliveryRoute(data);
        else if (command == "GET_ALL_ORDERS") return handleGetAllOrders();
        else if (command == "GET_ALL_USERS") return handleGetAllUsers();
        else if (command == "GET_SYSTEM_STATS") return handleGetSystemStats();
        else if (command == "ADD_RESTAURANT") return handleAddRestaurant(data);
        else if (command == "REMOVE_RESTAURANT") return handleRemoveRestaurant(data);
        else if (command == "ADD_MENU_ITEM") return handleAddMenuItem(data);
        else if (command == "REMOVE_MENU_ITEM") return handleRemoveMenuItem(data);
        else if (command == "ADD_RIDER") return handleAddRider(data);
        else if (command == "REMOVE_RIDER") return handleRemoveRider(data);
        else if (command == "CHANGE_USER_ROLE") return handleChangeUserRole(data);
        else if (command == "PING") return "PONG";
        
        return "ERROR:Unknown command";
    }
    
    // === COMMAND HANDLERS ===
    
    string handleLogin(const string& data, int clientId) {
        size_t pos = data.find('|');
        if (pos == string::npos) return "ERROR:Invalid format";
        
        string email = data.substr(0, pos);
        string password = data.substr(pos + 1);
        
        cout << "DEBUG LOGIN: Email='" << email << "', Password='" << password << "'\n";
        
        // Use the server's local userManager (not dbManager's)
        UserData* user = userManager.authenticateUser(email, password);
        
        if (user) {
            cout << "✓ Login successful: " << user->getName() << " (Role: " << user->getRole() << ")\n";
            clientTypes[clientId] = user->getRole();
            return "SUCCESS|" + to_string(user->id) + "|" + 
                   user->getName() + "|" + user->getRole();
        }
        
        cout << "✗ Login failed for: " << email << "\n";
        
        return "ERROR:Invalid credentials";
    }
    
    string handleRegister(const string& data) {
        vector<string> parts;
        size_t start = 0, end;
        
        while ((end = data.find('|', start)) != string::npos) {
            parts.push_back(data.substr(start, end - start));
            start = end + 1;
        }
        parts.push_back(data.substr(start));
        
        if (parts.size() < 5) return "ERROR:Invalid format";
        
        if (userManager.getUserByEmail(parts[1])) {
            return "ERROR:Email already registered";
        }
        
        int newId = userManager.getTotalUsers() + 100;
        bool success = userManager.registerUser(newId, parts[0], parts[1], 
                                               parts[3], parts[2], "customer", parts[4]);
        
        if (success) {
            UserData newUser(newId, parts[0], parts[1], parts[3], parts[2], "customer", parts[4]);
            dbManager.getDatabase().saveUser(newUser);
            return "SUCCESS|" + to_string(newId);
        }
        
        return "ERROR:Registration failed";
    }
    
    string handleGetRestaurants() {
        string result = "SUCCESS|";
        
        for (size_t i = 0; i < restaurants.size(); i++) {
            const auto& r = restaurants[i];
            result += to_string(r.getRestaurantId()) + ";" +
                     r.getName() + ";" +
                     r.getCuisine() + ";" +
                     r.getAddress() + ";" +
                     to_string(r.getRating()) + ";" +
                     to_string(r.getDeliveryTime());
            
            if (i < restaurants.size() - 1) result += "|";
        }
        
        return result;
    }
    
    string handleGetMenu(const string& data) {
        int restaurantId = stoi(data);
        vector<MenuItem> menuItems = dbManager.getMenuItemsByRestaurant(restaurantId);
        
        string result = "SUCCESS|";
        
        for (size_t i = 0; i < menuItems.size(); i++) {
            const auto& item = menuItems[i];
            result += to_string(item.id) + ";" +
                     item.getName() + ";" +
                     item.getDescription() + ";" +
                     to_string(item.price) + ";" +
                     to_string(item.stock) + ";" +
                     item.getCategory();
            
            if (i < menuItems.size() - 1) result += "|";
        }
        
        return result;
    }
    
    string handlePlaceOrder(const string& data) {
        size_t pos1 = data.find('|');
        size_t pos2 = data.find('|', pos1 + 1);
        
        if (pos1 == string::npos || pos2 == string::npos) {
            return "ERROR:Invalid format";
        }
        
        int customerId = stoi(data.substr(0, pos1));
        int restaurantId = stoi(data.substr(pos1 + 1, pos2 - pos1 - 1));
        string itemsStr = data.substr(pos2 + 1);
        
        int orderId = orders.empty() ? 1000 : orders.back().getOrderId() + 1;
        
        UserData* user = userManager.getUser(customerId);
        if (!user) return "ERROR:User not found";
        
        Order newOrder(orderId, customerId, restaurantId, 
                      user->getAddress(), -1, 0);
        
        size_t start = 0, end;
        while ((end = itemsStr.find(',', start)) != string::npos) {
            string itemPair = itemsStr.substr(start, end - start);
            size_t colon = itemPair.find(':');
            
            if (colon != string::npos) {
                int itemId = stoi(itemPair.substr(0, colon));
                int quantity = stoi(itemPair.substr(colon + 1));
                
                vector<MenuItem> menuItems = dbManager.getMenuItemsByRestaurant(restaurantId);
                for (const auto& item : menuItems) {
                    if (item.id == itemId) {
                        newOrder.addItem(itemId, item.getName(), quantity, item.price);
                        break;
                    }
                }
            }
            
            start = end + 1;
        }
        
        if (start < itemsStr.length()) {
            string itemPair = itemsStr.substr(start);
            size_t colon = itemPair.find(':');
            
            if (colon != string::npos) {
                int itemId = stoi(itemPair.substr(0, colon));
                int quantity = stoi(itemPair.substr(colon + 1));
                
                vector<MenuItem> menuItems = dbManager.getMenuItemsByRestaurant(restaurantId);
                for (const auto& item : menuItems) {
                    if (item.id == itemId) {
                        newOrder.addItem(itemId, item.getName(), quantity, item.price);
                        break;
                    }
                }
            }
        }
        
        orders.push_back(newOrder);
        dbManager.getDatabase().saveOrder(newOrder);
        
        return "SUCCESS|" + to_string(orderId);
    }
    
    string handleGetOrders(const string& data) {
        int userId = stoi(data);
        
        string result = "SUCCESS|";
        bool first = true;
        
        for (const auto& order : orders) {
            if (order.getCustomerId() == userId) {
                if (!first) result += "|";
                first = false;
                
                result += to_string(order.getOrderId()) + ";" +
                         to_string(order.getRestaurant()) + ";" +
                         to_string(order.getTotalAmount()) + ";" +
                         order.getStatusAsString();
            }
        }
        
        return result;
    }
    
    string handleGetRiders() {
        string result = "SUCCESS|";
        bool first = true;
        
        dbManager.getRidersHashTable().traverse([&](int id, Rider& rider) {
            if (!first) result += "|";
            first = false;
            
            result += to_string(rider.getId()) + ";" +
                     rider.getName() + ";" +
                     rider.getStatus();
        });
        
        return result;
    }
    
    string handleUpdateOrderStatus(const string& data) {
        size_t pos = data.find('|');
        if (pos == string::npos) return "ERROR:Invalid format";
        
        int orderId = stoi(data.substr(0, pos));
        string newStatus = data.substr(pos + 1);
        
        for (auto& order : orders) {
            if (order.getOrderId() == orderId) {
                if (newStatus == "Preparing") order.updateStatus(OrderStatus::Preparing);
                else if (newStatus == "Dispatched") order.updateStatus(OrderStatus::Dispatched);
                else if (newStatus == "In Transit") order.updateStatus(OrderStatus::InTransit);
                else if (newStatus == "Delivered") {
                    order.updateStatus(OrderStatus::Delivered);
                    
                    int riderId = order.getRiderID();
                    if (riderId != -1) {
                        if (riderStatistics.find(riderId) == riderStatistics.end()) {
                            riderStatistics[riderId] = RiderStats();
                            riderStatistics[riderId].riderId = riderId;
                        }
                        
                        riderStatistics[riderId].totalDeliveries++;
                        riderStatistics[riderId].todayDeliveries++;
                        riderStatistics[riderId].successfulDeliveries++;
                        riderStatistics[riderId].onTimeDeliveries++;
                        riderStatistics[riderId].totalEarnings += order.getTotalAmount() * 0.1;
                        
                        riderStatus[riderId] = "Active";
                        
                        cout << "✓ Rider " << riderId << " completed delivery. "
                             << "Earnings: $" << riderStatistics[riderId].totalEarnings << "\n";
                    }
                }
                else if (newStatus == "Cancelled") {
                    order.updateStatus(OrderStatus::Cancelled);
                    
                    int riderId = order.getRiderID();
                    if (riderId != -1 && riderStatistics.find(riderId) != riderStatistics.end()) {
                        riderStatistics[riderId].failedDeliveries++;
                    }
                }
                
                dbManager.updateOrder(order);
                return "SUCCESS";
            }
        }
        
        return "ERROR:Order not found";
    }
    
    string handleAssignRider(const string& data) {
        size_t pos = data.find('|');
        if (pos == string::npos) return "ERROR:Invalid format";
        
        int orderId = stoi(data.substr(0, pos));
        int riderId = stoi(data.substr(pos + 1));
        
        for (auto& order : orders) {
            if (order.getOrderId() == orderId) {
                order.assignRider(riderId);
                
                riderStatus[riderId] = "Busy";
                
                Rider* rider = dbManager.getRidersHashTable().getItem(riderId);
                if (rider) {
                    rider->setStatus("Busy");
                }
                
                dbManager.updateOrder(order);
                
                cout << "✓ Order " << orderId << " assigned to Rider " << riderId << "\n";
                
                return "SUCCESS";
            }
        }
        
        return "ERROR:Order not found";
    }
    
    string handleGetCityMap() {
        auto locations = cityGraph.getAllLocations();
        
        string result = "SUCCESS|";
        bool first = true;
        
        for (const auto& loc : locations) {
            if (!first) result += "|";
            first = false;
            
            result += to_string(loc.first) + ";" + loc.second;
        }
        
        return result;
    }
    
    // === NEW RIDER-SPECIFIC HANDLERS ===
    
    string handleGetAvailableOrders() {
        string result = "SUCCESS|";
        bool first = true;
        
        for (const auto& order : orders) {
            if (order.getRiderID() == -1 && 
                (order.getStatusAsString() == "Pending" || 
                 order.getStatusAsString() == "Preparing")) {
                
                if (!first) result += "|";
                first = false;
                
                string restaurantName = "Unknown";
                for (const auto& r : restaurants) {
                    if (r.getRestaurantId() == order.getRestaurant()) {
                        restaurantName = r.getName();
                        break;
                    }
                }
                
                double distance = 2.5 + (order.getOrderId() % 10) * 0.5;
                
                result += to_string(order.getOrderId()) + ";" +
                         restaurantName + ";" +
                         order.getDeliveryAddress() + ";" +
                         to_string(order.getTotalAmount()) + ";" +
                         to_string(distance);
            }
        }
        
        return result;
    }
    
    string handleGetRiderOrders(const string& data) {
        int riderId = stoi(data);
        
        string result = "SUCCESS|";
        bool first = true;
        
        for (const auto& order : orders) {
            if (order.getRiderID() == riderId) {
                if (!first) result += "|";
                first = false;
                
                string restaurantName = "Unknown";
                for (const auto& r : restaurants) {
                    if (r.getRestaurantId() == order.getRestaurant()) {
                        restaurantName = r.getName();
                        break;
                    }
                }
                
                string customerName = "Unknown";
                UserData* customer = userManager.getUser(order.getCustomerId());
                if (customer) {
                    customerName = customer->getName();
                }
                
                result += to_string(order.getOrderId()) + ";" +
                         restaurantName + ";" +
                         customerName + ";" +
                         order.getDeliveryAddress() + ";" +
                         order.getStatusAsString() + ";" +
                         to_string(order.getTotalAmount());
            }
        }
        
        return result;
    }
    
    string handleUpdateRiderStatus(const string& data) {
        size_t pos = data.find('|');
        if (pos == string::npos) return "ERROR:Invalid format";
        
        int riderId = stoi(data.substr(0, pos));
        string newStatus = data.substr(pos + 1);
        
        if (newStatus != "Active" && newStatus != "Busy" && newStatus != "Offline") {
            return "ERROR:Invalid status";
        }
        
        Rider* rider = dbManager.getRidersHashTable().getItem(riderId);
        if (!rider) {
            return "ERROR:Rider not found";
        }
        
        riderStatus[riderId] = newStatus;
        rider->setStatus(newStatus);
        
        if (riderStatistics.find(riderId) != riderStatistics.end()) {
            riderStatistics[riderId].lastActiveTime = time(nullptr);
        }
        
        cout << "✓ Rider " << riderId << " status: " << newStatus << "\n";
        
        return "SUCCESS";
    }
    
    string handleGetRiderStats(const string& data) {
        int riderId = stoi(data);
        
        if (riderStatistics.find(riderId) == riderStatistics.end()) {
            RiderStats stats;
            stats.riderId = riderId;
            riderStatistics[riderId] = stats;
        }
        
        const RiderStats& stats = riderStatistics[riderId];
        
        int totalAttempts = stats.successfulDeliveries + stats.failedDeliveries;
        double successRate = totalAttempts > 0 ? 
            (stats.successfulDeliveries * 100.0 / totalAttempts) : 98.5;
        
        double onTimeRate = stats.totalDeliveries > 0 ?
            (stats.onTimeDeliveries * 100.0 / stats.totalDeliveries) : 95.0;
        
        string result = "SUCCESS|" +
                       to_string(stats.totalDeliveries) + ";" +
                       to_string(stats.todayDeliveries) + ";" +
                       to_string(successRate) + ";" +
                       to_string(stats.averageRating) + ";" +
                       to_string(onTimeRate) + ";" +
                       to_string(stats.totalEarnings);
        
        return result;
    }
    
    string handleGetDeliveryRoute(const string& data) {
        size_t pos = data.find('|');
        if (pos == string::npos) return "ERROR:Invalid format";
        
        int orderId = stoi(data.substr(0, pos));
        int riderLocationId = stoi(data.substr(pos + 1));
        
        Order* targetOrder = nullptr;
        for (auto& order : orders) {
            if (order.getOrderId() == orderId) {
                targetOrder = &order;
                break;
            }
        }
        
        if (!targetOrder) {
            return "ERROR:Order not found";
        }
        
        int restaurantLoc = 1;
        for (const auto& r : restaurants) {
            if (r.getRestaurantId() == targetOrder->getRestaurant()) {
                restaurantLoc = r.getRestaurantId() % 20;
                break;
            }
        }
        
        int customerLoc = (targetOrder->getCustomerId() % 20) + 1;
        
        // Get shortest paths using Dijkstra from CityGraph
        auto pathResult1 = cityGraph.findShortestPath(riderLocationId, restaurantLoc);
        auto pathResult2 = cityGraph.findShortestPath(restaurantLoc, customerLoc);
        
        LinkedList<int> route1 = pathResult1.first;
        LinkedList<int> route2 = pathResult2.first;
        
        string result = "SUCCESS|";
        
        result += "TO_RESTAURANT|";
        auto* current1 = route1.getHead();
        bool firstNode = true;
        while (current1 != nullptr) {
            if (!firstNode) result += ",";
            result += to_string(current1->data);
            firstNode = false;
            current1 = current1->next;
        }
        
        result += "|TO_CUSTOMER|";
        auto* current2 = route2.getHead();
        firstNode = true;
        while (current2 != nullptr) {
            if (!firstNode) result += ",";
            result += to_string(current2->data);
            firstNode = false;
            current2 = current2->next;
        }
        
        return result;
    }
    
    // === ADMIN COMMANDS ===
    
    string handleGetAllOrders() {
        string result = "SUCCESS|";
        bool first = true;
        
        for (const auto& order : orders) {
            if (!first) result += "|";
            first = false;
            
            // Find customer name
            string customerName = "Unknown";
            UserData* customer = userManager.getUser(order.getCustomerId());
            if (customer) {
                customerName = customer->getName();
            }
            
            // Find restaurant name
            string restaurantName = "Unknown";
            for (const auto& r : restaurants) {
                if (r.getRestaurantId() == order.getRestaurant()) {
                    restaurantName = r.getName();
                    break;
                }
            }
            
            // Find rider name
            string riderName = "Unassigned";
            if (order.getRiderID() != -1) {
                Rider* rider = dbManager.getRidersHashTable().getItem(order.getRiderID());
                if (rider) {
                    riderName = rider->getName();
                }
            }
            
            result += to_string(order.getOrderId()) + ";" +
                     customerName + ";" +
                     restaurantName + ";" +
                     riderName + ";" +
                     order.getStatusAsString() + ";" +
                     to_string(order.getTotalAmount());
        }
        
        return result;
    }
    
    string handleGetAllUsers() {
        string result = "SUCCESS|";
        bool first = true;
        
        for (int id = 0; id < 10000; id++) {
            UserData* user = userManager.getUser(id);
            if (user) {
                if (!first) result += "|";
                first = false;
                
                result += to_string(user->id) + ";" +
                         user->getName() + ";" +
                         user->getEmail() + ";" +
                         user->getRole() + ";" +
                         user->getPhone();
            }
        }
        
        return result;
    }
    
    string handleGetSystemStats() {
        int totalUsers = 0;
        int totalCustomers = 0;
        int totalRiders = 0;
        int totalAdmins = 0;
        
        for (int id = 0; id < 10000; id++) {
            UserData* user = userManager.getUser(id);
            if (user) {
                totalUsers++;
                if (user->getRole() == "customer") totalCustomers++;
                else if (user->getRole() == "rider") totalRiders++;
                else if (user->getRole() == "admin") totalAdmins++;
            }
        }
        
        int totalRestaurants = restaurants.size();
        int totalOrders = orders.size();
        
        int pendingOrders = 0;
        int completedOrders = 0;
        double totalRevenue = 0.0;
        
        for (const auto& order : orders) {
            if (order.getStatusAsString() == "Delivered") {
                completedOrders++;
                totalRevenue += order.getTotalAmount();
            } else if (order.getStatusAsString() != "Cancelled") {
                pendingOrders++;
            }
        }
        
        string result = "SUCCESS|" +
                       to_string(totalUsers) + ";" +
                       to_string(totalCustomers) + ";" +
                       to_string(totalRiders) + ";" +
                       to_string(totalAdmins) + ";" +
                       to_string(totalRestaurants) + ";" +
                       to_string(totalOrders) + ";" +
                       to_string(pendingOrders) + ";" +
                       to_string(completedOrders) + ";" +
                       to_string(totalRevenue);
        
        return result;
    }
    
    string handleAddRestaurant(const string& data) {
    // Parse: name|cuisine|address|rating|deliveryTime
    vector<string> parts;
    size_t start = 0, end;
    
    while ((end = data.find('|', start)) != string::npos) {
        parts.push_back(data.substr(start, end - start));
        start = end + 1;
    }
    parts.push_back(data.substr(start));
    
    if (parts.size() < 5) return "ERROR:Invalid format";
    
    try {
        int newId = restaurants.empty() ? 100 : restaurants.back().getRestaurantId() + 1;
        double rating = stod(parts[3]);
        int deliveryTime = stoi(parts[4]);
        
        // === STEP 1: Generate unique location ID ===
        auto locations = cityGraph.getAllLocations();
        int locationNodeId;
        
        // Try to find an available node ID in the restaurant range (100-199)
        for (int i = 110; i <= 199; i += 10) {
            if (locations.find(i) == locations.end()) {
                locationNodeId = i;
                break;
            }
        }
        
        // If no slot found, use next available
        if (locationNodeId == 0) {
            int maxId = 0;
            for (const auto& loc : locations) {
                if (loc.first > maxId) maxId = loc.first;
            }
            locationNodeId = maxId + 10;
        }
        
        // === STEP 2: Add to City Graph ===
        string locationName = parts[0] + " Area";
        cout << "✓ Adding restaurant to city graph: " 
             << parts[0] << " at node " << locationNodeId << "\n";
        
        // Add the location node
        cityGraph.addLocation(locationNodeId, locationName, "restaurant");
        
        // === STEP 3: Connect to existing network ===
        cout << "  Connecting to existing network...\n";
        
        // Method 1: Connect to nearest existing restaurant
        int nearestRestaurant = cityGraph.findNearestNode(locationNodeId, "restaurant");
        if (nearestRestaurant != -1) {
            int distance = 600 + (rand() % 400); // 600-1000m
            cityGraph.addRoad(locationNodeId, nearestRestaurant, distance);
            cout << "  Connected to nearest restaurant [" << nearestRestaurant 
                 << "] - " << distance << "m\n";
        }
        
        // Method 2: Connect to city center (node 400 if exists)
        if (cityGraph.locationExists(400)) {
            int distance = 800 + (rand() % 400); // 800-1200m
            cityGraph.addRoad(locationNodeId, 400, distance);
            cout << "  Connected to City Center [400] - " << distance << "m\n";
        }
        
        // Method 3: Connect to a random customer district
        vector<int> customerDistricts;
        for (int i = 200; i <= 299; i++) {
            if (cityGraph.locationExists(i) && 
                cityGraph.getLocationType(i) == "home") {
                customerDistricts.push_back(i);
            }
        }
        
        if (!customerDistricts.empty()) {
            int randomDistrict = customerDistricts[rand() % customerDistricts.size()];
            int distance = 1000 + (rand() % 500); // 1000-1500m
            cityGraph.addRoad(locationNodeId, randomDistrict, distance);
            cout << "  Connected to Customer District [" << randomDistrict 
                 << "] - " << distance << "m\n";
        }
        
        // === STEP 4: Save the graph ===
        cityGraph.saveToDatabase();
        cout << "✓ City graph updated and saved\n";
        
        // === STEP 5: Create and save restaurant ===
        Restaurant newRestaurant(newId, parts[0], parts[1], parts[2], rating, deliveryTime);
        
        restaurants.push_back(newRestaurant);
        
        // Save to database
        dbManager.getDatabase().saveRestaurant(newRestaurant);
        
        cout << "✓ New restaurant added: " << parts[0] 
             << " (ID: " << newId << ", Location Node: " << locationNodeId << ")\n";
        
        return "SUCCESS|" + to_string(newId);
        
    } catch (const exception& e) {
        return "ERROR:" + string(e.what());
    }
}
    string handleRemoveRestaurant(const string& data) {
        try {
            int restaurantId = stoi(data);
            
            // Find and remove from local vector
            auto it = find_if(restaurants.begin(), restaurants.end(),
                [restaurantId](const Restaurant& r) {
                    return r.getRestaurantId() == restaurantId;
                });
            
            if (it != restaurants.end()) {
                // Remove from database first
                if (dbManager.getDatabase().deleteRestaurant(restaurantId)) {
                    // Then remove from local vector
                    restaurants.erase(it);
                    cout << "✓ Restaurant removed: ID " << restaurantId << "\n";
                    return "SUCCESS";
                } else {
                    return "ERROR:Failed to delete from database";
                }
            }
            return "ERROR:Restaurant not found";
        } catch (const exception& e) {
            return "ERROR:" + string(e.what());
        }
    }
    string handleAddMenuItem(const string& data) {
    // Parse: restaurantId|name|description|price|stock|category
    vector<string> parts;
    size_t start = 0, end;
    
    while ((end = data.find('|', start)) != string::npos) {
        parts.push_back(data.substr(start, end - start));
        start = end + 1;
    }
    parts.push_back(data.substr(start));
    
    if (parts.size() < 6) return "ERROR:Invalid format";
    
    try {
        int restaurantId = stoi(parts[0]);
        double price = stod(parts[3]);
        int stock = stoi(parts[4]);
        
        // Check if restaurant exists
        bool restaurantExists = false;
        for (const auto& restaurant : restaurants) {
            if (restaurant.getRestaurantId() == restaurantId) {
                restaurantExists = true;
                break;
            }
        }
        
        if (!restaurantExists) {
            return "ERROR:Restaurant not found";
        }
        
        // Generate new menu item ID
        vector<MenuItem> allItems = dbManager.getDatabase().loadAllMenuItems();
        int newId = allItems.empty() ? 1000 : allItems.back().id + 1;
        
        // FIXED: Correct MenuItem constructor call based on MenuItem.h
        // Constructor signature: (id, name, description, price, stock, category, restaurantId)
        MenuItem newItem(newId, parts[1], parts[2], price, stock, parts[5], restaurantId);
        
        // Save to database
        dbManager.getDatabase().saveMenuItem(newItem);
        
        // Update local restaurant's menu
        for (auto& restaurant : restaurants) {
            if (restaurant.getRestaurantId() == restaurantId) {
                restaurant.addMenuItemId(newId);
                break;
            }
        }
        
        cout << "✓ New menu item added: " << parts[1] << " (ID: " << newId << ")\n";
        return "SUCCESS|" + to_string(newId);
    } catch (const exception& e) {
        return "ERROR:" + string(e.what());
    }
}
    
    string handleRemoveMenuItem(const string& data) {
        // Parse: restaurantId|itemId
        size_t pos = data.find('|');
        if (pos == string::npos) return "ERROR:Invalid format";
        
        try {
            int restaurantId = stoi(data.substr(0, pos));
            int itemId = stoi(data.substr(pos + 1));
            
            // Remove from database
            if (dbManager.getDatabase().deleteMenuItem(itemId)) {
                // Update local restaurant
                for (auto& restaurant : restaurants) {
                    if (restaurant.getRestaurantId() == restaurantId) {
                        restaurant.removeMenuItemId(itemId);
                        break;
                    }
                }
                
                cout << "✓ Menu item removed: ID " << itemId << "\n";
                return "SUCCESS";
            } else {
                return "ERROR:Failed to delete from database";
            }
        } catch (const exception& e) {
            return "ERROR:" + string(e.what());
        }
    }
    
    string handleAddRider(const string& data) {
        // Parse: name|email|phone|password|address|vehicle
        vector<string> parts;
        size_t start = 0, end;
        
        while ((end = data.find('|', start)) != string::npos) {
            parts.push_back(data.substr(start, end - start));
            start = end + 1;
        }
        parts.push_back(data.substr(start));
        
        if (parts.size() < 6) return "ERROR:Invalid format";
        
        try {
            // Check if email already exists
            if (userManager.getUserByEmail(parts[1])) {
                return "ERROR:Email already registered";
            }
            
            // Generate new IDs
            vector<UserData> allUsers = dbManager.getDatabase().loadAllUsers();
            vector<Rider> allRiders = dbManager.getDatabase().loadAllRiders();
            
            int userId = allUsers.empty() ? 100 : allUsers.back().id + 1;
            int riderId = allRiders.empty() ? 100 : allRiders.back().getId() + 1;
            
            // 1. Create user account
            bool userSuccess = userManager.registerUser(userId, parts[0], parts[1], parts[2], 
                                                       parts[3], "rider", parts[4]);
            
            if (!userSuccess) {
                return "ERROR:Failed to create user account";
            }
            
            UserData newUser(userId, parts[0], parts[1], parts[2], 
                            "rider", parts[4], parts[3]);
            dbManager.getDatabase().saveUser(newUser);
            
            // 2. Create rider profile
            Rider newRider(riderId, parts[0], parts[1], parts[3], 
                          parts[2], parts[5], 4.5);
            newRider.setStatus("Active");
            dbManager.getDatabase().saveRider(newRider);
            dbManager.getRidersHashTable().insertItem(riderId, newRider);
            
            // Initialize rider stats
            RiderStats stats;
            stats.riderId = riderId;
            stats.lastActiveTime = time(nullptr);
            riderStatistics[riderId] = stats;
            riderStatus[riderId] = "Active";
            
            cout << "✓ New rider added: " << parts[0] 
                 << " (User ID: " << userId << ", Rider ID: " << riderId << ")\n";
            return "SUCCESS|" + to_string(riderId);
        } catch (const exception& e) {
            return "ERROR:" + string(e.what());
        }
    }
   string handleRemoveRider(const string& data) {
    try {
        int riderId = stoi(data);
        
        // Get rider info first
        Rider* rider = dbManager.getRidersHashTable().getItem(riderId);
        if (!rider) {
            return "ERROR:Rider not found";
        }
        
        // Remove from database
        if (dbManager.getDatabase().deleteRider(riderId)) {
            // Remove from hash table
            dbManager.getRidersHashTable().removeItem(riderId);
            
            // Remove from stats tracking
            riderStatus.erase(riderId);
            riderStatistics.erase(riderId);
            
            // Don't delete user account - change role to customer using UserManager
            UserData* user = userManager.getUser(riderId);
            if (user) {
                // FIXED: Use UserManager to update role instead of direct assignment
                userManager.updateUserRole(riderId, "customer");
                
                // Save the updated user to database
                UserData* updatedUser = userManager.getUser(riderId);
                if (updatedUser) {
                    dbManager.getDatabase().saveUser(*updatedUser);
                }
            }
            
            cout << "✓ Rider removed: ID " << riderId << "\n";
            return "SUCCESS";
        } else {
            return "ERROR:Failed to delete from database";
        }
    } catch (const exception& e) {
        return "ERROR:" + string(e.what());
    }
}
  string handleChangeUserRole(const string& data) {
    // Parse: userId|newRole
    size_t pos = data.find('|');
    if (pos == string::npos) return "ERROR:Invalid format";
    
    try {
        int userId = stoi(data.substr(0, pos));
        string newRole = data.substr(pos + 1);
        
        // Validate role
        if (newRole != "customer" && newRole != "rider" && 
            newRole != "restaurant_owner" && newRole != "admin") {
            return "ERROR:Invalid role";
        }
        
        // Update in userManager
        UserData* user = userManager.getUser(userId);
        if (!user) {
            return "ERROR:User not found";
        }
        
        string oldRole = user->getRole();
        
        // FIXED: Update role in userManager directly
        bool roleUpdated = userManager.updateUserRole(userId, newRole);
        if (!roleUpdated) {
            return "ERROR:Failed to update user role";
        }
        
        // Get updated user data
        UserData* updatedUser = userManager.getUser(userId);
        if (updatedUser) {
            // Save to database using the updated user
            dbManager.getDatabase().saveUser(*updatedUser);
        } else {
            return "ERROR:Failed to get updated user data";
        }
        
        // If changing to/from rider, update rider table
        if (newRole == "rider" && oldRole != "rider") {
            // Create rider entry
            vector<Rider> allRiders = dbManager.getDatabase().loadAllRiders();
            int riderId = allRiders.empty() ? 100 : allRiders.back().getId() + 1;
            
            Rider newRider(riderId, user->getName(), user->getEmail(), 
                          user->getPassword(), user->getPhone(), "Bike", 4.5);
            newRider.setStatus("Active");
            dbManager.getDatabase().saveRider(newRider);
            dbManager.getRidersHashTable().insertItem(riderId, newRider);
            
            // Initialize rider stats
            RiderStats stats;
            stats.riderId = riderId;
            stats.lastActiveTime = time(nullptr);
            riderStatistics[riderId] = stats;
            riderStatus[riderId] = "Active";
        } else if (oldRole == "rider" && newRole != "rider") {
            // Remove rider entry
            dbManager.getDatabase().deleteRider(userId);
            dbManager.getRidersHashTable().removeItem(userId);
            
            // Remove from stats tracking
            riderStatus.erase(userId);
            riderStatistics.erase(userId);
        }
        
        cout << "✓ User role changed: ID " << userId 
             << " from " << oldRole << " to " << newRole << "\n";
        return "SUCCESS";
    } catch (const exception& e) {
        return "ERROR:" + string(e.what());
    }
}
};

#endif // SERVER_H