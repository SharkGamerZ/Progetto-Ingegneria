#include "main.hpp"
#include <cstdio>
#include <string>
#include <vector>


void populateDB(int n) {
    pqxx::connection conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");


    cout<<"[INFO]Populating Users"<<endl;
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
            continue;
            cerr << e.what() << endl;
        }

        w.commit();
    }


    cout<<"[INFO]Populating Customers, Shippers, Suppliers"<<endl;
    // Riempimento customers, shippers, suppliers
    string ruoli[3] = {"customers", "shippers", "suppliers"};
    for (int i = 0; i < n; i++) {
        int ruolo = rand() % 3;

        bool success = true;
        do {
            success = true;
            pqxx::work w(conn);
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
                                                                        to_string(rand()%90000000000 + 10000000000) + "')";
                }
                w.exec(query);
            } catch (const std::exception &e) {
                // Se gia' esiste quella partita iva, riprova
                continue;
                cerr << e.what() << endl;
                success = false;
            }
            w.commit();
        } while(!success);

        pqxx::work w(conn);
    }



    cout<<"[INFO]Populating Products"<<endl;
    // Riempimento products
    vector<string> productNames = getRandomProductNames(n);
    vector<string> adjectives = getRandomAdjectives(n);

    vector<int> suppliers;
    try {
        pqxx::work w(conn);
        for (auto [id] : w.query<int>("SELECT userID FROM suppliers")) {
            suppliers.push_back(id);
        }
        w.commit();
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
    }
    for (int i = 0; i < n; i++) {
        if (suppliers.size() == 0) break;
        pqxx::work w(conn);
        try {
            double price =  (rand() % 20000) / 100.0; 
            string query = "INSERT INTO products (name, description, supplier, price, stock) VALUES ('" + 
                                        adjectives[i] + " " + productNames[i] + "', '" + 
                                        "Lorem Ipsum" + "', " + 
                                        to_string(suppliers[rand()%suppliers.size()]) + ", " +
                                        to_string(price) + ", " +
                                        to_string(rand()%100-1) + ")";

            w.exec(query);
        } catch (const std::exception &e) {
            cerr << e.what() << endl;
        }
        w.commit();
    }



    cout<<"[INFO]Populating Orders"<<endl;
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
        if (customers.size() == 0) break;

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
            continue;
            cerr << e.what() << endl;
        }
        w.commit();
    }



    cout<<"[INFO]Populating OrderProducts"<<endl;
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

    vector<tuple<int, string>> orders;
    try {
        pqxx::work w(conn);
        for (auto [id, instant] : w.query<int, string>("SELECT id, instant FROM orders")) {
            orders.push_back(make_tuple(id, instant));
        }
        w.commit();
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
    }


    for (int i = 0; i < orders.size(); i++) {
        if (products.size() == 0) break;
        // Genera numero casuale di prodotti in un ordine
        int numProducts = rand()%10+1;
        for (int j = 0; j < numProducts; j++) {
            bool success = true;
            do {
                success = true;
                pqxx::work w(conn);
                try {
                    string query = "INSERT INTO orderProducts (orderID, product, quantity) VALUES ('" + 
                                                to_string(get<0>(orders[i])) + "', '" +
                                                to_string(products[rand()%products.size()]) + "', '" + 
                                                to_string(rand()%10+1) + "')";

                    w.exec(query);
                } catch (const std::exception &e) {
                    continue;
                    cerr << e.what() << endl;
                    success = false;
                }
            w.commit();
            } while(!success);
        }

    }



    cout<<"[INFO]Populating Shippings"<<endl;
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
        if (shippers.size() == 0) break;
        bool success;
        do {
            success = true;

            int currOrder = get<0>(orders[i]);
            string currInstant = get<1>(orders[i]);
            struct tm tm1 = {};
            strptime(currInstant.c_str(), "%Y-%m-%d %H:%M:%S", &tm1);


            // Generate random timestamp
            time_t start = mktime(&tm1);  // 1 Gennaio 2014
            time_t end = 1704067200;    // 1 Gennaio 2024
            time_t time = start + rand()%(end-start);
            tm* tm = localtime(&time);
            stringstream randomTime;
            randomTime << put_time(tm, "%Y-%m-%d %H:%M:%S");

            pqxx::work w(conn);

            try {
                string query = "INSERT INTO shippings (orderID, shipper, handlingtime, state) VALUES ('" + 
                                            to_string(get<0>(orders[i])) + "', '" +
                                            to_string(shippers[rand()%shippers.size()]) + "', '" +
                                            randomTime.str() + "', '" +
                                            to_string(rand()%2) + "')";
                w.exec(query);
            } catch (const std::exception &e) {
                continue;
                cerr << e.what() << endl;
                success = false;
            }
        w.commit();

        } while(!success);

    }


    cout<<"[INFO]Populating Carts"<<endl;
    // Riempimento Carts
    for (int i = 0; i < customers.size(); i++) {
        if (products.size() == 0) break;
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
                    continue;
                    /*cerr << e.what() << endl;*/
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

void printMenu() {
    cout<<"Welcome to the E-Commerce Database"<<endl;
    cout<<"Make a choice:"<<endl;
    cout<<"1. Navigate DB"<<endl;
    cout<<"2. Populate DB"<<endl;
    cout<<"3. Test Functionalities"<<endl;
    cout<<"9. Exit"<<endl;

}


int main() {
    cout<<"[INFO]Initializing DB"<<endl;
    initDB();


    int choice, n;
    do {
        system("clear");
        printMenu();
        cin>>choice;
        switch(choice) {
            case 1:
                system("clear");
                system("psql -h localhost -U ecommerce -d ecommerce ");
                break;
            case 2:
                cout<<"Choose the number of elements to populate"<<endl;
                cin>>n;
                populateDB(n);
                break;
            case 3:
                cout<<"Testing Functionalities"<<endl;
                cout<<"Choose the number of elements to test"<<endl;
                cin>>n;
                testCustomer(n);
                break;
            case 9:
                cout<<"Exiting"<<endl;
                break;
            default:
                cout<<"Invalid choice"<<endl;
        }
    } while(choice != 9);

}


