#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define L_INPUT_MAX 17
#define L_NOME_MAX 256
#define DIM_RICETTE 3923
#define DIM_MAGAZZINO 907

typedef struct lotto
{
    unsigned int scadenza;
    unsigned int q;
    struct lotto *left;
    struct lotto *right;
    struct lotto *p;
} lotto_t;

typedef struct ingrediente_stock
{
    struct ingrediente_stock *next;
    char *nome;
    unsigned int totale;
    lotto_t *lotto;
    lotto_t *scadente;
} ingrediente_stock_t;

typedef struct ingrediente
{
    struct ingrediente_stock *stock;
    unsigned int q;
    struct ingrediente *next;
} ingrediente_t;

typedef struct
{
    ingrediente_stock_t *stock_ingredienti[DIM_MAGAZZINO];
} magazzino_hash_t;

typedef struct ricetta
{
    char *nome;
    ingrediente_t *lista_ingredienti;
    unsigned int pending;
    unsigned int peso;
    struct ricetta *next;
} ricetta_t;

typedef struct
{
    ricetta_t *elenco_ricette[DIM_RICETTE];
} ricettario_hash_t;

typedef struct ordine
{
    unsigned int t;
    ricetta_t *ricetta;
    unsigned int q;
    unsigned int peso;
    struct ordine *left;
    struct ordine *right;
    struct ordine *p;
} ordine_t;

int funzione_hash(char *stringa, int dim)
{
    unsigned int h;
    int i;

    h = 2166136261;

    for (i = 0; stringa[i] != '\0'; i++)
    {
        h ^= stringa[i];
        h *= 16777619;
    }

    return h % dim;
}

void elimina_lista_ingredienti(ingrediente_t *lista)
{
    ingrediente_t *temp;

    while (lista != NULL)
    {
        temp = lista;
        lista = lista->next;
        free(temp);
        temp = NULL;
    }
}

ordine_t *minimo_in_albero_ordini(ordine_t *albero)
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

ordine_t *massimo_in_albero_ordini(ordine_t *albero)
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

ordine_t *successore_albero_ordini(ordine_t *nodo)
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

ordine_t *inserimento_albero_ordini(ordine_t *albero, ordine_t *z)
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

ordine_t *cancella_nodo_albero_ordini(ordine_t *albero, ordine_t *z)
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

lotto_t *minimo_in_albero_lotti(lotto_t *albero)
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

lotto_t *massimo_in_albero_lotti(lotto_t *albero)
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

lotto_t *successore_albero_lotti(lotto_t *nodo)
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

lotto_t *inserimento_albero_lotti(lotto_t *albero, lotto_t *z)
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

lotto_t *cancella_nodo_albero_lotti(lotto_t *albero, lotto_t *z)
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

int confronto(ordine_t *consegna1, ordine_t *consegna2)
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

ordine_t *inserimento_albero_consegne(ordine_t *albero, ordine_t *z)
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

void elimina_albero_ordini(ordine_t *albero)
{
    if (albero != NULL)
    {

        elimina_albero_ordini(albero->left);
        elimina_albero_ordini(albero->right);

        free(albero);
    }
}

void elimina_albero_lotti(lotto_t *albero)
{
    if (albero != NULL)
    {

        elimina_albero_lotti(albero->left);
        elimina_albero_lotti(albero->right);

        free(albero);
    }
}

ricettario_hash_t *crea_hash_ricettario()
{
    ricettario_hash_t *ricettario;
    ricettario = malloc(sizeof(ricettario_hash_t));
    if (ricettario)
    {
        int i;
        for (i = 0; i < DIM_RICETTE; i++)
        {
            ricettario->elenco_ricette[i] = NULL;
        }
        return ricettario;
    }
    else
    {
        return NULL;
    }
}

magazzino_hash_t *crea_hash_magazzino()
{
    magazzino_hash_t *magazzino;
    magazzino = malloc(sizeof(magazzino_hash_t));
    if (magazzino)
    {
        int i;
        for (i = 0; i < DIM_MAGAZZINO; i++)
        {
            magazzino->stock_ingredienti[i] = NULL;
        }
        return magazzino;
    }
    else
    {
        return NULL;
    }
}

void elimina_hash_ricettario(ricettario_hash_t *ricettario)
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

void elimina_hash_magazzino(magazzino_hash_t *magazzino)
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

