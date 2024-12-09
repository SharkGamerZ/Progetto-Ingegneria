#include "Customer.hpp"

    
/**
 * Aggiunge un articolo al carrello di un customer.
 * 
 * Salva poi questa modifica su redis per manterne lo stato.
 * 
 * @param a Product da aggiungere al carrello.
 * @param c Customer a cui aggiungere gli articoli nel carrello.
 * 
 */
void Customer::addProductToCart(int productID, int qta) {
	RedisCache rc = RedisCache();
	DataService ds(rc);

	// Controlla la disponibilità dell'articolo
	cout<<"[INFO] Adding product with ID "<<productID<<" to the cart of "<<this->ID<<endl;
	vector<string> productString = ds.getData("products", to_string(productID));
	Product p;

	p.ID = stoi(productString[0]);
	p.name = productString[1];
	p.description = productString[2];
	p.supplierID = stoi(productString[3]);
	p.price = stof(productString[4]);
	p.stock = stoi(productString[5]);

	if (p.stock < qta) {
		cerr<<"[ERROR] Error adding product to cart.\n\tProduct "<<p.name<<" has "<<p.stock<< " elements in stock, but "<<qta<<" where requested."<<endl;
		return;
	}
	
	cart = ds.getCart(to_string(this->ID));
	string query;
	// Controlla se l'articolo è già presente nel carrello
	if (cart.find(productID) != cart.end()) {
		this->cart[productID] += qta;
		cout<<"[INFO] "<<qta<<" elements of product with ID "<<p.ID<<" added to the cart of "<<this->ID<<endl;
		query = "UPDATE carts SET quantity = " + to_string(cart[productID]) + " WHERE customer = " + to_string(this->ID) + " AND product = " + to_string(p.ID);
	} else {
		this->cart[productID] = qta;
		cout<<"[INFO] Product with ID "<<p.ID<<" added to the cart of "<<this->ID<<endl;
		query = "INSERT INTO carts (customer, product, quantity) VALUES (" + to_string(this->ID) + ", " + to_string(p.ID) + ", " + to_string(qta) + ")";
	}

	// Salva la modifica sul DB
	std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

	pqxx::work w(*conn);
	try {
		w.exec(query);
		w.commit();
	} catch (const std::exception &e) {
		cerr << e.what() << endl;
	}

	// Salva su redis
	ds.addCart(to_string(this->ID), to_string(productID), to_string(qta));
	
}

void Customer::removeProductFromCart(int productID, int qta) {
	// Controlla se l'articolo è presente nel carrello
	// e se la quantità da rimuovere è minore o uguale a quella presente
	if (this->cart.count(productID) == 0) {
		cerr<<"[ERROR] Error removing product from cart\n\tProduct with ID "<<productID<<" not found in the cart of "<<this->ID<<endl;
		return;
	}

	if (this->cart[productID] < qta) {
		cerr<<"[ERROR] Error removing product from cart\n\tProduct with ID "<<productID<<" has "<<this->cart[productID]<<" elements in the cart, but "<<qta<<" where requested to remove."<<endl;
		return;
	}

	if (this->cart[productID] == qta) {
		this->cart.erase(productID);
		cout<<"[INFO] Product with ID "<<productID<<" removed from the cart of "<<this->ID<<endl;
	} else {
		this->cart[productID] -= qta;
		cout<<"[INFO] "<<qta<<" elements of product with ID "<<productID<<" removed from the cart of "<<this->ID<<endl;
	}

	// Aggiorniamo il DB
	std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

	pqxx::work w(*conn);
	if (this->cart.count(productID) == 0) {
		try {
			w.exec("DELETE FROM carts WHERE customer = " + to_string(this->ID) + " AND product = " + to_string(productID));
			w.commit();
		} catch (const std::exception &e) {
			cerr << e.what() << endl;
		}
	} else {
		try {
			w.exec("UPDATE carts SET quantity = " + to_string(this->cart[productID]) + " WHERE customer = " + to_string(this->ID) + " AND product = " + to_string(productID));
			w.commit();
		} catch (const std::exception &e) {
			cerr << e.what() << endl;
		}
	}
}


/**
 * Funzione che acquista tutto ciò che c'è nel carrello di un customer.
 * 
 * Prima crea un ordine, lo salva sul db, aggiorna il magazzino dei prodotti, assegna l'ordine
 * ad un trasportatore e poi svuota il carrello.
 * 
 */
void Customer::buyCart() {
	RedisCache rc = RedisCache();
	DataService ds(rc);

	// Creazione dell'ordine
	Order order;

	order.customerID = this->ID;
	order.products = this->cart;

	// La query dovra' tornare una mappa ProductID->stock
	map <int, int> productStock;

	// Cicla sui prodotti nel carrello e prende la quantità in stock
	map<int, int>::iterator it;
	for (it = order.products.begin(); it != order.products.end(); it++) {
		int productID = it-> first;
		vector<string> productString = ds.getData("products", to_string(productID));

		Product p;
		p.ID = stoi(productString[0]);
		p.stock = stoi(productString[5]);

		productStock[p.ID] = p.stock;
	}



	// Aggiunge l'ordine al db
	std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

	pqxx::work w(*conn);
	try {
		w.exec("INSERT INTO orders (customer, instant) VALUES (" + to_string(order.customerID) + ", NOW())");
	} catch (const std::exception &e) {
		cerr << e.what() << endl;
	}

	// Aggiunge i vari prodotti all'ordine
	for (it = order.products.begin(); it != order.products.end(); it++) {
		int productID = it-> first;
		int qta = it-> second;

		try {
			// Aggiunge all'ultimo ordine inserito il prodotto
			w.exec("INSERT INTO order_products (orderID, productID, quantity) VALUES ((SELECT MAX(ID) FROM orders), " + to_string(productID) + ", " + to_string(qta) + ")");
		} catch (const std::exception &e) {
			cerr << e.what() << endl;
		}
	}


	// Crea la spedizione (TODO Romina)
	Shipper::newShipping(order.ID);

	// Svuota il carrello
	this->cart.clear();
}



Order* getPastOrders (Customer c) {
	// TODO query per prendere gli ordini passati
	Order* orders;

	return orders;
}


void updateCustomer (Customer oldCust, Customer newCust) {
	// TODO query con la update
}
