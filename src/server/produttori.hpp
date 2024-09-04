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

struct Magazzino {
    int ID;
    vector<int> ID_Articoli; // stesso indice in tot indica la quantità
    vector<int> tot; 
    string indirizzo;
    int ID_Produttore; // ID del produttore proprietario
};

/* Prototipo di funzione per aggiungere un nuovo articolo, di un certo produttore in un certo magazzino */
void nuovoArticolo(Magazzino *magaz, Articolo *arti, Produttore *prod);
    
/* Prototipo di funzione per modificare un certo articolo di un certo produttore */
void modificaArticolo(int ID_Articolo, Magazzino *magaz = NULL, string nome = "", string descrizione = "", float prezzo = -1, int sconto = -1, int qta = -1);



#endif