#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define L_NOME_MAX 256
#define DIM_RICETTE 107
#define DIM_MAGAZZINO 107

char del[13];

typedef struct ingrediente {  //tipologia di dato ingrediente: contiene nome e quantità e puntatore al successivo
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
    ordine* succ;
} ordine_t;

typedef struct lotto {
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

int funzione_hash(char* nome) { // accetta nome e restituisce indice. utilizza la define della grandezza di tabella
    int h, i, c;
    h=5381;
    i=0;
    while(nome[i]!='\0') {
        h = h * 33 + nome[i];
        i++;
    }
    return h % DIM_RICETTE;
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
            ricettario.elenco_ricette[i] = NULL; // indirizzo nell'elenco == NULL
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

int aggiungi_lotto(magazzino_hash_t* magazzino, char nome[], int scadenza, int quantita) { // riceve in ingresso un magazzino, il nome di un lotto, la scadenza e la quantità del lotto e cerca se c'è già un ingrediente per tale lotto (in caso aggiunge il lotto nella lista dei lotti e incrementa il totale disponibile), se non c'è lo crea. restituisce l'indice dell'ingrediente aggiunto, -1 se la malloc non ha funzionato.
    lotto_t* lottonuovo, temp;
    ingrediente_stock_t* ingrediente_temp;
    int h;

    lottonuovo = malloc(sizeof(lotto_t));

    if (lottonuovo) {
        lottonuovo->scadenza = scadenza;
        lottonuovo->q = quantita;
        lottonuovo.next = NULL;

        h = funzione_hash(nome);

        if (magazzino->stock_ingredienti[h] !=NULL) {

            magazzino->stock_ingredienti[h].next = NULL;
            strcpy(magazzino->stock_ingredienti[h].nome, nome);
            magazzino->stock_ingredienti[h].totale = quantita;
            return h;

        } else {
            
            for (ingrediente_temp = magazzino->stock_ingredienti[h]; !strcmp(ingrediente_temp.nome, nome) && ingrediente_temp.next != NULL; ingrediente_temp = ingrediente_temp->next);
            if (ingrediente_temp != NULL) {
                ingrediente_temp.totale += quantita;
                for (temp = ingrediente_temp.lotto; temp.next != NULL; temp = temp->next);
                temp.next = lottonuovo;
                return h;
            } else {
                ingrediente_temp.next = malloc(sizeof(ingrediente_stock_t));
                if (ingrediente_temp.next) {
                    ingrediente_temp->next->totale = quantita;
                    ingrediente_temp->next->lotto = lottonuovo;
                    strcpy (ingrediente_temp->next.nome, nome);
                } else {
                    return -1;
                }
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
            elimina_lista_ingredienti(ricettario->elenco_ricette[i].lista_ingredienti);
            free(ricettario->elenco_ricette[i]); //passo indirizzo della ricetta
        }
    }
    free(ricettario);
}

void elimina_hash_magazzino (magazzino_hash_t* magazzino) { // accetta un magazzino e lo elimina, eliminando anche la lista degli ingredienti e dei lotti al suo interno
    int i;
    lotto_t* temp;

    for (i=0; i<DIM_MAGAZZINO; i++) {
        if(magazzino->stock_ingredienti[i] != NULL){
            while (magazzino->stock_ingredienti[i]->lotto != NULL) {
                temp = magazzino->stock_ingredienti[i]->lotto;
                magazzino->stock_ingredienti[i]->lotto = magazzino->stock_ingredienti[i]->lotto->next;
                free (temp);
            }            
        }
    }
    free(magazzino);
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

int aggiungi_ricetta(ricettario_hash_t* ricettario,  char* nome_ricetta, ingrediente_t* lista_ingredienti, int peso) { // riceve in ingresso un ricettario, il nome di una ricetta, la lista degli ingredienti di tale ricetta e il suo peso e restituisce l'indice della ricetta aggiunta al ricettario se è andata a buon fine, -1 se l'allocazione non è andata a buon fine
    int i;
    ricetta_t* temp_ricetta, scorr;
    i = funzione_hash(nome_ricetta);
    temp_ricetta = malloc(sizeof(ricetta_t));
        if(temp_ricetta) {
            strcpy (temp_ricetta->nome, nome_ricetta);
            temp_ricetta->lista_ingredienti = lista_ingredienti;
            temp_ricetta->pending = 0;
            temp_ricetta->peso = peso;

            if (ricettario.elenco_ricette[i] == NULL) { // indirizzo della ricetta
                ricettario->elenco_ricette[i] = temp_ricetta;
                return i;
            } else {
                scorr = ricettario.elenco_ricette[i];
                while (scorr -> next != NULL) {
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
    i = funzione_hash(nome_ricetta);
    temp = ricettario.elenco_ricette[i];

    for (; temp != NULL; temp = temp -> next) {
        if (!strcmp(temp->nome, nome_ricetta)) {
            return temp;
        }
    }
    return NULL;
}

int elimina_ricetta(ricettario_hash_t* ricettario, char* nome_ricetta) { // riceve un ricettario e la ricetta da rimuovere e restituisce 1 se viene eliminata, -1 se non esiste, -2 se ci sono ordini in attesa e quindi non la elimina
    int i;
    ricetta_t* curr, prec, temp;
    prec = NULL;

    i = funzione_hash(nome_ricetta);
    curr = ricettario.elenco_ricette[i];

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
                    ricettario.elenco_ricette[i] = curr;
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

/*
int creazione_del() {
    del = malloc(sizeof(13*char));
    if (del) {
        strcpy(del, DEL);
        return 1;
    }
    return -1;
}

void rimozione_del() {
    free(del);
}
*/
int aggiungi_ordine_in_coda(ordine_t* coda, ricetta_t* ricetta, int q, int t) { // USARE AGGIUNGI ORDINE accetta la coda degli ordini (NULL se vuota), la ricetta dell'ordine da eseguire, la quantità, il tempo assoluto e restituisce 1 se l'operazione è andata a buon fine e -1 altrimenti
    ordine_t* ordine, temp;
    temp = coda;
    ordine = malloc(sizeof(ordine_t);)

    if (ordine) {
        ordine->q = q;
        ordine->t = t;
        ordine->ricetta = ricetta;
        ordine->peso = ricetta.peso;
        ordine->succ = NULL;
        if (temp) {
            for(; temp->succ != NULL; temp=temp->succ);
            temp->succ = ordine;
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
        ricettario->elenco_ricette[i].ricetta.pending++;
        return aggiungi_ordine_in_coda(coda, ricetta, q, t, i);
    } else {
        return -1;
    }
}

void aggiungi_ordine_in_lista(ordine_t* lista, ricettario_hash_t* ricettario, ordine_t* ordine) { //accetta la lista di prodotti pronti da spedire, il ricettario, l'ordine (mantiene l'entità ordine, non fare free ma toglierne riferimento dalla coda ordini) e aggiunge l'ordine passato alla coda degli ordini completati
    ordine_t* temp;
    int i, j;
    temp = lista;
    ordine->succ = NULL;
    if (temp) {
        for(; temp->succ != NULL && temp->t < ordine->t; temp = temp->succ);
        if (temp->succ == NULL) {
            temp->succ = ordine;
        } else {
            ordine->succ = temp->succ->succ;
            temp->succ = ordine;
        }
    } else {
        coda = ordine;
    }
}

void cucina() { //dopo l'aggiunta di un ordine o dopo il rifornimento va avviata: controlla gli elementi della coda ordini e prepara quelli i cui ingredienti sono disponibili nel magazzino

}

void scadenze(int t, magazzino_hash_t* magazzino) { //da eseguire prima di qualsiasi cosa, controlla i lotti del magazzino ed elimina quelli la cui scadenza eccede il tempo assoluto

}

void rifornimento() { // accetta un magazzino, un prodotto, una quantità e una scadenza e aggiunge al magazzino i lotti con le scadenze

}

void consegne (int peso, ordine_t* coda) {

}

int main () {
    char input[L_NOME_MAX], nome[L_NOME_MAX];
    int t, fc, pc;
    ricettario_hash_t * ricettario;
    ordine_t* coda_ordini, coda_prodotti;
    magazzino_hash_t * magazzino;

    ricettario = crea_hash_ricettario();
    magazzino = crea_hash_magazzino();

    if (ricettario == NULL || magazzino == NULL) {
        printf("Errore creazione magazzino e/o ricettario.\n");
        return 0;
    }
    t=0;
    if(scanf("%d %d ", &fc, &pc) )

    while(scanf("%s ", input) != -1) {

        if (t != 0) {
            scadenze(t, magazzino);
        };

        if (t!=0 && t % fc == 0) {
            consegne(t, coda_prodotti);
        }
        if (!strcmp(input, "aggiungi_ricetta")) {
            
        } else if (!strcmp(input, "rimuovi_ricetta")) {

        } else if (!strcmp(input, "rifornimento")) {

        } else if (!strcmp(input, "ordine")) {
            
        } else {
            printf ("Errore \n");
        }
        t++;
    }

    elimina_hash_ricettario(ricettario);
    elimina_hash_magazzino(magazzino);
    return 0;    
}