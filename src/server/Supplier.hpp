#ifndef PRODUTTORI_HPP
#define PRODUTTORI_HPP

#include "utils.hpp"
#include "User.hpp"
#include <string>
#include <ctime>
#include <vector>
#include <pqxx/pqxx>
#include "../dbutils/rdutils.hpp"
using namespace std;

class Supplier: public User {
public:
    string P_IVA;
    string pw;
    vector<int> ID_wh;
    
    Supplier() {}
    
    Supplier(int ID, string CF, string name, string surname, string email, string P_IVA);

    /** @brief Adds a certain quantity of a product to the DB
     * 
     * Adds the quantity q to the product with ID as key to the DB and updates the data in the cache 
     * 
     * @param ID The ID of a product
     * @param q The quantity of the product to be added
     */
    void addStock(int ID, int q);

    /** @brief Given the values for the columns of a product, it adds the product to both DB and cache
     * 
     * It creates a new tuple in the DB and set a new element in the Redis cache, following the always cached patter of the products
     * @param name The name of the product
     * @param des The description of the product
     * @param supplier The ID of the supplier of the product
     * @param price The price of the product
     * @param stock The number of said products in the stock 
     */
    void addProduct(string name, string des, int supplier, float price, int stock);

    /** @brief Sets a product to discontinued
     * 
     * Takes the ID of a existant product and sets it to discontinued (quantity of the product<0) 
     * @param ID The ID of a products
    */
    void setDiscontinuedProduct(int ID);

    /** @brief Returns the past orders of the supplier
     * 
     * Returns the past orders of the supplier (the orders of a customer that contains a product of the supplier)
     * 
     * @return A vector<int> of the IDs of orders?products? Una map<?,?,...>?  
     */
    vector<int> getPastOrders();
    // NON PENSO FARÒ QUESTA FUNZIONE CON REDIS
};

#endif
