#include "main.hpp"
#include <cstdio>
#include <string>
#include <vector>
#include <unistd.h>
#include <termios.h>

void populateDB(int n) {
    std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");


    cout<<"[INFO]Populating Users"<<endl;
    // Riempimento Users
    vector<string> names = getRandomNames(n);
    vector<string> surnames = getRandomSurnames(n);
    for (int i = 0; i < n; i++) {
        pqxx::work w(*conn);
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

            User u = Customer(i+1, names[i] + surnames[i] + to_string(i), names[i], surnames[i], email);
            users.push_back(u);


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
            pqxx::work w(*conn);
            try {
                string query;
                // Customer
                if (ruolo == 0) {
                    query = "INSERT INTO " + ruoli[ruolo] + " (userID) VALUES ('" + 
                                                                        to_string(i+1) + "')";

                    Customer c = Customer(i+1, names[i] + surnames[i] + to_string(i), names[i], surnames[i], names[i] + surnames[i] + to_string(i) + "@GMAIL.COM");
                    customers.push_back(c);
                }

                // Shipper
                if (ruolo == 1) {
                    string piva = to_string(rand()%90000000000 + 10000000000);
                    query = "INSERT INTO " + ruoli[ruolo] + " (userID, piva) VALUES ('" + 
                                                                        to_string(i+1) + "', '" + 
                                                                        piva + "')";
                    Shipper s = Shipper(i+1, names[i] + surnames[i] + to_string(i), names[i], surnames[i], names[i] + surnames[i] + to_string(i) + "@GMAIL.COM", piva));
                    shippers.push_back(s);
                }

                // Supplier
                if (ruolo == 2) {
                    string piva = to_string(rand()%90000000000 + 10000000000);
                    query = "INSERT INTO " + ruoli[ruolo] + " (userID, piva) VALUES ('" + 
                                                                        to_string(i+1) + "', '" + 
                                                                        piva + "')";

                    Supplier s = Supplier(i+1, names[i] + surnames[i] + to_string(i), names[i], surnames[i], names[i] + surnames[i] + to_string(i) + "@GMAIL.COM", piva);
                    suppliers.push_back(s);
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

        pqxx::work w(*conn);
    }



    cout<<"[INFO]Populating Products"<<endl;
    // Riempimento products
    vector<string> productNames = getRandomProductNames(n);
    vector<string> adjectives = getRandomAdjectives(n);

    for (int i = 0; i < n; i++) {
        if (suppliers.size() == 0) break;
        pqxx::work w(*conn);
        try {
            double price =  (rand() % 20000) / 100.0; 
            int supplierID = suppliers[rand()%suppliers.size()].ID;
            int stock = rand()%100-1;
            string query = "INSERT INTO products (name, description, supplier, price, stock) VALUES ('" + 
                                        adjectives[i] + " " + productNames[i] + "', '" + 
                                        "Lorem Ipsum" + "', " + 
                                        to_string(supplierID) + ", " +
                                        to_string(price) + ", " +
                                        to_string(stock) + ")";

            w.exec(query);

            Product p = Product(i+1, supplierID, adjectives[i] + " " + productNames[i], "Lorem Ipsum", price, stock);
            products.push_back(p);

        } catch (const std::exception &e) {
            cerr << e.what() << endl;
        }
        w.commit();
    }



    cout<<"[INFO]Populating Orders"<<endl;
    for (int i = 0; i < n; i++) {
        if (customers.size() == 0) break;
        pqxx::work w(*conn);

        time_t start = 1388530800;  // 1 Gennaio 2014
        time_t end = 1704067200;    // 1 Gennaio 2024
        time_t time = start + rand()%(end-start);
        tm* tm = localtime(&time);
        stringstream randomTime;
        randomTime << put_time(tm, "%Y-%m-%d %H:%M:%S");
        try {
            int customerID = customers[rand()%customers.size()].ID;
            string query = "INSERT INTO orders (customer, instant) VALUES ('" + 
                                        to_string(customerID) + "', '" + 
                                        randomTime.str() + "')";

            w.exec(query);

            
            Order o = Order(i+1, customerID, time);
            orders.push_back(o);

        } catch (const std::exception &e) {
            continue;
            cerr << e.what() << endl;
        }
        w.commit();
    }



    cout<<"[INFO]Populating OrderProducts"<<endl;
    // Riempimento orderProducts
    for (int i = 0; i < orders.size(); i++) {
        if (products.size() == 0) break;
        // Genera numero casuale di prodotti in un ordine
        int numProducts = rand()%10+1;
        for (int j = 0; j < numProducts; j++) {
            bool success = true;
            do {
                success = true;
                pqxx::work w(*conn);
                try {
                    Order o = orders[i];
                    Product p = products[rand()%products.size()];
                    int qty = rand()%10+1;
                    string query = "INSERT INTO orderProducts (orderID, product, quantity) VALUES ('" + 
                                                to_string(o.ID) + "', '" +
                                                to_string(p.ID) + "', '" + 
                                                to_string(qty) + "')";

                    w.exec(query);

                    orders[i].products.insert({p, qty});
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
    /*vector<int> shippers;*/
    /*try {*/
    /*    pqxx::work w(*conn);*/
    /*    for (auto [id] : w.query<int>("SELECT userID FROM shippers")) {*/
    /*        shippers.push_back(id);*/
    /*    }*/
    /*    w.commit();*/
    /*} catch (const std::exception &e) {*/
    /*    cerr << e.what() << endl;*/
    /*}*/

    for (int i = 0; i < orders.size(); i++) {
        if (shippers.size() == 0) break;
        bool success;
        do {
            success = true;

            int currOrder = orders[i].ID;
            Shipper s = shippers[rand()%shippers.size()];
            bool state = rand()%2;

            // Generate random timestamp
            time_t start = orders[i].orderTime; // Get order time to avoid shipping before order
            time_t end = 1704067200;    // 1 Gennaio 2024
            time_t time = start + rand()%(end-start);
            tm* tm = localtime(&time);
            stringstream randomTime;
            randomTime << put_time(tm, "%Y-%m-%d %H:%M:%S");

            pqxx::work w(*conn);

            try {
                string query = "INSERT INTO shippings (orderID, shipper, handlingtime, state) VALUES ('" + 
                                            to_string(currOrder) + "', '" +
                                            to_string(s.ID) + "', '" +
                                            randomTime.str() + "', '" +
                                            to_string(state) + "')";
                w.exec(query);

                Shipping s = Shipping(i+1, currOrder, s.ID, time, state);
                shippings.push_back(s);
            } catch (const std::exception &e) {
                continue;
                cerr << e.what() << endl;
                success = false;
            }
        w.commit();

        } while(!success);

    }

    // CONTINUARE COSI'
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
                pqxx::work w(*conn);
                try {
                    Product p = products[rand()%products.size()];
                    int qty = rand()%10+1;

                    string query = "INSERT INTO carts (customer, product, quantity) VALUES ('" + 
                                                to_string(customers[i]) + "', '" + 
                                                to_string(p.ID) + "', '" +
                                                to_string(qty) + "')";

                    w.exec(query);

                    customers[i].cart.insert({p, qty});

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



// Riscrizione della funzione getch
char getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return (buf);
}

void printMenu() {
    cout<<"Welcome to the E-Commerce Database"<<endl;
    cout<<"Make a choice:"<<endl;
    cout<<"1. Navigate DB"<<endl;
    cout<<"2. Populate DB"<<endl;
    cout<<"3. Test Functionalities"<<endl;
    cout<<"9. Exit"<<endl;

}


// Funzione per mostrare il menu con le checkbox
void displayChoiceMenu(const std::vector<std::string> &options, const std::vector<bool> &selected, int current) {
    std::system("clear || cls"); // Pulisce lo schermo (Linux: clear, Windows: cls)
    std::cout << "Usa frecce SU/GIU per navigare, SPAZIO per selezionare, INVIO per confermare:\n\n";
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << (i == current ? " > " : "   "); // Indicatore della posizione corrente
        std::cout << (selected[i] ? "[X] " : "[ ] ") << options[i] << "\n";
    }
}


void chooseTestOptions(int n) {
    std::vector<std::string> options = {"addProductToCart", "deleteProductFromCart", "buyCart", "getPastOrders"};
    std::vector<bool> selected(options.size(), false);
    int current = 0;

    while (true) {
        displayChoiceMenu(options, selected, current);
        int ch = getch(); 

        if (ch == '\033') { // Sequenza di escape (tasti freccia)
            getch();        // Ignora '['
            switch (getch()) {
                case 'A': // Freccia SU
                    current = (current == 0 ? options.size() - 1 : current - 1);
                    break;
                case 'B': // Freccia GIU
                    current = (current == options.size() - 1 ? 0 : current + 1);
                    break;
            }
        } else if (ch == ' ') { // Spazio per selezionare/deselezionare
            selected[current] = !selected[current];
        } else if (ch == '\n' || ch == '\r') { // Invio per confermare
            break;
        }

    }

    testCustomer(selected, n);
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
                cout<<"Press any key to continue..."<<flush;
                getch();
                break;
            case 3:
                cout<<"Testing Functionalities"<<endl;
                cout<<"Choose the number of elements to test"<<endl;
                cin>>n;
                chooseTestOptions(n);
                cout<<"Press any key to continue..."<<flush;
                getch();
                break;
            case 9:
                cout<<"Exiting"<<endl;
                cout<<"Press any key to continue..."<<flush;
                getch();
                break;
            default:
                cout<<"Invalid choice"<<endl;
        }
    } while(choice != 9);

}


