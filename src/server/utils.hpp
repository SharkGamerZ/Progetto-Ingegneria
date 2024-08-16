#pragma once

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