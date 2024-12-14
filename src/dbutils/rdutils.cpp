#include "rdutils.hpp"

RedisCache::RedisCache() {
    string host = "127.0.0.1";
    int port = 6379;
    // Establish a connection to Redis
    context = redisConnect(host.c_str(), port);
    if (context == nullptr || context->err) {
        if (context) {
            cerr << "Connection error: " << context->errstr << endl;
            redisFree(context);
        } else {
            cerr << "Connection error: Can't allocate redis context" << endl;
        }
        exit(1);  // Exit on error
    }
}

RedisCache::~RedisCache() {
    if (context) {
        redisFree(context);
    }
}

void RedisCache::initCache() {
    // Enstablish connection to DB and load the products table
    unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

    try {
        pqxx::work w(*conn);
        string query = "SELECT * FROM products";
        for (auto [id, supplierID, name, description, price, stock] : w.query<string, string, string, string, string, string>(query)) {
            set("products", id, supplierID + "_" + name + "_" + description + "_" + price + "_" + stock);
        }
        w.commit();
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        throw e;
    }
}

void RedisCache::emptyCache() {
    redisReply* reply = (redisReply*) redisCommand(context, "FLUSHALL");
}

bool RedisCache::exist(const string& table, const string& ID) {
    string key = "";

    key.append(table);
    key.append(ID);

    redisReply* reply = (redisReply*)redisCommand(context, "EXISTS %s", key.c_str());
    if (!reply) {
        cerr << "Failed to execute EXISTS command" << endl;
        return false;
    }
    bool exists = reply->integer == 1;
    
    freeReplyObject(reply);
    return exists;
}

string RedisCache::get(const string& table, const string& ID) {
    string key = "";

    key.append(table);
    key.append(ID);

    redisReply* reply = (redisReply*)redisCommand(context, "GET %s", key.c_str());
    if (!reply || reply->type != REDIS_REPLY_STRING) {
        cout<<"[INFO]Data not found in cache"<<endl;
        freeReplyObject(reply);
        return "";
    }
    string value = reply->str;
    freeReplyObject(reply);
    return value;
}


void RedisCache::set(const string& table, const string& ID, const string& value) {
    string key = "";

    key.append(table);
    key.append(ID);

    redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s", key.c_str(), value.c_str());
    if (!reply || reply->type != REDIS_REPLY_STATUS || string(reply->str) != "OK") {
        cerr << "Failed to set cache" << endl;
    }
    freeReplyObject(reply);
}

vector<string> RedisCache::getShippers() {
    unsigned long long cursor = 0;
    vector<string> shippers;
    
    do {
        // You can experiment with different COUNT (batch size) in the SCAN
        redisReply *reply = (redisReply*) redisCommand(context, "SCAN %llu MATCH shippers*", cursor);

        if(!reply) {
            cerr << "[ERROR] Failed to execute SCAN command" << endl;
            return shippers;
        }

        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            // Update the cursor
            cursor = strtoull(reply->element[0]->str, NULL, 10);

            // Check if the keys list is empty
            if (reply->element[1]->type == REDIS_REPLY_ARRAY && reply->element[1]->elements == 0) {
                printf("No shippers in cache. Dimensione %ld\n", reply->element[1]->elements);

                return shippers;
            } 
            else {
                // Get the data from the key
                string id = "";
                string key = "";
                
                for (size_t i = 0; i < reply->element[1]->elements; i++) {
                    key = reply->element[1]->element[i]->str;
                    id = key.substr(8, string::npos);
                    
                    string shipper = id + "_" + get("shippers", id);
                    shippers.insert(shippers.end(), shipper);
                }
            }
        } 
        else {
            printf("[ERROR]Unexpected reply structure.\n");
            freeReplyObject(reply);
            
            // Return an empty array
            return shippers;
        }
    
        freeReplyObject(reply);
    } while (cursor != 0);

    cout << "[INFO]Shippers in cache." << endl;
    // Returns the tuples of the shippers
    return shippers;
}

