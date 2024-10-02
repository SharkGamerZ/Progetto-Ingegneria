#include "trasportatori.hpp"

//per il mess di errore
#include <iostream>
#include <string>
using namespace std;

/*
Funzione con la quale il trasportatore ottiene le spedizioni a proprio carico.

Dal database prendere tutte le spedizioni affidate al Trasportatore t (?)
 */
vector<Spedizione> getShippings(Trasportatore t){
    return t.spedizioni_assegnate;
  };

/*
Aggiorna lo stato di una spedizione.
 */
void updateShipping(Spedizione old_sped, Spedizione new_sped){

    if (old_sped.ID == new_sped.ID) { //aggiorno sse è la stessa spedizione
      old_sped.stato= new_sped.stato;
    }
   /*
     Aggiorno solo lo stato perchè considero che la spedizione venga fatta solo dallo stesso trasportatore ??
     Nel caso in cui venga presa in carico da diversi trasportatori modifica!
    */
  };


/*
  Funzione che assegna l'ordine ad un trasportatore, qundi crea una spedizione relativa a quell'ordine
  che verrà presa in carico da
 */
Trasportatore trasportatore_disponibile(){
  Trasportatore t;
  //TODO va fatto considerando il DB
  return t;
  };

void newShipping(Ordine o){

  Trasportatore t= trasportatore_disponibile(); //sarebbe il trasportatore disponibile a cui assegnare la spedizione , TODO penso vada implementato considerando il DB
  // Se non troviamo un trasportatore disponibile, stampiamo un errore
  if (t.P_IVA.empty()) {
      cerr << "Nessun trasportatore disponibile!" << endl;
      return;
    }
  //creiamo una nuova spedizione
  Spedizione new_sped;
  new_sped.ID = ;     // TODO vedere come assegnare l'id
  new_sped.ID_Ordine = o.ID;
  new_sped.ID_Trasportatore = t.P_IVA;
  new_sped.presa_in_gestione=time(nullptr);
  new_sped.stato = false;  //stato inziale false dato che la sped non è stata ancora completata
  //TODO inserire la spedizione nel databse
  //aggioriniamo l'oridne con l'id della spedizione
  o.ID_Spedizione=new_sped.ID;
  // Aggiungiamo la spedizione alla lista di spedizioni del trasportatore
  t.spedizioni_assegnate.push_back(new_sped);
  //TODO aggiorniamo l'ordine del DB per includere l'id della spedizione
  }