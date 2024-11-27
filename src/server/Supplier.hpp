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
    
    Supplier(string P_IVA, string email, string pw, vector<int> ID_wh, int ID, string CF, string name, string surname) {
        this->P_IVA = P_IVA;
        this->email = email;
        this->pw = pw;
        this->ID_wh = ID_wh;
        this->CF = CF;
        this->name = name;
        this->surname = surname;
    }

    /** Prototipo di funzione che aggiunge nel DB un nuovo articolo 
     * 
     * @param a Product da aggiungere
     */
    void addProduct(Product a);
    
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
    vector<Product> deleteSoldProducts(Supplier p);
};

/* 
struct Magazzino {
    int ID;
    vector<int> ID_Articoli; // stesso indice in tot indica la quantità
    vector<int> tot; 
    string indirizzo;
    int ID_Supplier; // ID del Supplier proprietario
}; */

/* TODO magari forse potrebbe essere utile implementare qualche sicurezza */





#endif
