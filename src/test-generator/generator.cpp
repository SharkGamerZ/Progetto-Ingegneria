#include "generator.hpp"
#include <cstdlib>
#include <string>





vector<string> getRandomNames(int n) {
    srand(time(NULL));

    vector<string> names;

    // Reads from files and gets random names
    ifstream file("../src/test-generator/names.txt");
    string name;
    while (getline(file, name)) {
        names.push_back(name);
    }

    vector<string> randomNames;
    for (int i = 0; i < n; i++) {
        int randomIndex = rand() % names.size();
        randomNames.push_back(names[randomIndex]);
    }

    return randomNames;
}


vector<string> getRandomSurnames(int n) {
    srand(time(NULL));

    vector<string> surnames;

    // Reads from files and gets random surnames 
    ifstream file("../src/test-generator/surnames.txt");
    string surname;
    while (getline(file, surname)) {
        surnames.push_back(surname);
    }


    vector<string> randomSurnames;
    for (int i = 0; i < n; i++) {
        int randomIndex = rand() % surnames.size();
        randomSurnames.push_back(surnames[randomIndex]);
    }

    return randomSurnames;
}


vector<string> getRandomCities(int n) {
    srand(time(NULL));

    vector<string> cities;

    // Reads from files and gets random names
    ifstream file("../src/test-generator/cities.txt");
    string city;
    while (getline(file, city)) {
        cities.push_back(city);
    }


    vector<string> randomCities;
    for (int i = 0; i < n; i++) {
        int randomIndex = rand() % cities.size();
        randomCities.push_back(cities[randomIndex]);
    }

    return randomCities;
}


vector<string> getRandomProducts(int n) {
    srand(time(NULL));

    vector<string> products;

    // Reads from files and gets random names
    ifstream file("../src/test-generator/products.txt");
    string product;
    while (getline(file, product)) {
        products.push_back(product);
    }


    vector<string> randomProducts;
    for (int i = 0; i < n; i++) {
        int randomIndex = rand() % products.size();
        randomProducts.push_back(products[randomIndex]);
    }

    return randomProducts;
}


vector<string> getRandomProductNames(int n) {
    srand(time(NULL));

    vector<string> productNames;

    // Reads from files and gets random names
    ifstream file("../src/test-generator/products.txt");
    string productName;
    while (getline(file, productName)) {
        productNames.push_back(productName);
    }
    
    vector<string> randomProductNames;
    for (int i = 0; i < n; i++) {
        int randomIndex = rand() % productNames.size();
        randomProductNames.push_back(productNames[randomIndex]);
    }

    return randomProductNames;
}


vector<string> getRandomAdjectives(int n) {
    srand(time(NULL));

    vector<string> adjectives;

    // Reads from files and gets random names
    ifstream file("../src/test-generator/adjectives.txt");
    string adjective;
    while (getline(file, adjective)) {
        adjectives.push_back(adjective);
    }

    vector<string> randomAdjectives;
    for (int i = 0; i < n; i++) {
        int randomIndex = rand() % adjectives.size();
        randomAdjectives.push_back(adjectives[randomIndex]);
    }

    return randomAdjectives;
}






void testCustomer(std::vector<bool> selected, int n) {
    std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

    // Ci prendiamo customers e products
    cout<<"[INFO]Getting Customers"<<endl;
    try {
        pqxx::work w(*conn);
        for (auto [id, CF, name, surname, email] : w.query<int, string, string, string, string>("SELECT id, CF, name, surname, email FROM customers, users WHERE users.id=customers.userID")) {
            customers.push_back(Customer(id, CF, name, surname, email));
        }
        w.commit();
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
    }

    cout<<"[INFO]Getting Products"<<endl;
    try {
        pqxx::work w(*conn);
        for (auto [id, supplierID, name, description, price, stock] : w.query<int, int, string, string, double, int>("SELECT id, supplier, name, description, price, stock FROM products")) {
            products.push_back(Product(id, supplierID, name, description, price, stock));
        }
        w.commit();
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
    }


    // addProductToCart test
    if (selected[0]) {
        cout<<"[INFO]Testing addProductToCart"<<endl;
        for (int i = 0; i < n; i++) {
            if (customers.size() == 0) break;
            Customer c = customers[rand() % customers.size()];
            for (int j = 0; j < 5; j++) {
                Product p = products[rand() % products.size()];
                c.addProductToCart(p, rand() % 10 + 1);
            }
        }
    }


    // removeProductFromCart test
    if (selected[1]) {
        cout<<"[INFO]Testing removeProductFromCart"<<endl;
        for (int i = 0; i < n; i++) {
            if (customers.size() == 0) break;
            Customer c = customers[rand() % customers.size()];
            for (int j = 0; j < 5; j++) {
                Product p = products[rand() % products.size()];
                c.removeProductFromCart(p, rand() % 5 + 1);
            }
        }
    }


    // buyCart test
    if (selected[2]) {
        cout<<"[INFO]Testing buyCart"<<endl;
        for (int i = 0; i < n; i++) {
            if (customers.size() == 0) break;
            Customer c = customers[rand() % customers.size()];
            c.buyCart();
        }
    }
}