vector<string> RedisCache::getProducts() {
    unsigned long long cursor = 0;
    vector<string> products;
    // Iterates over the batches (cursor) of keys
    do {
        redisReply *reply = (redisReply*) redisCommand(context, "SCAN %llu MATCH products*", cursor);
        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            // Update the cursor
            cursor = strtoull(reply->element[0]->str, NULL, 10);

            // Check if the keys list is empty
            if (reply->element[1]->type == REDIS_REPLY_ARRAY && reply->element[1]->elements == 0) {
                printf("No products in cache.\n");
                // Return an empy array
                return products;
            } 
            else {
                // Get the data from the key
                string id = "";
                string key = "";
                
                for (size_t i = 0; i < reply->element[1]->elements; i++) {
                    key = reply->element[1]->element[i]->str;
                    id = key.substr(8, string::npos);
                    // Contains only the value of the redis element (not the key or table)
                    string product = id + "_" + get("products", id);
                    products.insert(products.end(), product);
                }
            }
        } 
        else {
            printf("[ERRORE]Unexpected reply structure.\n");
            freeReplyObject(reply);
            // Returns an empty array
            return products;
        }

        freeReplyObject(reply);
    // Loop stops when cursor loops back to 0
    } while (cursor != 0);

    // Return array of strings containing the products tuples
    return products;
}

DataService::DataService(RedisCache& cache) : cache(cache) {}

// Implementing the Cache-Aside pattern
vector<string> DataService::getData(const string& table, const string& ID) {
    // Check if the data exists in the cache
    string key = "";
    vector<string> res;
    string value;
    int pos = 0;
    string delimiter = "_";

    key.append(table);
    key.append(ID);

    if (cache.exist(table, ID)) {
        cout << "Cache hit for key: " << key << endl;
        string data = cache.get(table, ID);

        
        //Splits on delimiter
        while ((pos = data.find(delimiter)) != std::string::npos) {
            value = data.substr(0, pos);
            res.push_back(value);
            data.erase(0, pos + delimiter.length());
        }
        res.push_back(data);


        return res;
    } else {
        cout << "Cache miss for key: " << key << endl;
        // Simulate fetching data from a PostgreSQL database
        string data = fetchFromDatabase(table, ID);
        if (data.empty()) {
            // If the string is empty return the empty res
            return res;
        }
        // Store the data in the cache
        cache.set(table, ID, data);
        
        // Splits on delimiter
        while ((pos = data.find(delimiter)) != std::string::npos) {
            value = data.substr(0, pos);
            res.push_back(value);
            data.erase(0, pos + delimiter.length());
        }
        res.push_back(data);

        return res;
    }
}

void DataService::setData(const string& table, const string& ID, vector<string> columns) {
    string value = "";
    // Creates the value for the Redis element
    for (string column : columns) {
        value += column + "_";
    }
    value.pop_back();
    cache.set(table, ID, value);

}

void DataService::addCart(const string& ID, const string& prod, const string& qnt) {
    map<int,int> old = getCart(ID);
    string value = "";

    // Checks if the product is already in the cart
    if (old.find(stoi(prod)) != old.end()) {
        // Updates the quantity of the product already in the cart
        old[stoi(prod)] += stoi(qnt);
        for (auto [prod, qnt] : old) {
            value += to_string(prod) + "_" + to_string(qnt) + "_";
        }
        value.pop_back();
    }
    else{
        for (auto [prod, qnt] : old) {
            value += to_string(prod) + "_" + to_string(qnt) + "_";
        }
        // Adds the new product to the cart
        value += prod + "_" + qnt;
    }
    cache.set("carts", ID, value);
}

