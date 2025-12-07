// // // server.cpp - Backend\core
// // #include <iostream>
// // #include <thread>
// // #include <chrono>
// // #include <vector>
// // #include <string>
// // #include <winsock2.h>
// // #include <ws2tcpip.h>
// // #include "RequestHandler.h"
// // #include "../storage/SystemState.h"

// // #pragma comment(lib,"ws2_32.lib")

// // using namespace std;

// // class Server {
// // private:
// //     int port;
// //     SOCKET serverSocket;
// //     SystemState* systemState;
// //     RequestHandler* requestHandler;
// //     bool running;

// // public:
// //     Server(int p) : port(p), serverSocket(INVALID_SOCKET), running(false) {
// //         systemState = new SystemState();
// //         requestHandler = new RequestHandler(systemState);
// //     }
    
// //     ~Server() {
// //         stop();
// //         delete requestHandler;
// //         delete systemState;
// //     }

// //     void start() {
// //         WSADATA wsaData;
// //         if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
// //             cerr << "WSAStartup failed.\n";
// //             return;
// //         }

// //         serverSocket = socket(AF_INET, SOCK_STREAM, 0);
// //         if (serverSocket == INVALID_SOCKET) {
// //             cerr << "Socket creation failed.\n";
// //             WSACleanup();
// //             return;
// //         }

// //         sockaddr_in serverAddr;
// //         serverAddr.sin_family = AF_INET;
// //         serverAddr.sin_addr.s_addr = INADDR_ANY;
// //         serverAddr.sin_port = htons(port);

// //         if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
// //             cerr << "Bind failed.\n";
// //             closesocket(serverSocket);
// //             WSACleanup();
// //             return;
// //         }

// //         if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
// //             cerr << "Listen failed.\n";
// //             closesocket(serverSocket);
// //             WSACleanup();
// //             return;
// //         }

// //         cout << "QuickBite Express Server started on port " << port << endl;
// //         cout << "Server is running..." << endl;
        
// //         running = true;
        
// //         // Start order processing thread
// //         std::thread processingThread(&Server::processOrders, this);
        
// //         // Main accept loop
// //         while (running) {
// //             sockaddr_in clientAddr;
// //             int addrSize = sizeof(clientAddr);
// //             SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrSize);
            
// //             if (clientSocket == INVALID_SOCKET) {
// //                 if (running) {
// //                     cerr << "Accept failed.\n";
// //                 }
// //                 continue;
// //             }
            
// //             char clientIP[INET_ADDRSTRLEN];
// //             inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
// //             cout << "Client connected from " << clientIP << ":" << ntohs(clientAddr.sin_port) << endl;
            
// //             // Handle client in separate thread
// //             std::thread clientThread(&Server::handleClient, this, clientSocket);
// //             clientThread.detach();
// //         }
        
// //         processingThread.join();
// //         WSACleanup();
// //     }
    
// //     void stop() {
// //         running = false;
// //         if (serverSocket != INVALID_SOCKET) {
// //             closesocket(serverSocket);
// //             serverSocket = INVALID_SOCKET;
// //         }
// //     }

// // private:
// //     void handleClient(SOCKET clientSocket) {
// //         char buffer[4096];
// //         int bytesRead;
        
// //         while (running && (bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
// //             buffer[bytesRead] = '\0';
// //             string request(buffer);
            
// //             cout << "Received request: " << request << endl;
            
// //             string response = requestHandler->handleRequest(request);
            
// //             send(clientSocket, response.c_str(), response.size(), 0);
// //             cout << "Sent response: " << response << endl;
// //         }
        
// //         closesocket(clientSocket);
// //         cout << "Client disconnected." << endl;
// //     }
    
// //     void processOrders() {
// //         while (running) {
// //             std::this_thread::sleep_for(std::chrono::seconds(5)); // Process every 5 seconds
            
// //             // In a real implementation, you would:
// //             // 1. Process pending orders
// //             // 2. Assign riders to ready orders
// //             // 3. Update order statuses
// //             // 4. Check for delivered orders
            
// //             // For now, just print status
// //             static int counter = 0;
// //             if (counter++ % 12 == 0) { // Every minute
// //                 cout << "\n=== System Status ===" << endl;
// //                 cout << "Active orders: " << systemState->getActiveOrders() << endl;
// //                 cout << "Available riders: " << systemState->getAvailableRiders() << endl;
// //                 cout << "====================\n" << endl;
// //             }
// //         }
// //     }
// // };

