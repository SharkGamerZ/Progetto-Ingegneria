#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <ctime>
#include <vector>
using namespace std;

struct Articolo {
    int ID;
    int ID_Produttore;
    string nome;
    string descrizione;
    float prezzo; // Da limitare a 2 cifre decimali
    int sconto; // Da limitare da 0 a 100
    int qta;
};


struct Ordine {
    int ID;
    string ID_Customer;
    vector<Articolo> articoli;
    time_t istante;
    int ID_Spedizione;
};


struct Spedizione {
    int ID;
    int ID_Ordine;
    int ID_Trasportatore;
    time_t presa_in_gestione;
    bool stato;
};

/** TODO Prototipo di funzione per modificare un certo articolo di un certo produttore
 * 
 * @param oldArt Articolo da modificare
 * @param newArt Articolo con i nuovi dati
 */
void updateProduct(Articolo oldArt, Articolo newArt);


/** TODO Prototipo di funzione per ricercare i prodotti in vendita.
 *  Deve fare un check qta > 0;
 * 
 * @return Lista di tutti gli articoli in vendita.
 */
Articolo[] getProducts();


/** TODO Prototipo di funzione per ricercare i prodotti in vendita con dei filtri.
 *  I filtri includeranno produttore, nome, qta, prezzo.
 * 
 * @return Lista di tutti gli articoli in vendita.
 */
Articolo[] getFilteredProducts(Filters f);
#endif