// Implementing the Cache-Aside pattern
map<int,int> DataService::getCart(const string& ID) {
    // Check if the data exists in the cache controlling the ID of a user
    map<int, int> res;
    vector<string> vals;
    string prod, qnt;
    int pos = 0;
    string delimiter = "_";

    // Checks if the cart is in the cache
    if (cache.exist("carts", ID)) {
        cout << "Cache hit for cart key: " << ID << endl;
        string data = cache.get("carts", ID);
        
        if (data == "") {
            return res;
        }
        //Splits on delimiter following the format of the data for carts (productID_quantity ...)
        while ((pos = data.find(delimiter)) != std::string::npos) {
            prod = data.substr(0, pos);
            data.erase(0, pos + delimiter.length());
            if ((pos = data.find(delimiter) != std::string::npos)) {
                string qnt = data.substr(0, pos);
                data.erase(0, pos + delimiter.length());
                res[stoi(prod)] = stoi(qnt);
            }
        }
        res[stoi(prod)] = stoi(data);
        
        return res;
    } 
    else {
        cout << "Cache miss for customer key: " << ID << endl;
        // Fetching data from a PostgreSQL database
        string data = fetchCartFromDatabase(ID);
        if (data == "") {
            cout << "No data found in db" << endl;
            return res;
        }
        // Store the data in the cache
        cache.set("carts", ID, data);
        
        //Splits on delimiter following the format of the data for carts (productID_quantity ...)
        while ((pos = data.find(delimiter)) != std::string::npos) {
            prod = data.substr(0, pos);
            data.erase(0, pos + delimiter.length());
            if ((pos = data.find(delimiter) != std::string::npos)) {
                qnt = data.substr(0, pos);
                data.erase(0, pos + delimiter.length());
                res[stoi(prod)] = stoi(qnt);
            }
        }
        res[stoi(prod)] = stoi(data);

        return res;
    }
}
// [WARNING] the data of the shippers in the cache has to be updated, expecially the number of shippings (any update oon the shippings table means that the value of that shipper in the cache has to be updated)
vector<string> DataService::getAvailableShipper() { 
    vector<string> res;
    vector<string> shippersC;
    for (int i = 0; i < shippersC.size() && i < 2; i++) {
            cout << shippersC[i] << endl;
    }
    shippersC = cache.getShippers();
    for (int i = 0; i < shippersC.size() && i < 2; i++) {
            cout << shippersC[i] << endl;
    }
    // Checks if there are any shippers in the cache
    if(shippersC.empty()) {
        cout << "e sono effettivamente vuoto"<< endl;
        unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");
        try {        
            // Selects all the available shippers from the DB
            pqxx::work w(*conn);     
            pqxx::result r = w.exec("SELECT s.userID, u.CF, u.name, u.surname, u.email, s.piva,  COUNT(o.orderID) \
                FROM shippers s \
                JOIN users u ON s.userID = u.id \
                JOIN shippings o ON o.shipper = s.userID \
                GROUP BY s.userID, u.CF, u.name, u.surname, u.email, s.piva \
                HAVING COUNT(o.orderID) < 10");
            if (r.empty()) {        
                cout << "Nessun trasportatore disponibile" << endl;      
                // Empty res if there aren't any 
                return res;         
            }

            // Setting the newly found shippers in cache
            for (int i = 0; i < r.size(); i++) {
                string value = "";
                // Skipps from adding the userID to the value (it's part of the key)
                for (int j = 1; j < r[i].size(); j++) {
                    value += r[i][j].c_str();
                    value += "_";
                }
                value.pop_back();
                cout << value << endl;
                // Skip setting in cache if already in cache
                cout << r[i][0].c_str() << endl;
                if (cache.exist("shippers", r[i][0].c_str())) {
                    cout << "cache.exist() flag" << endl;
                    continue;
                }
                // Setting in cache
                cache.set("shippers", r[i][0].c_str(), value);
                cout << "cache.set() flag" << endl;
            }
            
            // Creating the tuple of the available shipper    
            res.insert(res.end(), r[0][0].as<string>());  // userID    
            res.insert(res.end(), r[0][5].as<string>());  // P_IVA    
            return res;        
        } 
        catch (const std::exception &e) { 
            cerr << "[ERROR] Error while selecting from the DB: " << e.what() << endl;
            throw e;
        }
    }
    // Else, there are shippers in the cache
    else {
        cout << "e non sono effettivamente vuoto"<< endl;
        for (int i = 0; i < shippersC.size() && i < 2; i++) {
            cout << shippersC[i] << endl;
        }
        // Iterates over the cached shippers
        for(auto value : shippersC) {
            vector<string> splitValue;
            string token;
            stringstream ss(value);

            while (std::getline(ss, token, '_')) {
                splitValue.push_back(token);
            }
            // Checks if the shipper is available (the number of shippings is the last element)
            if (stoi(splitValue.back()) < 10 ) {
                cout << "[INFO]Available from cache." << endl;
                return splitValue;
            }
        }
        unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

        try { 

            // Selects all the available shippers from the DB                   
            pqxx::work w(*conn);/* 
            w.exec("SELECT s.userID, u.CF, u.name, u.surname, u.email, s.piva,  COUNT(o.orderID) \
                FROM shippers s \
                JOIN users u ON s.userID = u.id \
                JOIN shippings o ON o.shipper = s.userID \
                GROUP BY s.userID, u.CF, u.name, u.surname, u.email, s.piva \
                HAVING COUNT(o.orderID) < 10"); */
            /* pqxx::result r = w.exec("SELECT s.userID, u.name, u.surname, u.email, s.piva, COUNT\
                FROM shippers s JOIN users u ON s.userID = u.id \
                WHERE (SELECT COUNT(*) FROM shippings \
                    WHERE shipper = s.userID AND state = FALSE) < 10 LIMIT 1"); */        
            pqxx::result r = w.exec("SELECT s.userID, u.CF, u.name, u.surname, u.email, s.piva,  COUNT(o.orderID) \
                FROM shippers s \
                JOIN users u ON s.userID = u.id \
                JOIN shippings o ON o.shipper = s.userID \
                GROUP BY s.userID, u.CF, u.name, u.surname, u.email, s.piva \
                HAVING COUNT(o.orderID) < 10");
            if (r.empty()) {        
                cout << "Nessun trasportatore disponibile" << endl;      
                // Empty res if there aren't any 
                return res;         
            }    
            // Setting the newly found shippers in cache
            for (int i = 0; i < r.size(); i++) {
                string value = "";
                // Skipps from adding the userID to the value (it's part of the key)
                for (int j = 1; j < r[i].size(); j++) {
                    value += r[i][j].c_str();
                    value += "_";
                }
                value.pop_back();
                // Skip setting in cache if already in cache
                if (cache.exist("shippers", r[i][0].c_str())) {
                    cout << "cache.exist() flag" << endl;
                    continue;
                }
                // Setting in cache
                cache.set("shippers", r[i][0].c_str(), value);
            }
            
            // Creating the tuple of the available shipper    
            res.insert(res.end(), r[0][0].c_str());  // userID    
            res.insert(res.end(), r[0][5].c_str());  // P_IVA    
            return res;
        } 
        catch (const std::exception &e) {    
            cerr << "Error in trasportatore_disponibile: " << e.what() << endl;    
            throw e;  
        }
    }
}

