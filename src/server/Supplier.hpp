#ifndef PRODUTTORI_HPP
#define PRODUTTORI_HPP

#include "utils.hpp"
#include "User.hpp"
#include <string>
#include <ctime>
#include <vector>
using namespace std;

class Supplier: public User {
public:
    string P_IVA;
    string pw;
    vector<int> ID_wh;
    
    Supplier(int ID, string CF, string name, string surname, string email, string P_IVA);
    /** Prototipo di funzione che aggiunge nel DB un nuovo articolo 
     * 
     * @param a Product da aggiungere
     */
    void addProduct(Product a, int q);

    /** Prototipo di funzione che permette di marcare un prodotto come discontinuo
     * 
     * @param a Product da marcare come discontinuo
    */
    void setDiscontinuedProduct(Product a);

    /** Prototipo di funzione che ritorna i propri articoli venduti
     * 
     * @param p Supplier di cui si vogliono vedere gli articoli venduti
     * @return Vector di articoli venduti
     */
    vector<Product> getPastOrders();
};

#endif
