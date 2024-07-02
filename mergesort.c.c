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
 
scadenze_t* SortedMerge(scadenze_t* a, scadenze_t* b)
{
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