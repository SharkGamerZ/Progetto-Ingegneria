#include "utils.hpp"
#include "User.hpp"
#include "../dbutils/pgutils.hpp"


class Customer : public User {
public:
    map<int, int> cart;         // Maps ProductIDs to quantities
    vector<Order> pastOrders;

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
    void addProductToCart(Product p, int qta);

    /**
     * Removes a product from the cart
     * @param a the product to remove from the cart
     */
    void removeProductFromCart(Product p, int qta);

    /**
     * Buys the cart.
     * Creates an order with the products
     * in the cart and clears the cart.
     */
    void buyCart();

    
    /**
     * Gets the list of the past orders.
     * @return the list of past orders
     */
    Order* getPastOrders();
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

