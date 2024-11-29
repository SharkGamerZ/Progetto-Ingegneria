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






void testCustomer(std::vector<bool> selected, int n, vector<int> customersID, vector<int> suppliersID, vector<int> productIDs) {
    std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

    RedisCache cache;
    DataService ds(cache);
    // addProductToCart test
    if (selected[0]) {
        cout<<"[INFO]Testing addProductToCart"<<endl;
        for (int i = 0; i < n; i++) {
            if (customersID.size() == 0) break;
            int randomCustomerID = customersID[rand() % customersID.size()];

            // Costruisco l'oggetto Customer
            Customer c;
            c.ID = randomCustomerID; 
            /*c.cart = ds.getCart(c.ID); // TODO aspettare Thomas*/

            for (int j = 0; j < 5; j++) {
                int randomProductID = productIDs[rand() % productIDs.size()];
                c.addProductToCart(randomProductID, rand() % 10 + 1);
            }
        }
    }


    // removeProductFromCart test
    if (selected[1]) {
        cout<<"[INFO]Testing removeProductFromCart"<<endl;
        for (int i = 0; i < n; i++) {
            if (customersID.size() == 0) break;
            int randomCustomerID = customersID[rand() % customersID.size()];

            // Costruisco l'oggetto Customer
            Customer c;
            c.ID = randomCustomerID;
            /*c.cart = ds.getCart(c.ID); // TODO aspettare Thomas*/
            for (int j = 0; j < 5; j++) {
                int randomProductID = productIDs[rand() % productIDs.size()];
                c.removeProductFromCart(randomProductID, rand() % 5 + 1);
            }
        }
    }


    // buyCart test
    if (selected[2]) {
        cout<<"[INFO]Testing buyCart"<<endl;
        for (int i = 0; i < n; i++) {
            if (customersID.size() == 0) break;
            int randomCustomerID = customersID[rand() % customersID.size()];

            Customer c;
            c.ID = randomCustomerID;
            /*c.cart = ds.getCart(c.ID); // TODO aspettare Thomas*/

            c.buyCart();
        }
    }
}
