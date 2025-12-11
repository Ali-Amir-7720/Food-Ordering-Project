#pragma once
#ifndef MENUITEM_H
#define MENUITEM_H

#include <iostream>
#include <string>
#include <cstring>

using namespace std;

class MenuItem {
public:
    int id;
    char name[100];
    char description[200];
    double price;
    int stock;
    char category[50];
    int restaurantId;
    
    MenuItem() : id(0), price(0.0), stock(0), restaurantId(0) {
        memset(name, 0, sizeof(name));
        memset(description, 0, sizeof(description));
        memset(category, 0, sizeof(category));
    }
    
    MenuItem(int _id, const string& _name, const string& _desc, 
             double _price, int _stock, const string& _cat, int _restId)
        : id(_id), price(_price), stock(_stock), restaurantId(_restId) {
        strncpy(name, _name.c_str(), sizeof(name) - 1);
        strncpy(description, _desc.c_str(), sizeof(description) - 1);
        strncpy(category, _cat.c_str(), sizeof(category) - 1);
        name[sizeof(name)-1] = '\0';
        description[sizeof(description)-1] = '\0';
        category[sizeof(category)-1] = '\0';
    }
    
    // Serialization method
    void serialize(char* buffer) const {
        int offset = 0;
        
        memcpy(buffer + offset, &id, sizeof(id));
        offset += sizeof(id);
        
        memcpy(buffer + offset, name, sizeof(name));
        offset += sizeof(name);
        
        memcpy(buffer + offset, description, sizeof(description));
        offset += sizeof(description);
        
        memcpy(buffer + offset, &price, sizeof(price));
        offset += sizeof(price);
        
        memcpy(buffer + offset, &stock, sizeof(stock));
        offset += sizeof(stock);
        
        memcpy(buffer + offset, category, sizeof(category));
        offset += sizeof(category);
        
        memcpy(buffer + offset, &restaurantId, sizeof(restaurantId));
        offset += sizeof(restaurantId);
    }
    
    // Deserialization method
    void deserialize(const char* buffer) {
        int offset = 0;
        
        memcpy(&id, buffer + offset, sizeof(id));
        offset += sizeof(id);
        
        memcpy(name, buffer + offset, sizeof(name));
        offset += sizeof(name);
        
        memcpy(description, buffer + offset, sizeof(description));
        offset += sizeof(description);
        
        memcpy(&price, buffer + offset, sizeof(price));
        offset += sizeof(price);
        
        memcpy(&stock, buffer + offset, sizeof(stock));
        offset += sizeof(stock);
        
        memcpy(category, buffer + offset, sizeof(category));
        offset += sizeof(category);
        
        memcpy(&restaurantId, buffer + offset, sizeof(restaurantId));
        offset += sizeof(restaurantId);
    }
    
    // Conversion back to string
    string getName() const { return string(name); }
    string getDescription() const { return string(description); }
    string getCategory() const { return string(category); }
    
    friend ostream& operator<<(ostream& os, const MenuItem& item) {
        os << item.id << ": " << item.name << " - $" << item.price 
           << " (Stock: " << item.stock << ")";
        return os;
    }
};

#endif