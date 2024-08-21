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
#endif