/*
 * ======================================================
 *  PROVA FINALE - PROGETTO DI ALGORITMI E STRUTTURE DATI
 * ======================================================
 *  Nome        : Stefano Ungaro
 *  Matricola   : 209901
 *  Valutazione : 30L
 * ------------------------------------------------------
 */

#include <stdio.h>          // libreria standard I/O
#include <stdlib.h>         // libreria standard
#include <string.h>         // libreria per le stringhe
#include <assert.h>         // libreria per le asserzioni (usate per leggere con scanf)

#define L_INPUT_MAX 17      // dimensione del comando più lungo che è possibile ricevere
#define L_NOME_MAX 256      // dimensione nome generico (da specifica)
#define VALORE1 2099012099  // valori di hash
#define VALORE2 10836481    // valori di hash
#define DIM_RICETTE 4787    // dimensione del ricettario (arbitraria, scelto un numero primo)
#define DIM_MAGAZZINO 1831  // dimensione del magazzino (arbitraria, scelto un numero primo)

typedef struct lotto
{
    unsigned int scadenza;  // scadenza del lotto sotto forma di intero
    unsigned int q;         // quantità del lotto
    struct lotto *left;     // puntatore al figlio sinistro
    struct lotto *right;    // puntatore al figlio destro
    struct lotto *p;        // puntatore al genitore
} lotto_t;                  // struttura per i lotti contenuta nei singoli ingredienti del magazzino

typedef struct ingrediente_stock
{
    struct ingrediente_stock *next; // puntatore all'ingrediente successivo
    char *nome;                     // nome dell'ingrediente 
    unsigned int totale;            // quantità totale dell'ingrediente (somma di tutti i lotti)
    lotto_t *lotto;                 // puntatore alla radice dell'albero dei lotti per quell'ingrediente
    lotto_t *scadente;              // puntatore al lotto con scadenza più vicina per evitare la scansione dell'albero ogni volta
} ingrediente_stock_t;              // struttura per gli ingredienti contenuti nel magazzino, cioè singoli elementi della tabella hash

typedef struct ingrediente
{
    struct ingrediente_stock *stock; // puntatore all'ingrediente nel magazzino per evitare la scansione dell'hash del magazzino ogni volta
    unsigned int q;                  // quantità richiesta dell'ingrediente
    struct ingrediente *next;        // puntatore all'ingrediente successivo
} ingrediente_t;                     // struttura per gli ingredienti contenuti nelle singole ricette

typedef struct
{
    ingrediente_stock_t *stock_ingredienti[DIM_MAGAZZINO]; // tabella hash per gli ingredienti del magazzino
} magazzino_hash_t;

typedef struct ricetta
{
    char *nome;                         // nome della ricetta   
    ingrediente_t *lista_ingredienti;   // lista degli ingredienti della ricetta
    unsigned int pending;               // numero di ordini in sospeso per quella ricetta
    unsigned int peso;                  // peso della ricetta (somma dei pesi di ogni ingrediente)
    struct ricetta *next;               // puntatore alla ricetta successiva
} ricetta_t;                            // struttura per le ricette contenute nel ricettario, cioè singoli elementi della tabella hash

typedef struct
{
    ricetta_t *elenco_ricette[DIM_RICETTE]; // tabella hash per le ricette del ricettario
} ricettario_hash_t;

typedef struct ordine
{
    unsigned int t;                     // tempo di arrivo dell'ordine
    ricetta_t *ricetta;                 // puntatore alla ricetta dell'ordine
    unsigned int q;                     // quantità richiesta dell'ordine
    unsigned int peso;                  // peso dell'ordine (quantità richiesta * peso della ricetta)
    struct ordine *left;                // puntatore al figlio sinistro
    struct ordine *right;               // puntatore al figlio destro
    struct ordine *p;                   // puntatore al genitore
    ingrediente_t *ultimo_mancante;     // puntatore all'ultimo ingrediente mancante per la preparazione dell'ordine (statisticamente il più probabile che manchi ancora)
} ordine_t;                             // struttura per gli ordini contenuti nella coda degli ordini e nell'albero degli ordini prodotti (la coda ordini è una lista che usa solo il figlio destro, l'albero ordini prodotti è un albero binario di ricerca)

int funzione_hash(char *stringa, int dim) // funzione hash per le stringhe
{
    unsigned int h;
    int i;

    h = VALORE1;

    for (i = 0; stringa[i] != '\0'; i++)
    {
        h ^= stringa[i];
        h *= VALORE2;
    }

    return h % dim;
}

void elimina_lista_ingredienti(ingrediente_t *lista) // funzione per eliminare la lista degli ingredienti di una ricetta
{
    ingrediente_t *temp;

    while (lista != NULL)
    {
        temp = lista;
        lista = lista->next;
        free(temp);
    }
}

