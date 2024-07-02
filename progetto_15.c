#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define L_NOME_MAX 256
#define DIM_RICETTE 3017
#define DIM_MAGAZZINO 3017

typedef struct scadenze {
    unsigned int s;
    struct scadenze *next;
} scadenze_t;

typedef struct ingrediente {
    char nome[L_NOME_MAX];
    int h;
    unsigned int q;
    struct ingrediente *next;
} ingrediente_t;

typedef struct ricetta {
    char nome[L_NOME_MAX];
    ingrediente_t *lista_ingredienti;
    unsigned int pending;
    unsigned int peso;
    struct ricetta *next;
} ricetta_t;

typedef struct {
    ricetta_t* elenco_ricette[DIM_RICETTE];
} ricettario_hash_t;

typedef struct ordine {
    unsigned int t;
    ricetta_t *ricetta;
    unsigned int q;
    unsigned int peso;
    struct ordine *left;
    struct ordine *right;
    struct ordine *p;
} ordine_t;

typedef struct lotto {
    unsigned int scadenza;
    unsigned int q;
    struct lotto *left;
    struct lotto *right;
    struct lotto *p;
} lotto_t;

typedef struct ingrediente_stock {
    struct ingrediente_stock* next;
    char nome[L_NOME_MAX];
    unsigned int totale;
    lotto_t* lotto;
} ingrediente_stock_t;

typedef struct {
    ingrediente_stock_t* stock_ingredienti[DIM_MAGAZZINO];
} magazzino_hash_t;

int funzione_hash(char* nome, int dim) {
    int i;
    unsigned long long h;

    h=5381;
    i=0;

    while(nome[i] != '\0') {
        h = ((h << 5) + h) + nome[i];
        i++;
    }
    return (int) (h % (unsigned long long) dim);
}

void elimina_lista_ingredienti(ingrediente_t* lista) {
    ingrediente_t* temp;

    while (lista != NULL) {
        temp = lista;
        lista = lista -> next;
        free(temp);
        temp = NULL;
    }
}

ordine_t* minimo_in_albero_ordini(ordine_t* albero) {
    ordine_t *temp;

    temp = albero;

    if (albero)  {
        while (temp->left != NULL) {
            temp = temp->left;
        }
    }
    return temp;
}

ordine_t* massimo_in_albero_ordini(ordine_t* albero) {
    ordine_t *temp;

    temp = albero;

    if (albero)  {
        while (temp->right != NULL) {
            temp = temp->right;
        }
    }
    return temp;
}

ordine_t *successore_albero_ordini (ordine_t* nodo) {
    ordine_t *y, *x;

    x = nodo;
    if (nodo->right != NULL) {
        return minimo_in_albero_ordini(nodo->right);
    } else {
        y = nodo->p;
        while (y != NULL && x == y->right) {
            x = y;
            y = y->p;
        }
        return y;
    }
}

