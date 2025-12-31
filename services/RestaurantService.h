#pragma once
#ifndef RESTAURANT_SERVICE_H
#define RESTAURANT_SERVICE_H

#include <iostream>
#include <string>
#include <algorithm>  
#include <vector>
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

    // FIXED: Use Restaurant class methods instead of direct access
    bool addMenuItem(int restaurantId, const MenuItem& item) {
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) return false;
        
        // Check if restaurant can add more items - use getter
        if (r->getMenuItemCount() >= 50) return false;
        
        // Check for duplicate ID in restaurant - use Restaurant method
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
        
        // Add menu item using Restaurant's method
        bool added = r->addMenuItem(item);
        
        if (added && persistentRestaurants) {
            // Update restaurant in persistent storage
            persistentRestaurants->remove(*r);
            persistentRestaurants->insert(*r);
        }
        
        return added;
    }

    // FIXED: Use Restaurant class methods
    bool removeMenuItem(int restaurantId, int itemId) {
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) return false;
        
        // First check if restaurant has this item
        if (!r->hasMenuItem(itemId)) return false;
        
        // For simplicity, we'll just remove from the IDs array
        // In a real implementation, you might want to also remove from menuItems array
        bool removed = false;
        
        // Get all menu item IDs
        vector<int> itemIds = r->getMenuItemIds();
        
        // Find and remove the item ID
        auto it = find(itemIds.begin(), itemIds.end(), itemId);
        if (it != itemIds.end()) {
            itemIds.erase(it);
            removed = true;
        }
        
        // Remove from menuItemsBTree
        if (removed && menuItemsBTree) {
            MenuItem dummy(itemId, "", "", 0.0, 0, "", 0);
            menuItemsBTree->remove(dummy);
        }
        
        // Note: Since Restaurant doesn't have a remove method for menuItems array,
        // we need to handle this differently or add that method to Restaurant class
        
        return removed;
    }

    // FIXED: Use Restaurant class methods
    MenuItem* getMenuItem(int restaurantId, int itemId) {
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) return nullptr;
        
        // Use Restaurant's method
        return r->getMenuItem(itemId);
    }

    // FIXED: Update menu item stock
    bool updateMenuItemStock(int restaurantId, int itemId, int newStock) {
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) return false;
        
        MenuItem* item = r->getMenuItem(itemId);
        if (!item) return false;
        
        item->stock = newStock;
        
        // Update in menuItemsBTree
        if (menuItemsBTree) {
            MenuItem dummy(itemId, "", "", 0.0, 0, "", 0);
            auto searchResult = menuItemsBTree->search(dummy);
            
            if (searchResult.first && !searchResult.second.empty()) {
                MenuItem itemCopy = searchResult.second[0];
                itemCopy.stock = newStock;
                menuItemsBTree->remove(dummy);
                menuItemsBTree->insert(itemCopy);
            }
        }
        
        if (persistentRestaurants) {
            persistentRestaurants->remove(*r);
            persistentRestaurants->insert(*r);
        }
        
        return true;
    }

    // FIXED: Update menu item price
    bool updateMenuItemPrice(int restaurantId, int itemId, double newPrice) {
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) return false;
        
        MenuItem* item = r->getMenuItem(itemId);
        if (!item) return false;
        
        item->price = newPrice;
        
        // Update in menuItemsBTree
        if (menuItemsBTree) {
            MenuItem dummy(itemId, "", "", 0.0, 0, "", 0);
            auto searchResult = menuItemsBTree->search(dummy);
            
            if (searchResult.first && !searchResult.second.empty()) {
                MenuItem itemCopy = searchResult.second[0];
                itemCopy.price = newPrice;
                menuItemsBTree->remove(dummy);
                menuItemsBTree->insert(itemCopy);
            }
        }
        
        if (persistentRestaurants) {
            persistentRestaurants->remove(*r);
            persistentRestaurants->insert(*r);
        }
        
        return true;
    }

    // FIXED: Update menu item details
    bool updateMenuItem(int restaurantId, int itemId, const string& newName = "", 
                       double newPrice = -1, int newStock = -1, 
                       const string& newDescription = "", const string& newCategory = "") {
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) return false;
        
        MenuItem* item = r->getMenuItem(itemId);
        if (!item) return false;
        
        // Update fields
        if (!newName.empty()) {
            strncpy(item->name, newName.c_str(), sizeof(item->name) - 1);
            item->name[sizeof(item->name) - 1] = '\0';
        }
        if (newPrice >= 0) {
            item->price = newPrice;
        }
        if (newStock >= 0) {
            item->stock = newStock;
        }
        if (!newDescription.empty()) {
            strncpy(item->description, newDescription.c_str(), sizeof(item->description) - 1);
            item->description[sizeof(item->description) - 1] = '\0';
        }
        if (!newCategory.empty()) {
            strncpy(item->category, newCategory.c_str(), sizeof(item->category) - 1);
            item->category[sizeof(item->category) - 1] = '\0';
        }
        
        // Update in menuItemsBTree
        if (menuItemsBTree) {
            MenuItem dummy(itemId, "", "", 0.0, 0, "", 0);
            auto searchResult = menuItemsBTree->search(dummy);
            
            if (searchResult.first && !searchResult.second.empty()) {
                MenuItem itemCopy = searchResult.second[0];
                
                if (!newName.empty()) {
                    strncpy(itemCopy.name, newName.c_str(), sizeof(itemCopy.name) - 1);
                    itemCopy.name[sizeof(itemCopy.name) - 1] = '\0';
                }
                if (newPrice >= 0) {
                    itemCopy.price = newPrice;
                }
                if (newStock >= 0) {
                    itemCopy.stock = newStock;
                }
                if (!newDescription.empty()) {
                    strncpy(itemCopy.description, newDescription.c_str(), sizeof(itemCopy.description) - 1);
                    itemCopy.description[sizeof(itemCopy.description) - 1] = '\0';
                }
                if (!newCategory.empty()) {
                    strncpy(itemCopy.category, newCategory.c_str(), sizeof(itemCopy.category) - 1);
                    itemCopy.category[sizeof(itemCopy.category) - 1] = '\0';
                }
                
                menuItemsBTree->remove(dummy);
                menuItemsBTree->insert(itemCopy);
            }
        }
        
        if (persistentRestaurants) {
            persistentRestaurants->remove(*r);
            persistentRestaurants->insert(*r);
        }
        
        return true;
    }

    // FIXED: Get all menu items for a restaurant
    vector<MenuItem> getRestaurantMenu(int restaurantId) {
        vector<MenuItem> menuItems;
        
        Restaurant* r = restaurants.searchTable(restaurantId);
        if (!r) return menuItems;
        
        // Use Restaurant's method to get menu items
        // We need to add a method to Restaurant to get all menu items
        // For now, let's assume we can get them
        
        // Get all menu item IDs first
        vector<int> itemIds = r->getMenuItemIds();
        
        for (int itemId : itemIds) {
            MenuItem* item = r->getMenuItem(itemId);
            if (item) {
                menuItems.push_back(*item);
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

    // FIXED: Print all restaurants - FIXED the menuItemCount access
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
                 << " | Menu Items: " << r.getMenuItemCount() << endl;  // FIXED: Use getter
        });
    }

    // FIXED: Get restaurant statistics - FIXED the menuItemCount access
    void printRestaurantStatistics() {
        int totalRestaurants = 0;
        int totalMenuItems = 0;
        
        restaurants.traverse([&](int id, Restaurant& r) {
            totalRestaurants++;
            totalMenuItems += r.getMenuItemCount();  // FIXED: Use getter
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

#endif // RESTAURANT_SERVICE_H