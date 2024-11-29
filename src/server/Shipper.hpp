#include "utils.hpp"

struct Trasportatore {
    int ID;
    string P_IVA;
    string ragione_sociale;
    string sede;            //sarebbe l'indirizzo (da aggiustare)
    vector<Shipping> spedizioni_assegnate;
};
