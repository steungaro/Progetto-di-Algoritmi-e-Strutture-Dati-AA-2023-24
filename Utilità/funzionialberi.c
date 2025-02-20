ordine_t *minimo_in_albero (ordine_t* albero) {
    ordine_t *temp;

    temp = albero;

    if (albero)  {
        while (temp->left != NULL) {
            temp = temp->left;
        }
    }
    return temp;
}

ordine_t *massimo_in_albero (ordine_t* albero) {
    ordine_t *temp;

    temp = albero;

    if (albero)  {
        while (temp->right != NULL) {
            temp = temp->right;
        }
    }
    return temp;
}

ordine_t *successore_albero (ordine_t* nodo) {
    ordine_t *y, *x;

    x = nodo;
    if (nodo->right != NULL) {
        return minimo_in_albero(nodo->right);
    } else {
        y = nodo->p;
        while (y != NULL && x != y->right) {
            x = y;
            y = y->p;
        }
        return y;
    }
}

void left_rotate (ordine_t *albero, ordine_t *x) {
    ordine_t *y;

    if (albero && x) {
        y = x->right;
        x->right = y->left;

        if (y->left != NULL) {
            y->left->p = x;
        }

        y->p = x->p;
        if (x->p == NULL) {
            albero = y;
        } else if (x == x->p->left) {
            x->p->left = y;
        } else {
            x->p->right = y;
        }
        y->left = x;
        x->p = y;
    }
}

void right_rotate (ordine_t *albero, ordine_t *y) {
    ordine_t *x;

    if (albero && y) {
        x = y->left;
        y->left = x->right;

        if (x->right != NULL) {
            x->right->p = y;
        }

        x->p = y->p;
        if (y->p == NULL) {
            albero = x;
        } else if (y == y->p->right) {
            y->p->right = x;
        } else {
            y->p->left = x;
        }
        x->right = y;
        y->p = x;
    }
}

void rb_insert_fixup(ordine_t* albero, ordine_t* x) {
    int c;
    ordine_t* z, *y, *w;
    z = NULL;
    y = NULL;
 
    while (x != albero && x->colore != BLACK && x->p->colore == RED) {
        z = x->p;
        y = x->p->p;
        if (y && z == y->left) {
 
            w = y->right;
            if (w != NULL && w->colore == RED) {
                y->colore = RED;
                x->colore = BLACK;
                w->colore = BLACK;
                x = y;
            } else {
                if (x == z->right) {
                    left_rotate(albero, z);
                    x = z;
                    z = x->p;
                } 
                right_rotate(albero, y);
                c = z->colore;
                z->colore = y->colore;
                y->colore = c;
                x = z;
            }
        } else if (y) {
            w = y->left;
            if (w != NULL && w->colore == RED) 
            {
                y->colore = RED;
                z->colore = BLACK;
                w->colore = BLACK;
                x = y;
            } else {
                if (x == z->left) {
                    right_rotate(albero, z);
                    x = z;
                    z = x->p;
                }
                left_rotate(albero, y);
                c = z->colore;
                z->colore = y->colore;
                y->colore = c;
                x = z;
            }
        }
    }
    albero->colore = BLACK;
}

ordine_t *inserimento_albero_ordini (ordine_t *albero, ordine_t *z) {
    ordine_t *x, *y;

    y = NULL;
    x = albero;


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
    z->left = NULL;
    z->right = NULL;
    z->colore = RED;
    rb_insert_fixup (albero, z);
    return albero;
}

void rb_delete_fixup(ordine_t *albero, ordine_t *x) {
    ordine_t *w;

    while (x && x != albero && x->colore == BLACK) {
        if (x->p && x == x->p->left) {
            w = x->p->right;
            if (w && w->colore == RED) {
                w->colore = BLACK;
                x->p->colore = RED;
                left_rotate(albero, x->p);
                w = x->p->right;
            }
            if (w->left && w->right && w->left->colore == BLACK && w->right->colore == BLACK) {
                w->colore = RED;
                x = x->p;
            } else if (w->left && w->right) {
                if (w->right->colore == BLACK) {
                    w->left->colore = BLACK;
                    w->colore = RED;
                    right_rotate(albero, w);
                    w = x->p->right;
                }
                w->colore = x->p->colore;
                x->p->colore = BLACK;
                w->right->colore = BLACK;
                left_rotate(albero, x->p);
                x = albero;
            }
        } else if (x->p) {
            w = x->p->left;
            if (w->colore == RED) {
                w->colore = BLACK;
                x->p->colore = RED;
                right_rotate(albero, x->p);
                w = x->p->left;
            }
            if (w->left && w->right && w->right->colore == BLACK && w->left->colore == BLACK) {
                w->colore = RED;
                x = x->p;
            } else if (w->left && w->right) {
                if (w->left->colore == BLACK) {
                    w->right->colore = BLACK;
                    w->colore = RED;
                    left_rotate(albero, w);
                    w = x->p->left;
                }
                w->colore = x->p->colore;
                x->p->colore = BLACK;
                w->left->colore = BLACK;
                right_rotate(albero, x->p);
                x = albero;
            }            
        }
    }
    x->colore = BLACK;
}

ordine_t *cancella_nodo_albero(ordine_t *albero, ordine_t *z) {
    ordine_t *x, *y;

    if (z != NULL && (z->left == NULL || z->right == NULL)) {
        y = z;
    } else {
        y = successore_albero(z);
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
    if (x && y != NULL && y->colore == BLACK) {
        rb_delete_fixup(albero, x);
    }
    return albero;
}

ordine_t *inserimento_albero_consegne (ordine_t *albero, ordine_t *z) {
    ordine_t *x, *y;

    y = NULL;
    x = albero;

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
    } else if (z->peso < y->peso) {
        y->left = z;
    } else {
        y->right = z;
    }
    z->left = NULL;
    z->right = NULL;
    z->colore = RED;
    rb_insert_fixup (albero, z);
    return albero;
}
