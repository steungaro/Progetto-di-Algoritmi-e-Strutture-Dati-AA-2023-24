#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define L_NOME_MAX 256
#define DIM_RICETTE 107
#define DIM_MAGAZZINO 107

char del[13];

typedef struct ingrediente {  //tipologia di dato ingrediente: contiene nome e quantità e puntatore al nextessivo
    char nome[L_NOME_MAX];
    int q;
    ingrediente* next;
} ingrediente_t;

typedef struct ricetta {  //tipologia di dato ricetta: contiene nom, lista ingredienti, pending (ordini in attesa), peso e ricetta successiva
    char nome[L_NOME_MAX];
    ingrediente_t* lista_ingredienti;
    int pending;
    int peso;
    ricetta* next;
} ricetta_t;

typedef struct { //tipologia dato tabella di ricette: contiene array di puntatori a ricette
    ricetta_t* elenco_ricette[DIM_RICETTE];
} ricettario_hash_t;

typedef struct ordine { //tipologia di dato ordine: contiene il puntatore alla ricetta, l'istante di ordine, la quantità, l'ordine successivo e il peso
    int t;
    ricetta_t* ricetta;
    int q;
    int peso;
    ordine* next;
} ordine_t;

typedef struct lotto { //tipologia dato lotto, contiene scadenza assoluta, quantità e puntatore al successivo
    int scadenza;
    int q;
    lotto* next;
} lotto_t;

typedef struct ingrediente_stock {
    ingrediente_stock* next;
    char nome[L_NOME_MAX];
    int totale;
    lotto_t* lotto;
} ingrediente_stock_t;

typedef struct {
    ingrediente_stock_t* stock_ingredienti[DIM_MAGAZZINO];
} magazzino_hash_t;

int funzione_hash(char* nome, int dim) { // accetta nome e dimensione hash table e restituisce indice.
    int h, i, c;
    h=5381;
    i=0;
    while(nome[i]!='\0') {
        h = h * 33 + nome[i];
        i++;
    }
    return h % dim;
}

void elimina_lista_ingredienti (ingrediente_t* lista) { //accetta la testa della lista di ingredienti ed elimina tutti i suoi componenti effettuando le free
    ingrediente_t* temp;
    while (lista != NULL) {
        temp = lista;
        lista = lista -> next;
        free(temp);
    }
}

ricettario_hash_t* crea_hash_ricettario() { // crea una tabella hash dinamica per il ricettario, con ciascuna ricetta puntante a NULL e restituisce l'indirizzo del ricettario oppure NULL se l'allocazione non è andata a buon fine
    ricettario_hash_t* ricettario;
    ricettario = malloc(sizeof(ricettario_hash_t));
    if (ricettario){
        int i;
        for (i = 0; i < DIM_RICETTE; i++) {
            ricettario->elenco_ricette[i] = NULL; // indirizzo nell'elenco == NULL
        }
        return ricettario;
    } else {
        return NULL;
    }
}

magazzino_hash_t* crea_hash_magazzino() { // crea una tabella hash dinamica per il magazzino, con ciascun ingrediente puntante a NULL e restituisce l'indirizzo del magazzino oppure NULL se l'allocazione non è andata a buon fine
    magazzino_hash_t* magazzino;
    magazzino = malloc(sizeof(magazzino_hash_t));
    if (magazzino){
        int i;
        for (i = 0; i < DIM_MAGAZZINO; i++) {
            magazzino->stock_ingredienti[i] = NULL; // indirizzo nell'elenco == NULL
        }
        return magazzino;
    } else {
        return NULL;
    }
}

