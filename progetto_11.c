#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define L_NOME_MAX 256
#define DIM_RICETTE 3017
#define DIM_MAGAZZINO 3017

typedef struct scadenze {
    int s;
    struct scadenze *next;
    struct scadenze *prev;
} scadenze_t;

typedef struct ingrediente {  //tipologia di dato ingrediente: contiene nome e quantità e puntatore al nextessivo
    char nome[L_NOME_MAX];
    int h;
    unsigned int q;
    struct ingrediente *next;
} ingrediente_t;

typedef struct ricetta {  //tipologia di dato ricetta: contiene nom, lista ingredienti, pending (ordini in attesa), peso e ricetta successiva
    char nome[L_NOME_MAX];
    ingrediente_t* lista_ingredienti;
    unsigned long long pending;
    unsigned long long peso;
    struct ricetta* next;
} ricetta_t;

typedef struct { //tipologia dato tabella di ricette: contiene array di puntatori a ricette
    ricetta_t* elenco_ricette[DIM_RICETTE];
} ricettario_hash_t;

typedef struct ordine { //tipologia di dato ordine: contiene il puntatore alla ricetta, l'istante di ordine, la quantità, l'ordine successivo e il peso
    unsigned int t;
    ricetta_t* ricetta;
    unsigned int q;
    unsigned long long peso;
    struct ordine* next;
    struct ordine* prev;
} ordine_t;

typedef struct lotto { //tipologia dato lotto, contiene scadenza assoluta, quantità e puntatore al successivo
    unsigned int scadenza;
    unsigned int q;
    struct lotto* next;
} lotto_t;

typedef struct ingrediente_stock {
    struct ingrediente_stock* next;
    char nome[L_NOME_MAX];
    unsigned long long totale;
    lotto_t* lotto;
} ingrediente_stock_t;

typedef struct {
    ingrediente_stock_t* stock_ingredienti[DIM_MAGAZZINO];
} magazzino_hash_t;

int funzione_hash(char* nome, int dim) { // accetta nome e dimensione hash table e restituisce indice.
    int i;
    unsigned long long h;
    h=5381;
    i=0;
    while(nome[i]!='\0') {
        h = ((h << 5) + h) + nome[i];
        i++;
    }
    return (int) (h % (unsigned long long) dim);
}

void elimina_lista_ingredienti (ingrediente_t* lista) { //accetta la testa della lista di ingredienti ed elimina tutti i suoi componenti effettuando le free
    ingrediente_t* temp;
    while (lista != NULL) {
        temp = lista;
        lista = lista -> next;
        free(temp);
        temp = NULL;
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

int rifornimento(magazzino_hash_t* magazzino, char nome[], unsigned int scadenza, unsigned int quantita) { // riceve in ingresso un magazzino, il nome di un lotto, la scadenza e la quantità del lotto e cerca se c'è già un ingrediente per tale lotto (in caso aggiunge il lotto nella lista dei lotti e incrementa il totale disponibile), se non c'è lo crea. restituisce l'indice dell'ingrediente aggiunto, -1 se la malloc non ha funzionato.
    lotto_t *lottonuovo, *temp, *prev;
    ingrediente_stock_t *ingrediente_temp, *ingrediente_prev;
    int h;

    ingrediente_prev = NULL;

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
            
            for (ingrediente_temp = magazzino->stock_ingredienti[h]; ingrediente_temp != NULL; ingrediente_temp = ingrediente_temp->next) { //percorro tutti gli ingredienti  finché non li ho esplorati tutti
                if (!strcmp(ingrediente_temp->nome, nome)) { // se i nomi combaciano
                    ingrediente_temp->totale += quantita;
                    prev = NULL;
                    for (temp = ingrediente_temp->lotto; temp != NULL && temp->scadenza < lottonuovo->scadenza; temp = temp->next) {
                        prev = temp;
                    } //scorro i lotti fino al penultimo (ce n'è almeno uno perché quando tolgo tutti i lotti tolgo anche l'ingrediente dal magazzino) oppure finché il mio lotto non scade dopo a quello che sto aggiungendo
                    if (prev != NULL) { // se non è il primo
                        prev->next = lottonuovo;
                        lottonuovo->next = temp;
                    } else {
                        ingrediente_temp->lotto = lottonuovo;
                        lottonuovo->next = temp;
                    }
                    return h;
                }
                ingrediente_prev = ingrediente_temp;
            } 
            ingrediente_prev->next = malloc(sizeof(ingrediente_stock_t));
            if (ingrediente_prev->next) { //se la malloc è andata a buon fine
                ingrediente_prev->next->totale = quantita;
                ingrediente_prev->next->lotto = lottonuovo;
                ingrediente_prev->next->next = NULL;
                strcpy (ingrediente_prev->next->nome, nome);
            } else {
                return -1;
            }
            return h;
            
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
            ricettario->elenco_ricette[i] = NULL;
        }
    }
    free(ricettario);
    ricettario = NULL;
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
                temp = NULL;
            }
            free(magazzino->stock_ingredienti[i]);
            magazzino->stock_ingredienti[i] = NULL;        
        }
    }
    free(magazzino); //distruggi magazzino
    magazzino = NULL;
}

