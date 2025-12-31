#ifndef DATABASE_REPAIR_H
#define DATABASE_REPAIR_H

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include "models/Restaurant.h"
#include "models/Order.h"

using namespace std;

class DatabaseRepair {
public:
    // Diagnose file corruption
    static void diagnoseDatabase() {
        cout << "\n========================================\n";
        cout << "      DATABASE DIAGNOSTIC TOOL         \n";
        cout << "========================================\n\n";
        
        // Check Restaurant file
        cout << "Analyzing restaurants.dat...\n";
        cout << "  Expected Record Size: " << sizeof(Restaurant) << " bytes\n";
        
        ifstream restFile("restaurants.dat", ios::binary);
        if (restFile) {
            restFile.seekg(0, ios::end);
            size_t fileSize = restFile.tellg();
            cout << "  Actual File Size: " << fileSize << " bytes\n";
            
            if (fileSize % sizeof(Restaurant) != 0) {
                cout << "  ✗ CORRUPTED: Size mismatch!\n";
                cout << "  Records (if aligned): " << (fileSize / sizeof(Restaurant)) << "\n";
                cout << "  Extra bytes: " << (fileSize % sizeof(Restaurant)) << "\n";
            } else {
                cout << "  ✓ File structure OK\n";
                cout << "  Records: " << (fileSize / sizeof(Restaurant)) << "\n";
            }
            restFile.close();
        } else {
            cout << "  ✗ File not found\n";
        }
        
        // Check Order file
        cout << "\nAnalyzing orders.dat...\n";
        cout << "  Expected Record Size: " << sizeof(Order) << " bytes\n";
        
        ifstream orderFile("orders.dat", ios::binary);
        if (orderFile) {
            orderFile.seekg(0, ios::end);
            size_t fileSize = orderFile.tellg();
            cout << "  Actual File Size: " << fileSize << " bytes\n";
            
            if (fileSize % sizeof(Order) != 0) {
                cout << "  ✗ CORRUPTED: Size mismatch!\n";
                cout << "  Records (if aligned): " << (fileSize / sizeof(Order)) << "\n";
                cout << "  Extra bytes: " << (fileSize % sizeof(Order)) << "\n";
            } else {
                cout << "  ✓ File structure OK\n";
                cout << "  Records: " << (fileSize / sizeof(Order)) << "\n";
            }
            orderFile.close();
        } else {
            cout << "  ✗ File not found\n";
        }
        
        cout << "\n========================================\n";
    }
    
    // Clear corrupted files
    static void clearCorruptedFiles() {
        cout << "\n========================================\n";
        cout << "      CLEARING CORRUPTED FILES         \n";
        cout << "========================================\n\n";
        
        // Clear restaurants.dat
        ofstream restFile("restaurants.dat", ios::binary | ios::trunc);
        if (restFile) {
            restFile.close();
            cout << "✓ Cleared restaurants.dat\n";
        }
        
        // Clear orders.dat
        ofstream orderFile("orders.dat", ios::binary | ios::trunc);
        if (orderFile) {
            orderFile.close();
            cout << "✓ Cleared orders.dat\n";
        }
        
        cout << "\n✓ Corrupted files cleared!\n";
        cout << "You can now initialize the database again.\n";
        cout << "========================================\n";
    }
    
    // Attempt to salvage readable records
    static vector<Restaurant> salvageRestaurants() {
        vector<Restaurant> salvaged;
        
        ifstream file("restaurants.dat", ios::binary);
        if (!file) {
            return salvaged;
        }
        
        file.seekg(0, ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, ios::beg);
        
        cout << "Attempting to salvage restaurants...\n";
        cout << "File size: " << fileSize << " bytes\n";
        cout << "Record size: " << sizeof(Restaurant) << " bytes\n";
        
        // Try reading records one by one
        int recordsRead = 0;
        while (file.tellg() < fileSize) {
            Restaurant temp;
            streampos beforeRead = file.tellg();
            
            file.read(reinterpret_cast<char*>(&temp), sizeof(Restaurant));
            
            if (file.fail()) {
                cout << "Failed at position: " << beforeRead << "\n";
                break;
            }
            
            // Validate the record
            if (temp.getRestaurantId() > 0 && temp.getRestaurantId() < 10000) {
                salvaged.push_back(temp);
                cout << "  Salvaged: " << temp.getName() << " (ID: " 
                     << temp.getRestaurantId() << ")\n";
                recordsRead++;
            } else {
                cout << "  Invalid record at position " << beforeRead << "\n";
            }
        }
        
        file.close();
        
        cout << "Salvaged " << salvaged.size() << " restaurants\n";
        return salvaged;
    }
    
    static vector<Order> salvageOrders() {
        vector<Order> salvaged;
        
        ifstream file("orders.dat", ios::binary);
        if (!file) {
            return salvaged;
        }
        
        file.seekg(0, ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, ios::beg);
        
        cout << "Attempting to salvage orders...\n";
        cout << "File size: " << fileSize << " bytes\n";
        cout << "Record size: " << sizeof(Order) << " bytes\n";
        
        int recordsRead = 0;
        while (file.tellg() < fileSize) {
            Order temp;
            streampos beforeRead = file.tellg();
            
            file.read(reinterpret_cast<char*>(&temp), sizeof(Order));
            
            if (file.fail()) {
                cout << "Failed at position: " << beforeRead << "\n";
                break;
            }
            
            // Validate the record
            if (temp.getOrderId() > 0 && temp.getOrderId() < 100000) {
                salvaged.push_back(temp);
                cout << "  Salvaged: Order #" << temp.getOrderId() 
                     << " (Customer: " << temp.getCustomerId() << ")\n";
                recordsRead++;
            } else {
                cout << "  Invalid record at position " << beforeRead << "\n";
            }
        }
        
        file.close();
        
        cout << "Salvaged " << salvaged.size() << " orders\n";
        return salvaged;
    }
    
    // Rebuild database from salvaged data
    static void rebuildDatabase() {
        cout << "\n========================================\n";
        cout << "      REBUILDING DATABASE              \n";
        cout << "========================================\n\n";
        
        // Salvage what we can
        vector<Restaurant> restaurants = salvageRestaurants();
        vector<Order> orders = salvageOrders();
        
        // Clear files
        clearCorruptedFiles();
        
        // Rewrite salvaged data
        if (!restaurants.empty()) {
            ofstream restFile("restaurants.dat", ios::binary);
            for (const auto& r : restaurants) {
                restFile.write(reinterpret_cast<const char*>(&r), sizeof(Restaurant));
            }
            restFile.close();
            cout << "\n✓ Restored " << restaurants.size() << " restaurants\n";
        }
        
        if (!orders.empty()) {
            ofstream orderFile("orders.dat", ios::binary);
            for (const auto& o : orders) {
                orderFile.write(reinterpret_cast<const char*>(&o), sizeof(Order));
            }
            orderFile.close();
            cout << "✓ Restored " << orders.size() << " orders\n";
        }
        
        cout << "\n========================================\n";
        cout << "      DATABASE REBUILD COMPLETE        \n";
        cout << "========================================\n";
    }
};

#endif // DATABASE_REPAIR_H