int rifornimento(magazzino_hash_t* magazzino, char nome[], int scadenza, int quantita) { // riceve in ingresso un magazzino, il nome di un lotto, la scadenza e la quantità del lotto e cerca se c'è già un ingrediente per tale lotto (in caso aggiunge il lotto nella lista dei lotti e incrementa il totale disponibile), se non c'è lo crea. restituisce l'indice dell'ingrediente aggiunto, -1 se la malloc non ha funzionato.
    lotto_t *lottonuovo, *temp, *prev;
    ingrediente_stock_t *ingrediente_temp;
    int h;

    lottonuovo = malloc(sizeof(lotto_t));

    if (lottonuovo) { // se la malloc è andata a buon fine
        lottonuovo->scadenza = scadenza;
        lottonuovo->q = quantita;
        lottonuovo->next = NULL;

        h = funzione_hash(nome, DIM_MAGAZZINO);

        if (magazzino->stock_ingredienti[h] == NULL) { //se l'indice mi porta a un blocco vuoto, creo l'ingrediente e aggiungo il lotto

            ingrediente_temp = malloc(sizeof(ingrediente_stock_t));
                if (ingrediente_temp) { // se la malloc è andata bene
                    ingrediente_temp->totale = quantita; // salvo quantità, il lotto che sto aggiungendo, il nome
                    ingrediente_temp->lotto = lottonuovo;
                    strcpy (ingrediente_temp->nome, nome);
                    magazzino->stock_ingredienti[h] = ingrediente_temp;
                    magazzino->stock_ingredienti[h]->next = NULL;
                } else {
                    return -1;
                }
            
            return h;

        } else { //se l'indice mi porta a uno slot occupato
            
            for (ingrediente_temp = magazzino->stock_ingredienti[h]; !strcmp(ingrediente_temp->nome, nome) && ingrediente_temp->next != NULL; ingrediente_temp = ingrediente_temp->next); //percorro tutti gli ingredienti finché non trovo quello col nome giusto oppure finché non li ho esplorati tutti
            if (!strcmp(ingrediente_temp->nome, nome)) { // se i nomi combaciano
                ingrediente_temp->totale += quantita;
                for (temp = ingrediente_temp->lotto; temp != NULL && temp->scadenza < lottonuovo->scadenza; temp = temp->next) {
                    prev = temp;
                } //scorro i lotti fino al penultimo (ce n'è almeno uno perché quando tolgo tutti i lotti tolgo anche l'ingrediente dal magazzino) oppure finché il mio lotto non scade dopo a quello che sto aggiungendo
                prev->next = lottonuovo;
                lottonuovo->next = temp;
                return h;
            } else { //se i nomi non combaciano e sono arrivato al penultimo
                ingrediente_temp->next = malloc(sizeof(ingrediente_stock_t));
                if (ingrediente_temp->next) { //se la malloc è andata a buon fine
                    ingrediente_temp->next->totale = quantita;
                    ingrediente_temp->next->lotto = lottonuovo;
                    strcpy (ingrediente_temp->next->nome, nome);
                } else {
                    return -1;
                }
                return h;
            }
        }
    } else {
        return -1;
    }
}

void elimina_hash_ricettario (ricettario_hash_t* ricettario) { // accetta un ricettario e lo elimina, eliminando anche la lista degli ingredienti al suo interno
    int i;
    for (i=0; i<DIM_RICETTE; i++) {
        if(ricettario->elenco_ricette[i] != NULL){
            elimina_lista_ingredienti(ricettario->elenco_ricette[i]->lista_ingredienti);
            free(ricettario->elenco_ricette[i]); //passo indirizzo della i-esima ricetta
        }
    }
    free(ricettario);
}

void elimina_hash_magazzino (magazzino_hash_t* magazzino) { // accetta un magazzino e lo elimina, eliminando anche la lista degli ingredienti e dei lotti al suo interno
    int i;
    lotto_t* temp;

    for (i=0; i < DIM_MAGAZZINO; i++) { //per ogni elemento del magazzino
        if(magazzino->stock_ingredienti[i] != NULL){ // se contiene qualcosa
            while (magazzino->stock_ingredienti[i]->lotto != NULL) {
                temp = magazzino->stock_ingredienti[i]->lotto;
                magazzino->stock_ingredienti[i]->lotto = magazzino->stock_ingredienti[i]->lotto->next;
                free (temp); //per ogni lotto distruggi lotto
            }            
        }
    }
    free(magazzino); //distruggi magazzino
}

