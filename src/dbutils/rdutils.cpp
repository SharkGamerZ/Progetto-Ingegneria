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

    pqxx::work w(*conn);
    string query = "SELECT * FROM products";
    for (auto [id, supplierID, name, description, price, stock] : w.query<string, string, string, string, string, string>(query)) {
        set("products", id, supplierID + "_" + name + "_" + description + "_" + price + "_" + stock);
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

        cout<<data<<endl;
        
        //Splits on delimiter
        while ((pos = data.find(delimiter)) != std::string::npos) {
            value = data.substr(0, pos);
            res.push_back(value);
            data.erase(0, pos + delimiter.length());
        }
        res.push_back(data);

        cout<<"[INFO]Data found in cache"<<endl;

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

        for (int i = 0; i < res.size(); i++) {
            cout << res[i] << endl;
        }
        return res;
    }
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
        
        //Splits on delimiter
        cout<<data<<endl;
        while ((pos = data.find(delimiter)) != std::string::npos) {
            value = data.substr(0, pos);
            vals.push_back(value);
            data.erase(0, pos + delimiter.length());
        }
        cout << "Finished splitting" << endl;
        vals.push_back(data);


        for(int i = 0; i<vals.size(); i+=2) {
            cout << "Product: " << vals[i] << " Quantity: " << vals[i+1] << endl;
            res[stoi(vals[i])] = stoi(vals[i+1]); 
        }

        cout<<"[INFO] Finished splitting 2"<<endl;
        return res;
    } else {
        cout << "Cache miss for customer key: " << ID << endl;
        // Simulate fetching data from a PostgreSQL database
        string data = fetchCartFromDatabase(ID);
        cout<<data<<endl;
        // Store the data in the cache
        cache.set("carts", ID, data);
        cout<<"[INFO]Data set in cache"<<endl;
        
        cout<<data<<endl;

        string delimiter = "_";
        string prod;
        // Splits on delimiter
        while ((pos = data.find(delimiter)) != std::string::npos) {
            prod = data.substr(0, pos);
            data.erase(0, pos + delimiter.length());
            if ((pos = data.find(delimiter) != std::string::npos)) {
                string qnt = data.substr(0, pos);
                data.erase(0, pos + delimiter.length());
                cout << "Product: " << prod << " Quantity: " << qnt << endl;
                cout << "Data: " << data << endl;
                res[stoi(prod)] = stoi(qnt);
            }
        }
        res[stoi(prod)] = stoi(data);
        cout << "Finished splitting 3" << endl;

        return res;
    }
} 
// ATTENZIONE per fare query su dati dei Shipper vuol dire che quelli presenti in cache devono essere correttamente aggiornati
vector<string> DataService::getAvailableShipper() {    
    vector<string> res;
    unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");
    try {        // Selezioniamo un trasportatore che non ha spedizioni in corso (stato FALSE)        
        pqxx::work w(*conn);     
        pqxx::result r = w.exec("SELECT s.userID, u.piva, u.ragione_sociale, u.sede FROM shippers s JOIN users u ON s.userID = u.id WHERE (SELECT COUNT(*) FROM shippings WHERE shipper = s.userID AND state = FALSE) < 10 LIMIT 1");  // Per restituire al massimo un trasportatore        
        if (r.empty()) {        
            cout << "Nessun trasportatore disponibile" << endl;      
            return ;  // Se non troviamo trasportatori, restituiamo un oggetto vuoto        
        }    
        // Assegniamo i dati del trasportatore trovato    
        res.append(r[0][0].as<string>());  // userID    
        t.P_IVA = r[0][1].as<string>();  // P_IVA    
        t.ragione_sociale = r[0][2].as<string>();  // ragione_sociale    
        t.sede = r[0][3].as<string>();  // sede    
        return t;  } 
        catch (const std::exception &e) {  //per catturare le eccezioni lanciate dal blocco try    
            cerr << "Error in trasportatore_disponibile: " << e.what() << endl;    throw e;  
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
