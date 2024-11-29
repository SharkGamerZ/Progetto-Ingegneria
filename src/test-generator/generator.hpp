#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "../server/Supplier.hpp"
#include "../server/Customer.hpp"
#include "../server/Shipper.hpp"


using namespace std;


vector<User> users;

vector<Customer> customers;
vector<Shipper> shippers;
vector<Supplier> suppliers;


vector<Product> products;
vector<Order> orders;
vector<Shipping> shippings;

vector<string> getRandomNames(int n); 
vector<string> getRandomSurnames(int n); 
vector<string> getRandomCities(int n); 

vector<string> getRandomProductNames(int n); 
vector<string> getRandomAdjectives(int n);


void testCustomer(std::vector<bool> selected, int n);
