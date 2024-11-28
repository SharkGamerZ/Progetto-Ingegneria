#include "Supplier.hpp"

Supplier::Supplier(string P_IVA, string email, string pw, vector<int> ID_wh, int ID, string CF, string name, string surname) {
        this->P_IVA = P_IVA;
        this->email = email;
        this->pw = pw;
        this->ID_wh = ID_wh;
        this->CF = CF;
        this->name = name;
        this->surname = surname;
    }

void Supplier::addProduct(Product a, int q) {
    bool exist = false; // TODO query per controllare se l'articolo è già presente
    if (!exist) {
        // TODO query per aggiungere l'articolo
    }
}

void Supplier::setDiscontinuedProduct(Product a) {
    // TODO
}

vector<Product> Supplier::getPastOrders() {
    // TODO
    return vector<Product>();
}
