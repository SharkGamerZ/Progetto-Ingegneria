#pragma once
#include <iomanip>
#include <string>
#include <ctime>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <iostream>
using namespace std;


class Product {
public:
    int ID;
    int supplierID;
    std::string name;
    std::string description;
    double price;
    int stock;

    Product() {}
    Product (int ID, int supplierID, std::string name, std::string description, double price, int stock) {
        this->ID = ID;
        this->supplierID = supplierID;
        this->name = name;
        this->description = description;
        this->price = price;
        this->stock = stock;
    }


};

class Order {
public:
    int ID;
    int customerID;
    map <int, int> products; // Maps productsID to quantities
    time_t orderTime;


    Order () {}
    Order (int ID, int customerID, time_t orderTime) {
        this->ID = ID;
        this->customerID = customerID;
        this->orderTime = orderTime;
    }
};

class Shipping {
public:
    int ID;
    int orderID;
    int shipperID;
    time_t handlingTime;
    bool state;

    Shipping() {}

    Shipping (int ID, int orderID, int shipperID, time_t handlingTime, bool state) {
        this->ID = ID;
        this->orderID = orderID;
        this->shipperID = shipperID;
        this->handlingTime = handlingTime;
        this->state = state;
    }

};


// Function to log errors onto file 
void logError (string filename, int line, string error);


