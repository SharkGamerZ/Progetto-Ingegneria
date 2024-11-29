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
void Customer::addProductToCart(Product p, int qta) {
	// Controlla la disponibilità dell'articolo
	if (p.stock < qta) {
		cerr<<"[ERROR] Error adding product to cart.\n\tProduct "<<p.name<<" has "<<p.stock<< " elements in stock, but "<<qta<<" where requested."<<endl;
		return;
	}
	
	// Controlla se l'articolo è già presente nel carrello
	if (this->cart.find(p) != this->cart.end()) {
		this->cart[p] += qta;
		cout<<"[INFO] "<<qta<<" elements of product with ID "<<p.ID<<" added to the cart of "<<this->ID<<endl;
	} else {
		this->cart[p] = qta;
		cout<<"[INFO] Product with ID "<<p.ID<<" added to the cart of "<<this->ID<<endl;
	}

	// TODO le cose per salvarlo su redis
}

void Customer::removeProductFromCart(Product p, int qta) {
	// Print cart
	for (auto [p,qta] : this->cart) {
		cout<<p.ID<<" "<<qta<<endl;
	}
	
	
	// Controlla se l'articolo è presente nel carrello
	// e se la quantità da rimuovere è minore o uguale a quella presente
	if (this->cart.count(p) == 0) {
		cerr<<"[ERROR] Error removing product from cart\n\tProduct with ID "<<p.ID<<" not found in the cart of "<<this->ID<<endl;
		return;
	}

	if (this->cart[p] < qta) {
		cerr<<"[ERROR] Error removing product from cart\n\tProduct with ID "<<p.ID<<" has "<<this->cart[p]<<" elements in the cart, but "<<qta<<" where requested to remove."<<endl;
		return;
	}

	if (this->cart[p] == qta) {
		this->cart.erase(p);
		cout<<"[INFO] Product with ID "<<p.ID<<" removed from the cart of "<<this->ID<<endl;
	} else {
		this->cart[p] -= qta;
		cout<<"[INFO] "<<qta<<" elements of product with ID "<<p.ID<<" removed from the cart of "<<this->ID<<endl;
	}

		// TODO le cose per salvarlo su redis
}


/**
 * Funzione che acquista tutto ciò che c'è nel carrello di un customer.
 * 
 * Prima crea un ordine, lo salva sul db, aggiorna il magazzino dei prodotti, assegna l'ordine
 * ad un trasportatore e poi svuota il carrello.
 * 
 */
void Customer::buyCart() {
	Order order;
	
	order.customerID = this->ID;
	order.products = this->cart;

	// TODO Fare query che restituisce quantita' dei vari prodotti
	// La query dovra' tornare una mappa ProductID->stock
	map <int, int> qta;
	try {
		std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

		pqxx::work w(*conn);
		string query = "SELECT id, stock FROM products";
		for (auto [id, stock] : w.query<int, int>(query)) {
			qta[id] = stock;
		}
		
		w.commit();
	} catch (const std::exception &e) {
		cerr << e.what() << endl;
	}

	// Controlla se c'è la quantità di articoli necessaria.
	map<int, int>::iterator it;
	for (it = order.products.begin(); it != order.products.end(); it++) {
		int productID = it-> first;
		int stockOrdered = it-> second;


		// Controlla che ci sia la quantita' necessaria
		if (qta[productID] < stockOrdered) {
			cerr<<"[ERROR] Error buying the cart"<<endl;
			return;
		}
		
		// Aggiorna la quantità
		qta[productID] -= stockOrdered;
	}

	// Aggiunge l'ordine al db
	

	// Crea la spedizione (TODO Romina)
	/*newShipping(order);*/

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
