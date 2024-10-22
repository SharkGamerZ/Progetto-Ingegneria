#include "utils.hpp"
#pragma once



class User {
public:
    int ID;
    string CF;
    string name;
    string surname;
    string email;

    ofstream *file;

    
    /**
    * Funzione che ritorna il file sul quale 
    * vengono fatti tutti i log.
    */
    void getLogFile();



    /**
    * Funzione che permette e registra quando
    * un utente fa il login
    */
    void login();

    /**
    * Funzione che permette e registra quando
    * un utente esce dalla piattaforma
    */
    void logout();
};
