#include "Shipper.hpp"
#include <pqxx/pqxx>
#include <iostream>

/*
Funzione con la quale il trasportatore ottiene le spedizioni a proprio carico.

Dal database prendere tutte le spedizioni affidate al Trasportatore t (?)
 */
std::vector<int> Shipper::getShippings() {
    // TODO ROMINA Implementare la funzione andando ad usare una query, senza usare redis
    return spedizioni_assegnate;
}


/*
 Funzione che restituisce le spedizioni che devono essere ancora completate.
 */
std::vector<int> Shipper::getActiveShippings() {
    std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");
    std::vector<int> activeShippings;

    try {
        pqxx::work w(*conn);
        pqxx::result r = w.exec(
            "SELECT s.id, s.orderID, s.shipper, s.handlingtime, s.state "
            "FROM shippings s "
            "WHERE s.shipper = " + std::to_string(this->ID) + " AND s.state = FALSE");

        for (auto row : r) {
            int shipmentID = row[0].as<int>();

            activeShippings.push_back(shipmentID);
        }
    } catch (const std::exception &e) {
        std::cerr << "Errore in getActiveShippings: " << e.what() << std::endl;
    }

    return activeShippings;
}

/*
Aggiorna lo stato di una spedizione.
 */
void shippingDelivered(int shippingID){
    // Prima prendi la spedizione dal database con quell'id e controlli che non sia stata gia' consegnata
    // Se non e' stata consegnata, aggiorna lo stato della spedizione a TRUE
  };


/*
Questa funzione cerca un trasportatore disponibile,che si può ottenere dalla tabella shippers.
Per "disponibile", si intende un trasportatore che non ha spedizioni attualmente
in corso (il campo state della spedizione è FALSE).
 */
static int trasportatore_disponibile() {
    std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");
  try {
    // Selezioniamo un trasportatore che non ha spedizioni in corso (stato FALSE)
    pqxx::work w(*conn);
    pqxx::result r = w.exec("SELECT s.userID "
                            "FROM shippers s "
                            "JOIN users u ON s.userID = u.id "
                            "WHERE (SELECT COUNT(*) FROM shippings WHERE shipper = s.userID AND state = FALSE) < 10 " //Per verificare che il trasportatore abbia meno di 10 spedizioni in corso
                            "LIMIT 1");  // Per restituire al massimo un trasportatore

    if (r.empty()) {
      cout << "Nessun trasportatore disponibile" << endl;
      return -1;  // Se non troviamo trasportatori, restituiamo -1 
    }

    // Assegniamo i dati del trasportatore trovato
    int shipperID = r[0][0].as<int>();  // userID
    return shipperID;
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
void newShipping(int orderID) {
    std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

    // Troviamo un trasportatore disponibile
    int shipperID = trasportatore_disponibile();
    
    // Se non troviamo un trasportatore disponibile
    if (shipperID == -1) {
        cerr << "Nessun trasportatore disponibile!" << endl;
        return;
    }

    try {
        pqxx::work w(*conn);

        // Creiamo la nuova spedizione nel database
        int shippingID = -1; // Variabile per memorizzare l'ID della spedizione
        try {
            // Creiamo la nuova spedizione nel database
            pqxx::result r = w.exec(
                "INSERT INTO shippings (shipper, handlingtime, state) "
                "VALUES (" + to_string(shipperID) + ", NOW(), FALSE) "
                "RETURNING orderID"); // Restituiamo l'ID della spedizione appena inserita

            // Otteniamo l'ID della spedizione appena creata
            shippingID = r[0][0].as<int>();
        } catch (const std::exception &e) {
            cerr << "Errore durante l'inserimento della nuova spedizione: " << e.what() << endl;
            return;
        }
        // Aggiungiamo la spedizione alla lista di spedizioni del trasportatore in memoria
        // TODO
        /*t.spedizioni_assegnate.push_back(shippingID);*/

        try {
            // Aggiorniamo l'ordine con l'ID della spedizione nel database
            w.exec("UPDATE orders SET shipping = " + to_string(shippingID) + " WHERE id = " + to_string(orderID));

            // Aggiorniamo la relazione tra trasportatore e spedizione nel database (tabella intermedia)
            w.exec("INSERT INTO shipper_shippings (shipperID, shippingID) VALUES (" +
                   to_string(shipperID) + ", " + to_string(shippingID) + ")");
        } catch (const std::exception &e) {
            cerr << "Errore durante l'aggiornamento dei dati: " << e.what() << endl;
            return;
        }

        // Commit dell'operazione
        w.commit();

        cout << "Nuova spedizione creata con ID: " << shippingID << endl;

    } catch (const std::exception &e) {
        cerr << "Error in newShipping: " << e.what() << endl;
        return;
    }
}

/*
Funzione per assegnare automaticamente tutti gli ordini non ancora assegnati ad un trasportatore disponibile.
 */
void assignUnassignedOrders() {
    std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");
    try {
        pqxx::work w(*conn);

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
            int shipperID = trasportatore_disponibile();

            // TODO
            /*if (shipperID.P_IVA.empty()) {*/
            /*    cerr << "Nessun trasportatore disponibile per l'ordine con ID: " << orderId << endl;*/
            /*    continue;*/
            /*}*/

            try {
                // Creazione della spedizione
                pqxx::result r = w.exec(
                    "INSERT INTO shippings (orderID, shipper, handlingtime, state) "
                    "VALUES (" + to_string(orderId) + ", " + to_string(shipperID) + ", NOW(), FALSE) "
                    "RETURNING orderID"
                );

                int shippingId = r[0][0].as<int>(); // id della spedizione appena creata
                cout << "Ordine ID: " << orderId << " Trasportatore con ID: " << shipperID
                     << " Spedizione ID: " << shippingId << endl;

                // Aggiornamento della relazione tra trasportatore e spedizione
                w.exec("INSERT INTO shipper_shippings (shipperID, shippingID) VALUES (" +
                       to_string(shipperID) + ", " + to_string(shippingId) + ")");

            } catch (const std::exception &e) {
                cerr << "Impossibile assegnare l'ordine con ID: " << orderId
                     << " al trasportatore con ID: " << shipperID<< " - " << e.what() << endl;
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