ordine_t *minimo_in_albero_ordini(ordine_t *albero) // implementata da pseudocodice slides, generica funzione di calcolo del minimo da un BST
{
    ordine_t *temp;

    temp = albero;

    if (albero)
    {
        while (temp->left != NULL)
        {
            temp = temp->left;
        }
    }
    return temp;
}

ordine_t *massimo_in_albero_ordini(ordine_t *albero) // implementata da pseudocodice slides, generica funzione di calcolo del massimo da un BST
{
    ordine_t *temp;

    temp = albero;

    if (albero)
    {
        while (temp->right != NULL)
        {
            temp = temp->right;
        }
    }
    return temp;
}

ordine_t *successore_albero_ordini(ordine_t *nodo) // implementata da pseudocodice slides, generica funzione di ricerca del successore in un BST
{
    ordine_t *y, *x;

    x = nodo;
    if (nodo->right != NULL)
    {
        return minimo_in_albero_ordini(nodo->right);
    }
    else
    {
        y = nodo->p;
        while (y != NULL && x == y->right)
        {
            x = y;
            y = y->p;
        }
        return y;
    }
}

ordine_t *predecessore_albero_ordini(ordine_t *nodo) // implementata da pseudocodice slides, generica funzione di ricerca del predecessore in un BST
{
    ordine_t *y, *x;

    x = nodo;
    if (nodo->left != NULL)
    {
        return massimo_in_albero_ordini(nodo->left);
    }
    else
    {
        y = nodo->p;
        while (y != NULL && x == y->left)
        {
            x = y;
            y = y->p;
        }
        return y;
    }
}

ordine_t *inserimento_albero_ordini(ordine_t *albero, ordine_t *z) // implementata da pseudocodice slides, generica funzione di inserimento in un BST, gli ordini sono ordinati per tempo d'arrivo, restituisce il puntatore alla radice dell'albero
{
    ordine_t *x, *y;

    y = NULL;
    x = albero;
    z->left = NULL;
    z->right = NULL;

    while (x != NULL)
    {
        y = x;
        if (z->t < x->t)
        {
            x = x->left;
        }
        else
        {
            x = x->right;
        }
    }
    z->p = y;
    if (y == NULL)
    {
        albero = z;
    }
    else if (z->t < y->t)
    {
        y->left = z;
    }
    else
    {
        y->right = z;
    }
    return albero;
}

ordine_t *cancella_nodo_albero_ordini(ordine_t *albero, ordine_t *z) // implementata da pseudocodice slides, generica funzione di eliminazione di un nodo in un BST, restituisce il puntatore alla radice dell'albero
{
    ordine_t *x, *y;

    if (z != NULL && (z->left == NULL || z->right == NULL))
    {
        y = z;
    }
    else
    {
        y = successore_albero_ordini(z);
    }
    if (y != NULL && y->left != NULL)
    {
        x = y->left;
    }
    else
    {
        x = y->right;
    }
    if (x != NULL)
    {
        x->p = y->p;
    }
    if (y != NULL && y->p == NULL)
    {
        albero = x;
    }
    else if (y != NULL && y == y->p->left)
    {
        y->p->left = x;
    }
    else if (y != NULL)
    {
        y->p->right = x;
    }
    if (y != z && y != NULL)
    {
        z->t = y->t;
        z->q = y->q;
        z->ricetta = y->ricetta;
        z->peso = y->peso;
    }
    return albero;
}

lotto_t *minimo_in_albero_lotti(lotto_t *albero) // implementata da pseudocodice slides, generica funzione di ricerca del minimo in un BST
{
    lotto_t *temp;

    temp = albero;

    if (albero)
    {
        while (temp->left != NULL)
        {
            temp = temp->left;
        }
    }
    return temp;
}

lotto_t *massimo_in_albero_lotti(lotto_t *albero) // implementata da pseudocodice slides, generica funzione di ricerca del massimo in un BST
{
    lotto_t *temp;

    temp = albero;

    if (albero)
    {
        while (temp->right != NULL)
        {
            temp = temp->right;
        }
    }
    return temp;
}

lotto_t *successore_albero_lotti(lotto_t *nodo) // implementata da pseudocodice slides, generica funzione di ricerca del successore in un BST
{
    lotto_t *y, *x;

    x = nodo;
    if (nodo->right != NULL)
    {
        return minimo_in_albero_lotti(nodo->right);
    }
    else
    {
        y = nodo->p;
        while (y != NULL && x == y->right)
        {
            x = y;
            y = y->p;
        }
        return y;
    }
}

lotto_t *inserimento_albero_lotti(lotto_t *albero, lotto_t *z) // implementata da pseudocodice slides, generica funzione di ricerca del successore in un BST, i lotti sono ordinati per scadenza, restituisce il puntatore alla radice dell'albero
{
    lotto_t *x, *y;

    y = NULL;
    x = albero;
    z->left = NULL;
    z->right = NULL;

    while (x != NULL)
    {
        y = x;
        if (z->scadenza < x->scadenza)
        {
            x = x->left;
        }
        else
        {
            x = x->right;
        }
    }
    z->p = y;
    if (y == NULL)
    {
        albero = z;
    }
    else if (z->scadenza < y->scadenza)
    {
        y->left = z;
    }
    else
    {
        y->right = z;
    }
    return albero;
}