ingrediente_t* aggiungi_ingrediente(ingrediente_t* lista,  char* nome, int q) { // riceve una lista di ingredienti, il nome di un ingrediente e la sua quantità e restituisce la lista aggiornata se l'aggiunta è andata a buon fine e NULL altrimenti
    ingrediente_t* temp;
    temp = malloc(sizeof(ingrediente_t));
    if(temp) {
        strcpy(temp->nome, nome);
        temp->next = lista;
        return temp;
    } else {
        return NULL;
    }
}

int aggiungi_ricetta(ricettario_hash_t* ricettario,  char* nome_ricetta, ingrediente_t* lista_ingredienti, int peso) { // riceve in ingresso un ricettario, il nome di una ricetta, la lista degli ingredienti di tale ricetta e il suo peso e restituisce l'indice della ricetta aggiunta al ricettario se è andata a buon fine, -1 se l'allocazione non è andata a buon fine e -2 se la ricetta era già presente
    int i;
    ricetta_t* temp_ricetta, *scorr;
    i = funzione_hash(nome_ricetta, DIM_RICETTE);

    temp_ricetta = NULL;
    temp_ricetta = malloc(sizeof(ricetta_t));
        if(temp_ricetta) {
            strcpy (temp_ricetta->nome, nome_ricetta);
            temp_ricetta->lista_ingredienti = lista_ingredienti;
            temp_ricetta->pending = 0;
            temp_ricetta->peso = peso;

            if (ricettario->elenco_ricette[i] == NULL) { // indirizzo della ricetta è vuoto
                ricettario->elenco_ricette[i] = temp_ricetta;
                return i;
            } else {
                scorr = ricettario.elenco_ricette[i];
                while (scorr -> next != NULL) {
                    if (!strcmp(scorr->nome, nome_ricetta)) { // se durante lo scorrimento la trovo, annullo l'allocazione di memoria e ritorno 0
                        free(temp_ricetta);
                        return -2;
                    }
                    scorr = scorr -> next;
                }
                scorr -> next = temp_ricetta;
                return i;
            }
        } else {
            return -1;
        }
    
}

ricetta_t* ricerca_ricetta(ricettario_hash_t* ricettario,  char* nome_ricetta) { // riceve un ricettario e il nome di una ricetta e restituisce il puntatore a quella ricetta e NULL se la ricetta non esiste
    int i;
    ricetta_t* temp;
    i = funzione_hash(nome_ricetta, DIM_RICETTE);
    temp = ricettario->elenco_ricette[i];

    for (; temp != NULL; temp = temp -> next) {
        if (!strcmp(temp->nome, nome_ricetta)) {
            return temp;
        }
    }
    return NULL;
}

int elimina_ricetta(ricettario_hash_t* ricettario, char* nome_ricetta) { // riceve un ricettario e la ricetta da rimuovere e restituisce 1 se viene eliminata, -1 se non esiste, -2 se ci sono ordini in attesa e quindi non la elimina
    int i;
    ricetta_t* curr, *prec, *temp;
    prec = NULL;

    i = funzione_hash(nome_ricetta, DIM_RICETTE);
    curr = ricettario->elenco_ricette[i];

    while (curr != NULL) {
        if (!strcmp(curr->nome, nome_ricetta)) {
            if (curr->pending > 0) {
                return -2;
            } else {
                temp = curr;
                curr = curr -> next;
                if(prec != NULL) {
                    prec -> next = curr;
                } else {
                    ricettario->elenco_ricette[i] = curr;
                }
                elimina_lista_ingredienti(temp->lista_ingredienti);
                free (temp);
            }
        } else {
            prec = curr;
            curr = curr -> next;
        }
    }           
    return -1;
}

