#include "main.hpp"
#include <cstdio>
#include <string>
#include <vector>


void populateDB(int n) {
    pqxx::connection conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");


    // Riempimento Users
    vector<string> names = getRandomNames(n);
    vector<string> surnames = getRandomSurnames(n);

    for (int i = 0; i < n; i++) {
        pqxx::work w(conn);
        try {
            string email = names[i] + surnames[i] + to_string(rand()%100) + "@GMAIL.COM";
            replace(email.begin(), email.end(), ' ', '_');
            // TODO creare il cf in modo corretto
            string query = "INSERT INTO users (cf, name, surname, email) VALUES ('"+ 
                                        names[i] + surnames[i] + to_string(i) + "','" + 
                                        names[i] + "', '" + 
                                        surnames[i] + "','" + 
                                        email + "')";

            w.exec(query);
        } catch (const std::exception &e) {
            /*cerr << e.what() << endl;*/
        }

        w.commit();
    }


    // Riempimento customers, shippers, suppliers
    string ruoli[3] = {"customers", "shippers", "suppliers"};
    for (int i = 0; i < n; i++) {
        pqxx::work w(conn);
        int ruolo = rand() % 3;
        try {
            string query;
            if (ruolo == 0) {
                query = "INSERT INTO " + ruoli[ruolo] + " (userID) VALUES ('" + 
                                                                    to_string(i+1) + "')";
            }
            // Inserimento con piva random
            else {
                query = "INSERT INTO " + ruoli[ruolo] + " (userID, piva) VALUES ('" + 
                                                                    to_string(i+1) + "', '" + 
                                                                    to_string(rand()%100000000000) + "')";
            }
            

            w.exec(query);
        } catch (const std::exception &e) {

            // Se ci sono duplicati, non fare nulla
            /*cerr << e.what() << endl;*/
        }
        w.commit();
    }



    // Riempimento products
    vector<string> productNames = getRandomProductNames(n);
    vector<string> adjectives = getRandomAdjectives(n);
    for (int i = 0; i < n; i++) {
        pqxx::work w(conn);
        try {
            double price =  (rand() % 20000) / 100.0; 
            string query = "INSERT INTO products (name, description, price, stock) VALUES ('" + 
                                        adjectives[i] + " " + productNames[i] + "', '" + 
                                        "Lorem Ipsum" + "', " + 
                                        to_string(price) + ", " +
                                        to_string(rand()%100-1) + ")";

            w.exec(query);
        } catch (const std::exception &e) {
            cerr << e.what() << endl;
        }
        w.commit();
    }



    // Riempimento orders
    vector<int> customers;
    try {
        pqxx::work w(conn);
        for (auto [id] : w.query<int>("SELECT userID FROM customers")) {
            customers.push_back(id);
        }
        w.commit();
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
    }

    for (int i = 0; i < n; i++) {
        pqxx::work w(conn);

        time_t start = 1388530800;  // 1 Gennaio 2014
        time_t end = 1704067200;    // 1 Gennaio 2024
        time_t time = start + rand()%(end-start);
        tm* tm = localtime(&time);
        stringstream randomTime;
        randomTime << put_time(tm, "%Y-%m-%d %H:%M:%S");
        try {
            string query = "INSERT INTO orders (customer, instant) VALUES ('" + 
                                        to_string(customers[rand()%customers.size()]) + "', '" + 
                                        randomTime.str() + "')";

            w.exec(query);
        } catch (const std::exception &e) {
            cerr << e.what() << endl;
        }
        w.commit();
    }



    // Riempimento orderProducts
    vector<int> products;
    try {
        pqxx::work w(conn);
        for (auto [id] : w.query<int>("SELECT id FROM products")) {
            products.push_back(id);
        }
        w.commit();
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
    }

    vector<int> orders;
    try {
        pqxx::work w(conn);
        for (auto [id] : w.query<int>("SELECT id FROM orders")) {
            orders.push_back(id);
        }
        w.commit();
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
    }

    for (int i = 0; i < orders.size(); i++) {
        // Genera numero casuale di prodotti in un ordine
        int numProducts = rand()%10+1;
        for (int j = 0; j < numProducts; j++) {
            bool success = true;
            do {
                success = true;
                pqxx::work w(conn);
                try {
                    string query = "INSERT INTO orderProducts (orderID, product, quantity) VALUES ('" + 
                                                to_string(orders[i]) + "', '" + 
                                                to_string(products[rand()%products.size()]) + "', '" + 
                                                to_string(rand()%10+1) + "')";

                    w.exec(query);
                } catch (const std::exception &e) {
                    /*cerr << e.what() << endl;*/
                    success = false;
                }
            w.commit();
            } while(!success);
        }

    }



    // Riempimento shippings
    vector<int> shippers;
    try {
        pqxx::work w(conn);
        for (auto [id] : w.query<int>("SELECT userID FROM shippers")) {
            shippers.push_back(id);
        }
        w.commit();
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
    }

    for (int i = 0; i < orders.size(); i++) {
        

        bool success;
        do {
            success = true;

            // Generate random timestamp
            time_t start = 1388530800;  // 1 Gennaio 2014
            time_t end = 1704067200;    // 1 Gennaio 2024
            time_t time = start + rand()%(end-start);
            tm* tm = localtime(&time);
            stringstream randomTime;
            randomTime << put_time(tm, "%Y-%m-%d %H:%M:%S");

            cout<<randomTime.str()<<endl;

            pqxx::work w(conn);

            try {
                string query = "INSERT INTO shippings (orderID, shipper, handlingtime, state) VALUES ('" + 
                                            to_string(orders[i]) + "', '" +
                                            to_string(shippers[rand()%shippers.size()]) + "', '" +
                                            randomTime.str() + "', '" +
                                            to_string(rand()%2) + "')";
                w.exec(query);
            } catch (const std::exception &e) {
                cerr << e.what() << endl;
                success = false;
            }
        w.commit();

        } while(!success);
                

    }


    // Riempimento Carts
    for (int i = 0; i < customers.size(); i++) {
        // Genera numero casuale di prodotti in un carrello
        int numProducts = rand()%10;
        for (int j = 0; j < numProducts; j++) {
            bool success = true;
            do {
                success = true;
                pqxx::work w(conn);
                try {
                    string query = "INSERT INTO carts (customer, product, quantity) VALUES ('" + 
                                                to_string(customers[i]) + "', '" + 
                                                to_string(products[rand()%products.size()]) + "', '" + 
                                                to_string(rand()%10+1) + "')";

                    w.exec(query);
                } catch (const std::exception &e) {
                    cerr << e.what() << endl;
                    success = false;
                }
            w.commit();
            } while(!success);
        }   
    }


}


void testCustomer() {
    string query = "SELECT cf, name, surname, email FROM users";

    pqxx::connection conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");
    pqxx::work w(conn);

    
    for (auto [cf, name, surname, email] : w.query<string, string, string, string>(query)) {
        /*cout << setw(30) << cf << ":" << name << " " << surname << " " << email << endl;*/
    }
    pqxx::result r = w.exec(query);
    w.commit();
 }

int main() {
    cout<<"[INFO]Initializing DB"<<endl;
    initDB();


    cout<<"[INFO]Populating DB"<<endl;
    populateDB(1000);


    cout<<"[INFO]Testing Customers"<<endl;
    testCustomer();


    system("psql -h localhost -U ecommerce -d ecommerce ");
}


