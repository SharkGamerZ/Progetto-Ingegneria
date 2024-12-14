#include "utils.hpp"
#include "User.hpp"
#include "Shipper.hpp"
#include "../dbutils/pgutils.hpp"
#include "../dbutils/rdutils.hpp"


class Customer : public User {
public:
    map<int, int> cart;         // Maps ProductIDs to quantities
    vector<int> pastOrders;     // List of past orders IDS

    Customer () {}

    Customer(int ID, string CF, string name, string surname, string email) {
        this->ID = ID;
        this->CF = CF;
        this->name = name;
        this->surname = surname;
        this->email = email;
    }

    /**
     * Adds a product to the cart
     * @param a the product to add to the cart
     */
    void addProductToCart(int productID, int qta);

    /**
     * Removes a product from the cart
     * @param a the product to remove from the cart
     */
    void removeProductFromCart(int productID, int qta);

    /**
     * Buys the cart.
     * Creates an order with the products
     * in the cart and clears the cart.
     */
    void buyCart();

    
    /**
     * Gets the list of the past orders.
     * @return the list of past orders IDS
     */
    vector<Order> getPastOrders();
};

struct IndirizzoSpedizione {
    int ID;
    string citta;
    string via;
    string civico;
};

struct Pagamento {
    int ID;
    string numero_carta;
    string scadenza;
};

