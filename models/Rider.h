// Rider.h
#ifndef RIDER_H
#define RIDER_H

#include <iostream>
#include <string>
#include <cstring>

using namespace std;

class Rider {
public:
    int id;
    char name[100];
    char email[100];
    char password[100];
    char phone[20];
    char vehicle[20];  
    int location;      
    char status[20];   
    double rating;
    int completedDeliveries;
    
    Rider() : id(0), location(0), rating(0.0), completedDeliveries(0) {
        memset(name, 0, sizeof(name));
        memset(email, 0, sizeof(email));
        memset(password, 0, sizeof(password));
        memset(phone, 0, sizeof(phone));
        memset(vehicle, 0, sizeof(vehicle));
        memset(status, 0, sizeof(status));
        strcpy(status, "available");
        strcpy(vehicle, "bike");
    }
    
    Rider(int _id, const string& _name, const string& _email, 
          const string& _password, const string& _status)
        : id(_id), location(0), rating(0.0), completedDeliveries(0) {
        
        strncpy(name, _name.c_str(), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        
        strncpy(email, _email.c_str(), sizeof(email) - 1);
        email[sizeof(email) - 1] = '\0';
        
        strncpy(password, _password.c_str(), sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0';
        
        memset(phone, 0, sizeof(phone));
        
        strcpy(vehicle, "bike");
        
        strncpy(status, _status.c_str(), sizeof(status) - 1);
        status[sizeof(status) - 1] = '\0';
    }
    
    Rider(int _id, const string& _name, const string& _email, 
          const string& _password, const string& _phone, 
          const string& _vehicle, int _location)
        : id(_id), location(_location), rating(0.0), completedDeliveries(0) {
        
        strncpy(name, _name.c_str(), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        
        strncpy(email, _email.c_str(), sizeof(email) - 1);
        email[sizeof(email) - 1] = '\0';
        
        strncpy(password, _password.c_str(), sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0';
        
        strncpy(phone, _phone.c_str(), sizeof(phone) - 1);
        phone[sizeof(phone) - 1] = '\0';
        
        strncpy(vehicle, _vehicle.c_str(), sizeof(vehicle) - 1);
        vehicle[sizeof(vehicle) - 1] = '\0';
        
        strcpy(status, "available");
    }
    
    // Getters
    string getName() const { return string(name); }
    string getEmail() const { return string(email); }
    string getPassword() const { return string(password); }
    string getStatus() const { return string(status); }
    string getPhone() const { return string(phone); }
    string getVehicle() const { return string(vehicle); }
    int getId() const { return id; }
    int getLocation() const { return location; }
    double getRating() const { return rating; }
    
    // Setters
    void setEmail(const string& _email) {
        strncpy(email, _email.c_str(), sizeof(email) - 1);
        email[sizeof(email) - 1] = '\0';
    }
    
    void setPassword(const string& _password) {
        strncpy(password, _password.c_str(), sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0';
    }
    
    void setName(const string& _name) {
        strncpy(name, _name.c_str(), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
    }
    
    void setPhone(const string& _phone) {
        strncpy(phone, _phone.c_str(), sizeof(phone) - 1);
        phone[sizeof(phone) - 1] = '\0';
    }
    
    void setVehicle(const string& _vehicle) {
        strncpy(vehicle, _vehicle.c_str(), sizeof(vehicle) - 1);
        vehicle[sizeof(vehicle) - 1] = '\0';
    }
    
    void setStatus(const string& s) {
        strncpy(status, s.c_str(), sizeof(status) - 1);
        status[sizeof(status) - 1] = '\0';
    }
    void setRating(double r) {
        rating = r;
    }
    // Authentication
    bool authenticate(const string& inputEmail, const string& inputPassword) const {
        return (string(email) == inputEmail && string(password) == inputPassword);
    }
    
    bool changePassword(const string& oldPassword, const string& newPassword) {
        if (string(password) == oldPassword) {
            setPassword(newPassword);
            return true;
        }
        return false;
    }
    
    // Rider operations
    void assignOrder(int orderId) {
        strcpy(status, "busy");
    }
    
    void completeDelivery(bool success) {
        completedDeliveries++;
        if (success) {
            rating = (rating * (completedDeliveries - 1) + 5.0) / completedDeliveries;
        }
        strcpy(status, "available");
    }
    
    void updateLocation(int newLocation) {
        location = newLocation;
    }
    
    double calculatePriority(int pickupLocation, int deliveryLocation) const {
        double distanceToPickup = abs(location - pickupLocation);
        return 100.0 / (distanceToPickup + 1);
    }
    
    // Display
    void displayInfo() const {
        cout << "┌─────────────────────────────────────┐\n";
        cout << "│ Rider ID: " << id << "\n";
        cout << "├─────────────────────────────────────┤\n";
        cout << "│ Name: " << name << "\n";
        cout << "│ Email: " << email << "\n";
        cout << "│ Phone: " << phone << "\n";
        cout << "│ Vehicle: " << vehicle << "\n";
        cout << "│ Location: " << location << "\n";
        cout << "│ Status: " << status << "\n";
        cout << "│ Rating: " << rating << "/5 ⭐\n";
        cout << "│ Completed Deliveries: " << completedDeliveries << "\n";
        cout << "└─────────────────────────────────────┘\n";
    }
    
    // For serialization/saving to database
    string toDatabaseString() const {
        return to_string(id) + ";" + name + ";" + email + ";" + password + ";" + 
               phone + ";" + vehicle + ";" + to_string(location) + ";" + 
               status + ";" + to_string(rating) + ";" + to_string(completedDeliveries);
    }
    
    // Load from database string
    static Rider fromDatabaseString(const string& data) {
        Rider rider;
        vector<string> parts;
        size_t start = 0, end;
        
        while ((end = data.find(';', start)) != string::npos) {
            parts.push_back(data.substr(start, end - start));
            start = end + 1;
        }
        parts.push_back(data.substr(start));
        
        if (parts.size() >= 10) {
            rider.id = stoi(parts[0]);
            strncpy(rider.name, parts[1].c_str(), sizeof(rider.name) - 1);
            strncpy(rider.email, parts[2].c_str(), sizeof(rider.email) - 1);
            strncpy(rider.password, parts[3].c_str(), sizeof(rider.password) - 1);
            strncpy(rider.phone, parts[4].c_str(), sizeof(rider.phone) - 1);
            strncpy(rider.vehicle, parts[5].c_str(), sizeof(rider.vehicle) - 1);
            rider.location = stoi(parts[6]);
            strncpy(rider.status, parts[7].c_str(), sizeof(rider.status) - 1);
            rider.rating = stod(parts[8]);
            rider.completedDeliveries = stoi(parts[9]);
        }
        
        return rider;
    }
    
    friend ostream& operator<<(ostream& os, const Rider& rider) {
        os << "Rider{ID: " << rider.id 
           << ", Name: " << rider.name 
           << ", Email: " << rider.email
           << ", Status: " << rider.status 
           << ", Location: " << rider.location 
           << ", Vehicle: " << rider.vehicle 
           << ", Rating: " << rider.rating << "}";
        return os;
    }
};

#endif // RIDER_H