#pragma once
#ifndef USERSERVICE_H
#define USERSERVICE_H

#include <iostream>
#include <string>
#include "../models/User.h"
#include "../dataStructures/BTree.h"

using namespace std;

class UserService {
private:
    PersistentBTree<UserData>* persistentUsers;
    UserManager userManager;

public:
    UserService(PersistentBTree<UserData>* persistentStorage) 
        : persistentUsers(persistentStorage) {
        
        // Load users from persistent storage into cache
        loadUsersFromPersistentStorage();
    }

    // Load users from BTree to UserManager cache
    void loadUsersFromPersistentStorage() {
        if (!persistentUsers || persistentUsers->isEmpty()) {
            cout << "No users in persistent storage.\n";
            return;
        }
        
        cout << "Loading users from persistent storage...\n";
        
        // Get all users from BTree
        vector<UserData> allUsers = persistentUsers->getAllKeys();
        
        for (const auto& user : allUsers) {
            userManager.registerUser(
                user.id,
                user.name,
                user.email,
                user.phone,
                user.password,
                user.role,
                user.address
            );
        }
        
        cout << "Loaded " << allUsers.size() << " users from persistent storage.\n";
    }

    // Register a new user with full details
    bool registerUser(int id, const string& name, const string& email, 
                     const string& phone, const string& password,
                     const string& role = "customer", const string& address = "") {
        
        // Check if user already exists in cache
        if (userManager.userExists(id)) {
            cout << "User with ID " << id << " already exists in cache.\n";
            return false;
        }
        
        // Check if email already exists
        if (userManager.emailExists(email)) {
            cout << "Email " << email << " already registered.\n";
            return false;
        }
        
        // Create user data
        UserData user(id, name, email, phone, role, address, password);
        
        // Validate user data
        if (!userManager.validateUserData(user)) {
            return false;
        }
        
        // Add to persistent storage (BTree)
        if (persistentUsers) {
            // Check if exists in persistent storage
            UserData searchUser;
            searchUser.id = id;
            auto result = persistentUsers->search(searchUser);
            if (result.first) {
                cout << "User already exists in persistent storage.\n";
                return false;
            }
            
            // Add to persistent storage
            try {
                persistentUsers->insert(user);
                cout << "User added to persistent storage.\n";
            } catch (const exception& e) {
                cout << "Failed to add user to persistent storage: " << e.what() << "\n";
                return false;
            }
        }
        
        // Add to cache (UserManager)
        bool cacheSuccess = userManager.registerUser(id, name, email, phone, password, role, address);
        
        if (cacheSuccess) {
            cout << "User registered successfully. ID: " << id << "\n";
            return true;
        }
        
        return false;
    }

    // Add user without password (for backward compatibility)
    bool addUser(int id, const string& name, const string& email, 
                const string& phone, const string& role = "customer", 
                const string& address = "") {
        return registerUser(id, name, email, phone, "", role, address);
    }

    // Remove user
    bool removeUser(int id) {
        // First remove from cache
        bool cacheRemoved = userManager.removeUser(id);
        
        if (!cacheRemoved) {
            cout << "User not found in cache.\n";
            return false;
        }
        
        // Remove from persistent storage
        if (persistentUsers) {
            UserData dummyUser;
            dummyUser.id = id;
            try {
                persistentUsers->remove(dummyUser);
                cout << "User removed from persistent storage.\n";
            } catch (const exception& e) {
                cout << "User not found in persistent storage (but removed from cache): " << e.what() << "\n";
            }
        }
        
        return true;
    }

    // Get user by ID
    UserData* getUser(int id) {
        // First check cache
        UserData* user = userManager.getUser(id);
        if (user) {
            return user;
        }
        
        // If not in cache, check persistent storage
        if (persistentUsers) {
            UserData searchUser;
            searchUser.id = id;
            auto result = persistentUsers->search(searchUser);
            
            if (result.first && !result.second.empty()) {
                // Found in persistent storage, add to cache
                UserData persistentUser = result.second[0]; // Get first match
                userManager.registerUser(
                    persistentUser.id,
                    persistentUser.name,
                    persistentUser.email,
                    persistentUser.phone,
                    persistentUser.password,
                    persistentUser.role,
                    persistentUser.address
                );
                
                // Return from cache
                return userManager.getUser(id);
            }
        }
        
        return nullptr;
    }

    // Get user by email
    UserData* getUserByEmail(const string& email) {
        return userManager.getUserByEmail(email);
    }

    // Authenticate user
    UserData* authenticateUser(const string& email, const string& password) {
        return userManager.authenticateUser(email, password);
    }

