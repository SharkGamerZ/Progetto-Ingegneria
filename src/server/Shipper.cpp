#include "Shipper.hpp"
#include <pqxx/pqxx>
#include <iostream>

/*
Funzione con la quale il trasportatore ottiene le spedizioni a proprio carico.

Dal database prendere tutte le spedizioni affidate al Trasportatore t (?)
 */
std::vector<Shipment> Shipper::getShippings() {
    return spedizioni_assegnate;
}


/*
 Funzione che restituisce le spedizioni che devono essere ancora completate.
 */
std::vector<Shipment> Shipper::getActiveShippings(pqxx::connection &conn) {
    std::vector<Shipment> activeShippings;

    try {
        pqxx::work w(conn);
        pqxx::result r = w.exec(
            "SELECT s.id, s.orderID, s.shipper, s.handlingtime, s.state "
            "FROM shippings s "
            "WHERE s.shipper = " + std::to_string(this->ID) + " AND s.state = FALSE");

        for (auto row : r) {
            Shipment s;
            s.ID = row[0].as<int>();
            s.orderID = row[1].as<int>();
            s.shipperID = row[2].as<int>();
            s.handlingTime = row[3].as<time_t>();
            s.state = row[4].as<bool>();

            activeShippings.push_back(s);
        }
    } catch (const std::exception &e) {
        std::cerr << "Errore in getActiveShippings: " << e.what() << std::endl;
    }

    return activeShippings;
}


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
Shipper Shipper::trasportatore_disponibile(pqxx::connection &conn) {
  Shipper t(0, "", "", "", "");  // Oggetto Shipper vuoto come valore di ritorno di default
  try {
    // Selezioniamo un trasportatore che non ha spedizioni in corso (stato FALSE)
    pqxx::work w(conn);
    pqxx::result r = w.exec("SELECT s.userID, u.piva, u.ragione_sociale, u.sede "
                            "FROM shippers s "
                            "JOIN users u ON s.userID = u.id "
                            "WHERE (SELECT COUNT(*) FROM shippings WHERE shipper = s.userID AND state = FALSE) < 10 " //Per verificare che il trasportatore abbia meno di 10 spedizioni in corso
                            "LIMIT 1");  // Per restituire al massimo un trasportatore

    if (r.empty()) {
      cout << "Nessun trasportatore disponibile" << endl;
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
    return;
  }
}

/*
 Questa funzione serve ad assegnare una spedizione ad un trasportatore disponibile,
  creare un record nella tabella shippings con i dettagli della spedizione e aggiornare l'ordine per associare
  l'ID della spedizione.
 */
void newShipping(Order o, pqxx::connection &conn) {
    // Troviamo un trasportatore disponibile
    Shipper t = trasportatore_disponibile(conn);

    // Se non troviamo un trasportatore disponibile
    if (t.P_IVA.empty()) {
        cerr << "Nessun trasportatore disponibile!" << endl;
        return;
    }

    try {
        pqxx::work w(conn);

        // Creiamo la nuova spedizione nel database
        int shipmentID = -1; // Variabile per memorizzare l'ID della spedizione
        try {
            // Creiamo la nuova spedizione nel database
            pqxx::result r = w.exec(
                "INSERT INTO shippings (shipper, handlingtime, state) "
                "VALUES (" + to_string(t.ID) + ", NOW(), FALSE) "
                "RETURNING id"); // Restituiamo l'ID della spedizione appena inserita

            // Otteniamo l'ID della spedizione appena creata
            shipmentID = r[0][0].as<int>();
        } catch (const std::exception &e) {
            cerr << "Errore durante l'inserimento della nuova spedizione: " << e.what() << endl;
            return;
        }
        // Creiamo un oggetto Shipment in memoria e lo popoliamo
        Shipment new_sped;
        new_sped.ID = shipmentID;  // ID assegnato dal database
        new_sped.orderID = o.ID;   // Associa l'ID dell'ordine
        new_sped.shipperID = t.ID; // ID del trasportatore
        new_sped.handlingTime = time(nullptr); // Tempo di gestione della spedizione
        new_sped.state = false;   // Stato iniziale (spedizione non completata)

        // Aggiungiamo la spedizione alla lista di spedizioni del trasportatore in memoria
        t.spedizioni_assegnate.push_back(new_sped);

        try {
            // Aggiorniamo l'ordine con l'ID della spedizione nel database
            w.exec("UPDATE orders SET shipping = " + to_string(shipmentID) + " WHERE id = " + to_string(o.ID));

            // Aggiorniamo la relazione tra trasportatore e spedizione nel database (tabella intermedia)
            w.exec("INSERT INTO shipper_shippings (shipperID, shippingID) VALUES (" +
                   to_string(t.ID) + ", " + to_string(shipmentID) + ")");
        } catch (const std::exception &e) {
            cerr << "Errore durante l'aggiornamento dei dati: " << e.what() << endl;
            return;
        }

        // Commit dell'operazione
        w.commit();

        cout << "Nuova spedizione creata con ID: " << shipmentID << endl;

    } catch (const std::exception &e) {
        cerr << "Error in newShipping: " << e.what() << endl;
        return;
    }
}

/*
Funzione per assegnare automaticamente tutti gli ordini non ancora assegnati ad un trasportatore disponibile.
 */
void assignUnassignedOrders(pqxx::connection &conn) {
    try {
        pqxx::work w(conn);

        // Recupera tutti gli ordini che non hanno una spedizione associata
        pqxx::result unassignedOrders = w.exec(
            "SELECT id FROM orders "
            "WHERE id NOT IN (SELECT orderID FROM shippings)"
        );

        if (unassignedOrders.empty()) {
            cout << "Nessun ordine non assegnato trovato." << endl;
            return;
        }

        // Itera su ogni ordine non assegnato
        for (auto orderRow : unassignedOrders) {
            int orderId = orderRow[0].as<int>();

            // Trova un trasportatore disponibile
            Shipper t = trasportatore_disponibile(conn);

            if (t.P_IVA.empty()) {
                cerr << "Nessun trasportatore disponibile per l'ordine con ID: " << orderId << endl;
                continue;
            }

            try {
                // Creazione della spedizione
                pqxx::result r = w.exec(
                    "INSERT INTO shippings (orderID, shipper, handlingtime, state) "
                    "VALUES (" + to_string(orderId) + ", " + to_string(t.ID) + ", NOW(), FALSE) "
                    "RETURNING id"
                );

                int shippingId = r[0][0].as<int>(); // id della spedizione appena creata
                cout << "Ordine ID: " << orderId << " Trasportatore con ID: " << t.ID
                     << " Spedizione ID: " << shippingId << endl;

                // Aggiornamento della relazione tra trasportatore e spedizione
                w.exec("INSERT INTO shipper_shippings (shipperID, shippingID) VALUES (" +
                       to_string(t.ID) + ", " + to_string(shippingId) + ")");

            } catch (const std::exception &e) {
                cerr << "Impossibile assegnare l'ordine con ID: " << orderId
                     << " al trasportatore con ID: " << t.ID << " - " << e.what() << endl;
                // Potresti voler eseguire un rollback qui se è necessario fermare la transazione per l'ordine
                // w.abort(); // Ad esempio se vuoi fermare l'inserimento dell'ordine corrente
                continue;  // Continua con il prossimo ordine, non fermare tutta la transazione
            }
        }

        // Commit dell'operazione solo se tutte le operazioni vanno a buon fine
        w.commit();
        cout << "Tutti gli ordini sono stati assegnati correttamente." << endl;

    } catch (const std::exception &e) {
        cerr << "Errore generale in assignUnassignedOrders: " << e.what() << endl;
    }
}