vector<string> DataService::getFilteredProducts(string& filters) {
    /* [ID],[nome],[=N,>N,<N,<=N,>=N] */
    vector<string> filt, products;
    int pos = 0;
    string delimiter = ",";
    string token;
    // Splits on delimiter
    while ((pos = filters.find(delimiter)) != std::string::npos) {
        token = filters.substr(0, pos);
        filters.erase(0, pos + delimiter.length());
        filt.insert(filt.end(), token);
    }
    filt.insert(filt.end(), filters);

    products = cache.getProducts();
    // getProducts()

    int len = products.size();
    string product, id, name, des, suppl;
    int price, stock;

    delimiter = "_";
    pos = 0;
    for (int i = len-1; i >= 0; i++) {
        product = products[i];
        pos = product.find(delimiter);
        id = product.substr(0, pos);
        product.erase(0, pos + delimiter.length());
        
        pos = product.find(delimiter);
        name = product.substr(0, pos);
        product.erase(0, pos + delimiter.length());

        pos = product.find(delimiter);
        des = product.substr(0, pos);
        product.erase(0, pos + delimiter.length());

        pos = product.find(delimiter);
        suppl = product.substr(0, pos);
        product.erase(0, pos + delimiter.length());

        pos = product.find(delimiter);
        price = stoi(product.substr(0, pos));
        product.erase(0, pos + delimiter.length());

        stock = stoi(product);

        if (filt[0] == id && filt[0] != "") {
            vector<string> res;
            res.insert(res.end(), products[i]);
            return res;
        }

        if (!name.contains(filt[1]) && filt[1] != "") {
            products.pop_back();
            continue;
        }

        if (filt[2] != "") {
            if (filt[2].length() == 2) {
                string comp = filt[2].substr(0, 1);
                int priceFilter = stof(filt[2].substr(1, filt[2].length()));

                if (comp == "=" && price != priceFilter) {
                    product.pop_back();
                    continue;
                }
                else if (comp == ">" && price <= priceFilter) {
                    product.pop_back();
                    continue;
                } 
                else if (comp == "<" && price >= priceFilter) {
                    product.pop_back();
                    continue;
                }
            }
            else {
                // Same code as before because since we know that the len is 3 we just need to check the first char (just as before)
                string comp = filt[2].substr(0, 1);
                int priceFilter = stof(filt[2].substr(2, filt[2].length()));

                if (comp == ">" && price < priceFilter) {
                    product.pop_back();
                    continue;
                } 
                else if (comp == "<" && price > priceFilter) {
                    product.pop_back();
                    continue;
                }
            }
        }
    }
    return products;
}


