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

typedef struct { //tipologia dato ricetta: contiene nome, se ci sono ordini in attesa e puntatore a tabella di ingredienti
    char nome[L_NOME_MAX];
    ingrediente_hash_t* tabella_ingredienti;
    int pending;
    int status;
} ricetta_t;

typedef struct { //tipologia dato tabella di ricette: contiene puntatore ad array di ricette
    ricetta_t* ricette[N_RICETTE_MAX]
} ricettario_hash_t;


typedef struct ordine {
    int t;
    char ricetta[L_NOME_MAX];
    int q;
    ordine* succ;
} ordine_t;

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
            ricettario->ricette[i] = NULL;
            ricettario->ricette[i].
        }
        return tabella;
    } else {
        return NULL;
    }
}

void elimina_hash_ricettario (ricettario_hash_t* tabella) {
    int i;
    for (i=0; i<N_INGREDIENTI_MAX; i++) {
        elimina_hash_ingredienti(tabella->ricette[i].tabella_ingredienti);
        free(tabella->ricette[i]);
    }
    free(tabella);
}

int ricerca_ingrediente(ingredienti_hash_t* tabella, char* nome) { // accetta una tabella hash di ingredienti e restituisce indice dell'ingrediente col nome passato, -1 se non l'ha trovato
    int i, k;
    k = 0;
    i = 0;

    while (k <= N_INGREDIENTI_MAX) {
        i = funzione_hash(nome, k);
        if (tabella->ingredienti[i] && !strcmp(tabella->ingredienti[i].nome, nome)) {
            return i;
        } else {
            if (tabella->ingrediente[i] == NULL){
                return -1;
            } else {
            k++;
            }
        }
    }
    return -1;
}

int aggiungi_ingrediente(ingredienti_hash_t* tabella,  char* nome, int q) { // restituisce 1 se è andata a buon fine e -1 se la tabella è piena e -2 se l'allocazione non è andata a buon fine
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

int aggiungi_ricetta(ricettario_hash_t* ricettario,  char* nome_ricetta, ingredienti_hash_t* tabella_ingredienti) { // restituisce 1 se è andata a buon fine e -1 se il ricettario è pieno e -2 se l'allocazione non è andata a buon fine
    int i, k;
    ricetta_t* temp_ricetta;
    while (k <= N_RICETTE_MAX) {
        i = funzione_hash(nome_ricetta, k);
        if (ricettario->ricette[i] != NULL || strcmp(ricettario->ricette[i] == del)) {
            temp_ricetta = malloc(sizeof(ricetta_t));
            if(temp_ricetta) {
                strcpy (temp_ricetta.nome, nome_ricetta);
                temp_ricetta->tabella_ingredienti = tabella_ingredienti;
                ricettario->ricette[i] = temp_ricetta;
                ricettario->ricette[i].pending = 0;
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

int ricerca_ricetta(ricettario_hash_t* ricettario,  char* nome_ricetta) {
    int i, k;
    k = 0;
    i = 0;

    while (k <= N_RICETTE_MAX) {
        i = funzione_hash(nome, k);
        if (!strcmp(ricettario.ricette[i].nome, nome)) {
            return i;
        } else {
            if (tabella->ingrediente[i] == NULL){
                return -1;
            } else {
            k++;
            }
        }
    }
    return -1;
}

int elimina_ricetta(ricettario_hash_t* ricettario, char* nome_ricetta) { //restituisce 1 se viene eliminata, -1 se non esiste, -2 se ci sono ordini in attesa
    int i;
    i = ricerca_ricetta(ricettario, nome_ricetta);

    if (i >= 0) {
        if (ricettario->ricette[i].pending > 0) {
            return -2;
        } else {
            elimina_hash_ingredienti(ricettario->ricette[i].tabella_ingredienti);
            free(ricettario->ricette[i]);
            ricettario->ricette[i] = del;
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
ordine_t* aggiungi_ordine(ordine_t* coda, char nome) {
    ordine
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