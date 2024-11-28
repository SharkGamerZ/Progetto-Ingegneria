#ifndef RDUTILS_HPP
#define RDUTILS_HPP

#include <hiredis/hiredis.h>
#include <pqxx/pqxx>
#include "../server/utils.hpp"
#include "pgutils.hpp"

class RedisCache{
public:
    /** @brief Connects to redis. */
    RedisCache(const string& host, int port);

    /** @brief Closes the connection to redis. */
    ~RedisCache();

    /** @brief Checks the existence of the key inside redis
     * @param table The table in the DB
     * @param ID The primary key of the tuple in the DB
     * @return A boolean, True if the key exists, otherwise False
     */
    bool exist(const string& table, const string& ID);

    /** @brief Gets the element from redis, or from the DB if not found
     * @param table The table in the DB
     * @param ID The primary key of the tuple in the DB
     * @return Returns the value of the element as a string
     */
    string get(const string& table, const string& ID);

    /** @brief Set an element in the redis cache
     * @param table The table in the DB
     * @param ID The primary key of the tuple in the DB
     * @param value A string of format "value1[,value2][,value3]..." that rapresents the values in the columns of the DB
    */
    void set(const string& table, const string& ID, const string& value);

private:
    redisContext* context;
};

class DataService{
public:
    DataService(RedisCache& cache);

    string getData(const string& key);

private:
    RedisCache& cache;

    //da vedere fetchFromDatabase se va tenuto
};

#endif
