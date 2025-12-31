#ifndef DATABASE_MANAGER_IMPL_H
#define DATABASE_MANAGER_IMPL_H

#include "database_manager.h"
#include "storage/SystemState.h"

// Implementation of helper methods that need complete SystemState definition

vector<Restaurant> DatabaseManager::exportSystemStateRestaurants() {
    if (systemState == nullptr) {
        return vector<Restaurant>();
    }
    return systemState->exportRestaurants();
}

vector<Order> DatabaseManager::exportSystemStateOrders() {
    if (systemState == nullptr) {
        return vector<Order>();
    }
    return systemState->exportOrders();
}

vector<Rider> DatabaseManager::exportSystemStateRiders() {
    if (systemState == nullptr) {
        return vector<Rider>();
    }
    return systemState->exportRiders();
}

#endif // DATABASE_MANAGER_IMPL_H