int aggiungi_ordine_in_coda(ordine_t* coda, ricetta_t* ricetta, int q, int t) { // USARE AGGIUNGI ORDINE accetta la coda degli ordini (NULL se vuota), la ricetta dell'ordine da eseguire, la quantità, il tempo assoluto e restituisce 1 se l'operazione è andata a buon fine e -1 altrimenti
    ordine_t* ordine, *temp;
    temp = coda;
    ordine = malloc(sizeof(ordine_t);)

    if (ordine) {
        ordine->q = q;
        ordine->t = t;
        ordine->ricetta = ricetta;
        ordine->peso = ricetta->peso;
        ordine->next = NULL;
        if (temp) {
            for(; temp->next != NULL; temp=temp->next);
            temp->next = ordine;
        } else {
            coda = ordine;
        }
        return 1;
    } else {
        return -1;
    }
}

int aggiungi_ordine(ordine_t* coda, ricettario_hash_t* ricettario, char nome, int q, int t) { //accetta una coda degli ordini, (NULL se vuota), il ricettario, il nome della ricetta, la quantità e il tempo assoluto e restituisce 1 se l'ordine è andato a buon fine, -1 se la ricetta non esiste. inoltre aggiunge pending alla ricetta in questione
    ricetta_t* ricetta;
    ricetta = ricerca_ricetta(ricettario, nome);
    if (ricetta) {
        ricettario->elenco_ricette[i]->pending++;
        return aggiungi_ordine_in_coda(coda, ricetta, q, t, i);
    } else {
        return -1;
    }
}

ordine_t* aggiungi_ordine_in_lista(ordine_t* lista, ricettario_hash_t* ricettario, ordine_t* ordine) { //accetta la lista di prodotti pronti da spedire, il ricettario, l'ordine (mantiene l'entità ordine, non fare free ma toglierne riferimento dalla coda ordini) e aggiunge l'ordine passato alla coda degli ordini completati ordinata per tempo
    ordine_t* temp;
    int i, j;
    temp = lista;
    ordine->next = NULL;
    if (temp) {
        for(; temp->next != NULL && temp->t < ordine->t; temp = temp->next);
        if (temp->next == NULL) {
            temp->next = ordine;
        } else {
            ordine->next = temp->next->next;
            temp->next = ordine;
        }
    } else {
        lista = ordine;
    }
    return lista;
}

int abbastanza_ingrediente(magazzino_hash_t* magazzino, char nome[], int qt) {
    int h;
    ingrediente_stock_t* ingrediente_temp;

    h = funzione_hash(nome, DIM_MAGAZZINO);

        if (magazzino->stock_ingredienti[h] == NULL) { //se l'indice mi porta a un blocco vuoto, ritorno 0
            return 0;

        } else { //se l'indice mi porta a uno slot occupato
            
            for (ingrediente_temp = magazzino->stock_ingredienti[h]; !strcmp(ingrediente_temp->nome, nome) && ingrediente_temp->next != NULL; ingrediente_temp = ingrediente_temp->next); //percorro tutti gli ingredienti finché non trovo quello col nome giusto oppure finché non li ho esplorati tutti
            if (!strcmp(ingrediente_temp->nome, nome)) { // se i nomi combaciano
                if (ingrediente_temp->totale >= qt) {
                    return 1;
                }
            } else { //se i nomi non combaciano e sono arrivato al penultimo
                return 0;
            }
        }
}

