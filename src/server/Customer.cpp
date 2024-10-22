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
	map<int,int> cart = this->cart;
	
	// Controlla la disponibilità dell'articolo
	if (p.quantity < qta) {
		throw -1;
	}
	
	// Controlla se l'articolo è già presente nel carrello
	if (cart.find(p.ID) != cart.end()) {
		cart[p.ID] += qta;
	} else {
		cart[p.ID] = qta;
	}

	// TODO le cose per salvarlo su redis
}

void Customer::removeProductFromCart(Product p, int qta) {
	map<int,int> cart = this->cart;
	
	// Controlla se l'articolo è presente nel carrello
	// e se la quantità da rimuovere è minore o uguale a quella presente
	if (cart.find(p.ID) == cart.end() || cart[p.ID] < qta) {
		throw -1;
	}

	if (cart[p.ID] == qta) {
		cart.erase(p.ID);
	} else {
		cart[p.ID] -= qta;


	}
		// TODO le cose per salvarlo su redis
}


/**
 * Funzione che acquista tutto ciò che c'è nel carrello di un customer.
 * 
 * Prima crea un ordine, lo salva sul db, aggiorna il magazzino dei prodotti, assegna l'ordine
 * ad un trasportatore e poi svuota il carrello.
 * 
 * @param c Customer di cui piazzare l'ordine.
 */
void buyCart(Customer customer) {
	Order order;
	order.ID = 0; // TODO Funzione per generare in automatico gli ID, o magari lo lasciamo gestire al db;
	
	order.customerID = customer.ID;
	order.products = customer.cart;

	// TODO Fare query che restituisce quantita' dei vari prodotti
	// La query dovra' tornare una mappa ProductID->Quantity
	vector<int> qta = {0};

	// Controlla se c'è la quantità di articoli necessaria.
	map<int, int>::iterator it;
	for (it = order.products.begin(); it != order.products.end(); it++) {
		int productID = it-> first;
		int quantityOrdered = it-> second;


		// Controlla che ci sia la quantita' necessaria
		if (qta[productID] < quantityOrdered) {
			throw -1;
		}
		
		// Aggiorna la quantità
		qta[productID] -= quantityOrdered;
	}

	// Crea la spedizione (TODO Romina)
	/*newShipping(order);*/

	// Svuota il carrello
	customer.cart.clear();
}



Order* getPastOrders (Customer c) {
	// TODO query per prendere gli ordini passati
}


void updateCustomer (Customer oldCust, Customer newCust) {
	// TODO query con la update
}