ingrediente_t *aggiungi_ingrediente(magazzino_hash_t *magazzino, ingrediente_t *lista, char **nome, unsigned int q)
{
    ingrediente_t *temp;
    ingrediente_stock_t *stock_temp, *stock_prev;
    int h, stop;

    stop = 0;
    stock_prev = NULL;
    temp = malloc(sizeof(ingrediente_t));
    if (temp)
    {
        h = funzione_hash(*nome, DIM_MAGAZZINO);
        stock_temp = magazzino->stock_ingredienti[h];

        if (stock_temp == NULL)
        {
            stock_temp = malloc(sizeof(ingrediente_stock_t));
            if (stock_temp)
            {
                stock_temp->nome = *nome;
                stock_temp->next = NULL;
                stock_temp->totale = 0;
                stock_temp->lotto = NULL;
                stock_temp->scadente = NULL;
                temp->next = lista;
                temp->q = q;
                temp->stock = stock_temp;
                magazzino->stock_ingredienti[h] = stock_temp;
                return temp;
            }
            else
            {
                return lista;
            }
        }
        else
        {
            for (stock_temp = magazzino->stock_ingredienti[h]; stock_temp != NULL && !stop; stock_temp = stock_temp->next)
            {
                if (!strcmp(stock_temp->nome, *nome))
                {
                    stop = 1;
                    temp->next = lista;
                    temp->q = q;
                    temp->stock = stock_temp;
                    free(*nome);
                    return temp;
                }
                stock_prev = stock_temp;
            }
            stock_prev->next = malloc(sizeof(ingrediente_stock_t));
            stock_temp = stock_prev->next;
            if (stock_temp)
            {
                stock_temp->totale = 0;
                stock_temp->lotto = NULL;
                stock_temp->next = NULL;
                stock_temp->nome = *nome;
                stock_temp->scadente = NULL;
                temp->next = lista;
                temp->q = q;
                temp->stock = stock_temp;
                return temp;
            }
            else
            {
                return lista;
            }
        }
    }
    else
    {
        printf("errore aggiunta ingrediente\n");
        return lista;
    }
}

int aggiungi_ricetta(ricettario_hash_t *ricettario, char **nome_ricetta, ingrediente_t *lista_ingredienti, unsigned int peso)
{
    int i;
    ricetta_t *temp_ricetta;
    i = funzione_hash(*nome_ricetta, DIM_RICETTE);

    temp_ricetta = NULL;
    temp_ricetta = malloc(sizeof(ricetta_t));
    if (temp_ricetta)
    {
        temp_ricetta->nome = *nome_ricetta;
        temp_ricetta->lista_ingredienti = lista_ingredienti;
        temp_ricetta->pending = 0;
        temp_ricetta->peso = peso;
        temp_ricetta->next = NULL;

        if (ricettario->elenco_ricette[i] == NULL)
        {
            ricettario->elenco_ricette[i] = temp_ricetta;
            return i;
        }
        else
        {
            temp_ricetta->next = ricettario->elenco_ricette[i];
            ricettario->elenco_ricette[i] = temp_ricetta;
            return i;
        }
    }
    else
    {
        printf("errore aggiunta ricetta\n");
        return -1;
    }
}

int rifornimento(magazzino_hash_t *magazzino, char **nome, unsigned int scadenza, unsigned int quantita)
{
    lotto_t *lottonuovo;
    ingrediente_stock_t *ingrediente_temp, *ingrediente_prev;
    int h;

    ingrediente_prev = NULL;

    lottonuovo = malloc(sizeof(lotto_t));

    if (lottonuovo)
    {
        lottonuovo->scadenza = scadenza;
        lottonuovo->q = quantita;
        lottonuovo->left = NULL;
        lottonuovo->right = NULL;
        lottonuovo->p = NULL;

        h = funzione_hash(*nome, DIM_MAGAZZINO);

        if (magazzino->stock_ingredienti[h] == NULL)
        {

            ingrediente_temp = malloc(sizeof(ingrediente_stock_t));
            if (ingrediente_temp)
            {
                ingrediente_temp->totale = quantita;
                ingrediente_temp->lotto = lottonuovo;
                ingrediente_temp->nome = *nome;
                ingrediente_temp->scadente = lottonuovo;
                magazzino->stock_ingredienti[h] = ingrediente_temp;
                magazzino->stock_ingredienti[h]->next = NULL;
            }
            else
            {
                return -1;
            }

            return h;
        }
        else
        {

            for (ingrediente_temp = magazzino->stock_ingredienti[h]; ingrediente_temp != NULL; ingrediente_temp = ingrediente_temp->next)
            {
                if (!strcmp(ingrediente_temp->nome, *nome))
                {
                    ingrediente_temp->totale += quantita;
                    ingrediente_temp->lotto = inserimento_albero_lotti(ingrediente_temp->lotto, lottonuovo);
                    if (ingrediente_temp->scadente == NULL || ingrediente_temp->scadente->scadenza > lottonuovo->scadenza)
                    {
                        ingrediente_temp->scadente = lottonuovo;
                    }
                    ingrediente_temp->scadente = minimo_in_albero_lotti(ingrediente_temp->lotto);
                    free(*nome);
                    return h;
                }
                ingrediente_prev = ingrediente_temp;
            }
            ingrediente_prev->next = malloc(sizeof(ingrediente_stock_t));
            if (ingrediente_prev->next)
            {
                ingrediente_temp = ingrediente_prev->next;
                ingrediente_temp->totale = quantita;
                ingrediente_temp->lotto = lottonuovo;
                ingrediente_temp->next = NULL;
                ingrediente_temp->scadente = lottonuovo;
                ingrediente_temp->nome = *nome;
            }
            else
            {
                return -1;
            }
            return h;
        }
    }
    else
    {
        return -1;
    }
}

