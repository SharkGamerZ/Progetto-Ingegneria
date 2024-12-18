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
    RedisCache cache;
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
        logError(e.what());
        throw e;
    }

}

void Supplier::addProduct(string name, string des, float price, int stock) {
    RedisCache cache;
    DataService redis = DataService(cache);
    unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");
    pqxx::result res;

    try {
        // Inserts a new row in the DB
        pqxx::work w(*conn);
        res = w.exec("INSERT INTO products (name, description, supplier, price, stock) \
            VALUES ('"+name+"', \
                '"+des+"', \
                "+to_string(ID)+", \
                "+to_string(price)+", \
                "+to_string(stock)+") RETURNING id");
    }
    catch (const exception &e) {
        logError(e.what());
        throw e;
    }

    vector<string> values;
    // Create the array of string columns
    values.insert(values.end(), name);
    values.insert(values.end(), des);
    values.insert(values.end(), to_string(ID));
    values.insert(values.end(), to_string(price));
    values.insert(values.end(), to_string(stock));

    // Sets the data in the cache
    redis.setData("products", res[0][0].c_str(), values);
}

void Supplier::setDiscontinuedProduct(int ID) {
    // Initialize the cache object and passes it to the DataService constructor
    RedisCache cache;
    DataService redis(cache);
    vector<string> data;
    
    data = redis.getData("products", to_string(ID));

    // Checking if the ID exist (in the cache or DB since getData searches in both cache and then DB)
    if (data.empty()) {
        cout << "[ERROR] ID of product doesn't exist.";
        return;
    }

    // Set the product to discontinued (quantity<0)
    data.back() = "-1";
    // Setting the new value in redis
    redis.setData("products", to_string(ID), data);

    unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");
    
    // Setting the value in the DB
    try {
        pqxx::work w(*conn);
        w.exec("UPDATE products SET stock = " + data.back() + " WHERE id = " + to_string(ID));
    } 
    catch (const exception &e) {
        logError(e.what());
        throw e;
    }

}

vector<vector<string>> Supplier::getSoldProducts() {
    vector<vector<string>> soldProducts;

    unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

    // Selecting all the sold products from the DB
    try {
        pqxx::work w(*conn);
        pqxx::result res = w.exec("SELECT o.customer, p.name, op.quantity, o.instant \
            FROM suppliers s JOIN products p ON s.userID = p.supplier \
            JOIN orderProducts op ON p.id = op.product \
            JOIN orders o ON o.id = op.orderID \
            WHERE s.userID = " + to_string(ID) + " \
            ORDER BY o.instant DESC");

        // Filling the vector of vector containing the fields
        for (int i = 0; i < res.size(); i++) {
            vector<string> soldProduct;
            for (pqxx::field field : res[i]) {
                soldProduct.insert(soldProduct.end(), field.c_str());
            }
            soldProducts.insert(soldProducts.end(), soldProduct);
        }
    }
    catch (const exception &e) {
        logError(e.what());
        throw e;
    }
    return soldProducts;
}
