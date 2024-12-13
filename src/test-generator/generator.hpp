#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include "../server/utils.hpp"
#include "../server/Shipper.hpp"
#include "../server/Supplier.hpp"
#include "../server/Customer.hpp"
#include "../dbutils/pgutils.hpp"
#include "../dbutils/rdutils.hpp"



using namespace std;

vector<string> getRandomNames(int n); 
vector<string> getRandomSurnames(int n); 
vector<string> getRandomCities(int n); 

vector<string> getRandomProductNames(int n); 
vector<string> getRandomAdjectives(int n);


void testCustomer(std::vector<bool> selected, int n, vector<int> customersID, vector<int> suppliersID, vector<int> productIDs);

void testShipper(std::vector<bool> selected, int n, vector<int> shippersIDs, vector<int> orderIDs);

void testSupplier(vector<bool> selected, int n, vector<int> suppliersIDs, vector<int> productsIDs);