lotto_t *cancella_nodo_albero_lotti(lotto_t *albero, lotto_t *z) // implementata da pseudocodice slides, generica funzione di eliminazione di un nodo in un BST, restituisce il puntatore alla radice dell'albero
{
    lotto_t *x, *y;

    if (z != NULL && (z->left == NULL || z->right == NULL))
    {
        y = z;
    }
    else
    {
        y = successore_albero_lotti(z);
    }
    if (y != NULL && y->left != NULL)
    {
        x = y->left;
    }
    else
    {
        x = y->right;
    }
    if (x != NULL)
    {
        x->p = y->p;
    }
    if (y != NULL && y->p == NULL)
    {
        albero = x;
    }
    else if (y != NULL && y == y->p->left)
    {
        y->p->left = x;
    }
    else if (y != NULL)
    {
        y->p->right = x;
    }
    if (y != z && y != NULL)
    {
        z->scadenza = y->scadenza;
        z->q = y->q;
    }
    return albero;
}

int confronto(ordine_t *consegna1, ordine_t *consegna2) // per confronto tra consegne in base a peso e ordine d'arrivo, restituisce -1 se consegna1 è minore di consegna2, 1 se è maggiore, non è possibile che siano uguali da specifica
{
    if (consegna1->peso < consegna2->peso)
    {
        return -1;
    }
    else if (consegna1->peso > consegna2->peso)
    {
        return 1;
    }

    if (consegna1->t < consegna2->t)
    {
        return 1;
    }
    else if (consegna1->t > consegna2->t)
    {
        return -1;
    }

    return 0;
}

ordine_t *inserimento_albero_consegne(ordine_t *albero, ordine_t *z) // implementata da pseudocodice slides, generica funzione di inserimento in un BST, gli ordini sono ordinati per peso e ordine d'arrivo, restituisce il puntatore alla radice dell'albero
{
    ordine_t *x, *y;

    y = NULL;
    x = albero;
    z->left = NULL;
    z->right = NULL;

    while (x != NULL)
    {
        y = x;
        if (confronto(z, x) == -1)
        {
            x = x->left;
        }
        else
        {
            x = x->right;
        }
    }
    z->p = y;
    if (y == NULL)
    {
        albero = z;
    }
    else if (confronto(z, y) == -1)
    {
        y->left = z;
    }
    else
    {
        y->right = z;
    }
    return albero;
}

void elimina_albero_ordini(ordine_t *albero) // eventualmente da eseguire alla fine per svuotare la memoria, per evitare memory leak va eseguita ma da specifica non è obbligatoria e rallenta il codice
{
    if (albero != NULL)
    {

        elimina_albero_ordini(albero->left);
        elimina_albero_ordini(albero->right);

        free(albero);
    }
}

void elimina_albero_lotti(lotto_t *albero) // eventualmente da eseguire alla fine per svuotare la memoria, per evitare memory leak va eseguita ma da specifica non è obbligatoria e rallenta il codice
{
    if (albero != NULL)
    {

        elimina_albero_lotti(albero->left);
        elimina_albero_lotti(albero->right);

        free(albero);
    }
}

ricettario_hash_t *crea_hash_ricettario() // init ricettario, tutto vuoto, restituisce il puntatore al ricettario
{
    ricettario_hash_t *ricettario;
    ricettario = calloc(1, sizeof(ricettario_hash_t));
    return ricettario;
}

magazzino_hash_t *crea_hash_magazzino() // init magazzino, tutto vuoto, restituisce il puntatore al magazzino
{
    magazzino_hash_t *magazzino;
    magazzino = calloc(1, sizeof(magazzino_hash_t));
    return magazzino;
}

void elimina_hash_ricettario(ricettario_hash_t *ricettario) // eventualmente da eseguire alla fine per svuotare la memoria
{
    int i;
    ricetta_t *elim, *temp;

    elim = NULL;
    for (i = 0; i < DIM_RICETTE; i++)
    {
        temp = ricettario->elenco_ricette[i];
        while (temp != NULL)
        {
            elimina_lista_ingredienti(temp->lista_ingredienti);
            elim = temp;
            temp = temp->next;
            free(elim->nome);
            free(elim);
            elim = NULL;
        }
    }
    free(ricettario);
    ricettario = NULL;
}

