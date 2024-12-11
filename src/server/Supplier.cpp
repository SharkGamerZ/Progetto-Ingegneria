#include "Supplier.hpp"

Supplier::Supplier(int ID, string CF, string name, string surname, string email, string P_IVA) {
    this->ID = ID;
    this->CF = CF;
    this->name = name;
    this->surname = surname;
    this->email = email;
    this->P_IVA = P_IVA;
}

void Supplier::addProduct(int ID, int q) {
    bool exist = false; // TODO query per controllare se l'articolo è già presente
    if (!exist) {
        // TODO query per aggiungere l'articolo
    }
}

void Supplier::setDiscontinuedProduct(int ID) {
    // TODO
}

vector<Product> Supplier::getPastOrders() {
    // TODO
    return vector<Product>();
}
