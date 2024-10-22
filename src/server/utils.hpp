#pragma once

#include <string>
#include <ctime>
#include <vector>
#include <map>
using namespace std;


class Product {
public:
    int ID;
    int supplierID;
    std::string name;
    std::string description;
    double price;
    int quantity;

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
    int deliveryID;
    map<Product, int> products;
    time_t orderTime;

};

class Shipment {
public:
    int ID;
    int orderID;
    int shipperID;
    time_t handlingTime;
    bool state;

    /** Funzione per controllare lo stato di una spedizione
     *
     *  @return true se la spedizione è stata consegnata, false altrimenti
     */
    bool getShipmentState();
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
