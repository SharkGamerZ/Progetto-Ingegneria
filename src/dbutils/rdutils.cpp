#include "rdutils.hpp"

RedisCache::RedisCache() {
    string host = "127.0.0.1";
    int port = 6379;
    // Establish a connection to Redis
    context = redisConnect(host.c_str(), port);
    if (context == nullptr || context->err) {
        if (context) {
            logError(context->errstr);
            redisFree(context);
        } else {
            logError("Connection error: Can't allocate redis context");
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
        for (auto [id, name, description, supplierID, price, stock] : w.query<string, string, string, string, string, string>(query)) {
            set("products", id, name + "_" + description + "_" + supplierID + "_" +  price + "_" + stock);
        }
        w.commit();
    } catch (const std::exception &e) {
        logError(e.what());
        throw e;
    }
}

void RedisCache::emptyCache() {
    redisReply* reply = (redisReply*) redisCommand(context, "FLUSHALL");
    if (!reply) {
        logError("Failed to execute FLUSHALL command");
        return;
    }
    if (reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK") {
        std::cout << "Cache successfully emptied." << std::endl;
    } else {
        logError("Failed to empty cache.");
    }
    freeReplyObject(reply);
}

bool RedisCache::exist(const string& table, const string& ID) {
    string key = "";

    key.append(table);
    key.append(ID);

    redisReply* reply = (redisReply*)redisCommand(context, "EXISTS %s", key.c_str());
    if (!reply) {
        logError("Failed to execute EXISTS command");
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
        logError("Failed to set cache");
    }
    freeReplyObject(reply);
}

vector<string> RedisCache::scanTable(const string& table) {
    unsigned long long cursor = 0;
    vector<string> rows;
    
    do {
        // Execute the SCAN command with a MATCH pattern for shippers
        redisReply *reply = (redisReply*) redisCommand(context, "SCAN %llu MATCH %s*", cursor, table.c_str());
        if (!reply) {
            logError("Failed to execute SCAN command");
            freeReplyObject(reply);
            return rows;
        }

        if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2) {
            // Update the cursor
            cursor = strtoull(reply->element[0]->str, NULL, 10);
            // cout << "[DEBUG] Updated cursor: " << cursor << endl;

            // Check if the keys list is empty
            if (reply->element[1]->type == REDIS_REPLY_ARRAY) {

                // On a empty cache this loop won't have any iterations
                for (size_t i = 0; i < reply->element[1]->elements; i++) {
                    string key = reply->element[1]->element[i]->str;
                    // cout << "[DEBUG] Found key: " << key << endl;
                    
                    // Cuts the ID off the key (tableID)
                    string id = key.substr(table.size());

                    // Get the data from the key
                    string row = id + "_" + get(table, id);
                    if (!row.empty()) {
                        rows.push_back(row);
                    } else {
                        cout << "[WARNING]Retrieved empty value for key: " << key << endl;
                    }
                }
            } else {
                cout << "[DEBUG]No keys found in this batch." << endl;
            }
        } else {
            logError("Failed to execute SCAN command");
            freeReplyObject(reply);
            return rows;
        }

        freeReplyObject(reply);
    } while (cursor != 0);
    
    if (!rows.empty()) {
        cout << "[INFO]Shippers in cache." << endl;
    }
    else {
        cout << "[INFO]No shippers in cache." << endl;
    }
    return rows;
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

    shippersC = cache.scanTable("shippers");
    // Checks if there are any shippers in the cache
    if(shippersC.empty()) {
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
                cout << "[WARNING]No available shipper." << endl;      
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
                    continue;
                }
                // Setting in cache
                cache.set("shippers", r[i][0].c_str(), value);
            }
            
            // Creating the tuple of the available shipper    
            res.insert(res.end(), r[0][0].as<string>());  // userID    
            res.insert(res.end(), r[0][5].as<string>());  // P_IVA    
            return res;        
        } 
        catch (const std::exception &e) { 
            logError(e.what());
            throw e;
        }
    }
    // Else, there are shippers in the cache
    else {
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
                res.insert(res.end(), splitValue[0]);  // userID    
                res.insert(res.end(), splitValue[5]);  // P_IVA    
                return res;
            }
        }
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
                cout << "[WARNING]No available shipper" << endl;      
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
            logError(e.what());
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

    products = cache.scanTable("products");
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
            logError("No data found for ID in db: " + ID);
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
        logError(e.what());
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
            logError("No data found for ID in db: " + ID);
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
        logError(e.what());
        return "";
    }
}
