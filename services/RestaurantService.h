#pragma once
#ifndef RESTAURANT_SERVICE_H
#define RESTAURANT_SERVICE_H

#include <iostream>
#include <string>
#include <algorithm>  // Add this for transform and sort
#include "../models/Restaurant.h"
#include "../models/MenuItem.h"
#include "../dataStructures/HashTable.h"
#include "../dataStructures/LinkedList.h"
#include "../dataStructures/BTree.h"
using namespace std;

class RestaurantService {
private:
    HashTable<Restaurant> restaurants;
    PersistentBTree<Restaurant>* persistentRestaurants;
    PersistentBTree<MenuItem>* menuItemsBTree;

public:
    RestaurantService() : persistentRestaurants(nullptr), menuItemsBTree(nullptr) {}
    
    RestaurantService(PersistentBTree<Restaurant>* persistent, PersistentBTree<MenuItem>* menuTree = nullptr)
        : persistentRestaurants(persistent), menuItemsBTree(menuTree) {
        if (persistentRestaurants) {
            loadFromPersistentStorage();
        }
    }

    void setMenuItemsBTree(PersistentBTree<MenuItem>* menuTree) {
        menuItemsBTree = menuTree;
    }

    void loadFromPersistentStorage() {
        if (persistentRestaurants) {
            vector<Restaurant> allRestaurants = persistentRestaurants->getAllKeys();
            for (const auto& restaurant : allRestaurants) {
                restaurants.insertItem(restaurant.getRestaurantId(), restaurant);
            }
        }
    }

    bool addRestaurant(const Restaurant& r) {
        if (restaurants.searchTable(r.getRestaurantId()) != nullptr) return false;
        
        restaurants.insertItem(r.getRestaurantId(), r);
        
        if (persistentRestaurants) {
            persistentRestaurants->insert(r);
        }
        
        return true;
    }

    bool removeRestaurant(int restaurantId) {
        if (restaurants.searchTable(restaurantId) == nullptr) return false;
        
        restaurants.removeItem(restaurantId);
        
        if (persistentRestaurants) {
            Restaurant dummy(restaurantId, "", "", "", "", 0);
            persistentRestaurants->remove(dummy);
        }
        
        return true;
    }

    Restaurant* getRestaurant(int restaurantId) {
        return restaurants.searchTable(restaurantId);
    }

    // FIXED: Add menu item to both restaurant and menuItemsBTree
    bool addMenuItem(int restaurantId, const MenuItem& item) {
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) return false;
        
        // Check if restaurant can add more items
        if (r->menuItemCount >= 50) return false;  // Changed from 20 to match array size
        
        // Check for duplicate ID in restaurant
        if (r->hasMenuItem(item.id)) return false;
        
        // Add menu item to menuItemsBTree
        if (menuItemsBTree) {
            MenuItem dummy(item.id, "", "", 0.0, 0, "", 0);
            auto searchResult = menuItemsBTree->search(dummy);
            if (searchResult.first) {
                return false; // Item already exists in system
            }
            menuItemsBTree->insert(item);
        }
        
        // Add menu item to restaurant's fixed array
        if (r->menuItemCount < 50) {
            // Create a copy of the item
            MenuItem newItem = item;
            r->menuItems[r->menuItemCount] = newItem;
            r->menuItemCount++;
            
            // Also add to vector
            r->menuItemIds.push_back(item.id);
        } else {
            // Just add to vector if array is full
            r->menuItemIds.push_back(item.id);
        }
        
        if (persistentRestaurants) {
            // Update restaurant in persistent storage
            persistentRestaurants->remove(*r);
            persistentRestaurants->insert(*r);
        }
        
