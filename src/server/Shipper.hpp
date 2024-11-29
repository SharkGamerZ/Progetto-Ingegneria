#pragma once
#include "utils.hpp"
#include "User.hpp"
#include "../dbutils/pgutils.hpp"

class Shipper : public User {

    public:
    string P_IVA;
    string ragione_sociale;
    string sede;            //sarebbe l'indirizzo (da aggiustare)
    vector<int> spedizioni_assegnate; //ShipmentIDs

    Shipper(int ID,string CF, string name, string surname, string email, string P_IVA, string ragione_sociale, string sede) {
        this->ID = ID;
        this->CF = CF;
        this->name = name;
        this->surname = surname;
        this->email = email;
        this->P_IVA = P_IVA;
        this->ragione_sociale = ragione_sociale;
        this->sede = sede;
    }

    // Both functions return the ids of the shippings
    std::vector<int> getShippings();
    std::vector<int> getActiveShippings();

    static Shipper trasportatore_disponibile();

    static void updateShipping(Shipping old_sped, Shipping new_sped);

    /*static void newShipping(Order o, pqxx::connection &conn);*/

};

void newShipping(int orderID);
void assignUnassignedOrders();
