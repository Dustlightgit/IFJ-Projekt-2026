// xnovacm00
// symtable.c - tabulka symbolu implementovana jako AVL strom

#include <stdlib.h>
#include <string.h>
 
#include "symtable.h"
#include "error.h"

#define GLOBAL_SCOPE 0

//--------------------------------------------------------------------------------//
// Pomocne funkce pro praci s pameti (pri chybe konci pres error_exit)
//--------------------------------------------------------------------------------//
 
static void *xmalloc(size_t size) {
    void *p = malloc(size);
    if (!p) error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
    return p;
}
 
static void *xcalloc(size_t n, size_t size) {
    void *p = calloc(n, size);
    if (!p) error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
    return p;
}
 
static void *xrealloc(void *ptr, size_t size) {
    void *p = realloc(ptr, size);
    if (!p) error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
    return p;
}
 
static char *xstrdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = xmalloc(len);
    memcpy(copy, s, len);
    return copy;
}


//--------------------------------------------------------------------------------//
// AVL strom s dvojitym klicem (jmeno, scope_id)
//--------------------------------------------------------------------------------//
 
typedef struct AvlNode {
    Symbol *symbol;        // Vlastni symbol, jmeno je symbol->name
    int scope_id;          // Cast klice: rozsah, ve kterem symbol zije
    struct AvlNode *left;
    struct AvlNode *right;
    int height;            // Vyska podstromu, list ma vysku 1
} AvlNode;
 
// Porovna klic (name, scope) s uzlem
// Vraci <0, kdyz je klic mensi nez uzel, 0 pri shode, >0, kdyz je vetsi
static int key_cmp(const char *name, int scope, const AvlNode *node) {
    int cmp = strcmp(name, node->symbol->name);
    if (cmp != 0) return cmp;                                  // 1. rozhoduje jmeno
    return (scope > node->scope_id) - (scope < node->scope_id);  // 2. rozhoduje cislo rozsahu
}


// Vyska podstromu, prazdny podstrom (NULL) ma vysku 0
static int height(const AvlNode *node) {
    return node ? node->height : 0;
}
 
static int max_int(int a, int b) {
    return a > b ? a : b;
}


// Prepocita vysku uzlu z vysek jeho potomku
static void update_height(AvlNode *node) {
    node->height = 1 + max_int(height(node->left), height(node->right));
}


// Faktor vyvazenosti: vyska leveho minus vyska praveho podstromu
// V AVL strome musi byt u kazdeho uzlu v rozsahu -1, 0, 1
static int balance_factor(const AvlNode *node) {
    return height(node->left) - height(node->right);
}


//--------------------------------------------------------------------------------//
// Rotace
//--------------------------------------------------------------------------------//


// Prava rotace kolem uzlu root, jeho levy potomek se stane novym korenem podstromu
// Vrati novy koren podstromu
static AvlNode *rotate_right(AvlNode *root) {
    AvlNode *new_root = root->left;
 
    root->left = new_root->right;  // pravy podstrom noveho korene se presune pod puvodni koren
    new_root->right = root;        // puvodni koren se stane pravym potomkem
 
    // Poradi je dulezite: nejdriv nizsi uzel (root), pak vyssi (new_root)
    update_height(root);
    update_height(new_root);
 
    return new_root;
}


// Leva rotace kolem uzlu root, jeho pravy potomek se stane novym korenem podstromu
// Vrati novy koren podstromu
static AvlNode *rotate_left(AvlNode *root) {
    AvlNode *new_root = root->right;
 
    root->right = new_root->left;  // levy podstrom noveho korene se presune pod puvodni koren
    new_root->left = root;         // puvodni koren se stane levym potomkem
 
    update_height(root);
    update_height(new_root);
 
    return new_root;
}


// Prepocita vysku uzlu a pripadne strom vyvazi rotacemi
// Vrati novy koren podstromu (stejny uzel, nebo ten, ktery se po rotaci dostal nahoru)
static AvlNode *rebalance(AvlNode *node) {
    update_height(node);
    int balance = balance_factor(node);
 
    // Levy podstrom je prilis vysoky
    if (balance > 1) {
        // Pripad LR: levy potomek je nakloneny doprava, nejdriv ho otocime doleva
        if (balance_factor(node->left) < 0) {
            node->left = rotate_left(node->left);
        }
        // Pak jeste jednou otocime
        return rotate_right(node);
    }
 
    // Pravy podstrom je prilis vysoky
    if (balance < -1) {
        // Pripad RL: pravy potomek je nakloneny doleva, nejdriv ho otocime doprava
        if (balance_factor(node->right) > 0) {
            node->right = rotate_right(node->right);
        }
        // Pak jeste jednou otocime
        return rotate_left(node);
    }
 
    // Uzel je vyvazeny, neni co delat
    return node;
}


// Vlozi hotovy uzel new_node do podstromu s korenem root
// Vrati novy koren podstromu, volajici ho musi ulozit zpet
// Volajici zarucuje, ze stejny klic (jmeno, rozsah) uz ve strome neni
static AvlNode *avl_insert(AvlNode *root, AvlNode *new_node) {
    // Prazdne misto, tady novy uzel zapadne
    if (root == NULL) {
        return new_node;
    }
 
    int cmp = key_cmp(new_node->symbol->name, new_node->scope_id, root);
    if (cmp < 0) {
        root->left = avl_insert(root->left, new_node);
    } else {
        root->right = avl_insert(root->right, new_node);
    }
 
    // Cestou zpet nahoru se na kazdem uzlu opravi vyska a pripadne vyvazeni
    return rebalance(root);
}


// Najde uzel s klicem (name, scope), NULL kdyz ve strome neni
static AvlNode *avl_find(AvlNode *root, const char *name, int scope) {
    AvlNode *current = root;
 
    while (current != NULL) {
        int cmp = key_cmp(name, scope, current);
        if (cmp == 0) {
            return current;
        }
        current = (cmp < 0) ? current->left : current->right;
    }
 
    return NULL;
}