ricetta_t *ricerca_ricetta(ricettario_hash_t *ricettario, char *nome_ricetta)
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

int elimina_ricetta(ricettario_hash_t *ricettario, char *nome_ricetta)
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
            if (curr->pending > 0)
            {
                return -2;
            }
            else
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

ordine_t *aggiungi_ordine(ordine_t *coda, ordine_t **last, ricettario_hash_t *ricettario, char *nome, unsigned int q, unsigned int t, unsigned int *ret)
{
    ricetta_t *ricetta;
    ordine_t *ordine, *ultimo;

    ricetta = ricerca_ricetta(ricettario, nome);
    if (ricetta)
    {
        ricetta->pending++;
        ordine = malloc(sizeof(ordine_t));
        if (ordine)
        {
            ordine->left = NULL;
            ordine->right = NULL;
            ordine->p = NULL;
            ordine->t = t;
            ordine->q = q;
            ordine->peso = ordine->q * ricetta->peso;
            ordine->ricetta = ricetta;
            *ret = 1;
            if (coda)
            {
                ultimo = (*last);
                ultimo->right = ordine;
                ordine->p = ultimo;
                *last = ordine;
            }
            else
            {
                coda = ordine;
                ordine->p = NULL;
                ordine->right = NULL;
                *last = coda;
            }
            return coda;
        }
        else
        {
            *ret = -1;
            return coda;
        }
    }
    else
    {
        *ret = -1;
        return coda;
    }
}

