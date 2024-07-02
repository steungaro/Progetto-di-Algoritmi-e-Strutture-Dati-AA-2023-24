#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define L_NOME_MAX 100
#define N_INGREDIENTI_MAX 199
#define N_RICETTE_MAX 307
#define OCC 1
#define EMP 0
#define DEL -1

char del[13];

typedef struct {  //tipologia di dato ingrediente: contiene nome e quantità
    char nome[L_NOME_MAX];
    int q;
} ingrediente_t;

typedef struct { //tipologia dato tabella ingredienti: contiene un puntatore ad array di ingredienti
    ingrediente_t* ingredienti[N_INGREDIENTI_MAX];
} ingredienti_hash_t;

typedef struct {
    char nome[L_NOME_MAX];
    ingrediente_hash_t* tabella_ingredienti;
    int pending;
} ricetta_t;

typedef struct { //tipologia dato ricetta: contiene nome, se ci sono ordini in attesa e puntatore a tabella di ingredienti
    ricetta_t* ricetta;
    int status;
} cella_ricettario_t;

typedef struct { //tipologia dato tabella di ricette: contiene puntatore ad array di ricette
    cella_ricettario_t* elenco_ricette[N_RICETTE_MAX]
} ricettario_hash_t;


typedef struct ordine {
    int t;
    char ricetta[L_NOME_MAX];
    int numero_ricetta;
    int q;
    int peso;
    ordine* succ;
} ordine_t;

/*
typedef struct prodotto {
    int t;
    int q;
    char ricetta[L_NOME_MAX];
    int peso;
    prodotto* succ;
} prodotto_t;
*/

int funzione_hash(char* nome, int k, int dim) { //generica accetta nome, numero di indirizzamento aperto e dimensione tabella e restituisce indice
    int h, i, c;
    h=5381;
    i=0;
    while(nome[i]!='\0') {
        h = h * 33 + nome[i];
        i++;
    }
    return (h + k) % dim;
}

ingredienti_hash_t* crea_hash_ingredienti() { // crea una tabella hash dinamica per gli ingredienti, con ciascun ingrediente puntante a NULL
    ingredienti_hash_t* tabella;
    tabella = malloc(sizeof(ingredienti_hash_t));
    int i;
    if (tabella) {
        for (i = 0; i < N_INGREDIENTI_MAX; i++) {
            tabella->ingredienti[i] = NULL;
        }
        return tabella;
    } else {
        return NULL;
    }
}

void elimina_hash_ingredienti (ingredienti_hash_t* tabella) {
    int i;
    for (i=0; i<N_INGREDIENTI_MAX; i++) {
        free(tabella->ingredienti[i]);
    }
    free(tabella);
}

ricettario_hash_t* crea_hash_ricettario() { // crea una tabella hash dinamica per il ricettario, con ciascuna ricetta puntante a NULL
    ricettario_hash_t* ricettario;
    ricettario = malloc(sizeof(ricettario_hash_t));
    if (ricettario){
        int i;
        for (i = 0; i < N_RICETTE_MAX; i++) {
            ricettario->elenco_ricette[i].ricetta = NULL;
            ricettario->elenco_ricette[i].status = EMP;
        }
        return ricettario;
    } else {
        return NULL;
    }
}

void elimina_hash_ricettario (ricettario_hash_t* ricettario) {
    int i;
    for (i=0; i<N_INGREDIENTI_MAX; i++) {
        elimina_hash_ingredienti(ricettario->elenco_ricette[i].ricetta.tabella_ingredienti);
        free(ricettario->elenco_ricette[i]);
    }
    free(ricettario);
}

int ricerca_ingrediente(ingredienti_hash_t* tabella_ingredienti, char* nome) { // accetta una tabella hash di ingredienti e restituisce indice dell'ingrediente col nome passato, -1 se non l'ha trovato
    int i, k;
    k = 0;
    i = 0;

    while (k <= N_INGREDIENTI_MAX) {
        i = funzione_hash(nome, k);
        if (tabella_ingredienti->ingredienti[i] != NULL && !strcmp(tabella_ingredienti->ingredienti[i].nome, nome)) {
            return i;
        } else {
            if (tabella_ingredienti->ingrediente[i] == NULL){
                return -1;
            } else {
            k++;
            }
        }
    }
    return -1;
}

int aggiungi_ingrediente(ingredienti_hash_t* tabella,  char* nome, int q) { // riceve una tabella di ingredienti, il nome di un ingrediente e la quantità e restituisce 1 se l'aggiunta è andata a buon fine e -1 se la tabella è piena e -2 se l'allocazione non è andata a buon fine
    int i, k;
    ingrediente_t* temp_ing;
    while (k <= N_INGREDIENTI_MAX) {
        i = funzione_hash(nome, k);
        if (tabella->ingredienti[i] == NULL) {
            temp_ing = malloc(sizeof(ingrediente_t));
            if(temp_ing) {
                strcpy(temp_ing.nome, nome);
                tabella->ingredienti[i] = temp_ing;
                return 1;
            } else {
                return -2;
            }
        } else {
        k++;
        }
    }
    return -1;
}