void elimina_coda (ordine_t *coda) {
    ordine_t* temp;
    if (coda) {
        if (coda->prev) {
            coda->prev->next = NULL;
        }
        while (coda != NULL) {
            temp = coda;
            
            coda = coda->next;
            free(temp);
        }
    }
}
/*
void MergeSort(scadenze_t** lista) {
    scadenze_t* testa = *lista;
    scadenze_t* a;
    scadenze_t* b;
 
    if ((testa == NULL) || (testa->next == NULL)) {
        return;
    }
 
    FrontBackSplit(testa, &a, &b);
    MergeSort(&a);
    MergeSort(&b);
    *lista = SortedMerge(a, b);
}
 
scadenze_t* SortedMerge(scadenze_t* a, scadenze_t* b) {
    scadenze_t* result = NULL;
 
    // Base cases 
    if (a == NULL){
        return (b);
    } else if (b == NULL){
        return (a); 
    }
    if (a->s <= b->s) {
        result = a;
        result->next = SortedMerge(a->next, b);
    } else {
        result = b;
        result->next = SortedMerge(a, b->next);
    }
    return (result);
}

void FrontBackSplit(scadenze_t* lista, scadenze_t** avanti, scadenze_t** indietro) {
    scadenze_t* veloce;
    scadenze_t* lento;
    lento = lista;
    veloce = lista->next;
    
    while (veloce != NULL) {
        veloce = veloce->next;
        if (veloce != NULL) {
            lento = lento->next;
            veloce = veloce->next;
        }
    }
 
    *avanti = lista;
    *indietro = lento->next;
    lento->next = NULL;
}
*/
scadenze_t *aggiungi_scadenza (scadenze_t* lista, int sc) {
    scadenze_t *tempscadenza, *scorr;
    tempscadenza = NULL;
    scorr = lista;

    if (lista) {
        for (; scorr->s < sc && scorr->next != scorr && scorr->next != lista; scorr = scorr->next);
        if (scorr->s == sc) {
            return NULL;
        } else {
            tempscadenza = malloc(sizeof(scadenze_t));
            if (tempscadenza) {
                if (scorr->next == scorr){
                    tempscadenza->s = sc;
                    scorr->prev = tempscadenza;
                    tempscadenza->prev = scorr;
                    scorr->next = tempscadenza;
                    tempscadenza->next = scorr;
                    if(scorr->s > scorr->next->s) {
                        lista = lista->next;
                    }
                    return lista;

                } else if (scorr->next == lista && scorr->s < sc) {
                    tempscadenza->s = sc;
                    tempscadenza->next = lista;
                    tempscadenza->prev = lista->prev;
                    lista->prev->next = tempscadenza;
                    lista->prev = tempscadenza;
                    return lista;
                } else if (scorr == lista) {
                    if (lista->s < sc) {
                        tempscadenza->s = sc;
                        tempscadenza->next = lista->next;
                        tempscadenza->prev = lista;
                        lista->next->prev = tempscadenza;
                        lista->next = tempscadenza;
                        return lista;
                    } else {
                        tempscadenza->s = sc;
                        tempscadenza->next = lista;
                        tempscadenza->prev = lista->prev;
                        lista->prev->next = tempscadenza;
                        lista->prev = tempscadenza;
                        lista = lista->prev;
                        return lista;
                    }
                } else {
                    tempscadenza->s = sc;
                    tempscadenza->prev = scorr->prev;
                    scorr->prev->next = tempscadenza;
                    scorr->prev = tempscadenza->prev;
                    tempscadenza->next = scorr;
                    return lista;
                }
            } else {
                printf("errore scadenze\n");
                return NULL;
            }
            return lista;
        }
    } else {
        lista = malloc (sizeof(scadenze_t));
        if (lista) {
            lista -> s = sc;
            lista -> next = lista;
            lista -> prev = lista;
            return lista;
        } else {
            printf("errore scadenze\n");
            return NULL;
        }
    }
    
}

