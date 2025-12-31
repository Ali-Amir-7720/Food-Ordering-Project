// Client.h
#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET SocketType;
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int SocketType;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define CLOSE_SOCKET close
#endif

using namespace std;

struct Message {
    char command[50];
    char data[4096];
    int clientId;
    char clientType[20];
};

class QuickBiteClient {
private:
    SocketType clientSocket;
    string serverIP;
    int serverPort;
    bool connected;
    
    int userId;
    string userName;
    string userRole;

public:
    QuickBiteClient(const string& ip = "127.0.0.1", int port = 8080)
        : serverIP(ip), serverPort(port), connected(false), userId(-1) {
        
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            cerr << "WSAStartup failed\n";
            exit(1);
        }
#endif
    }
    
    ~QuickBiteClient() {
        disconnect();
        
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    bool connectToServer() {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Failed to create socket\n";
            return false;
        }
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);
        
#ifdef _WIN32
        serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());
#else
        inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
#endif
        
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "Connection failed\n";
            CLOSE_SOCKET(clientSocket);
            return false;
        }
        
        connected = true;
        cout << "✓ Connected to server at " << serverIP << ":" << serverPort << "\n";
        
        return true;
    }
    
    void disconnect() {
        if (connected) {
            CLOSE_SOCKET(clientSocket);
            connected = false;
            cout << "Disconnected from server\n";
        }
    }
    
    bool isConnected() const { return connected; }
    
    void logout() {
        userId = -1;
        userName = "";
        userRole = "";
    }
    
    string sendCommand(const string& command, const string& data = "") {
        if (!connected) {
            return "ERROR:Not connected";
        }
        
        Message msg;
        strncpy(msg.command, command.c_str(), sizeof(msg.command) - 1);
        strncpy(msg.data, data.c_str(), sizeof(msg.data) - 1);
        msg.clientId = userId;
        strncpy(msg.clientType, userRole.c_str(), sizeof(msg.clientType) - 1);
        
        // Send message
        if (send(clientSocket, (char*)&msg, sizeof(msg), 0) == SOCKET_ERROR) {
            return "ERROR:Send failed";
        }
        
        // Receive response
        char buffer[8192] = {0};
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived <= 0) {
            connected = false;
            return "ERROR:Connection lost";
        }
        
        return string(buffer);
    }
    
    // User operations
    bool login(const string& email, const string& password) {
        string data = email + "|" + password;
        string response = sendCommand("LOGIN", data);
        
        if (response.substr(0, 7) == "SUCCESS") {
            // Parse: SUCCESS|userId|userName|userRole
            size_t pos1 = response.find('|');
            size_t pos2 = response.find('|', pos1 + 1);
            size_t pos3 = response.find('|', pos2 + 1);
            
            userId = stoi(response.substr(pos1 + 1, pos2 - pos1 - 1));
            userName = response.substr(pos2 + 1, pos3 - pos2 - 1);
            userRole = response.substr(pos3 + 1);
            
            return true;
        }
        
        return false;
    }
    
    bool registerUser(const string& name, const string& email, 
                     const string& password, const string& phone, 
                     const string& address) {
        string data = name + "|" + email + "|" + password + "|" + 
                     phone + "|" + address;
        string response = sendCommand("REGISTER", data);
        
        return response.substr(0, 7) == "SUCCESS";
    }
    
    // Restaurant operations
    vector<string> getRestaurants() {
        string response = sendCommand("GET_RESTAURANTS");
        vector<string> restaurants;
        
        if (response.substr(0, 7) == "SUCCESS") {
            string data = response.substr(8);
            
            size_t start = 0, end;
            while ((end = data.find('|', start)) != string::npos) {
                restaurants.push_back(data.substr(start, end - start));
                start = end + 1;
            }
            if (start < data.length()) {
                restaurants.push_back(data.substr(start));
            }
        }
        
        return restaurants;
    }
    
    vector<string> getMenu(int restaurantId) {
        string response = sendCommand("GET_MENU", to_string(restaurantId));
        vector<string> menuItems;
        
        if (response.substr(0, 7) == "SUCCESS") {
            string data = response.substr(8);
            
            size_t start = 0, end;
            while ((end = data.find('|', start)) != string::npos) {
                menuItems.push_back(data.substr(start, end - start));
                start = end + 1;
            }
            if (start < data.length()) {
                menuItems.push_back(data.substr(start));
            }
        }
        
        return menuItems;
    }
    
    // Order operations
    int placeOrder(int restaurantId, const vector<pair<int, int>>& items) {
        string itemsStr;
        for (size_t i = 0; i < items.size(); i++) {
            itemsStr += to_string(items[i].first) + ":" + 
                       to_string(items[i].second);
            if (i < items.size() - 1) itemsStr += ",";
        }
        
        string data = to_string(userId) + "|" + 
                     to_string(restaurantId) + "|" + itemsStr;
        
        string response = sendCommand("PLACE_ORDER", data);
        
        if (response.substr(0, 7) == "SUCCESS") {
            return stoi(response.substr(8));
        }
        
        return -1;
    }
    
    vector<string> getOrders() {
        string response = sendCommand("GET_ORDERS", to_string(userId));
        vector<string> orders;
        
        if (response.substr(0, 7) == "SUCCESS") {
            string data = response.substr(8);
            
            size_t start = 0, end;
            while ((end = data.find('|', start)) != string::npos) {
                orders.push_back(data.substr(start, end - start));
                start = end + 1;
            }
            if (start < data.length()) {
                orders.push_back(data.substr(start));
            }
        }
        
        return orders;
    }
    
    // Admin/Rider operations
    vector<string> getRiders() {
        string response = sendCommand("GET_RIDERS");
        vector<string> riders;
        
        if (response.substr(0, 7) == "SUCCESS") {
            string data = response.substr(8);
            
            size_t start = 0, end;
            while ((end = data.find('|', start)) != string::npos) {
                riders.push_back(data.substr(start, end - start));
                start = end + 1;
            }
            if (start < data.length()) {
                riders.push_back(data.substr(start));
            }
        }
        
        return riders;
    }
    
    bool assignRider(int orderId, int riderId) {
        string data = to_string(orderId) + "|" + to_string(riderId);
        string response = sendCommand("ASSIGN_RIDER", data);
        
        return response.substr(0, 7) == "SUCCESS";
    }
    
    // Getters
    int getUserId() const { return userId; }
    string getUserName() const { return userName; }
    string getUserRole() const { return userRole; }
};

#endif // CLIENT_H