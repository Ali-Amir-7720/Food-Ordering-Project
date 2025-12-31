#include "Server.h"
#include "database_manager.h"
#include <iostream>
#include <string>
#include <limits>

using namespace std;

void showServerMenu() {
    cout << "\n========================================\n";
    cout << "       QUICKBITE SERVER CONTROL        \n";
    cout << "========================================\n";
    cout << "1. View Statistics\n";
    cout << "2. Save Data\n";
    cout << "3. Backup Database\n";
    cout << "4. Stop Server\n";
    cout << "========================================\n";
}

int main() {
    cout << "╔═══════════════════════════════════════╗\n";
    cout << "║     QUICKBITE DELIVERY SYSTEM         ║\n";
    cout << "║           SERVER APPLICATION          ║\n";
    cout << "╚═══════════════════════════════════════╝\n\n";
    
    int port;
    cout << "Enter server port (default 8080): ";
    string portStr;
    getline(cin, portStr);
    
    if (portStr.empty()) {
        port = 8080;
    } else {
        port = stoi(portStr);
    }
    
    cout << "\n========================================\n";
    cout << "   INITIALIZING SERVER...              \n";
    cout << "========================================\n";
    
    QuickBiteServer server(port);
    
    cout << "\n========================================\n";
    cout << "   STARTING SERVER...                  \n";
    cout << "========================================\n";
    
    if (!server.start()) {
        cerr << "\n✗ Failed to start server\n";
        return 1;
    }
    
    // Server control loop
    while (true) {
        showServerMenu();
        
        int choice;
        cout << "\nEnter choice (1-4): ";
        
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input!\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        switch (choice) {
            case 1: {
                cout << "\n========================================\n";
                cout << "         SERVER STATISTICS             \n";
                cout << "========================================\n";
                cout << "Server Status: Running\n";
                cout << "Port: " << port << "\n";
                cout << "========================================\n";
                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
                
            case 2:
                cout << "\nSaving data...\n";
                server.saveSystemData();
                cout << "✓ Data saved successfully!\n";
                cout << "\nPress Enter to continue...";
                cin.get();
                break;
                
            case 3: {
                cout << "\nCreating backup...\n";
                DatabaseManager tempDB;
                tempDB.backupDatabase();
                cout << "✓ Backup created successfully!\n";
                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
                
            case 4:
                cout << "\nStopping server...\n";
                server.stop();
                cout << "✓ Server stopped. Goodbye!\n";
                return 0;
                
            default:
                cout << "Invalid choice!\n";
        }
    }
    
    return 0;
}