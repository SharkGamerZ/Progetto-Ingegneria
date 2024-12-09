#pragma once
#include <pqxx/pqxx>
#include "../server/utils.hpp"


std::unique_ptr<pqxx::connection> getConnection(string dbname, string host, string user, string password);

void createRole(pqxx::connection &conn, string role);
void dropRole(pqxx::connection &conn, string role);

void createTables(pqxx::connection &conn);

void createDatabase(pqxx::connection &conn);
void dropDatabase(pqxx::connection &conn);




void initDB();