string DataService::fetchCartFromDatabase(const string& ID) {
    try {
        // Connect to the PostgreSQL database
        unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

        // Query the database for the value corresponding to the key
        pqxx::work w(*conn);
        pqxx::result result = w.exec("SELECT * FROM carts WHERE customer = " + w.quote(ID) + "");

        if (result.empty()) {
            cerr << "No data found for ID in db: " << ID << endl;
            return "";
        }

        string data;
        // Return the data from the query result
        for(auto const &row: result) {
            pqxx::field const prod = row[1];
            pqxx::field const qnt = row[2];
            data += prod.as<string>() + "_";
            data += qnt.as<string>() + "_";
        }
        // Pops the last element of the string (the last char "_")
        data.pop_back();
        w.commit();
        return data;

    } catch (const exception &e) {
        cerr << "Error fetching from database: " << e.what() << endl;
        return "";
    }
}

string DataService::fetchFromDatabase(const string& table, const string& ID) {
    try {
        // Connect to the PostgreSQL database
        pqxx::result result;

        unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");
        // Query the database for the value corresponding to the key
        pqxx::work w(*conn);
        if (table == "shippers") {
            result = w.exec("SELECT u.CF, u.name, u.surname, u.email, s.piva,  COUNT(o.orderID) \
                FROM shippers s \
                JOIN users u ON s.userID = u.id \
                JOIN shippings o ON o.shipper = s.userID \
                WHERE s.userID = " + ID + " \
                GROUP BY s.userID, u.CF, u.name, u.surname, u.email, s.piva");
        }
        else {
            result = w.exec("SELECT * FROM " + table + " WHERE ID = " + w.quote(ID) + "");
        }
        if (result.empty()) {
            cerr << "No data found for ID in db: " << ID << endl;
            return "";
        }

        string data;
        pqxx::row const row = result[0];
        std::size_t const num_cols = result.columns();

        // Return the data from the query result
        for(std::size_t col=0u; col < num_cols; ++col) {
            pqxx::field const field = row[col];
            data += field.as<string>() + "_";
        }
        data.pop_back();

        w.commit();
        return data;

    } catch (const exception &e) {
        cerr << "Error fetching from database: " << e.what() << endl;
        return "";
    }
}