void elimina_hash_magazzino(magazzino_hash_t *magazzino) // eventualmente da eseguire alla fine per svuotare la memoria, per evitare memory leak va eseguita ma da specifica non è obbligatoria e rallenta il codice
{
    int i;
    ingrediente_stock_t *elim, *temp;

    elim = NULL;
    for (i = 0; i < DIM_MAGAZZINO; i++)
    {
        temp = magazzino->stock_ingredienti[i];
        while (temp != NULL)
        {
            elimina_albero_lotti(temp->lotto);
            elim = temp;
            temp = temp->next;
            free(elim->nome);
            free(elim);
            elim = NULL;
        }
    }
    free(magazzino);
    magazzino = NULL;
}

ingrediente_t *aggiungi_ingrediente(magazzino_hash_t *magazzino, ingrediente_t *lista, char **nome, unsigned int q) // funzione per aggiungere un ingrediente alla lista degli ingredienti di una ricetta, restituisce il puntatore all'ingrediente aggiunto
{
    ingrediente_t *temp;
    ingrediente_stock_t *stock_temp, *stock_prev;
    int h, stop;

    stop = 0;
    stock_prev = NULL;
    temp = malloc(sizeof(ingrediente_t));

    h = funzione_hash(*nome, DIM_MAGAZZINO);
    stock_temp = magazzino->stock_ingredienti[h];

    if (stock_temp == NULL) // se all'indirizzo fornito dalla funzione hash non c'è nessun ingrediente (prima aggiunta assoluta per quel codice hash)
    {
        stock_temp = calloc(1, sizeof(ingrediente_stock_t));
        stock_temp->nome = *nome;
        temp->next = lista;
        temp->q = q;
        temp->stock = stock_temp;
        magazzino->stock_ingredienti[h] = stock_temp;
        return temp;
    }
    else // se all'indirizzo fornito dalla funzione hash c'è almeno un ingrediente
    {
        for (stock_temp = magazzino->stock_ingredienti[h]; stock_temp != NULL && !stop; stock_temp = stock_temp->next)
        {
            if (!strcmp(stock_temp->nome, *nome)) // se l'ingrediente è già presente
            {
                stop = 1; // stop è un flag per evitare di continuare a scorrere la lista una volta trovato l'ingrediente (era stato aggiunto da una ricetta senza che ci fossero stati rifornimenti)
                temp->next = lista;
                temp->q = q;
                temp->stock = stock_temp;
                free(*nome);
                return temp;
            }
            stock_prev = stock_temp;
        }
        stock_prev->next = calloc(1, sizeof(ingrediente_stock_t));
        stock_prev->next->nome = *nome;
        temp->next = lista;
        temp->q = q;
        temp->stock = stock_prev->next;
        return temp;
    }
}

int aggiungi_ricetta(ricettario_hash_t *ricettario, char **nome_ricetta, ingrediente_t *lista_ingredienti, unsigned int peso) // funzione per aggiungere una ricetta al ricettario, restituisce l'indice della ricetta nel ricettario
{
    int i;
    ricetta_t *temp_ricetta;
    i = funzione_hash(*nome_ricetta, DIM_RICETTE);

    temp_ricetta = NULL;
    temp_ricetta = calloc(1, sizeof(ricetta_t));

    temp_ricetta->nome = *nome_ricetta;
    temp_ricetta->lista_ingredienti = lista_ingredienti;
    temp_ricetta->peso = peso;

    if (ricettario->elenco_ricette[i] == NULL) // se all'indirizzo fornito dalla funzione hash non c'è nessuna ricetta (prima aggiunta assoluta per quel codice hash)
    {
        ricettario->elenco_ricette[i] = temp_ricetta;
        return i;
    }
    else // se all'indirizzo fornito dalla funzione hash c'è almeno una ricetta aggiungo la nuova ricetta in testa. È garantito che non vengano aggiunte ricette con lo stesso nome
    {
        temp_ricetta->next = ricettario->elenco_ricette[i];
        ricettario->elenco_ricette[i] = temp_ricetta;
        return i;
    }
}