// // // Simple main to run server
// // int main() {
// //     cout << "=== QuickBite Express Backend Server ===" << endl;
// //     cout << "Starting server on port 8080..." << endl;
    
// //     Server server(8080);
// //     server.start();
    
// //     return 0;
// // }
// // server.cpp - Backend\core
// #include <iostream>
// #include <vector>
// #include <string>
// #include <winsock2.h>
// #include <windows.h>  // For threading and timing functions

// // Remove thread includes since we'll use Windows API threads
// // #include <thread>
// // #include <chrono>

// #include "RequestHandler.h"
// #include "../storage/SystemState.h"

// #pragma comment(lib,"ws2_32.lib")
// #pragma comment(lib,"user32.lib")

// using namespace std;

// // Thread function signature for Windows
// DWORD WINAPI ClientThread(LPVOID lpParam);
// DWORD WINAPI ProcessOrdersThread(LPVOID lpParam);

// // Structure to pass data to threads
// struct ThreadData {
//     SOCKET clientSocket;
//     RequestHandler* requestHandler;
// };

// class Server {
// private:
//     int port;
//     SOCKET serverSocket;
//     SystemState* systemState;
//     RequestHandler* requestHandler;
//     bool running;
//     HANDLE hProcessThread;  // Windows thread handle

// public:
//     Server(int p) : port(p), serverSocket(INVALID_SOCKET), running(false), 
//                     hProcessThread(NULL) {
//         systemState = new SystemState();
//         requestHandler = new RequestHandler(systemState);
//     }
    
//     ~Server() {
//         stop();
//         if (requestHandler) delete requestHandler;
//         if (systemState) delete systemState;
//     }

//     void start() {
//         WSADATA wsaData;
//         if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
//             cerr << "WSAStartup failed.\n";
//             return;
//         }

//         serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//         if (serverSocket == INVALID_SOCKET) {
//             cerr << "Socket creation failed.\n";
//             WSACleanup();
//             return;
//         }

//         sockaddr_in serverAddr;
//         serverAddr.sin_family = AF_INET;
//         serverAddr.sin_addr.s_addr = INADDR_ANY;
//         serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//         serverAddr.sin_port = htons(port);

//         if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
//             cerr << "Bind failed.\n";
//             closesocket(serverSocket);
//             WSACleanup();
//             return;
//         }

//         if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
//             cerr << "Listen failed.\n";
//             closesocket(serverSocket);
//             WSACleanup();
//             return;
//         }

//         cout << "QuickBite Express Server started on port " << port << endl;
//         cout << "Server is running..." << endl;
        
//         running = true;
        
//         // Start order processing thread using Windows API
//         DWORD threadId;
//         hProcessThread = CreateThread(
//             NULL,                   // Default security attributes
//             0,                      // Default stack size
//             ProcessOrdersThread,    // Thread function
//             (LPVOID)this,          // Parameter to thread function
//             0,                      // Default creation flags
//             &threadId              // Returns thread identifier
//         );
        
//         if (hProcessThread == NULL) {
//             cerr << "Failed to create processing thread.\n";
//         }
        
//         // Main accept loop
//         while (running) {
//             sockaddr_in clientAddr;
//             int addrSize = sizeof(clientAddr);
//             SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrSize);
            
//             if (clientSocket == INVALID_SOCKET) {
//                 if (running) {
//                     cerr << "Accept failed.\n";
//                 }
//                 continue;
//             }
            
//             // Get client IP using inet_ntoa (older but widely supported)
//             char* clientIP = inet_ntoa(clientAddr.sin_addr);
//             cout << "Client connected from " << (clientIP ? clientIP : "Unknown") 
//                  << ":" << ntohs(clientAddr.sin_port) << endl;
            
//             // Create thread data
//             ThreadData* data = new ThreadData;
//             data->clientSocket = clientSocket;
//             data->requestHandler = requestHandler;
            
//             // Create a thread for this client using Windows API
//             HANDLE hThread = CreateThread(
//                 NULL,                // Default security attributes
//                 0,                   // Default stack size
//                 ClientThread,        // Thread function
//                 (LPVOID)data,       // Parameter to thread function
//                 0,                   // Default creation flags
//                 &threadId           // Returns thread identifier
//             );
            