ingrediente_t* aggiungi_ingrediente(ingrediente_t* lista,  char* nome, unsigned int q) { // riceve una lista di ingredienti, il nome di un ingrediente e la sua quantità e restituisce la lista aggiornata se l'aggiunta è andata a buon fine e NULL altrimenti
    ingrediente_t* temp;
    temp = malloc(sizeof(ingrediente_t));
    if(temp) {
        strcpy(temp->nome, nome);
        temp->next = lista;
        temp->q = q;
        temp->h = funzione_hash(nome, DIM_MAGAZZINO);
        return temp;
    } else {
        return NULL;
    }
}

int aggiungi_ricetta(ricettario_hash_t* ricettario,  char* nome_ricetta, ingrediente_t* lista_ingredienti, unsigned long long peso) { // riceve in ingresso un ricettario, il nome di una ricetta, la lista degli ingredienti di tale ricetta e il suo peso e restituisce l'indice della ricetta aggiunta al ricettario se è andata a buon fine, -1 se l'allocazione non è andata a buon fine
    int i;
    ricetta_t* temp_ricetta;
    i = funzione_hash(nome_ricetta, DIM_RICETTE);

    temp_ricetta = NULL;
    temp_ricetta = malloc(sizeof(ricetta_t));
        if(temp_ricetta) {
            strcpy (temp_ricetta->nome, nome_ricetta);
            temp_ricetta->lista_ingredienti = lista_ingredienti;
            temp_ricetta->pending = 0;
            temp_ricetta->peso = peso;
            temp_ricetta->next = NULL;

            if (ricettario->elenco_ricette[i] == NULL) { // indirizzo della ricetta è vuoto
                ricettario->elenco_ricette[i] = temp_ricetta;
                return i;
            } else {
                temp_ricetta->next = ricettario->elenco_ricette[i];
                ricettario->elenco_ricette[i] = temp_ricetta;
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
    ricetta_t *curr, *prec, *temp;
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
                return 1;
            }
        } else {
            prec = curr;
            curr = curr -> next;
        }
    }           
    return -1;
}

ordine_t* aggiungi_ordine_in_coda(ordine_t* coda, ricetta_t* ricetta, unsigned int q, unsigned int t, unsigned int *ret) { // USARE AGGIUNGI ORDINE accetta la coda degli ordini (NULL se vuota), la ricetta dell'ordine da eseguire, la quantità, il tempo assoluto e restituisce 1 se l'operazione è andata a buon fine e -1 altrimenti
    ordine_t *ordine, *ultimo;
    ordine = malloc(sizeof(ordine_t));

    if (ordine) {
        ordine->q = q;
        ordine->t = t;
        ordine->ricetta = ricetta;
        ordine->peso = ricetta->peso * q;
        ordine->next = coda;
        if (coda) {
            ultimo = coda->prev;
            ultimo -> next = ordine;
            ordine->prev = ultimo;
            coda->prev = ordine;
        } else {
            coda = ordine;
            ordine->prev = ordine;
            ordine->next = coda;
        }
        *ret = 1;
        return coda;
    } else {
        *ret = -1;
        return coda;
    }
}

