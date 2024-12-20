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
            cout<<"[INFO]Customer ID: "<<c.ID<<endl;
            c.cart = ds.getCart(to_string(c.ID)); 
            cout<<"[INFO]Cart: "<<c.cart.size()<<endl;

            for (int j = 0; j < 5; j++) {
                int randomProductID = productIDs[rand() % productIDs.size()];
                c.addProductToCart(randomProductID, rand() % 9 + 1);
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
            c.cart = ds.getCart(to_string(c.ID)); 
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
            c.cart = ds.getCart(to_string(c.ID)); 

            c.buyCart();
        }
    }


    // getPastOrders test
    if (selected[3]) {
        cout<<"[INFO]Testing getPastOrders"<<endl;
        for (int i = 0; i < n; i++) {
            if (customersID.size() == 0) break;
            int randomCustomerID = customersID[rand() % customersID.size()];

            Customer c;
            c.ID = randomCustomerID;
            vector<Order> orders = c.getPastOrders();

            // Itera sugli ordini e stampa i dettagli
            for (const auto& order : orders) { cout << "Order Details:" << endl;
                cout << "  Order ID: " << order.ID << endl;
                cout << "  Customer ID: " << order.customerID << endl;
                cout << "  Order Time: " << put_time(localtime(&order.orderTime), "%Y-%m-%d %H:%M:%S") << endl;

                cout << "  Products:" << endl;
                if (order.products.empty()) {
                    cout << "    No products in this order." << endl;
                } else {
                    for (const auto& [productID, quantity] : order.products) {
                        cout << "    Product ID: " << productID << " | Quantity: " << quantity << endl;
                    }
                }

                cout << "---------------------------------" << endl; // Separatore tra gli ordini
            }
        }


    }

}

void testShipper(std::vector<bool> selected, int n, vector<int> shipperIDs, vector<int> orderIDs) {
    // Connessione al database
    std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

    RedisCache cache;
    DataService ds(cache);

    // Test assignUnassignedOrders
    if (selected[0]) {
        std::cout << "[INFO] Testing assignUnassignedOrders" << std::endl;
        for (int i = 0; i < n; i++) {
            if (shipperIDs.empty()) break;

            int randomShipperID = shipperIDs[rand() % shipperIDs.size()];
            Shipper shipper;
            shipper.ID = randomShipperID;

            std::cout << "[INFO] Shipper ID: " << shipper.ID << std::endl;
            shipper.assignUnassignedOrders();
        }
    }

    // Test newShipping
    if (selected[1]) {
        std::cout << "[INFO] Testing newShipping" << std::endl;
        for (int i = 0; i < n; i++) {
            if (shipperIDs.empty() || orderIDs.empty()) break;

            int randomShipperID = shipperIDs[rand() % shipperIDs.size()];
            int randomOrderID = orderIDs[rand() % orderIDs.size()];

            Shipper shipper;
            shipper.ID = randomShipperID;

            std::cout << "[INFO] Shipper ID: " << shipper.ID << " creating shipping for Order ID: " << randomOrderID << std::endl;
            Shipper::newShipping(randomOrderID);
        }
    }

    // Test trasportatore_disponibile
    if (selected[2]) {
        std::cout << "[INFO] Testing trasportatore_disponibile" << std::endl;
        for (int i = 0; i < n; i++) {
            if (shipperIDs.empty()) break;

            int randomShipperID = shipperIDs[rand() % shipperIDs.size()];
            Shipper shipper;
            shipper.ID = randomShipperID;

            bool isAvailable = Shipper::trasportatore_disponibile();
            std::cout << "[INFO] Shipper ID: " << shipper.ID << " Availability: " << (isAvailable ? "Available" : "Not Available") << std::endl;
        }
    }

    // Test shippingDelivered
    if (selected[3]) {
        std::cout << "[INFO] Testing shippingDelivered" << std::endl;
        for (int i = 0; i < n; i++) {
            /*if (shipperIDs.empty()) break;*/

            int randomShippingID = rand() % 100 + 1; // Assumiamo che gli ID delle spedizioni siano in un certo intervallo
            std::cout << "[INFO] Marking Shipping ID " << randomShippingID << " as Delivered" << std::endl;

            Shipper::shippingDelivered(randomShippingID);
        }
    }

    // Test getActiveShippings
    if (selected[4]) {
        std::cout << "[INFO] Testing getActiveShippings" << std::endl;
        for (int i = 0; i < n; i++) {
            if (shipperIDs.empty()) break;

            int randomShipperID = shipperIDs[rand() % shipperIDs.size()];
            Shipper shipper;
            shipper.ID = randomShipperID;

            std::cout << "[INFO] Shipper ID: " << shipper.ID << " Active Shippings: " << std::endl;
            auto activeShippings = shipper.getActiveShippings();

            for (const auto& shipping : activeShippings) {
                std::cout << "Shipping ID: " << shipping << std::endl;
            }
        }
    }

    // Test getShippings
    if (selected[5]) {
        std::cout << "[INFO] Testing getShippings" << std::endl;
        for (int i = 0; i < n; i++) {
            if (shipperIDs.empty()) break;

            int randomShipperID = shipperIDs[rand() % shipperIDs.size()];
            Shipper shipper;
            shipper.ID = randomShipperID;

            std::cout << "[INFO] Shipper ID: " << shipper.ID << " All Shippings: " << std::endl;
            auto shippings = shipper.getShippings();

            for (const auto& shipping : shippings) {
                std::cout << "Shipping ID: " << shipping << std::endl;
            }
        }
    }
}

