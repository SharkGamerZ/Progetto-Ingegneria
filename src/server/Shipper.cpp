#include "Shipper.hpp"

/*
Funzione con la quale il trasportatore ottiene le spedizioni a proprio carico.

Dal database prendere tutte le spedizioni affidate al Trasportatore t (?)
 */
vector<Shipment> getShippings(Trasportatore t){
    return t.spedizioni_assegnate;
  };

/*
Aggiorna lo stato di una spedizione.
 */
void updateShipping(Shipment old_sped, Shipment new_sped){

    if (old_sped.ID == new_sped.ID) { //aggiorno sse è la stessa spedizione
      old_sped.state= new_sped.state;
    }
   /*
     Aggiorno solo lo stato perchè considero che la spedizione venga fatta solo dallo stesso trasportatore ??
     Nel caso in cui venga presa in carico da diversi trasportatori modifica!
    */
  };


/*
Questa funzione cerca un trasportatore disponibile,che si può ottenere dalla tabella shippers.
Per "disponibile", si intende un trasportatore che non ha spedizioni attualmente
in corso (il campo state della spedizione è FALSE).
 */
Trasportatore trasportatore_disponibile(pqxx::connection &conn) {
  Trasportatore t;
  try {
    // Selezioniamo un trasportatore che non ha spedizioni in corso (stato FALSE)
    pqxx::work w(conn);
    pqxx::result r = w.exec("SELECT s.userID, u.piva, u.ragione_sociale, u.sede "
                            "FROM shippers s "
                            "JOIN users u ON s.userID = u.id "
                            "WHERE NOT EXISTS (SELECT 1 FROM shippings WHERE shipper = s.userID AND state = FALSE) "
                            "LIMIT 1");  // Per restituire al massimo un trasportatore

    if (r.empty()) {
      cout << "No available shippers" << endl;
      return t;  // Se non troviamo trasportatori, restituiamo un oggetto vuoto
    }

    // Assegniamo i dati del trasportatore trovato
    t.ID = r[0][0].as<int>();  // userID
    t.P_IVA = r[0][1].as<string>();  // P_IVA
    t.ragione_sociale = r[0][2].as<string>();  // ragione_sociale
    t.sede = r[0][3].as<string>();  // sede

    return t;
  } catch (const std::exception &e) {  //per catturare le eccezioni lanciate dal blocco try
    cerr << "Error in trasportatore_disponibile: " << e.what() << endl;
    throw e;
  }
}

/*
 Questa funzione serve ad assegnare una spedizione ad un trasportatore disponibile,
  creare un record nella tabella shippings con i dettagli della spedizione e aggiornare l'ordine per associare
  l'ID della spedizione.
 */
void newShipping(Order o, pqxx::connection &conn) {
    // Troviamo un trasportatore disponibile
    Trasportatore t = trasportatore_disponibile(conn);

    // Se non troviamo un trasportatore disponibile
    if (t.P_IVA.empty()) {
        cerr << "Nessun trasportatore disponibile!" << endl;
        return;
    }

    try {
        pqxx::work w(conn);

        // Creiamo la nuova spedizione nel database
        pqxx::result r = w.exec(
            "INSERT INTO shippings (shipper, handlingtime, state) "
            "VALUES (" + to_string(t.ID) + ", NOW(), FALSE) "
            "RETURNING id");  // Restituiamo l'ID della spedizione appena inserita

        // Otteniamo l'ID della spedizione appena creata
        int shipmentID = r[0][0].as<int>();

        // Creiamo un oggetto Shipment in memoria e lo popoliamo
        Shipment new_sped;
        new_sped.ID = shipmentID;  // ID assegnato dal database
        new_sped.orderID = o.ID;   // Associa l'ID dell'ordine
        new_sped.shipperID = t.ID; // ID del trasportatore
        new_sped.handlingTime = time(nullptr); // Tempo di gestione della spedizione
        new_sped.state = false;   // Stato iniziale (spedizione non completata)

        // Aggiungiamo la spedizione alla lista di spedizioni del trasportatore in memoria
        t.spedizioni_assegnate.push_back(new_sped);

        // Aggiorniamo l'ordine con l'ID della spedizione nel database
        w.exec("UPDATE orders SET shipping = " + to_string(shipmentID) + " WHERE id = " + to_string(o.ID));

        // Aggiorniamo la relazione tra trasportatore e spedizione nel database (tabella intermedia)
        w.exec("INSERT INTO shipper_shippings (shipperID, shippingID) VALUES (" +
               to_string(t.ID) + ", " + to_string(shipmentID) + ")");

        // Commit dell'operazione
        w.commit();

        cout << "Nuova spedizione creata con ID: " << shipmentID << endl;

    } catch (const std::exception &e) {
        cerr << "Error in newShipping: " << e.what() << endl;
        throw e;
    }
}