    // Update user profile
    bool updateUser(int id, const UserData& updates) {
        // Get current user data
        UserData* currentUser = getUser(id);
        if (!currentUser) {
            cout << "User not found.\n";
            return false;
        }
        
        // Create updated user data
        UserData updatedUser = *currentUser;
        
        // Apply updates
        if (!updates.name.empty()) updatedUser.name = updates.name;
        if (!updates.email.empty()) {
            // Check if new email is already in use by another user
            UserData* existing = getUserByEmail(updates.email);
            if (existing && existing->id != id) {
                cout << "Email already in use by another user.\n";
                return false;
            }
            updatedUser.email = updates.email;
        }
        if (!updates.phone.empty()) updatedUser.phone = updates.phone;
        if (!updates.address.empty()) updatedUser.address = updates.address;
        if (!updates.role.empty()) updatedUser.role = updates.role;
        if (!updates.password.empty()) updatedUser.password = updates.password;
        
        // Validate updated data
        if (!userManager.validateUserData(updatedUser)) {
            return false;
        }
        
        // Update in cache
        userManager.removeUser(id);
        bool cacheSuccess = userManager.registerUser(
            updatedUser.id,
            updatedUser.name,
            updatedUser.email,
            updatedUser.phone,
            updatedUser.password,
            updatedUser.role,
            updatedUser.address
        );
        
        if (!cacheSuccess) {
            cout << "Failed to update user in cache.\n";
            // Restore old user
            userManager.registerUser(
                currentUser->id,
                currentUser->name,
                currentUser->email,
                currentUser->phone,
                currentUser->password,
                currentUser->role,
                currentUser->address
            );
            return false;
        }
        
        // Update in persistent storage
        if (persistentUsers) {
            try {
                // Remove old entry
                UserData oldUser;
                oldUser.id = id;
                persistentUsers->remove(oldUser);
                
                // Add updated entry
                persistentUsers->insert(updatedUser);
                cout << "User updated in persistent storage.\n";
            } catch (const exception& e) {
                cout << "Failed to update user in persistent storage: " << e.what() << "\n";
                return false;
            }
        }
        
        return true;
    }

    // Update specific user fields
    bool updateUserName(int id, const string& newName) {
        UserData updates;
        updates.name = newName;
        return updateUser(id, updates);
    }
    
    bool updateUserEmail(int id, const string& newEmail) {
        UserData updates;
        updates.email = newEmail;
        return updateUser(id, updates);
    }
    
    bool updateUserPhone(int id, const string& newPhone) {
        UserData updates;
        updates.phone = newPhone;
        return updateUser(id, updates);
    }
    
    bool updateUserAddress(int id, const string& newAddress) {
        UserData updates;
        updates.address = newAddress;
        return updateUser(id, updates);
    }
    
    bool updateUserRole(int id, const string& newRole) {
        UserData updates;
        updates.role = newRole;
        return updateUser(id, updates);
    }
    
    bool updateUserPassword(int id, const string& oldPassword, const string& newPassword) {
        return userManager.updateUserPassword(id, oldPassword, newPassword);
    }

    // Check if user exists
    bool userExists(int id) const {
        return userManager.userExists(id);
    }

    // Check if email exists
    bool emailExists(const string& email) const {
        return userManager.emailExists(email);
    }

    // Get users by role
    LinkedList<UserData> getUsersByRole(const string& role) const {
        return userManager.getUsersByRole(role);
    }

    // Get all customers
    LinkedList<UserData> getAllCustomers() const {
        return userManager.getAllCustomers();
    }

    // Get all riders
    LinkedList<UserData> getAllRiders() const {
        return userManager.getAllRiders();
    }

    // Get all restaurant owners
    LinkedList<UserData> getAllRestaurantOwners() const {
        return userManager.getAllRestaurantOwners();
    }

    // Get all admins
    LinkedList<UserData> getAllAdmins() const {
        return userManager.getAllAdmins();
    }

    // Search users by name
    LinkedList<UserData> searchUsersByName(const string& name) const {
        return userManager.searchUsersByName(name);
    }

    // Print all users
    void printAllUsers() const {
        userManager.printAllUsers();
    }

    // Print users by role
    void printUsersByRole(const string& role) const {
        userManager.printUsersByRole(role);
    }

    // Print user statistics
    void printUserStatistics() const {
        userManager.printUserStatistics();
    }

    // Get total number of users
    int getTotalUsers() const {
        return userManager.getTotalUsers();
    }

    // Validate user data
    bool validateUserData(const UserData& user) const {
        return userManager.validateUserData(user);
    }

    // Sync cache with persistent storage
    void syncWithPersistentStorage() {
        cout << "Syncing cache with persistent storage...\n";
        
        // Clear cache
        // (Would need clear method in UserManager)
        
        // Reload from persistent storage
        loadUsersFromPersistentStorage();
        
        cout << "Sync completed.\n";
    }

    // Export users to file
    void exportUsers(const string& filename) const {
        userManager.exportUsers(filename);
    }

    // Import users from file
    void importUsers(const string& filename) {
        userManager.importUsers(filename);
        // After import, we should sync with persistent storage
        syncWithPersistentStorage();
    }

    // Clear all users (for testing)
    void clearAllUsers() {
        // Clear cache
        // (Would need clear method in UserManager)
        
        // Clear persistent storage
        if (persistentUsers) {
            persistentUsers->clear();
        }
        
        cout << "All users cleared.\n";
    }

    // Get the UserManager (for direct access if needed)
    UserManager& getUserManager() {
        return userManager;
    }
    
    // Const version
    const UserManager& getUserManager() const {
        return userManager;
    }
    
    // Get all users from persistent storage
    vector<UserData> getAllUsersFromPersistentStorage() const {
        if (!persistentUsers) {
            return {};
        }
        return persistentUsers->getAllKeys();
    }
    
    // Helper method to check if BTree operation succeeded
    bool insertIntoBTree(const UserData& user) {
        if (!persistentUsers) return false;
        
        // First check if already exists
        UserData searchUser;
        searchUser.id = user.id;
        auto result = persistentUsers->search(searchUser);
        if (result.first) {
            return false; // Already exists
        }
        
        // Try to insert
        try {
            persistentUsers->insert(user);
            return true;
        } catch (...) {
            return false;
        }
    }
};

#endif // USERSERVICE_H