#ifndef RDUTILS_HPP
#define RDUTILS_HPP

#include <hiredis/hiredis.h>
#include <pqxx/pqxx>
#include "../server/utils.hpp"

class RedisCache{
public:
    RedisCache(const string& host, int port);

    ~RedisCache();

    bool exist(const string& key);
    string get(const string& key);
    void set(const string& key, const string& value);

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