        return true;
    }

    // FIXED: Remove menu item from both restaurant and menuItemsBTree
    bool removeMenuItem(int restaurantId, int itemId) {
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) return false;
        
        // Remove from restaurant's fixed array
        bool removedFromArray = false;
        for (int i = 0; i < r->menuItemCount; i++) {
            if (r->menuItems[i].id == itemId) {
                // Shift remaining items left
                for (int j = i; j < r->menuItemCount - 1; j++) {
                    r->menuItems[j] = r->menuItems[j + 1];
                }
                r->menuItemCount--;
                removedFromArray = true;
                break;
            }
        }
        
        // Remove from vector
        auto it = find(r->menuItemIds.begin(), r->menuItemIds.end(), itemId);
        if (it != r->menuItemIds.end()) {
            r->menuItemIds.erase(it);
        }
        
        bool success = removedFromArray || (it != r->menuItemIds.end());
        
        if (success) {
            // Remove from menuItemsBTree
            if (menuItemsBTree) {
                MenuItem dummy(itemId, "", "", 0.0, 0, "", 0);
                menuItemsBTree->remove(dummy);
            }
            
            if (persistentRestaurants) {
                persistentRestaurants->remove(*r);
                persistentRestaurants->insert(*r);
            }
        }
        
        return success;
    }

    // FIXED: Get menu item from menuItemsBTree
    MenuItem* getMenuItem(int restaurantId, int itemId) {
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) return nullptr;
        
        // Check if restaurant has this item
        if (!r->hasMenuItem(itemId)) return nullptr;
        
        // Get from menuItemsBTree
        if (menuItemsBTree) {
            MenuItem dummy(itemId, "", "", 0.0, 0, "", 0);
            auto searchResult = menuItemsBTree->search(dummy);
            if (searchResult.first && !searchResult.second.empty()) {
                // Create a copy to return
                return new MenuItem(searchResult.second[0]);
            }
        }
        
        // Fallback: check in restaurant's array
        for (int i = 0; i < r->menuItemCount; i++) {
            if (r->menuItems[i].id == itemId) {
                return new MenuItem(r->menuItems[i]);
            }
        }
        
        return nullptr;
    }

    // FIXED: Update menu item stock
    bool updateMenuItemStock(int restaurantId, int itemId, int newStock) {
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) return false;
        
        // Check if restaurant has this item
        if (!r->hasMenuItem(itemId)) return false;
        
        bool updated = false;
        
        // Update in restaurant's array first
        for (int i = 0; i < r->menuItemCount; i++) {
            if (r->menuItems[i].id == itemId) {
                r->menuItems[i].stock = newStock;
                updated = true;
                break;
            }
        }
        
        // Update in menuItemsBTree
        if (menuItemsBTree) {
            MenuItem dummy(itemId, "", "", 0.0, 0, "", 0);
            auto searchResult = menuItemsBTree->search(dummy);
            
            if (searchResult.first && !searchResult.second.empty()) {
                MenuItem item = searchResult.second[0];
                item.stock = newStock;
                menuItemsBTree->remove(dummy);
                menuItemsBTree->insert(item);
                updated = true;
            }
        }
        
        if (updated && persistentRestaurants) {
            persistentRestaurants->remove(*r);
            persistentRestaurants->insert(*r);
        }
        
        return updated;
    }

    // FIXED: Update menu item price
    bool updateMenuItemPrice(int restaurantId, int itemId, double newPrice) {
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) return false;
        
        if (!r->hasMenuItem(itemId)) return false;
        
        bool updated = false;
        
        // Update in restaurant's array
        for (int i = 0; i < r->menuItemCount; i++) {
            if (r->menuItems[i].id == itemId) {
                r->menuItems[i].price = newPrice;
                updated = true;
                break;
            }
        }
        
        // Update in menuItemsBTree
        if (menuItemsBTree) {
            MenuItem dummy(itemId, "", "", 0.0, 0, "", 0);
            auto searchResult = menuItemsBTree->search(dummy);
            
            if (searchResult.first && !searchResult.second.empty()) {
                MenuItem item = searchResult.second[0];
                item.price = newPrice;
                menuItemsBTree->remove(dummy);
                menuItemsBTree->insert(item);
                updated = true;
            }
        }
        
        if (updated && persistentRestaurants) {
            persistentRestaurants->remove(*r);
            persistentRestaurants->insert(*r);
        }
        
        return updated;
    }

    // FIXED: Update menu item details - REMOVED the problematic lines
    bool updateMenuItem(int restaurantId, int itemId, const string& newName = "", 
                       double newPrice = -1, int newStock = -1, 
                       const string& newDescription = "", const string& newCategory = "") {
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) return false;
        
        if (!r->hasMenuItem(itemId)) return false;
        
        bool updated = false;
        
        // Update in restaurant's array
        for (int i = 0; i < r->menuItemCount; i++) {
            if (r->menuItems[i].id == itemId) {
                if (!newName.empty()) {
                    strncpy(r->menuItems[i].name, newName.c_str(), sizeof(r->menuItems[i].name) - 1);
                    r->menuItems[i].name[sizeof(r->menuItems[i].name) - 1] = '\0';
                }
                if (newPrice >= 0) {
                    r->menuItems[i].price = newPrice;
                }
                if (newStock >= 0) {
                    r->menuItems[i].stock = newStock;
                }
                if (!newDescription.empty()) {
                    strncpy(r->menuItems[i].description, newDescription.c_str(), sizeof(r->menuItems[i].description) - 1);
                    r->menuItems[i].description[sizeof(r->menuItems[i].description) - 1] = '\0';
                }
                if (!newCategory.empty()) {
                    strncpy(r->menuItems[i].category, newCategory.c_str(), sizeof(r->menuItems[i].category) - 1);
                    r->menuItems[i].category[sizeof(r->menuItems[i].category) - 1] = '\0';
                }
                updated = true;
                break;
            }
        }
        
        // Update in menuItemsBTree
        if (menuItemsBTree) {
            MenuItem dummy(itemId, "", "", 0.0, 0, "", 0);
            auto searchResult = menuItemsBTree->search(dummy);
            
            if (searchResult.first && !searchResult.second.empty()) {
                MenuItem item = searchResult.second[0];
                
                if (!newName.empty()) {
                    strncpy(item.name, newName.c_str(), sizeof(item.name) - 1);
                    item.name[sizeof(item.name) - 1] = '\0';
                }
                if (newPrice >= 0) {
                    item.price = newPrice;
                }
                if (newStock >= 0) {
                    item.stock = newStock;
                }
                if (!newDescription.empty()) {
                    strncpy(item.description, newDescription.c_str(), sizeof(item.description) - 1);
                    item.description[sizeof(item.description) - 1] = '\0';
                }
                if (!newCategory.empty()) {
                    strncpy(item.category, newCategory.c_str(), sizeof(item.category) - 1);
                    item.category[sizeof(item.category) - 1] = '\0';
                }
                
                menuItemsBTree->remove(dummy);
                menuItemsBTree->insert(item);
                updated = true;
            }
        }
        
        if (updated && persistentRestaurants) {
            persistentRestaurants->remove(*r);
            persistentRestaurants->insert(*r);
        }
        
        return updated;
    }

    // FIXED: Get all menu items for a restaurant
    vector<MenuItem> getRestaurantMenu(int restaurantId) {
        vector<MenuItem> menuItems;
        
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) return menuItems;
        
        // First, get from restaurant's array
        for (int i = 0; i < r->menuItemCount; i++) {
            menuItems.push_back(r->menuItems[i]);
        }
        
        // Then try to get from menuItemsBTree for items only in vector
        if (menuItemsBTree) {
            for (int id : r->menuItemIds) {
                bool found = false;
                for (const auto& item : menuItems) {
                    if (item.id == id) {
                        found = true;
                        break;
                    }
                }
                
                if (!found) {
                    MenuItem dummy(id, "", "", 0.0, 0, "", 0);
                    auto searchResult = menuItemsBTree->search(dummy);
                    if (searchResult.first && !searchResult.second.empty()) {
                        menuItems.push_back(searchResult.second[0]);
                    }
                }
            }
        }
        
        return menuItems;
    }

    // FIXED: Print restaurant menu
    void printRestaurantMenu(int restaurantId) {
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) {
            cout << "Restaurant not found!\n";
            return;
        }
        
        r->printMenu();
    }

    // FIXED: Get restaurants by cuisine
    LinkedList<Restaurant> getRestaurantsByCuisine(const string& cuisine) {
        LinkedList<Restaurant> result;
        restaurants.traverse([&](int id, Restaurant& r) {
            if (r.getCuisine() == cuisine) {
                result.push_back(r);
            }
        });
        return result;
    }

    // FIXED: Get restaurants by location
    LinkedList<Restaurant> getRestaurantsByLocation(int location) {
        LinkedList<Restaurant> result;
        restaurants.traverse([&](int id, Restaurant& r) {
            if (r.getLocationNode() == location) {
                result.push_back(r);
            }
        });
        return result;
    }

    // FIXED: Print all restaurants
    void printAllRestaurants() {
        cout << "=== All Restaurants ===\n";
        restaurants.traverse([](int id, Restaurant& r) {
            cout << "ID: " << id 
                 << " | Name: " << r.getName() 
                 << " | Cuisine: " << r.getCuisine()
                 << " | Location: " << r.getLocationNode()
                 << " | Phone: " << r.getPhone() 
                 << " | Rating: " << r.getRating()
                 << " | Delivery Time: " << r.getDeliveryTime() << " mins"
                 << " | Menu Items: " << r.menuItemCount << endl;
        });
    }

    // FIXED: Get restaurant statistics
    void printRestaurantStatistics() {
        int totalRestaurants = 0;
        int totalMenuItems = 0;
        
        restaurants.traverse([&](int id, Restaurant& r) {
            totalRestaurants++;
            totalMenuItems += r.menuItemCount;
        });
        
        cout << "=== Restaurant Statistics ===\n";
        cout << "Total Restaurants: " << totalRestaurants << "\n";
        cout << "Total Menu Items (across all restaurants): " << totalMenuItems << "\n";
        cout << "Average Menu Items per Restaurant: " 
             << (totalRestaurants > 0 ? (double)totalMenuItems / totalRestaurants : 0) << "\n";
    }

    // FIXED: Search restaurants by name
    LinkedList<Restaurant> searchRestaurantsByName(const string& name) {
        LinkedList<Restaurant> result;
        string searchName = name;
        transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);
        
        restaurants.traverse([&](int id, Restaurant& r) {
            string restaurantName = r.getName();
            transform(restaurantName.begin(), restaurantName.end(), restaurantName.begin(), ::tolower);
            
            if (restaurantName.find(searchName) != string::npos) {
                result.push_back(r);
            }
        });
        
        return result;
    }

    // FIXED: Get top-rated restaurants
    LinkedList<Restaurant> getTopRatedRestaurants(int limit = 5) {
        vector<Restaurant> allRestaurants;
        restaurants.traverse([&](int id, Restaurant& r) {
            allRestaurants.push_back(r);
        });
        
        sort(allRestaurants.begin(), allRestaurants.end(),
             [](const Restaurant& a, const Restaurant& b) {
                 return a.getRating() > b.getRating();
             });
        
        LinkedList<Restaurant> result;
        int count = min(limit, (int)allRestaurants.size());
        for (int i = 0; i < count; i++) {
            result.push_back(allRestaurants[i]);
        }
        
        return result;
    }
};

#endif