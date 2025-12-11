#pragma once
#ifndef USER_H
#define USER_H

#include <iostream>
#include <string>
#include <functional>
#include <algorithm>
#include <cctype>
#include <cstring>
#include "../dataStructures/HashTable.h"
#include "../dataStructures/LinkedList.h"

using namespace std;

// POD-compatible struct for binary file I/O
struct UserData {
    int id;
    char name[100];
    char email[100];
    char phone[20];
    char role[30];      // "customer", "rider", "restaurant_owner", "admin"
    char address[200];
    char password[50];  // For authentication (in real system, would be hashed)
    
    // Default constructor
    UserData() : id(0) {
        memset(name, 0, sizeof(name));
        memset(email, 0, sizeof(email));
        memset(phone, 0, sizeof(phone));
        memset(role, 0, sizeof(role));
        memset(address, 0, sizeof(address));
        memset(password, 0, sizeof(password));
        strcpy(role, "customer");
    }
    
    // Constructor with parameters
    UserData(int i, const string& n, const string& e, const string& p, 
             const string& pass, const string& r = "customer", const string& a = "")
        : id(i) {
        
        strncpy(name, n.c_str(), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        
        strncpy(email, e.c_str(), sizeof(email) - 1);
        email[sizeof(email) - 1] = '\0';
        
        strncpy(phone, p.c_str(), sizeof(phone) - 1);
        phone[sizeof(phone) - 1] = '\0';
        
        strncpy(password, pass.c_str(), sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0';
        
        strncpy(role, r.c_str(), sizeof(role) - 1);
        role[sizeof(role) - 1] = '\0';
        
        strncpy(address, a.c_str(), sizeof(address) - 1);
        address[sizeof(address) - 1] = '\0';
    }
    
    // Helper methods to get string versions
    string getName() const { return string(name); }
    string getEmail() const { return string(email); }
    string getPhone() const { return string(phone); }
    string getPassword() const { return string(password); }
    string getRole() const { return string(role); }
    string getAddress() const { return string(address); }
    int getUserId() const { return id; }
    
    bool operator==(const UserData& other) const {
        return id == other.id;
    }
    
    bool operator<(const UserData& other) const {
        return id < other.id;
    }
    
    bool operator>(const UserData& other) const {
        return id > other.id;
    }
    
    friend ostream& operator<<(ostream& os, const UserData& u) {
        os << "ID: " << u.id << " | Name: " << u.name 
           << " | Role: " << u.role 
           << " | Email: " << u.email;
        return os;
    }
    
    void printDetails() const {
        cout << "=== User Details ===\n";
        cout << "ID:       " << id << "\n";
        cout << "Name:     " << name << "\n";
        cout << "Email:    " << email << "\n";
        cout << "Phone:    " << phone << "\n";
        cout << "Role:     " << role << "\n";
        cout << "Address:  " << address << "\n";
    }
    
    bool isCustomer() const {
        return strcmp(role, "customer") == 0;
    }
    
    bool isRider() const {
        return strcmp(role, "rider") == 0;
    }
    
    bool isRestaurantOwner() const {
        return strcmp(role, "restaurant_owner") == 0;
    }
    
    bool isAdmin() const {
        return strcmp(role, "admin") == 0;
    }
    
    bool authenticate(const string& pass) const {
        return strcmp(password, pass.c_str()) == 0;
    }
    
    void changePassword(const string& newPass) {
        strncpy(password, newPass.c_str(), sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0';
    }
    
    void displayUserInfo() const {
        cout << "Name: " << name << "\n";
        cout << "Email: " << email << "\n";
        cout << "Address: " << address << "\n";
        cout << "Phone: " << phone << "\n";
    }
    
    void updateProfile(const string& newName = "", const string& newEmail = "",
                      const string& newPhone = "", const string& newAddress = "") {
        if (!newName.empty()) {
            strncpy(name, newName.c_str(), sizeof(name) - 1);
            name[sizeof(name) - 1] = '\0';
        }
        if (!newEmail.empty()) {
            strncpy(email, newEmail.c_str(), sizeof(email) - 1);
            email[sizeof(email) - 1] = '\0';
        }
        if (!newPhone.empty()) {
            strncpy(phone, newPhone.c_str(), sizeof(phone) - 1);
            phone[sizeof(phone) - 1] = '\0';
        }
        if (!newAddress.empty()) {
            strncpy(address, newAddress.c_str(), sizeof(address) - 1);
            address[sizeof(address) - 1] = '\0';
        }
    }
};

class UserManager {
private:
    HashTable<UserData> users; // key = user ID
    
public:
    UserManager() {}
    
    ~UserManager() {
        // HashTable handles its own cleanup
    }
    
    // Register a new user
    bool registerUser(int id, const string& name, const string& email, 
                     const string& phone, const string& password,
                     const string& role = "customer", const string& address = "") {
        if (users.searchTable(id) != nullptr) {
            cout << "User with ID " << id << " already exists.\n";
            return false;
        }
        
        UserData u(id, name, email, phone, password, role, address);
        users.insertItem(id, u);
        cout << "User registered successfully. ID: " << id << "\n";
        return true;
    }
    
    // Add user without password (for backward compatibility)
    bool addUser(int id, const string& name, const string& email, 
                const string& phone, const string& role = "customer", 
                const string& address = "") {
        return registerUser(id, name, email, phone, "", role, address);
    }
    
    // Remove user by ID
    bool removeUser(int id) {
        if (users.searchTable(id) == nullptr) {
            cout << "User with ID " << id << " not found.\n";
            return false;
        }
        
        users.removeItem(id);
        cout << "User " << id << " removed successfully.\n";
        return true;
    }
    
    // Get user by ID
    UserData* getUser(int id) {
        return users.searchTable(id);
    }
    
    // Get user by email (linear search)
    UserData* getUserByEmail(const string& email) {
        UserData* found = nullptr;
        users.traverse([&](int id, UserData& u) {
            if (strcmp(u.email, email.c_str()) == 0 && found == nullptr) {
                found = &u;
            }
        });
        return found;
    }
    
    // Const version for const methods
    const UserData* getUserByEmail(const string& email) const {
        const UserData* found = nullptr;
        users.traverse([&](int id, const UserData& u) {
            if (strcmp(u.email, email.c_str()) == 0 && found == nullptr) {
                found = &u;
            }
        });
        return found;
    }
    
    // Authenticate user
    UserData* authenticateUser(const string& email, const string& password) {
        UserData* user = getUserByEmail(email);
        if (user && user->authenticate(password)) {
            return user;
        }
        return nullptr;
    }
    
    // Update user name
    bool updateUserName(int id, const string& newName) {
        UserData* u = users.searchTable(id);
        if (!u) {
            cout << "User not found.\n";
            return false;
        }
        strncpy(u->name, newName.c_str(), sizeof(u->name) - 1);
        u->name[sizeof(u->name) - 1] = '\0';
        cout << "User name updated.\n";
        return true;
    }
    
    // Update user email
    bool updateUserEmail(int id, const string& newEmail) {
        UserData* u = users.searchTable(id);
        if (!u) {
            cout << "User not found.\n";
            return false;
        }
        
        // Check if email already exists
        UserData* existing = getUserByEmail(newEmail);
        if (existing && existing->id != id) {
            cout << "Email already in use.\n";
            return false;
        }
        
        strncpy(u->email, newEmail.c_str(), sizeof(u->email) - 1);
        u->email[sizeof(u->email) - 1] = '\0';
        cout << "User email updated.\n";
        return true;
    }
    
    // Update user phone
    bool updateUserPhone(int id, const string& newPhone) {
        UserData* u = users.searchTable(id);
        if (!u) {
            cout << "User not found.\n";
            return false;
        }
        strncpy(u->phone, newPhone.c_str(), sizeof(u->phone) - 1);
        u->phone[sizeof(u->phone) - 1] = '\0';
        cout << "User phone updated.\n";
        return true;
    }
    
    // Update user address
    bool updateUserAddress(int id, const string& newAddress) {
        UserData* u = users.searchTable(id);
        if (!u) {
            cout << "User not found.\n";
            return false;
        }
        strncpy(u->address, newAddress.c_str(), sizeof(u->address) - 1);
        u->address[sizeof(u->address) - 1] = '\0';
        cout << "User address updated.\n";
        return true;
    }
    
    // Update user role
    bool updateUserRole(int id, const string& newRole) {
        UserData* u = users.searchTable(id);
        if (!u) {
            cout << "User not found.\n";
            return false;
        }
        
        // Validate role
        if (newRole != "customer" && newRole != "rider" && 
            newRole != "restaurant_owner" && newRole != "admin") {
            cout << "Invalid role. Must be: customer, rider, restaurant_owner, or admin.\n";
            return false;
        }
        
        strncpy(u->role, newRole.c_str(), sizeof(u->role) - 1);
        u->role[sizeof(u->role) - 1] = '\0';
        cout << "User role updated to " << newRole << ".\n";
        return true;
    }
    
    // Update user password
    bool updateUserPassword(int id, const string& oldPassword, const string& newPassword) {
        UserData* u = users.searchTable(id);
        if (!u) {
            cout << "User not found.\n";
            return false;
        }
        
        if (!u->authenticate(oldPassword)) {
            cout << "Incorrect old password.\n";
            return false;
        }
        
        u->changePassword(newPassword);
        cout << "Password updated successfully.\n";
        return true;
    }
    
    // Update multiple fields at once
    bool updateUser(int id, const UserData& updates) {
        UserData* u = users.searchTable(id);
        if (!u) {
            cout << "User not found.\n";
            return false;
        }
        
        if (strlen(updates.name) > 0) {
            strncpy(u->name, updates.name, sizeof(u->name) - 1);
            u->name[sizeof(u->name) - 1] = '\0';
        }
        if (strlen(updates.email) > 0) {
            // Check if new email is already in use
            UserData* existing = getUserByEmail(updates.email);
            if (existing && existing->id != id) {
                cout << "Email already in use.\n";
                return false;
            }
            strncpy(u->email, updates.email, sizeof(u->email) - 1);
            u->email[sizeof(u->email) - 1] = '\0';
        }
        if (strlen(updates.phone) > 0) {
            strncpy(u->phone, updates.phone, sizeof(u->phone) - 1);
            u->phone[sizeof(u->phone) - 1] = '\0';
        }
        if (strlen(updates.address) > 0) {
            strncpy(u->address, updates.address, sizeof(u->address) - 1);
            u->address[sizeof(u->address) - 1] = '\0';
        }
        if (strlen(updates.role) > 0) {
            strncpy(u->role, updates.role, sizeof(u->role) - 1);
            u->role[sizeof(u->role) - 1] = '\0';
        }
        if (strlen(updates.password) > 0) {
            strncpy(u->password, updates.password, sizeof(u->password) - 1);
            u->password[sizeof(u->password) - 1] = '\0';
        }
        
        cout << "User profile updated.\n";
        return true;
    }
    
    // Print all users (const version)
    void printAllUsers() const {
        cout << "=== All Users ===\n";
        if (users.isEmpty()) {
            cout << "No users registered.\n";
            return;
        }
        
        users.traverse([](int id, const UserData& u){
            cout << "ID: " << id 
                 << " | Name: " << u.name 
                 << " | Role: " << u.role 
                 << " | Email: " << u.email 
                 << " | Phone: " << u.phone 
                 << " | Address: " << u.address << endl;
        });
        cout << "Total users: " << getTotalUsers() << endl;
    }
    
    // Print users by role (const version)
    void printUsersByRole(const string& role) const {
        cout << "=== Users with Role: " << role << " ===\n";
        int count = 0;
        
        users.traverse([&](int id, const UserData& u) {
            if (strcmp(u.role, role.c_str()) == 0) {
                cout << "ID: " << id << " | Name: " << u.name 
                     << " | Email: " << u.email << endl;
                count++;
            }
        });
        
        if (count == 0) {
            cout << "No users found with role " << role << ".\n";
        } else {
            cout << "Total: " << count << " users.\n";
        }
    }
    
    // Get users by role (const version)
    LinkedList<UserData> getUsersByRole(const string& role) const {
        LinkedList<UserData> result;
        users.traverse([&](int id, const UserData& u) {
            if (strcmp(u.role, role.c_str()) == 0) {
                result.insertAtEnd(u);
            }
        });
        return result;
    }
    
    // Get all customers (const version)
    LinkedList<UserData> getAllCustomers() const {
        return getUsersByRole("customer");
    }
    
    // Get all riders (const version)
    LinkedList<UserData> getAllRiders() const {
        return getUsersByRole("rider");
    }
    
    // Get all restaurant owners (const version)
    LinkedList<UserData> getAllRestaurantOwners() const {
        return getUsersByRole("restaurant_owner");
    }
    
    // Get all admins (const version)
    LinkedList<UserData> getAllAdmins() const {
        return getUsersByRole("admin");
    }
    
    // Search users by name (partial match) - const version
    LinkedList<UserData> searchUsersByName(const string& searchTerm) const {
        LinkedList<UserData> result;
        string searchName = searchTerm;
        transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);
        
        users.traverse([&](int id, const UserData& u) {
            string userName = string(u.name);
            transform(userName.begin(), userName.end(), userName.begin(), ::tolower);
            
            if (userName.find(searchName) != string::npos) {
                result.insertAtEnd(u);
            }
        });
        return result;
    }
    
    // Check if user exists
    bool userExists(int id) const {
        return users.searchTable(id) != nullptr;
    }
    
    // Check if email exists (const version)
    bool emailExists(const string& email) const {
        return getUserByEmail(email) != nullptr;
    }
    
    // Get total number of users
    int getTotalUsers() const {
        int count = 0;
        users.traverse([&](int id, const UserData& u) {
            count++;
        });
        return count;
    }
    
    // Get user statistics (const version)
    void printUserStatistics() const {
        int total = 0;
        int customers = 0;
        int riders = 0;
        int owners = 0;
        int admins = 0;
        
        users.traverse([&](int id, const UserData& u) {
            total++;
            if (strcmp(u.role, "customer") == 0) customers++;
            else if (strcmp(u.role, "rider") == 0) riders++;
            else if (strcmp(u.role, "restaurant_owner") == 0) owners++;
            else if (strcmp(u.role, "admin") == 0) admins++;
        });
        
        cout << "=== User Statistics ===\n";
        cout << "Total Users:       " << total << "\n";
        cout << "Customers:         " << customers << "\n";
        cout << "Riders:            " << riders << "\n";
        cout << "Restaurant Owners: " << owners << "\n";
        cout << "Admins:            " << admins << "\n";
    }
    
    // Validate user data
    bool validateUserData(const UserData& user) const {
        if (user.id <= 0) {
            cout << "Invalid user ID.\n";
            return false;
        }
        
        if (strlen(user.name) == 0) {
            cout << "Name cannot be empty.\n";
            return false;
        }
        
        if (strlen(user.email) == 0) {
            cout << "Email cannot be empty.\n";
            return false;
        }
        
        // Simple email validation
        if (strchr(user.email, '@') == nullptr) {
            cout << "Invalid email format.\n";
            return false;
        }
        
        if (strlen(user.phone) == 0) {
            cout << "Phone cannot be empty.\n";
            return false;
        }
        
        // Validate role
        string roleStr = string(user.role);
        if (roleStr != "customer" && roleStr != "rider" && 
            roleStr != "restaurant_owner" && roleStr != "admin") {
            cout << "Invalid role.\n";
            return false;
        }
        
        return true;
    }
    // Add this to UserManager class
vector<UserData> getAllUsersAsVector() const {
    vector<UserData> allUsers;
    users.traverse([&](int id, const UserData& u) {
        allUsers.push_back(u);
    });
    return allUsers;
}
};

#endif // USER_H