ordine_t* aggiungi_ordine(ordine_t* coda, ricettario_hash_t* ricettario, char* nome, unsigned int q, unsigned int t, unsigned int *ret) { //accetta una coda degli ordini, (NULL se vuota), il ricettario, il nome della ricetta, la quantità e il tempo assoluto e restituisce 1 se l'ordine è andato a buon fine, NULL se la ricetta non esiste. inoltre aggiunge pending alla ricetta in questione
    ricetta_t* ricetta;
    ricetta = ricerca_ricetta(ricettario, nome);
    if (ricetta) {
        ricetta->pending ++;
        return aggiungi_ordine_in_coda(coda, ricetta, q, t, ret);
    } else {
        *ret = -1;
        return coda;
    }
}

ordine_t* aggiungi_ordine_in_lista(ordine_t* lista, ordine_t* ordine) { //accetta la lista di prodotti pronti da spedire, il ricettario, l'ordine (mantiene l'entità ordine, non fare free ma toglierne riferimento dalla coda ordini) e aggiunge l'ordine passato alla coda degli ordini completati ordinata per tempo
    ordine_t* temp, *prec;
    int stop;

    stop = 1;
    temp = lista;
    prec = NULL;
    ordine->next = NULL;
    ordine->prev = NULL;

    if (temp) {
        while(temp != NULL && stop) {
            if (temp->t < ordine->t) {
            prec = temp;
            temp = temp->next;
            } else {
                stop = 0;
            }
        }
        if (temp == NULL) {
           prec->next = ordine;
           ordine->prev = prec;
        } else {
            if (prec) {
            ordine->next = temp;
            temp->prev = ordine;
            prec->next = ordine;
            ordine->prev = prec;
            } else {
                lista = ordine;
                ordine -> next = temp;
                temp->prev = ordine;
            }
        }
    } else {
        lista = ordine;
    }
    return lista;
}

int abbastanza_ingrediente(magazzino_hash_t* magazzino, char nome[], int h, unsigned long long qt) {
    ingrediente_stock_t* ingrediente_temp;    

    if (magazzino->stock_ingredienti[h] == NULL) { //se l'indice mi porta a un blocco vuoto, ritorno 0
        return 0;
    } else { //se l'indice mi porta a uno slot occupato
        
        for (ingrediente_temp = magazzino->stock_ingredienti[h]; ingrediente_temp != NULL; ingrediente_temp = ingrediente_temp->next){ //percorro tutti gli ingredienti finché non trovo quello col nome giusto oppure finché non li ho esplorati tutti
            if (!strcmp(ingrediente_temp->nome, nome)) { // se i nomi combaciano
                if (ingrediente_temp->totale >= qt) {
                    return 1;
                } else {
                    return 0;
                }
            }
        }
        return 0;
    }
}

void togli_ingrediente(magazzino_hash_t* magazzino, char nome[], unsigned long long qt) {
    int h, stop;
    unsigned long long resto;
    ingrediente_stock_t* ingrediente_temp, *elim, *prev;
    lotto_t* lotto_temp;

    prev = NULL;
    stop = 0;

    h = funzione_hash(nome, DIM_MAGAZZINO);
    ingrediente_temp = magazzino->stock_ingredienti[h];
    while (ingrediente_temp != NULL && !stop) {
        if (!strcmp(ingrediente_temp->nome, nome)) {
            stop = 1; // se i nomi combaciano
            if (ingrediente_temp->totale >= qt) {
                ingrediente_temp->totale -= qt;
                if (ingrediente_temp->totale == 0) {
                    lotto_temp = ingrediente_temp->lotto;
                    while (lotto_temp != NULL) {
                        ingrediente_temp->lotto = lotto_temp->next;
                        free(lotto_temp);
                        lotto_temp = ingrediente_temp->lotto;
                    }
                    ingrediente_temp->lotto = NULL;
                    elim = ingrediente_temp;
                    ingrediente_temp = ingrediente_temp->next;
                    if(prev != NULL) {
                        prev->next = ingrediente_temp;
                    } else {
                        magazzino->stock_ingredienti[h] = ingrediente_temp;
                    }
                    free(elim);
                    elim = NULL;
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
                            lotto_temp = NULL;
                            
                        }
                    }
                    ingrediente_temp = ingrediente_temp->next;
                }
            }
        } else {
            prev = ingrediente_temp;
            ingrediente_temp = ingrediente_temp->next;
        }
    }
}