ordine_t *inserimento_albero_ordini (ordine_t *albero, ordine_t *z) {
    ordine_t *x, *y;

    y = NULL;
    x = albero;
    z->left = NULL;
    z->right = NULL;

    while (x != NULL) {
        y = x;
        if (z->t < x->t) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    z->p = y;
    if (y == NULL) {
        albero = z;
    } else if (z->t < y->t) {
        y->left = z;
    } else {
        y->right = z;
    }
    return albero;
}

ordine_t *cancella_nodo_albero_ordini(ordine_t *albero, ordine_t *z) {
    ordine_t *x, *y;

    if (z != NULL && (z->left == NULL || z->right == NULL)) {
        y = z;
    } else {
        y = successore_albero_ordini(z);
    }
    if (y != NULL && y->left != NULL) {
        x = y->left;
    } else {
        x = y->right;
    }
    if (x != NULL) {
        x->p = y->p;
    }
    if (y != NULL && y->p == NULL) {
        albero = x;
    } else if (y != NULL && y == y->p->left) {
        y->p->left = x;
    } else if (y != NULL) {
        y->p->right = x;
    } 
    if (y != z && y != NULL) {
        z->t = y->t;
        z->q = y->q;
        z->ricetta = y->ricetta;
        z->peso = y->peso;
    }
    return albero;
}

lotto_t *minimo_in_albero_lotti (lotto_t* albero) {
    lotto_t *temp;

    temp = albero;

    if (albero)  {
        while (temp->left != NULL) {
            temp = temp->left;
        }
    }
    return temp;
}

lotto_t *massimo_in_albero_lotti (lotto_t* albero) {
    lotto_t *temp;

    temp = albero;

    if (albero)  {
        while (temp->right != NULL) {
            temp = temp->right;
        }
    }
    return temp;
}

lotto_t *successore_albero_lotti (lotto_t* nodo) {
    lotto_t *y, *x;

    x = nodo;
    if (nodo->right != NULL) {
        return minimo_in_albero_lotti(nodo->right);
    } else {
        y = nodo->p;
        while (y != NULL && x == y->right) {
            x = y;
            y = y->p;
        }
        return y;
    }
}

lotto_t *inserimento_albero_lotti (lotto_t *albero, lotto_t *z) {
    lotto_t *x, *y;

    y = NULL;
    x = albero;
    z->left = NULL;
    z->right = NULL;

    while (x != NULL) {
        y = x;
        if (z->scadenza < x->scadenza) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    z->p = y;
    if (y == NULL) {
        albero = z;
    } else if (z->scadenza < y->scadenza) {
        y->left = z;
    } else {
        y->right = z;
    }
    return albero;
}

lotto_t *cancella_nodo_albero_lotti (lotto_t *albero, lotto_t *z) {
    lotto_t *x, *y;

    if (z != NULL && (z->left == NULL || z->right == NULL)) {
        y = z;
    } else {
        y = successore_albero_lotti(z);
    }
    if (y != NULL && y->left != NULL) {
        x = y->left;
    } else {
        x = y->right;
    }
    if (x != NULL) {
        x->p = y->p;
    }
    if (y != NULL && y->p == NULL) {
        albero = x;
    } else if (y != NULL && y == y->p->left) {
        y->p->left = x;
    } else if (y != NULL) {
        y->p->right = x;
    } 
    if (y != z && y != NULL) {
        z->scadenza = y->scadenza;
        z->q = y->q;
    }
    return albero;
}


int confronto(ordine_t *consegna1, ordine_t *consegna2) {
    if (consegna1->peso < consegna2->peso) {
        return -1;
    } else if (consegna1->peso > consegna2->peso) {
        return 1;
    }

    if (consegna1->t < consegna2->t) {
        return 1;
    } else if (consegna1->t > consegna2->t) {
        return -1;
    }

    return 0;
}

ordine_t *inserimento_albero_consegne (ordine_t *albero, ordine_t *z) {
    ordine_t *x, *y;

    y = NULL;
    x = albero;
    z->left = NULL;
    z->right = NULL;

    while (x != NULL) {
        y = x;
        if (confronto(z, x) == -1) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    z->p = y;
    if (y == NULL) {
        albero = z;
    } else if (confronto(z, y) == -1) {
        y->left = z;
    } else {
        y->right = z;
    }
    return albero;
}

void elimina_albero_ordini(ordine_t *albero) {
    if (albero != NULL) {

    elimina_albero_ordini(albero->left);
    elimina_albero_ordini(albero->right);

    free(albero);
    }
}

void elimina_albero_lotti(lotto_t *albero) {
    if (albero != NULL) {

    elimina_albero_lotti(albero->left);
    elimina_albero_lotti(albero->right);

    free(albero);
    }
}

ricettario_hash_t* crea_hash_ricettario() {
    ricettario_hash_t* ricettario;
    ricettario = malloc(sizeof(ricettario_hash_t));
    if (ricettario){
        int i;
        for (i = 0; i < DIM_RICETTE; i++) {
            ricettario->elenco_ricette[i] = NULL;
        }
        return ricettario;
    } else {
        return NULL;
    }
}

magazzino_hash_t* crea_hash_magazzino() {
    magazzino_hash_t* magazzino;
    magazzino = malloc(sizeof(magazzino_hash_t));
    if (magazzino){
        int i;
        for (i = 0; i < DIM_MAGAZZINO; i++) {
            magazzino->stock_ingredienti[i] = NULL;
        }
        return magazzino;
    } else {
        return NULL;
    }
}

void elimina_hash_ricettario (ricettario_hash_t* ricettario) {
    int i;
    ricetta_t *temp;

    temp = NULL;
    for (i=0; i<DIM_RICETTE; i++) {
        while (ricettario->elenco_ricette[i] != NULL){
            elimina_lista_ingredienti(ricettario->elenco_ricette[i]->lista_ingredienti);
            temp = ricettario->elenco_ricette[i];
            ricettario->elenco_ricette[i] = ricettario->elenco_ricette[i]->next;
            free(temp);
            temp = NULL;
        }
    }
    free(ricettario);
    ricettario = NULL;
}

void elimina_hash_magazzino (magazzino_hash_t* magazzino) {
    int i;
    ingrediente_stock_t *elim;

    elim = NULL;
    for (i=0; i < DIM_MAGAZZINO; i++) {
        while(magazzino->stock_ingredienti[i] != NULL) {
            elimina_albero_lotti(magazzino->stock_ingredienti[i]->lotto);
            elim = magazzino->stock_ingredienti[i];
            magazzino->stock_ingredienti[i] = magazzino->stock_ingredienti[i]->next;
            free(elim);
            elim = NULL;        
        }
    }
    free(magazzino);
    magazzino = NULL;
}

ingrediente_t* aggiungi_ingrediente(ingrediente_t* lista,  char* nome, unsigned int q) {
    ingrediente_t* temp;
    temp = malloc(sizeof(ingrediente_t));
    if(temp) {
        strcpy(temp->nome, nome);
        temp->next = lista;
        temp->q = q;
        temp->h = funzione_hash(nome, DIM_MAGAZZINO);
        return temp;
    } else {
        printf("errore aggiunta ingrediente\n");
        return lista;
    }
}

int aggiungi_ricetta(ricettario_hash_t* ricettario,  char* nome_ricetta, ingrediente_t* lista_ingredienti, unsigned int peso) { // riceve in ingresso un ricettario, il nome di una ricetta, la lista degli ingredienti di tale ricetta e il suo peso e restituisce l'indice della ricetta aggiunta al ricettario se è andata a buon fine, -1 se l'allocazione non è andata a buon fine
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
            printf("errore aggiunta ricetta\n");
            return -1;
        }
    
}

int rifornimento(magazzino_hash_t* magazzino, char nome[], unsigned int scadenza, unsigned int quantita) { // riceve in ingresso un magazzino, il nome di un lotto, la scadenza e la quantità del lotto e cerca se c'è già un ingrediente per tale lotto (in caso aggiunge il lotto nella lista dei lotti e incrementa il totale disponibile), se non c'è lo crea. restituisce l'indice dell'ingrediente aggiunto, -1 se la malloc non ha funzionato.
    lotto_t *lottonuovo;
    ingrediente_stock_t *ingrediente_temp, *ingrediente_prev;
    int h;

    ingrediente_prev = NULL;

    lottonuovo = malloc(sizeof(lotto_t));

    if (lottonuovo) { // se la malloc è andata a buon fine
        lottonuovo->scadenza = scadenza;
        lottonuovo->q = quantita;
        lottonuovo->left = NULL;
        lottonuovo->right = NULL;
        lottonuovo->p = NULL;

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
                    ingrediente_temp->lotto = inserimento_albero_lotti(ingrediente_temp->lotto, lottonuovo);
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

ordine_t* aggiungi_ordine(ordine_t* albero, ricettario_hash_t* ricettario, char* nome, unsigned int q, unsigned int t, unsigned int *ret) {
    ricetta_t* ricetta;
    ordine_t *ordine;

    ricetta = ricerca_ricetta(ricettario, nome);
    if (ricetta) {
        ricetta->pending ++;
        ordine = malloc(sizeof(ordine_t));
        if (ordine) {
            ordine->left = NULL;
            ordine->right = NULL;
            ordine->p = NULL;
            ordine->t = t;
            ordine->q = q;
            ordine->peso = ordine->q * ricetta->peso;
            ordine->ricetta = ricetta;
            *ret = 1;
            return inserimento_albero_ordini(albero, ordine);
        } else {
            *ret = -1;
            return albero;
        }
    } else {
        *ret = -1;
        return albero;
    }
}

int abbastanza_ingrediente(magazzino_hash_t* magazzino, char nome[], int h, unsigned int qt) { // accetta il magazzino, il nome ingrediente, l'indice dell'ingrediente e la quantità e restituisce 1 se ce n'è abbastanza, 0 altrimenti
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

void togli_ingrediente(magazzino_hash_t* magazzino, char nome[], int h, unsigned int qt) { // accetta il magazzino, il nome ingrediente, l'indice dell'ingrediente e lo rimuove
    unsigned int resto, stop;
    ingrediente_stock_t* ingrediente_temp, *elim, *prev;
    lotto_t* lotto_temp;

    prev = NULL;
    stop = 0;

    for (ingrediente_temp = magazzino->stock_ingredienti[h]; ingrediente_temp != NULL && !stop; ingrediente_temp = ingrediente_temp->next) {
        if (!strcmp(ingrediente_temp->nome, nome)) { // se i nomi combaciano
            stop = 1;
            if (ingrediente_temp->totale >= qt) {
                ingrediente_temp->totale -= qt;
                if (ingrediente_temp->totale == 0) {
                    elimina_albero_lotti(ingrediente_temp->lotto);
                    ingrediente_temp->lotto = NULL;
                    elim = ingrediente_temp;
                    ingrediente_temp = ingrediente_temp->next;
                    if(prev != NULL) {
                        prev->next = ingrediente_temp;
                    } else {
                        magazzino->stock_ingredienti[h] = ingrediente_temp;
                    }
                    free(elim);
                } else {
                    resto = qt;        
                    while (resto > 0) {
                        lotto_temp = minimo_in_albero_lotti(ingrediente_temp->lotto);
                        if (lotto_temp->q > resto) {
                            lotto_temp->q -= resto;
                            resto = 0;
                        } else {
                            resto -= lotto_temp->q;
                            ingrediente_temp->lotto = cancella_nodo_albero_lotti(ingrediente_temp->lotto, lotto_temp);
                            free(lotto_temp); 
                        }
                    }
                }
            }
        } else {
            prev = ingrediente_temp;
        }
    }
    
}

ordine_t* cucina(magazzino_hash_t* magazzino, ordine_t* albero_ordini, ordine_t** albero_prodotti, int tipo) { //se di tipo 1 controlla solo l'ultimo, se di tipo 0 controlla tutti gli ordini e vede se sono cucinabili, in tal caso li cucina
    int cucinabile;
    ordine_t* temp_ordine, *prodotto;
    ingrediente_t* temp_ingrediente;

    if (tipo) {
        temp_ordine = massimo_in_albero_ordini(albero_ordini);
    } else {
        temp_ordine = minimo_in_albero_ordini(albero_ordini);
    }

    while (temp_ordine != NULL) {
        cucinabile = 1;
        for(temp_ingrediente = temp_ordine->ricetta->lista_ingredienti; temp_ingrediente != NULL && cucinabile; temp_ingrediente = temp_ingrediente->next) {
            cucinabile *= abbastanza_ingrediente(magazzino, temp_ingrediente->nome, temp_ingrediente->h, temp_ingrediente->q * temp_ordine->q);
        }
        
        if (cucinabile) {
            for(temp_ingrediente = temp_ordine->ricetta->lista_ingredienti; temp_ingrediente != NULL; temp_ingrediente = temp_ingrediente->next) {
                togli_ingrediente(magazzino, temp_ingrediente->nome, temp_ingrediente->h, temp_ingrediente->q * temp_ordine->q);
            }
            prodotto = temp_ordine;
            if (tipo) {
                temp_ordine = NULL;
            } else {
                temp_ordine = successore_albero_ordini(prodotto);
            }
            albero_ordini = cancella_nodo_albero_ordini(albero_ordini, prodotto);
            prodotto->left = NULL;
            prodotto->right = NULL;
            prodotto->p = NULL;
            *albero_prodotti = inserimento_albero_ordini(*albero_prodotti, prodotto);
        } else {
            if (tipo) {
                temp_ordine = NULL;
            } else {
                temp_ordine = successore_albero_ordini(temp_ordine);
            }          
        }
    }
    return albero_ordini;
}

void scadenze(magazzino_hash_t* magazzino, unsigned int t) { //a ogni t corrispondente a una scadenza scorre tutti i lotti ed elimina quelli scaduti
    ingrediente_stock_t *ingrediente_temp, *prev, *elim;
    lotto_t *lotto_temp;
    int i;

    for (i = 0; i < DIM_MAGAZZINO; i++) {
        ingrediente_temp = magazzino->stock_ingredienti[i];
        prev = NULL;
        while (ingrediente_temp != NULL) {
            lotto_temp = minimo_in_albero_lotti(ingrediente_temp->lotto);
            while (lotto_temp && lotto_temp->scadenza <= t) {
                ingrediente_temp->totale -= lotto_temp->q;
                ingrediente_temp->lotto = cancella_nodo_albero_lotti(ingrediente_temp->lotto, lotto_temp);
                free(lotto_temp);
                lotto_temp = minimo_in_albero_lotti(ingrediente_temp->lotto);
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

ordine_t* consegne (unsigned int peso, ordine_t* albero_prodotti) { //controlla tutti i prodotti disponibili e fa il controllo per il peso del furgoncino, poi li spedisce in ordine di peso
    ordine_t *albero_consegne, *temp_consegna, *temp_prodotto, *next_prodotto;
    int cons, stop;
    unsigned long long residuo;

    residuo = peso;
    cons = 0;
    albero_consegne = NULL;
    stop = 1;
    next_prodotto = NULL;

    temp_prodotto = minimo_in_albero_ordini(albero_prodotti);

    if(temp_prodotto) {
        while (temp_prodotto != NULL && stop) { //scorro tutti i prodotti fino a quando non arrivo a NULL o finché non ho trovato il primo che non ci sta sul camioncino (scorro in ordine temporale)
            if (temp_prodotto->peso <= residuo) { //se il peso del mio prodotto è minore= di quanto ancora posso caricare
                cons = 1; //ho fatto almeno una consegna
                residuo -= temp_prodotto->peso; //aggiorno il peso residuo che posso ancora caricare togliendo quanto caricato

                next_prodotto = successore_albero_ordini(temp_prodotto);
                albero_prodotti = cancella_nodo_albero_ordini(albero_prodotti, temp_prodotto);
                temp_prodotto->left = NULL;
                temp_prodotto->right = NULL;
                temp_prodotto->p = NULL;
                albero_consegne = inserimento_albero_consegne(albero_consegne, temp_prodotto);
                temp_prodotto = next_prodotto;

            } else { //se il prodotto è troppo grande mi fermo
                stop = 0;
            }
        }
        temp_consegna = massimo_in_albero_ordini(albero_consegne);
        if (cons) {
            while (temp_consegna) {
                printf ("%d %s %d\n", temp_consegna->t, temp_consegna->ricetta->nome, temp_consegna->q);
                temp_consegna->ricetta->pending--;
                albero_consegne = cancella_nodo_albero_ordini(albero_consegne, temp_consegna);
                free(temp_consegna);
                temp_consegna = NULL;
                temp_consegna = massimo_in_albero_ordini(albero_consegne);
            }
            elimina_albero_ordini(albero_consegne);
            temp_consegna = NULL;
            albero_consegne = NULL;
        } else {
            printf("camioncino vuoto\n");
        }
        return albero_prodotti;
        
    } else {
        printf("camioncino vuoto\n");
        return albero_prodotti;
    }
}

int main () {
    char input[L_NOME_MAX], nome[L_NOME_MAX], ingrediente[L_NOME_MAX];
    int t, fc, pc, qt, peso, sc;
    unsigned int ret;
    char nl;
    ricettario_hash_t *ricettario;
    ordine_t *albero_prodotti, *albero_ordini;
    magazzino_hash_t *magazzino;
    ingrediente_t *lista_ingredienti;
    ricetta_t *tempricetta;

    //if(freopen("open11.txt", "r", stdin));
    //if(freopen("open4.output.txt", "w", stdout));

    ricettario = NULL;
    albero_ordini = NULL;
    albero_prodotti = NULL;
    magazzino = NULL;

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

                if (sc > t) {
                    ret = rifornimento(magazzino, nome, sc, qt);
                }

                if (scanf("%c", &nl));
            }
            printf("rifornito\n");
            scadenze(magazzino, t);
            albero_ordini = cucina(magazzino, albero_ordini, &albero_prodotti, 0);

        } else if (!strcmp(input, "ordine")) {
            if (scanf(" %s %d", nome, &qt));
            albero_ordini = aggiungi_ordine(albero_ordini, ricettario, nome, qt, t, &ret);
            if(ret == 1) {
                printf("accettato\n");
                scadenze(magazzino, t);
                albero_ordini = cucina(magazzino, albero_ordini, &albero_prodotti, 1);
            } else {
                printf("rifiutato\n");
            }
            
        } else {
            printf ("Errore \n");
        }
        t++;

        if (t!=0 && t % fc == 0) { // eseguito a ogni frequenza di passaggio del corriere
            albero_prodotti = consegne(pc, albero_prodotti);
        }
    }

    elimina_albero_ordini(albero_ordini);
    elimina_albero_ordini(albero_prodotti);
    elimina_hash_ricettario(ricettario);
    elimina_hash_magazzino(magazzino);
    return 0;    
}