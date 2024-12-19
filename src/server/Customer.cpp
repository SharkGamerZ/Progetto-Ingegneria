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

	// Prints the productString
	for (int i = 0; i < productString.size(); i++) {
		cout << "[Product]" << productString[i] << endl;
	}
	
	p.ID = productID;
	p.name = productString[0];
	p.description = productString[1];
	p.supplierID = stoi(productString[2]);
	p.price = stof(productString[3]);
	p.stock = stoi(productString[4]);

	if (p.stock < qta) {
		logError(__FILE__, __LINE__, "Error adding product to cart\n\tProduct " + p.name + " has " + to_string(p.stock) + " elements in stock, but " + to_string(qta) + " where requested.");
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
		logError(__FILE__, __LINE__, e.what());
	}

	// Salva su redis
	ds.addCart(to_string(this->ID), to_string(productID), to_string(qta));
	
}

void Customer::removeProductFromCart(int productID, int qta) {
	// Controlla se l'articolo è presente nel carrello
	// e se la quantità da rimuovere è minore o uguale a quella presente
	if (this->cart.count(productID) == 0) {
		logError(__FILE__, __LINE__, "Error removing product from cart\n\tProduct with ID " + to_string(productID) + " not found in the cart of " + to_string(this->ID));
		return;
	}

	if (this->cart[productID] < qta) {
		logError(__FILE__, __LINE__, "Error removing product from cart\n\tProduct with ID " + to_string(productID) + " has " + to_string(this->cart[productID]) + " elements in the cart, but " + to_string(qta) + " where requested to remove.");
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
			logError(__FILE__, __LINE__, e.what());
		}
	} else {
		try {
			w.exec("UPDATE carts SET quantity = " + to_string(this->cart[productID]) + " WHERE customer = " + to_string(this->ID) + " AND product = " + to_string(productID));
			w.commit();
		} catch (const std::exception &e) {
			logError(__FILE__, __LINE__, e.what());
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
		pqxx::result r = w.exec(
			"INSERT INTO orders (customer, instant)"
			"VALUES (" + to_string(order.customerID) + ", NOW()) "
			"RETURNING id"); // Restituiamo l'ID della spedizione appena inserita

		// Otteniamo l'ID della spedizione appena creata
		order.ID = r[0][0].as<int>();
	} catch (const std::exception &e) {
		logError(__FILE__, __LINE__, e.what());
	}

	// Aggiunge i vari prodotti all'ordine
	for (it = order.products.begin(); it != order.products.end(); it++) {
		int productID = it-> first;
		int qta = it-> second;

		try {
			// Aggiunge all'ultimo ordine inserito il prodotto
			w.exec("INSERT INTO order_products (orderID, productID, quantity) VALUES ((SELECT MAX(ID) FROM orders), " + to_string(productID) + ", " + to_string(qta) + ")");
		} catch (const std::exception &e) {
			logError(__FILE__, __LINE__, e.what());
		}
	}


	// Crea la spedizione (TODO Romina)
	Shipper::newShipping(order.ID);

	// Svuota il carrello
	this->cart.clear();
}



vector<Order> Customer::getPastOrders () {
	// Prende tutti gli ordini passati, con anche i prodotti e quantita' per ogni ordine
	string query = "SELECT id, customer, instant, orderid, product, quantity FROM orders, orderProducts WHERE orders.customer = " + to_string(this->ID) + " AND orders.id = orderProducts.orderID";


	pqxx::connection conn("dbname=ecommerce user=ecommerce password=ecommerce host=localhost");
	pqxx::work w(conn);
	pqxx::result r;
	try {
		r = w.exec(query);
	} catch (const std::exception &e) {
		logError(__FILE__, __LINE__, e.what());
	}

	// Transforma il risultato della query in un array di ordini
	vector<Order> orders;
	for (pqxx::result::const_iterator row = r.begin(); row != r.end(); ++row) {
		Order o;

		// Checks if the order is already in the vector
		bool found = false;
		for (int i = 0; i < orders.size(); i++) {
			if (orders[i].ID == row[0].as<int>()) {
				found = true;

				// If the order is already in the vector, just add the product to the order
				o.products[row[4].as<int>()] = row[5].as<int>();
				break;
			}
		}
		if (found) { continue; }
		o.ID = row[0].as<int>();
		o.customerID = row[1].as<int>();
		// Parse the time from the string
		struct tm tm;
		strptime(row[2].c_str(), "%Y-%m-%d %H:%M:%S", &tm);
		o.orderTime = mktime(&tm);


		o.products[row[4].as<int>()] = row[5].as<int>();

		orders.push_back(o);
	}

	return orders;
}


void updateCustomer (Customer oldCust, Customer newCust) {
	// TODO query con la update
}
