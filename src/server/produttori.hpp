#ifndef PRODUTTORI_HPP
#define PRODUTTORI_HPP

#include "utils.hpp"
#include <string>
#include <ctime>
#include <vector>
using namespace std;

struct Produttore {
    string P_IVA;
    string mail;
    string password;
    vector<int> ID_Magazzini;
};
/* 
struct Magazzino {
    int ID;
    vector<int> ID_Articoli; // stesso indice in tot indica la quantità
    vector<int> tot; 
    string indirizzo;
    int ID_Produttore; // ID del produttore proprietario
}; */

/* TODO magari forse potrebbe essere utile implementare qualche sicurezza */

/** Prototipo di funzione che aggiunge nel DB un nuovo articolo 
 * 
 * @param a Articolo da aggiungere
 */
void addProduct(Articolo a);

/** Prototipo di funzione che permette di marcare un prodotto come discontinuo
 * 
 * @param a Articolo da marcare come discontinuo
*/
void setDiscontinuedProduct(Articolo a);

/** Prototipo di funzione che ritorna i propri articoli venduti
 * 
 * @param p Produttore di cui si vogliono vedere gli articoli venduti
 * @return Vector di articoli venduti
 */
vector<Articolo> getPastOrders(Produttore p);



#endif