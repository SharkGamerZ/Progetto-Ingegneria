#include "utils.hpp"
#include "../dbutils/rdutils.hpp"
#include "../dbutils/pgutils.hpp"
#include <iostream>
#include <string>
#include <random>
#include <unordered_map>
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
    string login(string username) {
        RedisCache rc;

        // Controlla se lo username ha già un token
        string token = rc.get("", username);

        if (token.empty()) {
            // Genera un nuovo token
            token = generateToken();

            // Salva il nuovo token utilizzando la funzione set
            rc.set("", username, { token });
        }


        cout << "User " << username << " ha il seguente token: " << token << endl;

        return token;
    }


    /**
    * Funzione che permette e registra quando
    * un utente esce dalla piattaforma
    */
    void logout();

private:
    /**
    * Genera un token casuale
    */
    string generateToken() {
        static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        int len = 32; // Lunghezza del token
        string token;
        random_device rd;  //generatore di numeri casuali
        mt19937 gen(rd());  //generatore di numeri casuali
        uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);   //distributore uniforme per creare un token alfanumerico di len 32

        for (int i = 0; i < len; ++i) {
            token += alphanum[dis(gen)];
        }
        return token;
    }
};

