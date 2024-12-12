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
    
    data = redis.getData("products", to_string(ID));

    // Checking if the ID exist (in the cache or DB since the cache is always updated)
    if (data.empty()) {
        cout << "[ERROR] ID of product doesn't exist.";
        return;
    }

    // Updating the value
    data.back() = to_string(stoi(data.back()) + q);
    // Setting the new value in redis
    redis.setData("products", to_string(ID), data);

    unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");
    
    // Setting the value in the DB
    try {
        pqxx::work w(*conn);
        w.exec("UPDATE products SET stock = " + data.back() + " WHERE id = " + to_string(ID));
    } 
    catch (const exception &e) {
        cerr << e.what() << endl;
        throw e;
    }
}

void Supplier::addProduct(string name, string des, int supplier, int price, int stock) {

}

void Supplier::setDiscontinuedProduct(int ID) {
    // TODO
}

vector<int> Supplier::getPastOrders() {
    // TODO
    return vector<int>();
}
