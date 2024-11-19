#include "pgutils.hpp"




/**
 * Create the connection to the database
 * @param conn the connection object
 * @return the connection object
 */
std::unique_ptr<pqxx::connection> getConnection(string dbname, string host, string user, string password) {
    try {
        std::unique_ptr<pqxx::connection> conn = std::make_unique<pqxx::connection>("dbname=" + dbname + " user=" + user + " password=" + password + " host=" + host + " port=5432");
        return conn;
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        throw e;
    }
}

/**
 * Create a new role in the database, 
 * if the role already exists it does nothing
 * @param conn the connection object
 * @param role the role to create
 */
void createRole(std::unique_ptr<pqxx::connection> &conn, string role) {
    // Checks if the role already exists
    try {
        pqxx::work w(*conn);
        pqxx::result r = w.exec("SELECT * FROM pg_roles WHERE rolname = '" + role + "'");
        w.commit();
        if (!r.empty()) {
            cout << "SELECT * FROM pg_roles WHERE rolname = '" + role + "'";
            cout << "Role " + role + " already exists" << endl;
            cout << r.size() << endl;
            return;
        }
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        throw e;
    }

    // Creates a new role in the database
    try {
        pqxx::work w(*conn);
        w.exec("CREATE ROLE " + role + " NOSUPERUSER CREATEDB NOCREATEROLE INHERIT LOGIN NOREPLICATION NOBYPASSRLS PASSWORD 'ecommerce'");
        w.commit();
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        throw e;
    }
}

void dropRole(std::unique_ptr<pqxx::connection> &conn, string role) {
    // Checks if the role already exists
    try {
        pqxx::work w(*conn);
        pqxx::result r = w.exec("SELECT * FROM pg_roles WHERE rolname = '" + role + "'");
        if (r.empty()) {
            cout << "Role " + role + " does not exist" << endl;
            return;
        }
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        throw e;
    }

    // Drops the role
    try {
        pqxx::work w(*conn);
        w.exec("DROP ROLE " + role);
        w.commit();
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        throw e;
    }
}
/**
 * Creates all the tables in the database
 * @param conn the connection object
 */
void createTables(std::unique_ptr<pqxx::connection> &conn) {
    try {
        pqxx::work w(*conn);
        w.exec("CREATE TABLE users (id SERIAL PRIMARY KEY, \
                                    cf VARCHAR(255) UNIQUE NOT NULL, \
                                    name VARCHAR(255) NOT NULL, \
                                    surname VARCHAR(255) NOT NULL, \
                                    email VARCHAR(255) NOT NULL)");


        w.exec("CREATE TABLE customers (userID INTEGER PRIMARY KEY references users(id))");

        w.exec("CREATE TABLE suppliers (userID INTEGER PRIMARY KEY references users(id), \
                                        piva VARCHAR(255) UNIQUE NOT NULL)");

        w.exec("CREATE TABLE shippers (userID INTEGER PRIMARY KEY references users(id), \
                                        piva VARCHAR(255) UNIQUE NOT NULL)");


        w.exec("CREATE TABLE products (id SERIAL PRIMARY KEY, \
                                        name VARCHAR(255) NOT NULL, \
                                        description VARCHAR(255) NOT NULL, \
                                        supplier INTEGER NOT NULL references suppliers(userID), \
                                        price REAL NOT NULL, \
                                        stock INTEGER NOT NULL)");

        w.exec("CREATE TABLE orders (id SERIAL PRIMARY KEY, \
                                    customer INTEGER NOT NULL references customers(userID), \
                                    instant timestamp NOT NULL)");

       
        w.exec("CREATE TABLE shippings (orderID INTEGER PRIMARY KEY references orders(id), \
                                        shipper INTEGER NOT NULL references shippers(userID), \
                                        handlingtime timestamp, \
                                        state BOOLEAN NOT NULL)");


        w.exec("CREATE TABLE orderProducts (orderID INTEGER NOT NULL references orders(id), \
                                            product INTEGER NOT NULL references products(id), \
                                            quantity INTEGER NOT NULL, \
                                            PRIMARY KEY (orderID, product))");

        w.exec("CREATE TABLE carts (customer INTEGER NOT NULL references customers(userID), \
                                    product INTEGER NOT NULL references products(id), \
                                    quantity INTEGER NOT NULL, \
                                    PRIMARY KEY (customer, product))");




        w.exec("CREATE OR REPLACE FUNCTION check_handling_time()\n\
                RETURNS TRIGGER AS $$\n\
                DECLARE\n\
                    order_instant TIMESTAMP;\n\
                BEGIN\n\
                    -- Recupera l'istante dell'ordine collegato\n\
                    SELECT instant INTO order_instant\n\
                    FROM orders\n\
                    WHERE id = NEW.orderID;\n\
                    \n\
                    -- Controlla che handlingTime sia >= di instant\n\
                    IF NEW.handlingTime < order_instant THEN\n\
                        RAISE EXCEPTION 'handlingTime deve essere maggiore o uguale a instant di orders';\n\
                    END IF;\n\
                    \n\
                    RETURN NEW;\n\
                END;\n\
                $$ LANGUAGE plpgsql;");



        w.exec("CREATE TRIGGER check_handling_time \
                BEFORE INSERT OR UPDATE ON shippings \
                FOR EACH ROW \
                EXECUTE FUNCTION check_handling_time();");



        w.commit();
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        throw e;
    }

}

void createDatabase(std::unique_ptr<pqxx::connection> &conn) {
    // Checks if the database already exists
    try {
        pqxx::work w(*conn);
        pqxx::result r = w.exec("SELECT 1 FROM pg_database WHERE datname = 'ecommerce'");
        if (r.size() > 0) {
            cout << "Database ecommerce already exists" << endl;
            return;
        }
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        throw e;
    }

    // Creates a new database
    try {
        pqxx::nontransaction w(*conn);
        w.exec("CREATE DATABASE ecommerce");
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        throw e;
    }
}

/**
 * Drops the database
 * @param conn the connection object
 */
void dropDatabase(std::unique_ptr<pqxx::connection> &conn) {
    // Checks if the database already exists
    try {
        pqxx::work w(*conn);
        pqxx::result r = w.exec("SELECT 1 FROM pg_database WHERE datname = 'ecommerce'");
        if (r.size() == 0) {
            cout << "Database ecommerce does not exist" << endl;
            return;
        }
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        throw e;
    }

    // Drops the database
    try {
        pqxx::nontransaction w(*conn);
        w.exec("DROP DATABASE ecommerce");
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        throw e;
    }
}



/**
* Initialize the database
* @return 0 if success, -1 if error
*/
void initDB() {
    std::unique_ptr<pqxx::connection> conn = getConnection("postgres", "localhost", "postgres", "postgres");

    cout << "[INFO]Dropping the database" << endl;
    dropDatabase(conn);
    dropRole(conn, "ecommerce");

    createRole(conn, "ecommerce"); 

    conn = getConnection("postgres", "localhost", "ecommerce", "ecommerce");
    createDatabase(conn);

    conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

    createTables(conn);
}


