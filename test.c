#include <stdio.h>
#include <string.h>
#define L_NOME_MAX 256
int main () {
    char input[L_NOME_MAX], nome[L_NOME_MAX], ingrediente[L_NOME_MAX];
    int t, fc, pc, qt;
    char nl;
    //ricettario_hash_t * ricettario;
    //ordine_t* coda_ordini, coda_prodotti;
    //magazzino_hash_t * magazzino;
    //ingrediente_t* lista_ingredienti;

    //ricettario = crea_hash_ricettario();
    //magazzino = crea_hash_magazzino();

    /*if (ricettario == NULL || magazzino == NULL) {
        printf("Errore creazione magazzino e/o ricettario.\n");
        return 0;
    }
    */
    t=0;
    if(scanf("%d %d ", &fc, &pc) )

    while(scanf("%s", input) != -1) {
/*
        if (t != 0) { // eseguito ogni volta
            scadenze(t, magazzino);
        };

        if (t!=0 && t % fc == 0) { // eseguito a ogni frequenza di passaggio del corriere
            consegne(t, coda_prodotti);
        }
*/
        if (!strcmp(input, "aggiungi_ricetta")) {
            printf ("1\n");
            if (scanf (" %s", nome));
            //lista_ingredienti=NULL;
            printf ("%s\n", nome);
            //peso = 0;
            if (scanf("%c", &nl));
            while (nl != '\n') {
                if (scanf("%s %d", ingrediente, &qt));
                printf ("%s\n%d\n", ingrediente, qt);
                //peso += qt;
                //lista_ingredienti = aggiungi_ingrediente(lista_ingredienti, ingrediente, qt);
                if (scanf("%c", &nl));
            }
            //aggiungi_ricetta(ricettario, nome, lista_ingredienti, peso);

        } else if (!strcmp(input, "rimuovi_ricetta")) {
            if (scanf (" %[^\n]", nome));
            printf("2\n%s\n", nome);
        } else if (!strcmp(input, "rifornimento")) {

        } else if (!strcmp(input, "ordine")) {
            
        } else {
            printf ("Errore \n");
        }
        t++;
    }

    //elimina_hash_ricettario(ricettario);
    //elimina_hash_magazzino(magazzino);
    return 0;    
}