void togli_ingrediente(magazzino_hash_t* magazzino, char nome[], int qt) {
    int h, resto;
    ingrediente_stock_t* ingrediente_temp;
    lotto_t* lotto_temp, lotto_prev;

    h = funzione_hash(nome, DIM_MAGAZZINO);

    for (ingrediente_temp = magazzino->stock_ingredienti[h]; !strcmp(ingrediente_temp->nome, nome) && ingrediente_temp->next != NULL; ingrediente_temp = ingrediente_temp->next); //percorro tutti gli ingredienti finché non trovo quello col nome giusto oppure finché non li ho esplorati tutti
    if (!strcmp(ingrediente_temp->nome, nome)) { // se i nomi combaciano
        if (ingrediente_temp->totale >= qt) {
            ingrediente_temp->totale -= qt;
            if (ingrediente_temp->totale == 0) {
                lotto_temp = ingrediente_temp->lotto;
                while (lotto_temp != NULL) {
                    lotto_prev = lotto_temp;
                    lotto_temp = lotto_temp->next;
                    free(lotto_temp);
                }
            } else {
                resto = qt;
                
                while (resto > 0) {
                    lotto_temp = ingrediente_temp->lotto;
                    if (lotto_temp->q > resto) {
                        lotto_temp->q -= resto;
                        resto = 0;
                    } else {
                        resto -= lotto_temp->q;
                        ingrediente_temp->lotto = lotto_temp->next;
                        free(lotto_temp); 
                    }
                }
            }
        }
    }
}

ordine_t* cucina(magazzino_hash_t* magazzino, ordine_t* codaordini, ordine_t** codaprodotti) { //dopo l'aggiunta di un ordine o dopo il rifornimento va avviata: controlla gli elementi della coda ordini e prepara quelli i cui ingredienti sono disponibili nel magazzino, restituisce la nuova coda ordini e modifica la coda prodotti passata
    int i, h, cucinabile;
    ordine_t* temp_ordine, prev_ordine, prodotto;
    ingrediente_t* temp_ingrediente;

    temp_ordine = codaordini;
    prev_ordine = NULL;

    while (temp_ordine != NULL) {
        cucinabile = 1;
        for(temp_ingrediente = temp_ordine->ricetta->lista_ingredienti; temp_ingrediente != NULL; temp_ingrediente = temp_ingrediente->next) {
            cucinabile *= abbastanza_ingrediente(magazzino, temp_ingrediente->nome, temp_ingrediente->q);
        }
        if (cucinabile) {
            for(temp_ingrediente = temp_ordine->ricetta->lista_ingredienti; temp_ingrediente != NULL; temp_ingrediente = temp_ingrediente->next) {
                togli_ingrediente(magazzino, nome, qt);
            }
            prodotto = temp_ordine;
            temp_ordine = temp_ordine->next;
            if (prev_ordine != NULL) {
                prev_ordine -> next = temp_ordine;
            } else {
                codaordini = temp_ordine;
            }
            *codaprodotti = aggiungi_ordine_in_lista(*codaprodotti, ricettario, prodotto);
        } else {
            prev_ordine = temp_ordine;
            temp_ordine = temp_ordine -> next;            
        }
    }
    return codaordini;
}

void scadenze(magazzino_hash_t* magazzino, int t) { //da eseguire prima di qualsiasi cosa, controlla i lotti del magazzino ed elimina quelli la cui scadenza eccede il tempo assoluto
    ingrediente_stock_t *ingrediente_temp;
    lotto_t *lotto_temp;
    int i;

    for (i = 0; i < DIM_MAGAZZINO; i++) {
        ingrediente_temp = magazzino->stock_ingredienti[i];
        while (ingrediente_temp != NULL) {
            lotto_temp = ingrediente_temp->lotto;
            while (lotto_temp && lotto_temp->scadenza <= t) {
                ingrediente_temp->lotto = lotto_temp->next;
                ingrediente_temp->totale -= lotto_temp->q;
                free(lotto_temp);
                lotto_temp = ingrediente_temp->lotto;
            }
        }
    }
}


