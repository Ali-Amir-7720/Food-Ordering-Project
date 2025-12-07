#pragma once
#ifndef USER_H
#define USER_H

#include <iostream>
#include <string>
#include <functional>
#include <algorithm>
#include <cctype>
#include "../dataStructures/HashTable.h"
#include "../dataStructures/LinkedList.h"

using namespace std;

struct UserData {
    int id;
    string name;
    string email;
    string phone;
    string role; // "customer", "rider", "restaurant_owner", "admin"
    string address;
    string password; // For authentication (in real system, would be hashed)
    
    UserData() : id(0), name(""), email(""), phone(""), 
                 role("customer"), address(""), password("") {}
    
    UserData(int i, const string& n, const string& e, const string& p, 
             const string& r = "customer", const string& a = "", 
             const string& pass = "")
        : id(i), name(n), email(e), phone(p), role(r), address(a), password(pass) {}
    
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
        return role == "customer";
    }
    
    bool isRider() const {
        return role == "rider";
    }
    
    bool isRestaurantOwner() const {
        return role == "restaurant_owner";
    }
    
    bool isAdmin() const {
        return role == "admin";
    }
    
    bool authenticate(const string& pass) const {
        // In a real system, compare hashed passwords
        return password == pass;
    }
    
    void changePassword(const string& newPass) {
        password = newPass;
    }
    int getUserId() const { return id; }
    string getName() const { return name; }
    string getEmail() const { return email; }
    string getPassword() const { return password; }
    string getAddress() const { return address; }
    string getPhone() const { return phone; }
    
    void displayUserInfo() const {
        cout << "Name: " << name << "\n";
        cout << "Email: " << email << "\n";
        cout << "Address: " << address << "\n";
        cout << "Phone: " << phone << "\n";
    }
    
    void updateProfile(const string& newName = "", const string& newEmail = "",
                      const string& newPhone = "", const string& newAddress = "") {
        if (!newName.empty()) name = newName;
        if (!newEmail.empty()) email = newEmail;
        if (!newPhone.empty()) phone = newPhone;
        if (!newAddress.empty()) address = newAddress;
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
        
        UserData u(id, name, email, phone, role, address, password);
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
            if (u.email == email && found == nullptr) {
                found = &u;
            }
        });
        return found;
    }
    
    // Const version for const methods
    const UserData* getUserByEmail(const string& email) const {
        const UserData* found = nullptr;
        users.traverse([&](int id, const UserData& u) {
            if (u.email == email && found == nullptr) {
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
        u->name = newName;
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
        
        u->email = newEmail;
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
        u->phone = newPhone;
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
        u->address = newAddress;
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
        
        u->role = newRole;
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
        
        if (!updates.name.empty()) u->name = updates.name;
        if (!updates.email.empty()) {
            // Check if new email is already in use
            UserData* existing = getUserByEmail(updates.email);
            if (existing && existing->id != id) {
                cout << "Email already in use.\n";
                return false;
            }
            u->email = updates.email;
        }
        if (!updates.phone.empty()) u->phone = updates.phone;
        if (!updates.address.empty()) u->address = updates.address;
        if (!updates.role.empty()) u->role = updates.role;
        if (!updates.password.empty()) u->password = updates.password;
        
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
        
        // Use const reference in lambda
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
        
        // Use const reference in lambda
        users.traverse([&](int id, const UserData& u) {
            if (u.role == role) {
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
        // Use const reference
        users.traverse([&](int id, const UserData& u) {
            if (u.role == role) {
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
    LinkedList<UserData> searchUsersByName(const string& name) const {
        LinkedList<UserData> result;
        string searchName = name;
        // Convert to lowercase for case-insensitive search
        transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);
        
        users.traverse([&](int id, const UserData& u) {
            string userName = u.name;
            transform(userName.begin(), userName.end(), userName.begin(), ::tolower);
            
            if (userName.find(searchName) != string::npos) {
                result.insertAtEnd(u);
            }
        });
        return result;
    }
    
    // Non-const version for internal use
    LinkedList<UserData> searchUsersByName(const string& name) {
        LinkedList<UserData> result;
        string searchName = name;
        transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);
        
        users.traverse([&](int id, UserData& u) {
            string userName = u.name;
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
        // Use const reference
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
            if (u.role == "customer") customers++;
            else if (u.role == "rider") riders++;
            else if (u.role == "restaurant_owner") owners++;
            else if (u.role == "admin") admins++;
        });
        
        cout << "=== User Statistics ===\n";
        cout << "Total Users:      " << total << "\n";
        cout << "Customers:        " << customers << "\n";
        cout << "Riders:           " << riders << "\n";
        cout << "Restaurant Owners:" << owners << "\n";
        cout << "Admins:           " << admins << "\n";
    }
    
    // Export users to file (basic implementation)
    void exportUsers(const string& filename) const {
        // Simple export to text file
        cout << "Exporting users to " << filename << "...\n";
        // Implementation would write to file
    }
    
    // Import users from file (basic implementation)
    void importUsers(const string& filename) {
        // Simple import from text file
        cout << "Importing users from " << filename << "...\n";
        // Implementation would read from file
    }
    
    // Clear all users (for testing)
    void clearAllUsers() {
        // Need to implement clear() in HashTable first
        cout << "Clearing all users...\n";
    }
    
    // Validate user data
    bool validateUserData(const UserData& user) const {
        if (user.id <= 0) {
            cout << "Invalid user ID.\n";
            return false;
        }
        
        if (user.name.empty()) {
            cout << "Name cannot be empty.\n";
            return false;
        }
        
        if (user.email.empty()) {
            cout << "Email cannot be empty.\n";
            return false;
        }
        
        // Simple email validation
        if (user.email.find('@') == string::npos) {
            cout << "Invalid email format.\n";
            return false;
        }
        
        if (user.phone.empty()) {
            cout << "Phone cannot be empty.\n";
            return false;
        }
        
        // Validate role
        if (user.role != "customer" && user.role != "rider" && 
            user.role != "restaurant_owner" && user.role != "admin") {
            cout << "Invalid role.\n";
            return false;
        }
        
        return true;
    }
    
};

#endif // USER_H