ordine_t* cucina(magazzino_hash_t* magazzino, ordine_t* codaordini, ordine_t** codaprodotti, int tipo) { //dopo l'aggiunta di un ordine o dopo il rifornimento va avviata: controlla gli elementi della coda ordini e prepara quelli i cui ingredienti sono disponibili nel magazzino, restituisce la nuova coda ordini e modifica la coda prodotti passata
    int cucinabile, controlla;
    ordine_t* temp_ordine, *prodotto;
    ingrediente_t* temp_ingrediente;

    temp_ordine = codaordini;
    controlla = 1;

    if (tipo) {
        temp_ordine = codaordini->prev;
    }

    if (codaordini == NULL) {
        controlla = 0;
    }

    while (controlla) {
        cucinabile = 1;
        for(temp_ingrediente = temp_ordine->ricetta->lista_ingredienti; temp_ingrediente != NULL && cucinabile; temp_ingrediente = temp_ingrediente->next) {
            cucinabile *= abbastanza_ingrediente(magazzino, temp_ingrediente->nome, temp_ingrediente->h, temp_ingrediente->q * temp_ordine->q);
        }
        
        if (cucinabile) {
            for(temp_ingrediente = temp_ordine->ricetta->lista_ingredienti; temp_ingrediente != NULL; temp_ingrediente = temp_ingrediente->next) {
                togli_ingrediente(magazzino, temp_ingrediente->nome, temp_ingrediente->q * temp_ordine->q);
            }
            prodotto = temp_ordine;
            temp_ordine = temp_ordine->next;
            if (prodotto->next != prodotto) {
            
                prodotto->prev->next = prodotto->next;
                prodotto->next->prev = prodotto->prev;
                if (codaordini == prodotto) {
                    codaordini = prodotto->next;
                }
            } else {
                codaordini = NULL;
                controlla = 0;
            }
            *codaprodotti = aggiungi_ordine_in_lista(*codaprodotti, prodotto);
            if (controlla && temp_ordine->next->t < temp_ordine->t) {
                controlla = 0;
            }
        } else {
            temp_ordine = temp_ordine->next;
            if (controlla && temp_ordine->next->t <= temp_ordine->t) {
                controlla = 0;
            }
        }
        
    }
    return codaordini;
}

void scadenze(magazzino_hash_t* magazzino, unsigned int t) { //da eseguire prima di qualsiasi cosa, controlla i lotti del magazzino ed elimina quelli la cui scadenza eccede il tempo assoluto
    ingrediente_stock_t *ingrediente_temp, *prev, *elim;
    lotto_t *lotto_temp;
    int i;

    for (i = 0; i < DIM_MAGAZZINO; i++) {
        ingrediente_temp = magazzino->stock_ingredienti[i];
        while (ingrediente_temp != NULL) {
            prev = NULL;
            lotto_temp = ingrediente_temp->lotto;
            while (lotto_temp && lotto_temp->scadenza <= t) {
                ingrediente_temp->lotto = lotto_temp->next;
                ingrediente_temp->totale -= lotto_temp->q;
                free(lotto_temp);
                lotto_temp = ingrediente_temp->lotto;
            }
            if (ingrediente_temp->totale == 0) {
                elim = ingrediente_temp;
                ingrediente_temp = ingrediente_temp->next;
                if(prev != NULL) {
                    prev->next = ingrediente_temp;
                } else {
                    magazzino->stock_ingredienti[i] = ingrediente_temp;
                }
                free(elim);
                elim = NULL;                
            } else {
                prev = ingrediente_temp;
                ingrediente_temp = ingrediente_temp->next;
            }
        }
    }
}

