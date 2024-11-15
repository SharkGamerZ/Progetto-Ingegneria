#include "generator.hpp"
#include <cstdlib>


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