int rifornimento(magazzino_hash_t *magazzino, char **nome, unsigned int scadenza, unsigned int quantita) // funzione per rifornire il magazzino di un nuovo lotto di un ingrediente, restituisce l'indice dell'ingrediente nel magazzino
{
    lotto_t *lottonuovo;
    ingrediente_stock_t *ingrediente_temp, *ingrediente_prev;
    int h;

    ingrediente_prev = NULL;

    lottonuovo = calloc(1, sizeof(lotto_t));

    lottonuovo->scadenza = scadenza;
    lottonuovo->q = quantita;
    h = funzione_hash(*nome, DIM_MAGAZZINO);

    if (magazzino->stock_ingredienti[h] == NULL) // se all'indirizzo fornito dalla funzione hash non c'è nessun ingrediente (prima aggiunta assoluta per quel codice hash)
    {

        ingrediente_temp = malloc(sizeof(ingrediente_stock_t));
        ingrediente_temp->totale = quantita;
        ingrediente_temp->lotto = lottonuovo;
        ingrediente_temp->nome = *nome;
        ingrediente_temp->scadente = lottonuovo;
        magazzino->stock_ingredienti[h] = ingrediente_temp;
        magazzino->stock_ingredienti[h]->next = NULL;

        return h;
    }
    else // se all'indirizzo fornito dalla funzione hash c'è almeno un ingrediente
    {

        for (ingrediente_temp = magazzino->stock_ingredienti[h]; ingrediente_temp != NULL; ingrediente_temp = ingrediente_temp->next)
        {
            if (!strcmp(ingrediente_temp->nome, *nome)) // se l'ingrediente è già presente
            {
                ingrediente_temp->totale += quantita;
                ingrediente_temp->lotto = inserimento_albero_lotti(ingrediente_temp->lotto, lottonuovo);
                if (ingrediente_temp->scadente == NULL || ingrediente_temp->scadente->scadenza > lottonuovo->scadenza)
                {
                    ingrediente_temp->scadente = lottonuovo;
                }
                free(*nome);
                return h;
            }
            ingrediente_prev = ingrediente_temp;
        } // se l'ingrediente non è presente lo aggiungo in coda alla lista
        ingrediente_prev->next = malloc(sizeof(ingrediente_stock_t));
        ingrediente_temp = ingrediente_prev->next;
        ingrediente_temp->totale = quantita;
        ingrediente_temp->lotto = lottonuovo;
        ingrediente_temp->next = NULL;
        ingrediente_temp->scadente = lottonuovo;
        ingrediente_temp->nome = *nome;

        return h;
    }
}

ricetta_t *ricerca_ricetta(ricettario_hash_t *ricettario, char *nome_ricetta) // funzione per cercare una ricetta nel ricettario, restituisce il puntatore alla ricetta se presente, NULL altrimenti
{
    int i;
    ricetta_t *temp;
    i = funzione_hash(nome_ricetta, DIM_RICETTE);
    temp = ricettario->elenco_ricette[i];

    for (; temp != NULL; temp = temp->next)
    {
        if (!strcmp(temp->nome, nome_ricetta))
        {
            return temp;
        }
    }
    return NULL;
}

int elimina_ricetta(ricettario_hash_t *ricettario, char *nome_ricetta) // funzione per eliminare una ricetta dal ricettario, restituisce 1 se la ricetta è stata eliminata, -1 se non è stata trovata, -2 se ci sono ordini in sospeso per quella ricetta
{
    int i;
    ricetta_t *curr, *prec, *temp;
    prec = NULL;

    i = funzione_hash(nome_ricetta, DIM_RICETTE);
    curr = ricettario->elenco_ricette[i];

    while (curr != NULL)
    {
        if (!strcmp(curr->nome, nome_ricetta))
        {
            if (curr->pending > 0) // se ci sono ordini in sospeso non posso eliminare la ricetta
            {
                return -2;
            }
            else // se non ci sono ordini in sospeso elimino la ricetta
            {
                temp = curr;
                curr = curr->next;
                if (prec != NULL)
                {
                    prec->next = curr;
                }
                else
                {
                    ricettario->elenco_ricette[i] = curr;
                }
                elimina_lista_ingredienti(temp->lista_ingredienti);
                free(temp->nome);
                free(temp);
                return 1;
            }
        }
        else
        {
            prec = curr;
            curr = curr->next;
        }
    }
    return -1;
}

ordine_t *aggiungi_ordine(ordine_t *coda, ordine_t **last, ricettario_hash_t *ricettario, char *nome, unsigned int q, unsigned int t, unsigned int *ret) // funzione per aggiungere un ordine alla coda degli ordini, restituisce la coda aggiornata. La coda è ottimizzata per l'inserimento in coda e la cancellazione in testa, visto che il tempo di arrivo degli ordini è monotono crescente. ret restituisce 1 se l'ordine è stato aggiunto, -1 se la ricetta non è stata trovata
{
    ricetta_t *ricetta;
    ordine_t *ordine, *ultimo;

    ricetta = ricerca_ricetta(ricettario, nome);
    if (ricetta) // se la ricetta è presente
    {
        ricetta->pending++;
        ordine = calloc(1, sizeof(ordine_t));

        ordine->t = t;
        ordine->q = q;
        ordine->peso = ordine->q * ricetta->peso;
        ordine->ricetta = ricetta;
        *ret = 1;
        if (coda) // se la coda non è vuota
        {
            ultimo = (*last);
            ultimo->right = ordine;
            ordine->p = ultimo;
            *last = ordine;
        }
        else // se la coda è vuota (prima aggiunta))
        {
            coda = ordine;
            ordine->p = NULL;
            ordine->right = NULL;
            *last = coda;
        }
        return coda;
    }
    else // se la ricetta non è presente
    {
        *ret = -1;
        return coda;
    }
}

