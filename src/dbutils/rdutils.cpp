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

void DataService::addCart(const string& ID, const string& prod, const string& qnt) {
    map<int,int> old = getCart(ID);
    string value = "";

    if (old.find(stoi(prod)) != old.end()) {
        old[stoi(prod)] += stoi(qnt);
        string value = "";
        for (auto [prod, qnt] : old) {
            value += to_string(prod) + "_" + to_string(qnt) + "_";
        }
        value.pop_back();
    }
    else{
        for (auto [prod, qnt] : old) {
            value += to_string(prod) + "_" + to_string(qnt) + "_";
        }
        value += prod + "_" + qnt;
    }
    cache.set("carts", ID, value);
}

// Implementing the Cache-Aside pattern
map<int,int> DataService::getCart(const string& ID) {
    // Check if the data exists in the cache
    map<int, int> res;
    vector<string> vals;
    string value;
    int pos = 0;
    string delimiter = "_";


    if (cache.exist("carts", ID)) {
        cout << "Cache hit for customer key: " << ID << endl;
        string data = cache.get("carts", ID);
        
        if (data == "") {
            return res;
        }
        //Splits on delimiter
        while ((pos = data.find(delimiter)) != std::string::npos) {
            value = data.substr(0, pos);
            vals.push_back(value);
            data.erase(0, pos + delimiter.length());
        }
        vals.push_back(data);

        return res;
    } else {
        cout << "Cache miss for customer key: " << ID << endl;
        // Simulate fetching data from a PostgreSQL database
        string data = fetchCartFromDatabase(ID);
        if (data == "") {
            cout << "No data found in db" << endl;
            return res;
        }
        // Store the data in the cache
        cache.set("carts", ID, data);
        
        string delimiter = "_";
        string prod;
        // Splits on delimiter
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
        unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

        // Query the database for the value corresponding to the key
        pqxx::work w(*conn);
        pqxx::result result = w.exec("SELECT * FROM " + table + " WHERE ID = " + w.quote(ID) + "");

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
            data += field.as<string>() +"_";
        }
        data.pop_back();

        w.commit();
        return data;

    } catch (const exception &e) {
        cerr << "Error fetching from database: " << e.what() << endl;
        return "";
    }
}
