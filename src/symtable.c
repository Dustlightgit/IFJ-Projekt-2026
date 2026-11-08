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
