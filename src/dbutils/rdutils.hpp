#ifndef RDUTILS_HPP
#define RDUTILS_HPP

#include <hiredis/hiredis.h>
#include <pqxx/pqxx>
#include "../server/utils.hpp"
#include "pgutils.hpp"

/** @brief Class that creates the redis cache and operates on it */
class RedisCache{
public:
    /** @brief Connects to redis, and load the full Products table in the cache. */
    RedisCache();

    /** @brief Closes the connection to redis. */
    ~RedisCache();

    /** @brief Populates the cache with the needed tables (products) */
    void initCache();

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

    vector<string> getShippers();

    /** @brief Set an element in the redis cache
     * @param table The table in the DB
     * @param ID The primary key of the tuple in the DB
     * @param value A string of format "value1[,value2][,value3]..." that rapresents the values in the columns of the DB
    */
    void set(const string& table, const string& ID, const string& value);

private:
    redisContext* context;
};

/** @brief Class that is used as interface with RedisCache */
class DataService{
public:
    DataService(RedisCache& cache);

    /** @brief Given the table and ID of a tuple in the db, returns the values in a string.
     * 2
     * The function before checks if the searched data are in the redis cache, if it is it returns it, otherwise it performs a select on the DB, loads the data in the cache and then returns it. 
     * @param table The table in which is going to be searched the ID
     * @param ID The ID of the search tuple
     * @return A vector of strings that contains the returner values
    */
    vector<string> getData(const string& table, const string& ID);
    // da vedere se fare funzione o mettere set in codice
    //void setData(const string& table, const vector<any>& values);

    void addCart(const string& ID, const string& prod, const string& qnt);

    map<int, int> getCart(const string& ID);


    vector<string> getAvailableShipper();

    
    vector<string> getFilteredProducts(const string& filters);    

private:
    RedisCache& cache;

    /** @brief Performs a select on the DB
     * 
     * The function is a private function called from the function getData in the case the pair table-ID is not found as a key in the redis cache.
     * @param table The table in which is going to be searched the ID
     * @param ID The ID of the search tuple
     */
    string fetchFromDatabase(const string& table, const string& ID);

    string fetchCartFromDatabase(const string& ID);
};


#endif