ordine_t* consegne (int peso, ordine_t* coda) { //controlla tutti i prodotti disponibili e fa il controllo per il peso del furgoncino, quando l'ordine viene spedito va fatta la free dell'entità ordine
    ordine_t *temp_prodotto, *prec_prodotto, *consegna, *temp_consegna, *prec_consegna;
    int residuo, cons, stop;

    residuo = peso;
    cons = 0;
    consegna = NULL;

    /* Periodicamente, il corriere si reca dalla pasticceria a ritirare gli ordini pronti. All’arrivo del corriere, gli ordini da caricare vengono scelti in ordine cronologico di arrivo. Il processo si ferma appena viene incontrato un ordine che supera la capienza rimasta (in grammi) sul camioncino. Si assuma che il peso di ogni dolce preparato sia uguale alla somma delle quantità in grammi di ciascun ingrediente. Ogni ordine viene sempre caricato nella sua interezza. Scelti gli ordini, la pasticceria procede a caricarli in ordine di peso decrescente. A parità di peso, gli ordini vanno caricati in ordine cronologico di arrivo.

    In aggiunta alle stampe sopraelencate, il programma stampa gli ordini contenuti nel camioncino del corriere come una sequenza di triple ⟨istante_di_arrivo_ordine⟩ ⟨nome_ricetta⟩ ⟨numero_elementi_ordinati⟩, una per rigo, in ordine di caricamento. Dato un corriere di periodicità n, la stampa è effettuata prima di gestire i comandi al tempo kn con k ∈ 1, 2, . . .. Se il camioncino è vuoto, viene stampato il messaggio camioncino vuoto. */

    temp_prodotto = coda; 
    prec_prodotto = NULL;
    stop = 1;

    while (temp_prodotto != NULL && stop) { //scorro tutti i prodotti fino a quando non arrivo a NULL o finché non ho trovato il primo che non ci sta sul camioncino (scorro in ordine temporale)
        if (temp_prodotto->peso <= residuo) { //se il peso del mio prodotto è minore di quanto ancora posso caricare
            cons = 1; //ho fatto almeno una consegna
            residuo -= temp_prodotto->peso; //aggiorno il peso residuo che posso caricare togliendo quanto caricato


            if (consegna == NULL) { //aggiunta del prodotto nella nuova coda consegne, se la lista è vuota prende il prodotto
                consegna = temp_prodotto; //il prodotto attuale diventa il primo della lista consegne
                temp_prodotto = temp_prodotto->next; //il prodotto che analizzo alla prossima iterazione del while è il successivo a quello di ora
            } else { // se non è vuota (lista ordinata per peso decrescente)
                prec_consegna = NULL;
                temp_consegna = consegna;
                while (temp_consegna != NULL && temp_consegna->peso > temp_prodotto->peso) { //scorro finché la temp_consegna non punta a qualcosa di peso <= al mio ordine
                    prec_consegna = temp_consegna;
                    temp_consegna = temp_consegna -> next;
                }
                if (prec_consegna == NULL) { //mi trovo al primo elemento della lista, che pesa <= al mio ordine
                    if (temp_consegna->peso < temp_prodotto->peso) { //se l'elemento è più leggero
                        temp_prodotto->next = temp_consegna;
                        consegna->next = temp_prodotto;
                    } else { //se l'elemento pesa uguale
                        if (temp_consegna->t< temp_prodotto->t) { //se l'elemento pesa uguale ma è più giovane (t più grande)
                        temp_prodotto->next = temp_consegna;
                        consegna->next = temp_prodotto;
                        }
                    }
                } else { // se non sono all'inizio della lista
                    if (temp_consegna->peso < temp_prodotto->peso) { //se l'elemento è più leggero
                    temp_prodotto->next = temp_consegna;
                    prec->next = temp_prodotto;
                    } else { //se l'elemento pesa uguale
                        if (temp_consegna->t< temp_prodotto->t) { //se l'elemento pesa uguale ma è più giovane (t più grande)
                        temp_prodotto->next = temp_consegna;
                        prec->next = temp_prodotto;
                        }
                    }   
                }
            }

        } else { //se il prodotto è troppo grande mi fermo
            stop = 0;
        }
        
    }
}