ordine_t* consegne (unsigned long long peso, ordine_t* coda) { //controlla tutti i prodotti disponibili e fa il controllo per il peso del furgoncino, quando l'ordine viene spedito va fatta la free dell'entità ordine
    ordine_t *consegna, *temp_consegna, *prec_consegna, *next_prodotto;
    int cons, stop;
    unsigned long long residuo;

    residuo = peso;
    cons = 0;
    consegna = NULL;
    stop = 1;

    if (coda) {
        while (coda != NULL && stop) { //scorro tutti i prodotti fino a quando non arrivo a NULL o finché non ho trovato il primo che non ci sta sul camioncino (scorro in ordine temporale)
            if (coda->peso <= residuo) { //se il peso del mio prodotto è minore= di quanto ancora posso caricare
                cons = 1; //ho fatto almeno una consegna
                residuo -= coda->peso; //aggiorno il peso residuo che posso ancora caricare togliendo quanto caricato
                next_prodotto = coda->next; //salvo il successivo a quello che sto caricando

                if (consegna == NULL) { //aggiunta del prodotto nella nuova coda consegne, se la lista è vuota prende il prodotto e basta
                    consegna = coda; //il prodotto attuale diventa il primo della lista consegne
                    coda = next_prodotto; //la nuova coda parte dal successivo
                    if(coda) {
                        coda->prev = NULL;
                    }
                    consegna->next = NULL;
                    consegna->prev = NULL;
                } else { // se non è vuota (lista ordinata per peso decrescente)
                    prec_consegna = NULL;
                    temp_consegna = consegna;
                    while (temp_consegna != NULL && (temp_consegna->peso >= coda->peso)) { //scorro finché la temp_consegna non punta a qualcosa di peso <= al mio ordine
                        prec_consegna = temp_consegna;
                        temp_consegna = temp_consegna -> next;
                    }
                    if (temp_consegna == NULL) {
                        prec_consegna->next = coda;
                        coda->prev = prec_consegna;
                        coda->next = NULL;
                        coda = next_prodotto;
                        if (coda){
                            coda->prev = NULL;
                        }
                    } else { //se la consegna è più leggera
                        coda->next = temp_consegna;
                        coda->prev = temp_consegna->prev;
                        if(prec_consegna) {
                            prec_consegna->next = coda;
                        } else {
                            consegna = coda;
                        }
                        coda = next_prodotto;
                        if (coda){
                            coda->prev = NULL;
                        }
                    }
                    /*if (prec_consegna == NULL) { //mi trovo al primo elemento della lista, che pesa <= al mio ordine
                        if (temp_consegna->peso < coda->peso) { //se l'elemento è più leggero
                            coda->next = temp_consegna;
                            consegna->next = coda;
                            coda = next_prodotto; //per il prossimo ciclo while
                        } else { //se l'elemento pesa uguale
                            if (temp_consegna->t < coda->t) { //se l'elemento pesa uguale ma è più recente (t più grande)
                                coda->next = temp_consegna;
                                consegna->next = coda;
                                coda = next_prodotto; //per il prossimo ciclo while
                            } else { //se l'elemento pesa uguale ma è più vecchio (t minore)
                                coda->next = temp_consegna->next;
                                temp_consegna->next = coda;
                                coda = next_prodotto;
                            }
                        }
                    } else { // se non sono all'inizio della lista
                        if (temp_consegna->peso < coda->peso) { //se l'elemento è più leggero
                            coda->next = temp_consegna;
                            prec_consegna->next = coda;
                            coda = next_prodotto; //per il prossimo ciclo while
                        } else { //se l'elemento pesa uguale
                            if (temp_consegna->t < coda->t) { //se l'elemento pesa uguale ma è più recente (t più grande)
                                coda->next = temp_consegna;
                                prec_consegna->next = coda;
                                coda = next_prodotto; //per il prossimo ciclo while
                            } else { //se l'elemento pesa uguale ma è più vecchio (t minore)
                                coda->next = temp_consegna->next;
                                temp_consegna->next = coda;
                                coda = next_prodotto;
                            }
                        }  
                    }*/
                }
            } else { //se il prodotto è troppo grande mi fermo
                stop = 0;
            }
        }
        // dopo aver creato la lista delle consegne, effettuo le consegne
        temp_consegna = NULL;
        if (cons) {
            while (consegna) {
                printf ("%d %s %d\n", consegna->t, consegna->ricetta->nome, consegna->q);
                consegna->ricetta->pending--;
                temp_consegna = consegna;
                consegna = consegna->next;
                free(temp_consegna);
            }
        } else {
            printf("camioncino vuoto\n");
        }
        return coda;
    } else {
        printf("camioncino vuoto\n");
        return coda;
    }
}

