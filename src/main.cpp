#include "main.hpp"
#include <cstdio>
#include <string>
#include <vector>


void populateDB(int n) {
    pqxx::connection conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");
    pqxx::work w(conn);

    vector<string> names = getRandomNames(n);
    vector<string> surnames = getRandomSurnames(n);

    for (int i = 0; i < n; i++) {
        string query = "INSERT INTO users (cf, name, surname, email) VALUES ('" + to_string(i) + "','" + names[i] + "', '" + surnames[i] + "','mail' )";

        w.exec(query);
    }


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
    cout<<"[INFO]Initializing DB"<<endl;
    initDB();


    cout<<"[INFO]Populating DB"<<endl;
    populateDB(50000);


    cout<<"[INFO]Testing Customers"<<endl;
    testCustomer();


}


