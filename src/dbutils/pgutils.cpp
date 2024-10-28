#include "pgutils.hpp"


/**
 * Create the connection to the database
 * @param conn the connection object
 * @return the connection object
 */
pqxx::connection getConnection() {
    try {
        pqxx::connection conn ("dbname=ecommerce hostaddr=localhost user=ecommerce passowrd=");
        return conn;
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        throw e;
    }
}

/**
* Initialize the database
* @return 0 if success, -1 if error
*/
int initDB() {
    // Connect to the default database
    pqxx::connection conn = getConnection();


    // Temporary "work" object to do init querys
    pqxx::work w((conn));
    w.exec("CREATE USER ecommerce WITH PASSWORD");
    w.exec("CREATE DATABASE ecommerce");


    w.exec("CREATE TABLE users (id SERIAL PRIMARY KEY, \
                                cf VARCHAR(255) UNIQUE NOT NULL, \
                                name VARCHAR(255) NOT NULL, \
                                surname VARCHAR(255) NOT NULL, \
                                email VARCHAR(255) NOT NULL)");


    w.exec("CREATE TABLE customers (user INTEGER PRIMARY KEY references users(id), \
                                    ord INTEGER UNIQUE NOT NULL references orders(id))");

    w.exec("CREATE TABLE suppliers (user INTEGER PRIMARY KEY references users(id), \
                                    piva VARCHAR(255) UNIQUE NOT NULL)");

    w.exec("CREATE TABLE shippers (user INTEGER PRIMARY KEY references users(id), \
                                    piva VARCHAR(255) UNIQUE NOT NULL)");

    w.exec("CREATE TABLE orders (id SERIAL PRIMARY KEY, \
                                customer INTEGER NOT NULL references customers(user), \
                                shipping INTEGER NOT NULL references shippings(user), \
                                instant DATETIME NOT NULL)");

    w.exec("CREATE TABLE products (id SERIAL PRIMARY KEY, \
                                    name VARCHAR(255) NOT NULL, \
                                    description VARCHAR(255) NOT NULL, \
                                    price FLOAT NOT NULL, \
                                    stock INTEGER NOT NULL)");

    w.exec("CREATE TABLE shippings (id SERIAL PRIMARY KEY, \
                                    shipper INTEGER NOT NULL references shippers(user), \
                                    order INTEGER NOT NULL references orders(id), \
                                    handlingtime DATETIME NOT NULL, \
                                    state BOOLEAN NOT NULL)");

    w.exec("CREATE TABLE cart (id SERIAL PRIMARY KEY, \
                                customer INTEGER NOT NULL references customers(user), \
                                product INTEGER NOT NULL references products(id), \
                                quantity INTEGER NOT NULL)");



    return 0;
}


