#include "Supplier.hpp"

Supplier::Supplier(int ID, string CF, string name, string surname, string email, string P_IVA) {
    this->ID = ID;
    this->CF = CF;
    this->name = name;
    this->surname = surname;
    this->email = email;
    this->P_IVA = P_IVA;
}

void Supplier::addStock(int ID, int q) {
    RedisCache cache = RedisCache();
    DataService redis(cache);
    vector<string> data;
    //unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");
    
    data = redis.getData("products", to_string(ID));

    if (data.empty()) {
        cout << "[ERROR] ID of product doesn't exist.";
        return;
    }

    data.back() = stoi(data.back()) + q;

    redis.setData("products", to_string(ID), data);
}

void Supplier::setDiscontinuedProduct(int ID) {
    // TODO
}

vector<int> Supplier::getPastOrders() {
    // TODO
    return vector<int>();
}
