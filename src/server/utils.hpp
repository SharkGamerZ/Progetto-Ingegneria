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
    int ID_Mezzo;
    time_t presa_in_gestione;
    vector<int>     ID_Magazzini;
    vector<time_t>  arriviMagazzini;
    time_t arrivoConsegna;
};

/** Prototipo di funzione per modificare un certo articolo di un certo produttore
 * 
 * @param oldArt Articolo da modificare
 * @param newArt Articolo con i nuovi dati
 */
void updateProduct(Articolo oldArt, Articolo newArt);

/** Prototipo di funzione che permette di vedere lo stato di un prodotto 
 * 
 * @param o Ordine da controllare
 * @return true se l'ordine è stato consegnato, false altrimenti
 */
bool getOrderState(Ordine o);


bool getOrderState(Ordine o) {
    bool delivered = false;
    bool exist = false; // TODO query per controllare se nell'ordine è presente 

    if (!exist) {
        delivered = true; // TODO query per controllare se l'ordine è stato consegnato
    }
    else {
        throw -1;
    }

    return delivered;
}

#endif