void togli_ingrediente(ingrediente_stock_t *ingrediente, unsigned int qt) // funzione per togliere un ingrediente dal magazzino nella quantità richiesta, aggiorna anche l'albero dei lotti consumando quello che scade prima, se l'ingrediente è esaurito non lo elimina dal magazzino ma lo lascia con quantità 0 (per integruità dei puntatori nelle ricette e per evitare di doverlo ricreare se arriva un rifornimento). Si assume che la quantità richiesta sia sempre minore o uguale alla quantità totale dell'ingrediente dato l'utilizzo della funzione solo dopo la verifica che ce ne siano abbastanza
{
    unsigned int resto;
    lotto_t *lotto_temp, *elim;

    if (ingrediente->totale >= qt)
    {
        ingrediente->totale -= qt;
        if (ingrediente->totale == 0) // se l'ingrediente è esaurito lo lascio a zero ma tolgo i suoi lotti indistintamente
        {
            elimina_albero_lotti(ingrediente->lotto);
            ingrediente->lotto = NULL;
            ingrediente->scadente = NULL;
        }
        else // se l'ingrediente non è esaurito tolgo i lotti partendo da quello con scadenza più vicina
        {
            resto = qt;
            lotto_temp = ingrediente->scadente;
            while (resto > 0)
            {
                if (lotto_temp->q > resto)
                {
                    lotto_temp->q -= resto;
                    resto = 0;
                }
                else
                {
                    resto -= lotto_temp->q;
                    elim = lotto_temp;
                    lotto_temp = successore_albero_lotti(lotto_temp);
                    ingrediente->lotto = cancella_nodo_albero_lotti(ingrediente->lotto, elim);
                    ingrediente->scadente = lotto_temp;
                    free(elim);
                    elim = NULL;
                }
            }
        }
    }
}

ordine_t *cucina(magazzino_hash_t *magazzino, ordine_t *codaordini, ordine_t **last, ordine_t **albero_prodotti, int tipo) // funzione per cucinare gli ordini, restituisce la coda degli ordini aggiornata e aggiunge gli ordini cucinati all'albero degli ordini prodotti. Il tipo aiuta a ottimizzare la funzione, se tipo è 1 si parte dall'ultimo ordine arrivato (eseguita quando è creato un ordine), se è 0 si parte dal primo ordine in coda (eseguito dopo ogni rifornimento)
{
    int cucinabile;
    ordine_t *temp_ordine, *prodotto;
    ingrediente_t *temp_ingrediente;

    if (tipo)
    {
        temp_ordine = *last;
    }
    else
    {
        temp_ordine = codaordini;
    }

    while (temp_ordine != NULL)
    {
        cucinabile = 1;
        if (temp_ordine->ultimo_mancante != NULL && temp_ordine->ultimo_mancante->stock->totale < temp_ordine->ultimo_mancante->q * temp_ordine->q) // se l'ultimo ingrediente mancante è ancora mancante non è cucinabile (ottimizzazione statistica)
        {
            cucinabile = 0;
        }

        for (temp_ingrediente = temp_ordine->ricetta->lista_ingredienti; cucinabile && temp_ingrediente != NULL; temp_ingrediente = temp_ingrediente->next)
        {
            if (temp_ingrediente->stock->totale >= temp_ingrediente->q * temp_ordine->q) // controllo se c'è abbastanza di ogni ingrediente
            {
                cucinabile = 1;
            }
            else
            {
                cucinabile = 0;
                temp_ordine->ultimo_mancante = temp_ingrediente;
            }
        }

        if (cucinabile) // se l'ordine è cucinabile (ho abbastanza di ogni ingrediente)
        {
            for (temp_ingrediente = temp_ordine->ricetta->lista_ingredienti; temp_ingrediente != NULL; temp_ingrediente = temp_ingrediente->next)
            {
                togli_ingrediente(temp_ingrediente->stock, temp_ingrediente->q * temp_ordine->q);
            }
            prodotto = temp_ordine;
            temp_ordine = temp_ordine->right;
            if (temp_ordine == NULL)
            {
                *last = prodotto->p;
                if (*last)
                {
                    (*last)->right = NULL;
                }
            }
            else
            {
                temp_ordine->p = prodotto->p;
            }
            if (prodotto->p != NULL)
            {
                prodotto->p->right = temp_ordine;
            }
            else
            {
                codaordini = temp_ordine;
                if (temp_ordine)
                {
                    temp_ordine->p = NULL;
                }
                else
                {
                    *last = NULL;
                }
            }
            prodotto->left = NULL;
            prodotto->right = NULL;
            prodotto->p = NULL;
            *albero_prodotti = inserimento_albero_ordini(*albero_prodotti, prodotto);
        }
        else
        {
            temp_ordine = temp_ordine->right;
        }
    }
    return codaordini;
}

