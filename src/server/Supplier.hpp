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
    
    Supplier(int ID, string CF, string name, string surname, string email, string P_IVA);
    /** @brief Adds a certain quantity of a product to the DB
     * 
     * Adds the quantity q to the product with ID as key 
     * 
     * @param ID The ID of a product
     * @param q The quantity of the product to be added
     */
    void addStock(int ID, int q);

    /** @brief Sets a product to discontinued
     * 
     * Takes the ID of a existant product and sets it to discontinued 
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