int main () {
    char input[L_NOME_MAX], nome[L_NOME_MAX], ingrediente[L_NOME_MAX];
    int t, fc, pc, qt, peso, sc;
    unsigned int ret;
    char nl;
    ricettario_hash_t *ricettario;
    ordine_t *coda_ordini, *coda_prodotti;
    magazzino_hash_t *magazzino;
    ingrediente_t *lista_ingredienti;
    ricetta_t *tempricetta;
    scadenze_t *listascadenze, *tempscadenza;

    if(freopen("open4.txt", "r", stdin));
    //if(freopen("open4.output.txt", "w", stdout));

    ricettario = NULL;
    coda_ordini = NULL;
    coda_prodotti = NULL;
    magazzino = NULL;
    listascadenze = NULL;

    ricettario = crea_hash_ricettario();
    magazzino = crea_hash_magazzino();

    if (ricettario == NULL || magazzino == NULL) {
        printf("Errore creazione magazzino e/o ricettario.\n");
        return 0;
    }
    t=0;
    if(scanf("%d %d ", &fc, &pc));

    while(scanf(" %s", input) != -1) {
        nome[0] = '\0';

        if (!strcmp(input, "aggiungi_ricetta")) { 
            tempricetta = NULL;
            if (scanf (" %s", nome));

            tempricetta = ricerca_ricetta(ricettario, nome);

            if (tempricetta == NULL) {
                lista_ingredienti=NULL;
                peso = 0;
                if (scanf("%c", &nl));
                while (nl != '\n') {
                    if (scanf("%s %d", ingrediente, &qt));
                    peso += qt;
                    lista_ingredienti = aggiungi_ingrediente(lista_ingredienti, ingrediente, qt);
                    if (scanf("%c", &nl));
                }
                ret = aggiungi_ricetta(ricettario, nome, lista_ingredienti, peso);
                if (ret == -1) {
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
            while (nl != '\n') {
                if (scanf("%s %d %d", nome, &qt, &sc));
                ret = rifornimento(magazzino, nome, sc, qt);

                listascadenze = aggiungi_scadenza(listascadenze, sc);

                if (ret == -1) {
                    printf("errore rifornimento\n");
                }
                if (scanf("%c", &nl));
            }
            printf("rifornito\n");

            coda_ordini = cucina(magazzino, coda_ordini, &coda_prodotti, 0);

        } else if (!strcmp(input, "ordine")) {
            if (scanf(" %s %d", nome, &qt));
            coda_ordini = aggiungi_ordine(coda_ordini, ricettario, nome, qt, t, &ret);
            if(ret == 1) {
                printf("accettato\n");
                coda_ordini = cucina(magazzino, coda_ordini, &coda_prodotti, 1);
            } else {
                printf("rifiutato\n");
            }
            
        } else {
            printf ("Errore \n");
        }
        t++;

        if (t != 0) { // eseguito ogni volta
            if (listascadenze) {
                if (listascadenze->s == t) {
                    if (listascadenze->next == listascadenze) {
                        free(listascadenze);
                        listascadenze = NULL;
                    } else {
                        listascadenze->next->prev = listascadenze->prev;
                        tempscadenza = listascadenze;
                        listascadenze->prev->next = listascadenze->next;
                        listascadenze = listascadenze->next;
                        free(tempscadenza);
                        tempscadenza=NULL;
                    }
                    scadenze(magazzino, t);
                }
            }
        };

        if (t!=0 && t % fc == 0) { // eseguito a ogni frequenza di passaggio del corriere
            coda_prodotti = consegne(pc, coda_prodotti);
        }
    }

    elimina_coda(coda_ordini);
    elimina_coda(coda_prodotti);
    elimina_hash_ricettario(ricettario);
    elimina_hash_magazzino(magazzino);

    if (listascadenze) {
        listascadenze->prev->next = NULL;
         
        while (listascadenze != NULL) {
            tempscadenza = listascadenze;
            listascadenze = listascadenze->next;
            free(tempscadenza);
            tempscadenza = NULL;
        }
    }
    return 0;    
}