int aggiungi_ricetta(ricettario_hash_t* ricettario,  char* nome_ricetta, ingredienti_hash_t* tabella_ingredienti) { // riceve in ingresso un ricettario, il nome di una ricetta e la tabella degli ingredienti per tale ricetta e restituisce l'indice della ricetta aggiunta al ricettario se è andata a buon fine, -1 se il ricettario è pieno e -2 se l'allocazione non è andata a buon fine
    int i, k;
    ricetta_t* temp_ricetta;
    while (k <= N_RICETTE_MAX) {
        i = funzione_hash(nome_ricetta, k);
        if (ricettario->elenco_ricette[i].status != OCC) {
            temp_ricetta = malloc(sizeof(ricetta_t));
            if(temp_ricetta) {
                strcpy (temp_ricetta.nome, nome_ricetta);
                temp_ricetta->tabella_ingredienti = tabella_ingredienti;
                ricettario->elenco_ricette[i].ricetta = temp_ricetta;
                ricettario->elenco_ricette[i].ricetta.pending = 0;
                ricettario->elenco_ricette[i].status = OCC;
                return i;
            } else {
                return -2;
            }
        } else {
        k++;
        }
    }
    return -1;
}

int ricerca_ricetta(ricettario_hash_t* ricettario,  char* nome_ricetta) { // riceve un ricettario e il nome di una ricetta e restituisce l'indice nella hash table della ricetta il cui nome è passato e -1 se la ricetta non esiste
    int i, k;
    k = 0;
    i = 0;

    while (k <= N_RICETTE_MAX) {
        i = funzione_hash(nome_ricetta, k);
        if (!strcmp(ricettario.elenco_ricette[i].nome, nome_ricetta)) {
            return i;
        } else {
            if (ricettario->elenco_ricette[i].status == EMP){
                return -1;
            } else{
            k++;
            }
        }
    }
    return -1;
}

int elimina_ricetta(ricettario_hash_t* ricettario, char* nome_ricetta) { // riceve un ricettario e la ricetta da rimuovere e restituisce 1 se viene eliminata, -1 se non esiste, -2 se ci sono ordini in attesa e quindi non la elimina
    int i;
    i = ricerca_ricetta(ricettario, nome_ricetta);

    if (i >= 0) {
        if (ricettario->elenco_ricette[i].ricetta.pending > 0) {
            return -2;
        } else {
            elimina_hash_ingredienti(ricettario->elenco_ricette[i].ricetta.tabella_ingredienti);
            free(ricettario->elenco_ricette[i].ricetta);
            ricettario->elenco_ricette[i].status = DEL;
            return 1;
        }
    } else {
        return -1;
    }
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
int aggiungi_ordine_in_coda(ordine_t* coda, char nome, int q, int t, int n) { // USARE AGGIUNGI ORDINE accetta la coda degli ordini (NULL se vuota), il nome dell'ordine da eseguire, la quantità, il tempo assoluto, l'indice della ricetta e restituisce 1 se l'operazione è andata a buon fine e -1 altrimenti
    ordine_t* ordine, temp;
    temp = coda;
    ordine = malloc(sizeof(ordine_t);)

    if (ordine) {
        ordine->q = q;
        ordine->t = t;
        ordine->numero_ricetta = n;
        strcpy(ordine.ricetta, nome);
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

int aggiungi_ordine(ordine_t* coda, ricettario_hash_t* ricettario, char nome, int q, int t) { //accetta una coda degli ordini, (NULL se vuota), il ricettario, il nome della ricetta, la quantità e il tempo assoluto e restituisce 1 se l'ordine è andato a buon fine, -1 altrimenti. inoltre aggiunge pending alla ricetta in questione
    int i;
    i = ricerca_ricetta(ricettario, nome);
    if (i>=0) {
        ricettario->elenco_ricette[i].ricetta.pending++;
        i = aggiungi_ordine_in_coda(coda, nome, q, t, i);
        return i;
    } else {
        return i;
    }
}

void aggiungi_ordine_in_lista(prodotto_t* lista, ricettario_hash_t* ricettario, ordine_t* ordine) { //accetta la lista di prodotti pronti da spedire, il ricettario, l'ordine (mantiene l'entità ordine, non fare free ma toglierne riferimento dalla coda ordini) e aggiunge l'ordine passato alla coda degli ordini completati calcolandone il peso
    ordine_t* temp;
    int i, j;
    temp = lista;
    ordine->succ = NULL;
    ordine->peso = 0;
    for(j=0; j<N_INGREDIENTI_MAX; j++) {
        if (ricettario->elenco_ricette[i].tabella_ingredienti[j] != NULL) {
            ordine->peso = prodotto->peso + ricettario->elenco_ricette[i].tabella_ingredienti[j].q;
        }
        ordine->peso = ordine->peso * ordine->q;
    }
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

void scadenze() { //da eseguire prima di qualsiasi cosa, controlla i lotti del magazzino ed elimina quelli la cui scadenza eccede il tempo assoluto

}

void rifornimento() { // accetta un magazzino, un prodotto, una quantità e una scadenza e aggiunge al magazzino i lotti con le scadenze

}
int main () {
    /*
    if(creazione_del()){


        rimozione_del();
    } else {
        printf("Errore\n");
    }
    */
    return 0;    
}