#include "Shipper.hpp"
#include <pqxx/pqxx>
#include <iostream>

/*
Funzione con la quale il trasportatore ottiene le spedizioni a proprio carico.

Dal database prendere tutte le spedizioni affidate al Trasportatore t (?)
 */
std::vector<int> Shipper::getShippings() {
    std::vector<int> spedizioni_assegnate;

    try {
        // Connessione al database "ecommerce"
        std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

        // Creazione della transazione per eseguire la query
        pqxx::work w(*conn);

        // Query per ottenere gli orderID associati allo Shipper (userID)
        std::string query = "SELECT orderID FROM shippings WHERE shipper = " + w.quote(this->ID);

        pqxx::result result = w.exec(query);

        // Estrapoliamo gli orderID dalla risposta della query e li aggiungiamo al vector
        for (const auto& row : result) {
            spedizioni_assegnate.push_back(row["orderID"].as<int>());
        }

        // Completamento della transazione
        w.commit();
    } catch (const std::exception &e) {
        logError(e.what());
        // In caso di errore ritorniamo un vettore vuoto
    }

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
            "SELECT s.orderID, s.shipper, s.handlingtime, s.state "
            "FROM shippings s "
            "WHERE s.shipper = " + std::to_string(this->ID) + " AND s.state = FALSE");

        for (auto row : r) {
            int shipmentID = row[0].as<int>();

            activeShippings.push_back(shipmentID);
        }
    } catch (const std::exception &e) {
        logError(e.what());
    }

    return activeShippings;
}

/*
Aggiorna lo stato di una spedizione.
Se la spedizione non è già stata consegnata, aggiorna il suo stato a TRUE.
*/
void Shipper::shippingDelivered(int shippingID) {
    // Creazione della connessione al database
    std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

    pqxx::result r;
    try {
        pqxx::work w(*conn);

        // Controlla se la spedizione è già stata consegnata
        r = w.exec(
            "SELECT state FROM shippings WHERE orderID = " + std::to_string(shippingID));

        if (r.empty()) {
            logError("Errore: spedizione con ID " + std::to_string(shippingID) + " non trovata.");
            return;
        }

        bool state = r[0][0].as<bool>();
        if (state) {
            logError("Errore: spedizione con ID " + std::to_string(shippingID) + " già consegnata.");
            return;
        }
    } catch (const std::exception &e) {
        logError(e.what());
        return;
    }

    try {
        pqxx::work w(*conn);

        // Aggiorna lo stato della spedizione a TRUE
        w.exec(
            "UPDATE shippings SET state = TRUE WHERE orderID = " + std::to_string(shippingID));

        // Commit delle modifiche
        w.commit();
        std::cout << "[SUCCESS]La spedizione con ID " << shippingID << " è stata segnata come consegnata." << std::endl;

    } catch (const std::exception &e) {
        logError(e.what());
    }
}



/*
Questa funzione cerca un trasportatore disponibile,che si può ottenere dalla tabella shippers.
Per "disponibile", si intende un trasportatore che non ha spedizioni attualmente
in corso (il campo state della spedizione è FALSE).
 */
int Shipper::trasportatore_disponibile() {
    RedisCache rc = RedisCache();
    DataService ds(rc);
    vector<string> s;

    s = ds.getAvailableShipper();

    // Verifica che il vettore non sia vuoto (cioè abbiamo trovato un trasportatore)
    if (s.empty()) {
        cout << "Nessun trasportatore disponibile" << endl;
        return -1;
    }

    try {
        string shipperID = s[0];  // userID del trasportatore

        // Convertiamo shipperID in int
        int shipperID_int = stoi(shipperID);

        cout << "Trasportatore disponibile:" << endl;

        // Ritorniamo solo l'ID del trasportatore trovato
        return shipperID_int;  // Restituisci solo l'ID (intero) del trasportatore
    }
    catch (const std::exception &e) {
        logError(e.what());
        return -1;  // In caso di errore, restituiamo un ID di errore
    }
}

/*
 Questa funzione serve ad assegnare una spedizione ad un trasportatore disponibile,
  creare un record nella tabella shippings con i dettagli della spedizione e aggiornare l'ordine per associare
  l'ID della spedizione.
 */
void Shipper::newShipping(int orderID) {
    std::unique_ptr<pqxx::connection> conn = getConnection("ecommerce", "localhost", "ecommerce", "ecommerce");

    // Troviamo un trasportatore disponibile
    int shipperID = trasportatore_disponibile();

    // Se non troviamo un trasportatore disponibile
    if (shipperID == -1) {
        logError("Nessun trasportatore disponibile!");
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
            logError(e.what());
            return;
        }
        // Aggiungiamo la spedizione alla lista di spedizioni del trasportatore in memoria
        // TODO
        /*/t.spedizioni_assegnate.push_back(shippingID);/*/

        try {
            // Aggiorniamo l'ordine con l'ID della spedizione nel database
            w.exec("UPDATE orders SET shipping = " + to_string(shippingID) + " WHERE orderID = " + to_string(orderID));

            // Aggiorniamo la relazione tra trasportatore e spedizione nel database (tabella intermedia)
            w.exec("INSERT INTO shipper_shippings (shipperID, shippingID) VALUES (" +
                   to_string(shipperID) + ", " + to_string(shippingID) + ")");
        } catch (const std::exception &e) {
            logError(e.what());
            return;
        }

        // Commit dell'operazione
        w.commit();

        cout << "Nuova spedizione creata con ID: " << shippingID << endl;

    } catch (const std::exception &e) {
        logError(e.what());
        return;
    }
}

/*
Funzione per assegnare automaticamente tutti gli ordini non ancora assegnati ad un trasportatore disponibile.
 */
void Shipper::assignUnassignedOrders() {
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
            /*/if (shipperID.P_IVA.empty()) {/*/
            /*    cerr << "Nessun trasportatore disponibile per l'ordine con ID: " << orderId << endl;*/
            /*    continue;*/
            /*/}/*/

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
                logError("Impossibile assegnare l'ordine con ID: " + to_string(orderId) +
                         " al trasportatore con ID: " + to_string(shipperID) + " - " + e.what());
                // Potresti voler eseguire un rollback qui se è necessario fermare la transazione per l'ordine
                // w.abort(); // Ad esempio se vuoi fermare l'inserimento dell'ordine corrente
                continue;  // Continua con il prossimo ordine, non fermare tutta la transazione
            }
        }

        // Commit dell'operazione solo se tutte le operazioni vanno a buon fine
        w.commit();
        cout << "Tutti gli ordini sono stati assegnati correttamente." << endl;

    } catch (const std::exception &e) {
        logError("Errore generale in assignUnassignedOrders: " + string(e.what()));
    }
}