int main () {
    char input[L_NOME_MAX], nome[L_NOME_MAX], ingrediente[L_NOME_MAX], ordine[L_NOME_MAX];
    int t, fc, pc, qt, peso, ret, sc;
    char nl;
    ricettario_hash_t * ricettario;
    ordine_t* coda_ordini, *coda_prodotti;
    magazzino_hash_t * magazzino;
    ingrediente_t* lista_ingredienti;
    ricetta_t* tempricetta;

    ricettario = NULL;
    coda_ordini = NULL;
    coda_prodotti = NULL;
    magazzino = NULL;

    ricettario = crea_hash_ricettario();
    magazzino = crea_hash_magazzino();

    if (ricettario == NULL || magazzino == NULL) {
        printf("Errore creazione magazzino e/o ricettario.\n");
        return 0;
    }
    t=0;
    if(scanf("%d %d ", &fc, &pc) )

    while(scanf(" %s", input) != -1) {

        if (t != 0) { // eseguito ogni volta
            scadenze(magazzino, t);
        };

        if (t!=0 && t % fc == 0) { // eseguito a ogni frequenza di passaggio del corriere
            coda_prodotti = consegne(t, coda_prodotti);
        }

        if (!strcmp(input, "aggiungi_ricetta")) { 
            tempricetta = NULL;
            if (scanf (" %s", nome));

            tempricetta = ricerca_ricetta(ricettario, nome);

            if (tempricetta == NULL) {
                lista_ingredienti=NULL;
                peso = 0;
                if (scanf("%c", &nl));
                while (nl != "\n") {
                    if (scanf("%s %d", ingrediente, &qt));
                    peso += qt;
                    lista_ingredienti = aggiungi_ingrediente(lista_ingredienti, ingrediente, qt);
                    if (scanf("%c", &nl));
                }
                ret = aggiungi_ricetta(ricettario, nome, lista_ingredienti, peso);
                if (ret == -2) {
                    elimina_lista_ingredienti(lista_ingredienti);
                    printf("ignorato\n");
                } else if (ret == -1) {
                    elimina_lista_ingredienti(lista_ingredienti);
                    printf("errore aggiunta ricetta.\n");
                } else {
                    printf("aggiunta\n");
                }
            } else {
                if (scanf (" %*[^\n]"));
                printf("ignorato\n");
            }

        } else if (!strcmp(input, "rimuovi_ricetta")) {
            if (scanf (" %[^\n]", nome));
            ret = elimina_ricetta(ricettario, nome);
            if (ret == 1) {
                printf("rimossa\n");
            } else if (ret == -1) {
                printf("non presente\n");
            } else {
                printf("ordini in sospeso\n");
            }

        } else if (!strcmp(input, "rifornimento")) {
            if (scanf("%c", &nl));
            while (nl != "\n") {
                if (scanf("%s %d %d", nome, &qt, &sc));
                ret = rifornimento(magazzino, nome, sc, qt);
                if (ret == -1) {
                    printf("errore rifornimento\n");
                }
                if (scanf("%c", &nl));
            }
            printf("rifornito\n");
            coda_ordini = cucina(magazzino, coda_ordini, &coda_prodotti);

        } else if (!strcmp(input, "ordine")) {
            if (scanf(" %s %d", nome, &qt));
            ret = aggiungi_ordine(coda_ordini, ricettario, nome, qt, t);
            if(ret == 1) {
                printf("accettato\n");
                coda_ordini = cucina(magazzino, coda_ordini, &coda_prodotti);
            } else {
                printf("rifiutato\n");
            }
        } else {
            printf ("Errore \n");
        }
        t++;
    }

    elimina_hash_ricettario(ricettario);
    elimina_hash_magazzino(magazzino);
    return 0;    
}