//             if (hThread == NULL) {
//                 cerr << "Failed to create client thread.\n";
//                 closesocket(clientSocket);
//                 delete data;
//             } else {
//                 CloseHandle(hThread); // We don't need to keep the handle
//             }
//         }
        
//         // Wait for processing thread to finish
//         if (hProcessThread) {
//             WaitForSingleObject(hProcessThread, INFINITE);
//             CloseHandle(hProcessThread);
//         }
        
//         WSACleanup();
//     }
    
//     void stop() {
//         running = false;
//         if (serverSocket != INVALID_SOCKET) {
//             closesocket(serverSocket);
//             serverSocket = INVALID_SOCKET;
//         }
        
//         // Signal processing thread to exit
//         if (hProcessThread) {
//             WaitForSingleObject(hProcessThread, 1000); // Wait 1 second
//         }
//     }

//     // Make these public so thread functions can access them
//     SystemState* getSystemState() { return systemState; }
//     bool isRunning() { return running; }

// private:
//     // Made private to prevent direct calling
//     void handleClient(SOCKET clientSocket) {
//         char buffer[4096];
//         int bytesRead;
        
//         while (running && (bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
//             buffer[bytesRead] = '\0';
//             string request(buffer);
            
//             cout << "Received request: " << request << endl;
            
//             string response = requestHandler->handleRequest(request);
            
//             send(clientSocket, response.c_str(), response.size(), 0);
//             cout << "Sent response: " << response << endl;
//         }
        
//         closesocket(clientSocket);
//         cout << "Client disconnected." << endl;
//     }
    
//     void processOrders() {
//         while (running) {
//             Sleep(5000); // Sleep for 5 seconds (Windows API)
            
//             // In a real implementation, you would:
//             // 1. Process pending orders
//             // 2. Assign riders to ready orders
//             // 3. Update order statuses
//             // 4. Check for delivered orders
            
//             // For now, just print status
//             static int counter = 0;
//             if (counter++ % 12 == 0) { // Every minute
//                 cout << "\n=== System Status ===" << endl;
//                 cout << "Active orders: " << systemState->getActiveOrders() << endl;
//                 cout << "Available riders: " << systemState->getAvailableRiders() << endl;
//                 cout << "====================\n" << endl;
//             }
//         }
//     }
// };

// // Windows thread function for handling clients
// DWORD WINAPI ClientThread(LPVOID lpParam) {
//     ThreadData* data = (ThreadData*)lpParam;
    
//     char buffer[4096];
//     int bytesRead;
    
//     while (data->requestHandler && 
//            (bytesRead = recv(data->clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
//         buffer[bytesRead] = '\0';
//         string request(buffer);
        
//         cout << "Received request: " << request << endl;
        
//         string response = data->requestHandler->handleRequest(request);
        
//         send(data->clientSocket, response.c_str(), response.size(), 0);
//         cout << "Sent response: " << response << endl;
//     }
    
//     closesocket(data->clientSocket);
//     cout << "Client disconnected." << endl;
    
//     delete data;
//     return 0;
// }

// // Windows thread function for processing orders
// DWORD WINAPI ProcessOrdersThread(LPVOID lpParam) {
//     Server* server = (Server*)lpParam;
    
//     while (server->isRunning()) {
//         Sleep(5000); // Sleep for 5 seconds
        
//         // In a real implementation, you would:
//         // 1. Process pending orders
//         // 2. Assign riders to ready orders
//         // 3. Update order statuses
//         // 4. Check for delivered orders
        
//         // For now, just print status
//         static int counter = 0;
//         if (counter++ % 12 == 0) { // Every minute
//             SystemState* state = server->getSystemState();
//             if (state) {
//                 cout << "\n=== System Status ===" << endl;
//                 cout << "Active orders: " << state->getActiveOrders() << endl;
//                 cout << "Available riders: " << state->getAvailableRiders() << endl;
//                 cout << "====================\n" << endl;
//             }
//         }
//     }
    
//     return 0;
// }

// // Simple main to run server
// int main() {
//     cout << "=== QuickBite Express Backend Server ===" << endl;
//     cout << "Starting server on port 8080..." << endl;
    
//     Server server(8080);
//     server.start();
    
//     return 0;
// }