void scadenze(magazzino_hash_t *magazzino, unsigned int t) // funzione per eliminare i lotti scaduti, aggiorna il magazzino
{
    ingrediente_stock_t *ingrediente_temp;
    lotto_t *lotto_temp, *lotto_elim;
    int i;

    for (i = 0; i < DIM_MAGAZZINO; i++) // scorro tutto il magazzino
    {
        ingrediente_temp = magazzino->stock_ingredienti[i];
        while (ingrediente_temp != NULL)
        {
            if (ingrediente_temp->totale != 0) // se l'ingrediente non è esaurito
            {
                lotto_temp = ingrediente_temp->scadente; // parto dal lotto con scadenza più vicina
                while (lotto_temp && lotto_temp->scadenza <= t)
                {
                    ingrediente_temp->totale -= lotto_temp->q;
                    lotto_elim = lotto_temp;
                    lotto_temp = successore_albero_lotti(lotto_temp);
                    ingrediente_temp->lotto = cancella_nodo_albero_lotti(ingrediente_temp->lotto, lotto_elim);
                    free(lotto_elim);
                    lotto_elim = NULL;
                    ingrediente_temp->scadente = lotto_temp;
                }
            }

            ingrediente_temp = ingrediente_temp->next;
        }
    }
}

ordine_t *consegne(unsigned int peso, ordine_t *albero_prodotti) // funzione per consegnare gli ordini, restituisce l'albero degli ordini prodotti aggiornato e si occupa delle stampe delle consegne
{
    ordine_t *albero_consegne, *temp_consegna, *temp_prodotto, *next_prodotto, *elim_consegna;
    int cons;
    unsigned long long residuo;

    residuo = peso;
    cons = 0;
    albero_consegne = NULL;
    next_prodotto = NULL;

    temp_prodotto = minimo_in_albero_ordini(albero_prodotti);

    if (temp_prodotto == NULL) // se non ci sono ordini prodotti
    {
        printf("camioncino vuoto\n");
        return albero_prodotti;
    }
    else // se ci sono ordini prodotti
    {
        while (temp_prodotto != NULL && temp_prodotto->peso <= residuo)
        {
            cons = 1;
            residuo -= temp_prodotto->peso;
            next_prodotto = successore_albero_ordini(temp_prodotto);
            albero_prodotti = cancella_nodo_albero_ordini(albero_prodotti, temp_prodotto);
            temp_prodotto->left = NULL;
            temp_prodotto->right = NULL;
            temp_prodotto->p = NULL;
            albero_consegne = inserimento_albero_consegne(albero_consegne, temp_prodotto);
            temp_prodotto = next_prodotto;
        }
        temp_consegna = massimo_in_albero_ordini(albero_consegne);
        if (cons) // se ci sono consegne
        {
            while (temp_consegna)
            {
                printf("%d %s %d\n", temp_consegna->t, temp_consegna->ricetta->nome, temp_consegna->q);
                temp_consegna->ricetta->pending--;
                elim_consegna = temp_consegna;
                temp_consegna = predecessore_albero_ordini(temp_consegna);
                albero_consegne = cancella_nodo_albero_ordini(albero_consegne, elim_consegna);
                free(elim_consegna);
                elim_consegna = NULL;
            }
            temp_consegna = NULL;
            albero_consegne = NULL;
        }
        else
        {
            printf("camioncino vuoto\n");
        }
        return albero_prodotti;
    }
}

