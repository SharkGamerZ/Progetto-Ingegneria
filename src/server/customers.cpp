#include "customers.hpp"

    
/**
 * Aggiunge un articolo al carrello di un customer.
 * 
 * Salva poi questa modifica su redis per manterne lo stato.
 * 
 * @param a Articolo da aggiungere al carrello.
 * @param c Customer a cui aggiungere gli articoli nel carrello.
 * 
 */
void addProductToCart(Articolo a, Customer c) {
	cart = c.carrello;

	// Placeholder per la quantità, TODO da aggiungere
	qta = 1;

	// Controlla la disponibilità dell'articolo
	if (a.qta < qta) {
		throw -1;
	}

	cart.push_back(a);

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
void buyCart(Customer c) {
	Ordine o;
	o.ID = 0; // TODO Funzione per generare in automatico gli ID, o magari lo lasciamo gestire al db;
	
	o.ID_Customer = c.CF;
	o.articoli = c.carrello;

	// Placeholder
	qta = 1;

	// Controlla se c'è la quantità di articoli necessaria.
	for (int i = 0; i < o.articoli.size(); i++) {
		articolo = o.articoli[i];

		if (articoli.qta < qta) {
			throw -1;
		}
	}


	// Scala la quantità
	for (int i = 0; i < o.articoli.size(); i++) {
		oldArt = o.articoli[i];

		// Crea il nuovo articolo e lo aggiorna
		Articolo newArt = oldArt;
		newArt.qta -= qta;

		updateProduct(oldArt, newArt);
	}


	// Crea la spedizione
	newShipping(o);

	// Svuota il carrello
	c.carrello.clear();
}



Ordine[] getPastOrders (Customer c) {
	// TODO query per prendere gli ordini passati
}