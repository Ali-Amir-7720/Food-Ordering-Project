#pragma once
#ifndef CITYMAPDATABASE_H
#define CITYMAPDATABASE_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "models/CityMapData.h"
using namespace std;
class CityMapDatabase {
private:
    const string LOCATIONS_FILE = "city_locations.dat";
    const string ROADS_FILE = "city_roads.dat";
    
    template<typename T>
    bool saveAllToFile(const string& filename, const vector<T>& items) {
        try {
            ofstream file(filename, ios::binary);
            if (!file.is_open()) return false;
            
            for (const auto& item : items) {
                file.write(reinterpret_cast<const char*>(&item), sizeof(T));
            }
            file.close();
            return true;
        } catch (...) {
            return false;
        }
    }
    
    template<typename T>
    vector<T> loadAllFromFile(const string& filename) {
        vector<T> items;
        try {
            ifstream file(filename, ios::binary);
            if (!file.is_open()) return items;
            
            T item;
            while (file.read(reinterpret_cast<char*>(&item), sizeof(T))) {
                items.push_back(item);
            }
            file.close();
        } catch (...) {}
        return items;
    }
    
public:
    bool saveAllLocations(const vector<LocationData>& locations) {
        return saveAllToFile(LOCATIONS_FILE, locations);
    }
    
    vector<LocationData> loadAllLocations() {
        return loadAllFromFile<LocationData>(LOCATIONS_FILE);
    }
    
    // Save/Load roads
    bool saveAllRoads(const vector<RoadData>& roads) {
        return saveAllToFile(ROADS_FILE, roads);
    }
    
    vector<RoadData> loadAllRoads() {
        return loadAllFromFile<RoadData>(ROADS_FILE);
    }
    
    // Clear all map data
    void clearMapData() {
        remove(LOCATIONS_FILE.c_str());
        remove(ROADS_FILE.c_str());
    }
};
#endif