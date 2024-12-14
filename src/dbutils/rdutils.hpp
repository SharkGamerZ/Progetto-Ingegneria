#ifndef RDUTILS_HPP
#define RDUTILS_HPP

#include <hiredis/hiredis.h>
#include <pqxx/pqxx>
#include "../server/utils.hpp"
#include "pgutils.hpp"

/** @brief Class that creates the redis cache and operates on it */
class RedisCache{
public:
    /** @brief Connects to redis */
    RedisCache();

    /** @brief Closes the connection to redis. */
    ~RedisCache();

    /** @brief Populates the cache with the needed tables (products) */
    void initCache();

    void emptyCache();

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

    vector<string> scanTable(const string& table);

    /** @brief Return all the shippers in the cache
     * @return Returns a vector of strings in which each string is the tuple of the shipper
     */
    vector<string> getShippers();

    /** @brief Return all products in the cache (same in the DB) 
     * @return Return a vector of strings in which each string is the tuple of the products
    */
    vector<string> getProducts();

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
     * 
     * The function before checks if the searched data are in the redis cache, if it is it returns it, otherwise it performs a select on the DB, loads the data in the cache and then returns it.
     * 
     * The return is empty if the searched tuple/Redis element isn't present
     * @param table The table in which is going to be searched the ID
     * @param ID The ID of the search tuple
     * @return A vector of strings that contains the values
    */
    vector<string> getData(const string& table, const string& ID);

    /** @brief Given the table, ID and values it creates an element in the redis cache.
     * 
     * @param table The table of the redis element in the DB
     * @param ID The primary key of the element in the DB
     * @param values The columns of the element in the DB
     */
    void setData(const string& table, const string& ID, vector<string> values);
    // da vedere se fare funzione o mettere set in codice
    //void setData(const string& table, const vector<any>& values);
    
    /** @brief Adds product to a cart
     * This function gets the cart of a user then adds a product to the cart. If the product is already in the cart it updates the old quantity for that product
     * 
     * @param ID The ID of the user owner of the cart
     * @param prod The ID of a product to add to the cart
     * @param qnt The quantity of the product to add to the cart
     */
    void addCart(const string& ID, const string& prod, const string& qnt);

    /** @brief Gets the cart for an ID of a customer
     * This function gets a string that is the ID of a customer and returns a map<productID, quantity>
     * 
     * @param ID The ID of a customer
     * @return A map<productID, quantity> that represent a cart
    */
    map<int, int> getCart(const string& ID);

    /** @brief Returns a single shipper that is aavailable (with less than 10 orders)
     * 
     * This function returns the first aavailable shipper that it finds. It checks if there are any shippers in the cache, then checks if it finds one from the cached ones otherwise it seaarches it in the DB.
     * Return an empty vector if there aren't any available shippers.
     * @return The tuple of a shipper
    */
    vector<string> getAvailableShipper();

    /** @brief Gets all the products from the cache that fits the filters
     * 
     * This function loads all the products from the cache, then filters them accordingly to the filters string passed as argument.
     * @param filters A string of format [ID],[nome],[=N,>N,<N,<=N,>=N]
     * @return A vector of string in which each string is a tuple of products
     */
    vector<string> getFilteredProducts(string& filters);    

private:
    RedisCache& cache;

    /** @brief Performs a select on the DB
     * 
     * The function is a private function called from the function getData in the case the pair table-ID is not found as a key in the redis cache.
     * @param table The table in which is going to be searched the ID
     * @param ID The ID of the searched tuple
     */
    string fetchFromDatabase(const string& table, const string& ID);

    /** @brief Performs a select from the carts
     * 
     * The function is a private function called from the function getCart in the case the pair cart-ID is not found as a key in the redis cache.
     * It return a string of pairs "productId_quantity"
     * @param ID The ID of the key of a cart (it is also the ID of a user)
     */
    string fetchCartFromDatabase(const string& ID);
};


#endif
