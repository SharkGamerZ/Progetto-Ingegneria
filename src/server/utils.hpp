#pragma once

#include <string>
#include <ctime>
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

    Product (int ID, int supplierID, std::string name, std::string description, double price, int stock) {
        this->ID = ID;
        this->supplierID = supplierID;
        this->name = name;
        this->description = description;
        this->price = price;
        this->stock = stock;
    }


    /** Funzione per aggiornare i dati di un prodotto
     * 
     * @param newProduct Product con i nuovi dati
     */
    void updateProduct(Product newProduct);

};

class Order {
public:
    int ID;
    int customerID;
    int ShippingID;
    map <int, int> products; // Maps productsID to quantities
    time_t orderTime;


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

    /** Funzione per controllare lo stato di una spedizione
     *
     *  @return true se la spedizione è stata consegnata, false altrimenti
     */
    bool getShippingState();
};

/** TODO Prototipo di funzione per ricercare i prodotti in vendita.
 *  Deve fare un check qta > 0;
 * 
 * @return Lista di tutti gli articoli in vendita.
 */
vector<Product> getProducts();


/** TODO Prototipo di funzione per ricercare i prodotti in venditafilter
 * con dei filtri.
 *  I filtri includeranno produttore, nome, qta, prezzo.
 * 
 * @return Lista di tutti gli articoli in vendita.
 */
vector<Product> getFilteredProducts(int id);

/** TODO @brief Function that updates the values of the product of ID "ID" to the values in the product passed
 * @param ID Integer that indentifies a product
 * @param mod A product that has the updated value 
 */
void updateProduct(int ID, const Product& mod);

/** TODO @brief Function to get the state of an order
 * @param ID Integer that identifies an order
 * @return A boolean that indicates if the order has been shipped
*/
bool getOrderState(int ID);