void togli_ingrediente(ingrediente_stock_t *ingrediente, unsigned int qt)
{
    unsigned int resto;
    lotto_t *lotto_temp, *elim;

    if (ingrediente->totale >= qt)
    {
        ingrediente->totale -= qt;
        if (ingrediente->totale == 0)
        {
            elimina_albero_lotti(ingrediente->lotto);
            ingrediente->lotto = NULL;
            ingrediente->scadente = NULL;
        }
        else
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

ordine_t *cucina(magazzino_hash_t *magazzino, ordine_t *codaordini, ordine_t **last, ordine_t **albero_prodotti, int tipo)
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
        for (temp_ingrediente = temp_ordine->ricetta->lista_ingredienti; cucinabile && temp_ingrediente != NULL; temp_ingrediente = temp_ingrediente->next)
        {
            if (temp_ingrediente->stock->totale >= temp_ordine->q * temp_ingrediente->q)
            {
                cucinabile = 1;
            }
            else
            {
                cucinabile = 0;
            }
        }

        if (cucinabile)
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

void scadenze(magazzino_hash_t *magazzino, unsigned int t)
{
    ingrediente_stock_t *ingrediente_temp;
    lotto_t *lotto_temp, *lotto_elim;
    int i;

    for (i = 0; i < DIM_MAGAZZINO; i++)
    {
        ingrediente_temp = magazzino->stock_ingredienti[i];
        while (ingrediente_temp != NULL)
        {
            if (ingrediente_temp->totale != 0)
            {
                lotto_temp = ingrediente_temp->scadente;
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

ordine_t *consegne(unsigned int pc, ordine_t *albero_prodotti)
{
    ordine_t *albero_consegne, *temp_consegna, *temp_prodotto, *next_prodotto;
    int cons, stop;
    unsigned long long residuo;

    residuo = pc;
    cons = 0;
    stop = 1;
    albero_consegne = NULL;
    next_prodotto = NULL;

    temp_prodotto = minimo_in_albero_ordini(albero_prodotti);

    while (temp_prodotto != NULL && stop)
    {
        if (temp_prodotto->peso <= residuo)
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
        else
        {
            stop = 0;
        }
    }
    temp_consegna = massimo_in_albero_ordini(albero_consegne);
    if (cons)
    {
        while (temp_consegna)
        {
            printf("%d %s %d\n", temp_consegna->t, temp_consegna->ricetta->nome, temp_consegna->q);
            temp_consegna->ricetta->pending--;
            albero_consegne = cancella_nodo_albero_ordini(albero_consegne, temp_consegna);
            free(temp_consegna);
            temp_consegna = NULL;
            temp_consegna = massimo_in_albero_ordini(albero_consegne);
        }
        elimina_albero_ordini(albero_consegne);
        temp_consegna = NULL;
        albero_consegne = NULL;
    }
    else
    {
        printf("camioncino vuoto\n");
    }
    return albero_prodotti;
}

int main()
{
    char input[L_INPUT_MAX], nome[L_NOME_MAX], ingrediente[L_NOME_MAX];
    int t, fc, pc, qt, peso, sc, primascadenza;
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
    primascadenza = 0;

    ricettario = crea_hash_ricettario();
    magazzino = crea_hash_magazzino();

    if (ricettario == NULL || magazzino == NULL)
    {
        printf("Errore creazione magazzino e/o ricettario.\n");
        return 0;
    }
    t = 0;
    if (scanf("%d %d ", &fc, &pc))
        ;

    while (scanf(" %s", input) != -1)
    {
        nome[0] = '\0';

        if (!strcmp(input, "aggiungi_ricetta"))
        {
            tempricetta = NULL;
            if (scanf(" %s", nome))
                ;

            tempricetta = ricerca_ricetta(ricettario, nome);

            if (tempricetta == NULL)
            {
                lista_ingredienti = NULL;
                peso = 0;
                nl = getchar();
                while (nl != '\n')
                {
                    if (scanf("%s %d", ingrediente, &qt))
                        ;
                    peso += qt;
                    tempstringa = malloc(sizeof(char) * (strlen(ingrediente) + 1));
                    strcpy(tempstringa, ingrediente);
                    lista_ingredienti = aggiungi_ingrediente(magazzino, lista_ingredienti, &tempstringa, qt);
                    nl = getchar();
                    tempstringa = NULL;
                }
                tempstringa = malloc(sizeof(char) * (strlen(nome) + 1));
                strcpy(tempstringa, nome);
                ret = aggiungi_ricetta(ricettario, &tempstringa, lista_ingredienti, peso);
                if (ret == -1)
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
            else
            {
                if (scanf(" %*[^\n]"))
                    ;
                printf("ignorato\n");
            }
        }
        else if (!strcmp(input, "rimuovi_ricetta"))
        {
            if (scanf(" %[^\n]", nome))
                ;
            ret = elimina_ricetta(ricettario, nome);
            if (ret == 1)
            {
                printf("rimossa\n");
            }
            else if (ret == -1)
            {
                printf("non presente\n");
            }
            else
            {
                printf("ordini in sospeso\n");
            }
        }
        else if (!strcmp(input, "rifornimento"))
        {
            nl = getchar();
            while (nl != '\n')
            {
                if (scanf("%s %d %d", nome, &qt, &sc))
                    ;
                tempstringa = malloc(sizeof(char) * (strlen(nome) + 1));
                strcpy(tempstringa, nome);

                if (sc > t)
                {
                    rif = 1;
                    ret = rifornimento(magazzino, &tempstringa, sc, qt);
                    if (primascadenza == 0)
                    {
                        primascadenza = sc;
                    }
                    else if (primascadenza > sc || primascadenza < t)
                    {
                        primascadenza = sc;
                    }
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
            coda_ordini = cucina(magazzino, coda_ordini, &last, &albero_prodotti, 0);
        }
        else if (!strcmp(input, "ordine"))
        {
            if (scanf(" %s %d", nome, &qt))
                ;
            coda_ordini = aggiungi_ordine(coda_ordini, &last, ricettario, nome, qt, t, &ret);
            if (ret == 1)
            {
                printf("accettato\n");
                if (rif)
                {
                    if (primascadenza <= t)
                    {
                        scadenze(magazzino, t);
                        primascadenza = 0;
                    }
                    coda_ordini = cucina(magazzino, coda_ordini, &last, &albero_prodotti, 1);
                }
            }
            else
            {
                printf("rifiutato\n");
            }
        }
        else
        {
            printf("Errore \n");
        }
        t++;

        if (t != 0 && t % fc == 0)
        {
            if (albero_prodotti == NULL)
            {
                printf("camioncino vuoto\n");
            }
            else
            {
                albero_prodotti = consegne(pc, albero_prodotti);
            }
        }
    }

    elimina_albero_ordini(coda_ordini);
    elimina_albero_ordini(albero_prodotti);
    elimina_hash_ricettario(ricettario);
    elimina_hash_magazzino(magazzino);
    return 0;
}