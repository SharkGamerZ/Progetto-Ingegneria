#include "utils.hpp"
#include "User.hpp"
#include "../dbutils/pgutils.hpp"

class Shipper : public User {

    public:
    string P_IVA;
    string ragione_sociale;
    string sede;            //sarebbe l'indirizzo (da aggiustare)
    vector<Shipping> spedizioni_assegnate;

    Shipper(int ID,string CF, string name, string surname, string email) {
        this->ID = ID;
        this->CF = CF;
        this->name = name;
        this->surname = surname;
        this->email = email;
    }

    std::vector<Shipping> getShippings();

    std::vector<Shipping> getActiveShippings(pqxx::connection& conn);

    static Shipper trasportatore_disponibile(pqxx::connection &conn);

    static void updateShipping(Shipping old_sped, Shipping new_sped);

    static void newShipping(Order o, pqxx::connection &conn);

    static void assignUnassignedOrders(pqxx::connection &conn);
};
