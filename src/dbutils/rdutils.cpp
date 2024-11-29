#include "rdutils.hpp"

RedisCache::RedisCache(const string& host = "127.0.0.1", int port = 6379) {
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


bool RedisCache::initCache() {
    // Enstablish connection to DB and load the products table
    unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

    pqxx::work w(*conn);
    string query = "SELECT * FROM products";
    for (auto [id, supplierID, name, description, price, stock] : w.query<string, string, string, string, string, string>(query)) {
        set("products", id, supplierID + "_" + name + "_" + description + "_" + price + "_" + stock);
    }
}

bool RedisCache::exist(const string& table, const string& ID) {
    string key = "";

    key.append(ID);
    key.append(table);

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

    key.append(ID);
    key.append(table);

    redisReply* reply = (redisReply*)redisCommand(context, "GET %s", key.c_str());
    if (!reply || reply->type != REDIS_REPLY_STRING) {
        freeReplyObject(reply);
        return "";
    }
    string value = reply->str;
    freeReplyObject(reply);
    return value;
}

void RedisCache::set(const string& table, const string& ID, const string& value) {
    string key = "";

    key.append(ID);
    key.append(table);


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

vector<string> DataService::getAvailableShipper() {
    unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

    try {
        // Selezioniamo un trasportatore che non ha spedizioni in corso (stato FALSE)
        pqxx::work w(*conn);
        pqxx::result r = w.exec("SELECT s.userID, u.piva, u.ragione_sociale, u.sede FROM shippers s JOIN users u ON s.userID = u.id WHERE (SELECT COUNT(*) FROM shippings WHERE shipper = s.userID AND state = FALSE) < 10 LIMIT 1");  // Per restituire al massimo un trasportatore

        if (r.empty()) {
        cout << "Nessun trasportatore disponibile" << endl;
        return t;  // Se non troviamo trasportatori, restituiamo un oggetto vuoto
    }

    // Assegniamo i dati del trasportatore trovato
    t.ID = r[0][0].as<int>();  // userID
    t.P_IVA = r[0][1].as<string>();  // P_IVA
    t.ragione_sociale = r[0][2].as<string>();  // ragione_sociale
    t.sede = r[0][3].as<string>();  // sede

    return t;
  } catch (const std::exception &e) {  //per catturare le eccezioni lanciate dal blocco try
    cerr << "Error in trasportatore_disponibile: " << e.what() << endl;
    throw e;
  }
}

string DataService::fetchFromDatabase(const string& table, const string& ID) {
    try {
        // Connect to the PostgreSQL database
        unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

        // Query the database for the value corresponding to the key
        pqxx::work w(*conn);
        pqxx::result result = w.exec("SELECT * FROM " + w.quote(table) + " WHERE ID = " + w.quote(ID) + "");

        if (result.empty()) {
            cerr << "No data found for ID in db: " << ID << endl;
            return "";
        }

        // Return the data from the query result
        string data = result[0][0].as<string>();
        w.commit();
        return data;

    } catch (const exception &e) {
        cerr << "Error fetching from database: " << e.what() << endl;
        return "";
    }
}
