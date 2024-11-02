#include "main.hpp"
#include <cstdio>


void populateDB() {
    pqxx::connection conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");
    pqxx::work w(conn);

    w.exec("INSERT INTO users (cf, name, surname, email) VALUES ('MRCMS','Mario', 'Rossi','mail' )");
    w.exec("INSERT INTO users (cf, name, surname, email) VALUES ('LGV','Luigi', 'Verdi','mail' )");
    w.exec("INSERT INTO users (cf, name, surname, email) VALUES ('GBNC','Giovanni', 'Bianchi','mail' )");

    w.commit();
}


void testCustomer() {
    string query = "SELECT name,surname,id FROM users";

    pqxx::connection conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");
    pqxx::work w(conn);

    
    for (auto [name, surname, id] : w.query<string, string, int>(query)) {
        cout << name << " " << surname << " " << id << endl;
    }
    pqxx::result r = w.exec(query);
    w.commit();
 }

int main() {
    cout<<"Initializing DB"<<endl;
    initDB();


    cout<<"Populating DB"<<endl;
    populateDB();


    cout<<"Testing Customers"<<endl;
    testCustomer();


}


