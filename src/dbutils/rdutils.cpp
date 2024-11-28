#include "rdutils.hpp"

class RedisCache {
public:
    RedisCache(const string& host = "127.0.0.1", int port = 6379) {
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

    ~RedisCache() {
        if (context) {
            redisFree(context);
        }
    }

    bool exists(const string& table, const string& ID) {
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

    string get(const string& table, const string& ID) {
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

    void set(const string& table, const string& ID, const string& value) {
        string key = "";

        key.append(ID);
        key.append(table);


        redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s", key.c_str(), value.c_str());
        if (!reply || reply->type != REDIS_REPLY_STATUS || string(reply->str) != "OK") {
            cerr << "Failed to set cache" << endl;
        }
        freeReplyObject(reply);
    }

private:
    redisContext* context;
};


/**Classe che si occupa della gestione dei dati tra cache e db*/
class DataService {
public:
    DataService(RedisCache& cache) : cache(cache) {}

    // Implementing the Cache-Aside pattern
    string getData(const string& table, const string& ID) {
        // Check if the data exists in the cache
        string key = "";

        key.append(table);
        key.append(ID);

        if (cache.exists(key)) {
            cout << "Cache hit for key: " << key << endl;
            return cache.get(key);
        } else {
            cout << "Cache miss for key: " << key << endl;
            // Simulate fetching data from a PostgreSQL database
            string data = fetchFromDatabase(key);
            // Store the data in the cache
            cache.set(key, data);
            return data;
        }
    }

private:
    RedisCache& cache;

    string fetchFromDatabase(const string& key) {
        try {
            // Connect to the PostgreSQL database
            std::unique_ptr<pqxx::connection> conn = getConnection();

            // Query the database for the value corresponding to the key
            pqxx::work w(*conn);
            pqxx::result result = w.exec("SELECT data FROM my_table WHERE key = '" + w.quote(key) + "'");

            if (result.empty()) {
                cerr << "No data found for key: " << key << endl;
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
};