int main()
{
    char input[L_INPUT_MAX], nome[L_NOME_MAX], ingrediente[L_NOME_MAX]; 
    int t, fc, pc, qt, peso, sc;
    unsigned int ret, rif;
    char nl;
    ricettario_hash_t *ricettario;
    ordine_t *albero_prodotti, *coda_ordini, *last;
    magazzino_hash_t *magazzino;
    ingrediente_t *lista_ingredienti;
    ricetta_t *tempricetta;
    char *tempstringa;

    rif = 0;
    ricettario = NULL;
    coda_ordini = NULL;
    albero_prodotti = NULL;
    magazzino = NULL;
    last = NULL;
    tempstringa = NULL;

    // inizializzazione strutture dati e check
    ricettario = crea_hash_ricettario();
    magazzino = crea_hash_magazzino();

    if (ricettario == NULL || magazzino == NULL)
    {
        printf("Errore creazione magazzino e/o ricettario.\n");
        return 0;
    }

    t = 0;

    // uso le assert per evitare problemi con l'inutilizzo del valore di ritorno di scanf, eventualmente posso usare if(scanf) ma è meno intuitivo
    assert(scanf("%d %d ", &fc, &pc));

    // ciclo principale che arriva alla fine dello stdin
    while (scanf(" %s", input) != EOF)
    {
        nome[0] = '\0';

        if (input[2] == 'g')    // agGiungi_ricetta
        {
            tempricetta = NULL;
            assert(scanf(" %s", nome));

            tempricetta = ricerca_ricetta(ricettario, nome);

            if (tempricetta == NULL) // se la ricetta non è già presente
            {
                lista_ingredienti = NULL;
                peso = 0;
                nl = getchar();
                while (nl != '\n') // leggo gli ingredienti e li aggiungo alla lista degli ingredienti della ricetta
                {
                    assert(scanf("%s %d", ingrediente, &qt));
                    peso += qt;
                    tempstringa = malloc(sizeof(char) * (strlen(ingrediente) + 1));
                    strcpy(tempstringa, ingrediente);
                    lista_ingredienti = aggiungi_ingrediente(magazzino, lista_ingredienti, &tempstringa, qt);
                    nl = getchar();
                    tempstringa = NULL;
                }
                // uso stringhe dinamiche per allocare meno memoria possibile, evitando di allocare spazi inutili
                tempstringa = malloc(sizeof(char) * (strlen(nome) + 1));
                strcpy(tempstringa, nome);
                ret = aggiungi_ricetta(ricettario, &tempstringa, lista_ingredienti, peso);
                if (ret == -1) // se non è stata aggiunta la ricetta
                {
                    elimina_lista_ingredienti(lista_ingredienti);
                    free(tempstringa);
                    printf("errore aggiunta ricetta.\n");
                }
                else
                {
                    printf("aggiunta\n");
                }
                tempstringa = NULL;
            }
            else // se la ricetta è già presente ignoro tutto fino al prossimo comando
            {
                assert(!scanf(" %*[^\n]")); // valore di ritorno della scanf che ignora la riga è 0
                printf("ignorato\n");
            }
        }
        else if (input[2] == 'm')   // riMuovi_ricetta
        {
            assert(scanf(" %[^\n]", nome));
            ret = elimina_ricetta(ricettario, nome);
            if (ret == 1)           // se la ricetta è stata eliminata
            {
                printf("rimossa\n");
            }
            else if (ret == -1)     // se la ricetta non viene trovata
            {
                printf("non presente\n");
            }
            else                    // se ci sono ordini in sospeso (ret = -2)
            {
                printf("ordini in sospeso\n");
            }
        }
        else if (input[2] == 'f')   // riFornimento
        {
            nl = getchar();         // leggo il carattere di spazio (sarà ' ' oppure '\n')
            while (nl != '\n')
            {
                assert(scanf("%s %d %d", nome, &qt, &sc));
                // uso stringhe dinamiche per allocare meno memoria possibile, evitando di allocare spazi inutili
                tempstringa = malloc(sizeof(char) * (strlen(nome) + 1));
                strcpy(tempstringa, nome);

                if (sc > t)         // se la scadenza è futura (nei test ci arrivano anche prodotti già scaduti)
                {
                    rif = 1;        // flag per rifornimento (implementazione abbastanza superflua ma evita i primi controlli inutili se non ci sono ancora stati rifornimenti)
                    ret = rifornimento(magazzino, &tempstringa, sc, qt); // non uso il valore di ritorno perché non previsto dalla specifica ma utile per debug
                }
                else
                {
                    free(tempstringa);
                }

                tempstringa = NULL;

                nl = getchar();
            }
            printf("rifornito\n");
            scadenze(magazzino, t);
            // se ci sono ordini in sospeso li cucino con il codice 0, che controlla l'intera coda ordini
            coda_ordini = cucina(magazzino, coda_ordini, &last, &albero_prodotti, 0);
        }
        else if (input[2] == 'd')   // orDine
        {
            assert(scanf(" %s %d", nome, &qt));
            coda_ordini = aggiungi_ordine(coda_ordini, &last, ricettario, nome, qt, t, &ret);
            if (ret == 1)           // se l'ordine è stato aggiunto
            {
                printf("accettato\n");
                if (rif)            // se c'è stato un rifornimento
                {
                    scadenze(magazzino, t);
                    // cucino l'ordine appena aggiunto con il codice 1, che parte dall'ultimo ordine aggiunto, visto che l'ordine in sé non cambia il magazzino e se gli ordini precedenti non potevano essere cucinati non lo saranno ancora
                    coda_ordini = cucina(magazzino, coda_ordini, &last, &albero_prodotti, 1);
                }
            }
            else                    // se l'ordine non è stato aggiunto (ricetta non trovata)
            {
                printf("rifiutato\n");
            }
        }
        else                        // non ci sono altri comandi, implementazione per evitare errori in debug
        {
            printf("Errore \n");
        }
        t++;        // incremento del tempo generale

        if (t != 0 && t % fc == 0)  // se è il momento delle consegne (il corriere passa ogni fc istanti di tempo)
        {
            // la funzione consegne prende il gestione anche stdout
            albero_prodotti = consegne(pc, albero_prodotti);
        }
    }

    // eliminazione strutture dati, non necessarie per specifica ma in un utilizzo reale sarebbero necessarie per evitare memory leak
    // rallentano notevolmente il programma

    /*
    elimina_albero_ordini(coda_ordini);
    elimina_albero_ordini(albero_prodotti);
    elimina_hash_ricettario(ricettario);
    elimina_hash_magazzino(magazzino);
    */

    return 0;       // fine programma
}