void testSupplier(vector<bool> selected, int n, vector<int> suppliersIDs, vector<int> productsIDs) {
    unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

    RedisCache cache;
    DataService redis(cache);

    // Test addStock
    if (selected[0]) {
        cout << "[INFO]Testing addStock" << endl;
        for (int i = 0; i < n; i++) {

            if (suppliersIDs.empty()) break;

            int randomSupplierID = suppliersIDs[rand() % suppliersIDs.size()];
            int randomProductID = productsIDs[rand() % productsIDs.size()];
            int randomQnt = rand() % 1000;


            Supplier supplier;
            supplier.ID = randomSupplierID;
            
            cout << right << setw(20) << "[INFO]Supplier ID: " << supplier.ID << endl; 
            cout << right << setw(20) << "Product ID: " << randomProductID << endl;
            cout << right << setw(20) << "Quantity: " << randomQnt << endl;
            supplier.addStock(randomProductID, randomQnt);

        }
    }

    // Test addProduct
    if (selected[1]) {
        cout << "[INFO]Testing addProduct" << endl;
        vector<string> productNames = getRandomProductNames(n);
        for (int i = 0; i < n; i++) {
            if (suppliersIDs.empty()) break;

            int randomSupplierID = suppliersIDs[rand() % suppliersIDs.size()];
            string des = "Lorem ipsum";
            float price = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
            int qnt = rand();

            Supplier supplier;
            supplier.ID = randomSupplierID;

            cout << right << setw(20) << "[INFO]Supplier ID: " << randomSupplierID << endl;
            cout << right << setw(20) << "Name: " << productNames[i] << endl;
            cout << right << setw(20) << "Descrption: " << des << endl;
            cout << right << setw(20) << "Price: " << price << endl;
            cout << right << setw(20) << "Quantity: " << qnt << endl;

            supplier.addProduct(productNames[i], des, price, qnt);
        }
    }

    // Test setDiscontinuedProduct
    if (selected[2]) {
        cout << "[INFO]Testing setDiscontinuedProduct" << endl;
        for (int i = 0; i < n; i++) {
            if (suppliersIDs.empty()) break;

            int randomSupplierID = suppliersIDs[rand() % suppliersIDs.size()];
            int randomProductID = productsIDs[rand() % productsIDs.size()];

            Supplier supplier;
            supplier.ID = randomSupplierID;

            cout << right << setw(20) << "[INFO]Supplier ID: " << randomSupplierID << endl;
            cout << right << setw(20) << "Product ID: " << randomProductID << endl;

            supplier.setDiscontinuedProduct(randomProductID);
        }
    }

    // Test getSoldProducts
    if (selected[3]) {
        cout << "[INFO]Testing getSoldProducts" << endl;
        for (int i = 0; i < n; i++) {
            if (suppliersIDs.empty()) break;

            int randomSupplierID = suppliersIDs[rand() % suppliersIDs.size()];

            Supplier supplier;
            supplier.ID = randomSupplierID;

            cout << "[INFO]Supplier ID: " << randomSupplierID << endl;
            vector<vector<string>> pastOrders = supplier.getSoldProducts();

            cout << left << setw(12) << "CustomerID" << setw(50) << "Product name" << setw(10) << "Quantity" << setw(19) << "Instant" << endl;
            for (vector<string> order : pastOrders) {
                cout << left << setw(12) << order[0] << setw(50) << order[1] << setw(10) << order[2] << setw(19) << order[3] << endl;            
            }

            // Magari printare il risultato
        }
    }
}
