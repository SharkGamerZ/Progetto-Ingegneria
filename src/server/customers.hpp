#include "utils.hpp"

struct Customer {
    string nome;
    string cognome;
    string mail;
    string password;

    vector<Articolo> carrello;
    vector<Ordine> ordini_passati;
};

struct IndirizzoSpedizione {
    int ID;
    string citta;
    string via;
    string civico;
};

struct Pagamento {
    int ID;
    string numero_carta;
    string scadenza;
};

