#pragma once
#ifndef CITYMAPDATA_H
#define CITYMAPDATA_H

#include <cstring>
#include <string>
using namespace std;
struct LocationData {
    int nodeId;
    char name[100];
    char type[20]; 
    LocationData() : nodeId(0) {
        memset(name, 0, sizeof(name));
        memset(type, 0, sizeof(type));
    }
    
    LocationData(int id, const string& n, const string& t) : nodeId(id) {
        strncpy(name, n.c_str(), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        strncpy(type, t.c_str(), sizeof(type) - 1);
        type[sizeof(type) - 1] = '\0';
    }
};

// POD struct for storing road/edge data in binary file
struct RoadData {
    int fromNode;
    int toNode;
    int distance;  // in meters
    
    RoadData() : fromNode(0), toNode(0), distance(0) {}
    
    RoadData(int from, int to, int dist) 
        : fromNode(from), toNode(to), distance(dist) {}
};

#endif
