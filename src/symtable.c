// xnovacm00
// symtable.c - tabulka symbolu implementovana jako AVL strom

#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "symtable.h"

#define GLOBAL_SCOPE 0

//--------------------------------------------------------------------------------//
// Pomocne funkce pro praci s pameti (pri chybe konci pres error_exit)
//--------------------------------------------------------------------------------//

static void *xmalloc(size_t size) {
  void *p = malloc(size);
  if (!p)
    error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
  return p;
}

static void *xcalloc(size_t n, size_t size) {
  void *p = calloc(n, size);
  if (!p)
    error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
  return p;
}

static void *xrealloc(void *ptr, size_t size) {
  void *p = realloc(ptr, size);
  if (!p)
    error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
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
  Symbol *symbol; // Vlastni symbol, jmeno je symbol->name
  int scope_id;   // Cast klice: rozsah, ve kterem symbol zije
  struct AvlNode *left;
  struct AvlNode *right;
  int height; // Vyska podstromu, list ma vysku 1
} AvlNode;

// Porovna klic (name, scope) s uzlem
// Vraci <0, kdyz je klic mensi nez uzel, 0 pri shode, >0, kdyz je vetsi
static int key_cmp(const char *name, int scope, const AvlNode *node) {
  int cmp = strcmp(name, node->symbol->name);
  if (cmp != 0)
    return cmp; // 1. rozhoduje jmeno
  return (scope > node->scope_id) -
         (scope < node->scope_id); // 2. rozhoduje cislo rozsahu
}

// Vyska podstromu, prazdny podstrom (NULL) ma vysku 0
static int height(const AvlNode *node) { return node ? node->height : 0; }

static int max_int(int a, int b) { return a > b ? a : b; }

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

  root->left =
      new_root
          ->right; // pravy podstrom noveho korene se presune pod puvodni koren
  new_root->right = root; // puvodni koren se stane pravym potomkem

  // Poradi je dulezite: nejdriv nizsi uzel (root), pak vyssi (new_root)
  update_height(root);
  update_height(new_root);

  return new_root;
}

// Leva rotace kolem uzlu root, jeho pravy potomek se stane novym korenem podstromu
// Vrati novy koren podstromu
static AvlNode *rotate_left(AvlNode *root) {
  AvlNode *new_root = root->right;

  root->right =
      new_root
          ->left; // levy podstrom noveho korene se presune pod puvodni koren
  new_root->left = root; // puvodni koren se stane levym potomkem

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

//--------------------------------------------------------------------------------//
// Uvolnovani symbolu a stromu
//--------------------------------------------------------------------------------//

// Uvolni symbol a vsechno, co vlastni (jmeno, typy)
// Definice funkce v AST (def) je nevlastni, ta se neuvolnuje
static void free_symbol(Symbol *symbol) {
  if (symbol == NULL) {
    return;
  }

  free(symbol->name);

  if (symbol->kind == SYM_VAR) {
    type_free(symbol->as.var.type);
  } else {
    // param_types[index] muze byt NULL (parametr typu Any), type_free to snese
    for (int index = 0; index < symbol->as.func.param_count; index++) {
      type_free(symbol->as.func.param_types[index]);
    }
    free(symbol->as.func.param_types);
    type_free(symbol->as.func.return_type);
  }

  free(symbol);
}

// Uvolni cely podstrom vcetne symbolu v uzlech (postorder: nejdriv potomci, pak uzel)
static void avl_free(AvlNode *node) {
  if (node == NULL) {
    return;
  }

  avl_free(node->left);
  avl_free(node->right);
  free_symbol(node->symbol);
  free(node);
}

//--------------------------------------------------------------------------------//
// Tabulka symbolu
//--------------------------------------------------------------------------------//

#define INITIAL_SCOPE_CAPACITY 8

struct SymTable {
  AvlNode *root; // Koren jedineho stromu, ve kterem jsou symboly vsech rozsahu
  int *scope_stack; // Zasobnik cisel prave otevrenych rozsahu, [0] je globalni
  int scope_depth;  // Kolik rozsahu je na zasobniku otevrenych
  int scope_capacity; // Kolik rozsahu se vejde do alokovaneho zasobniku
  int next_scope_id; // Cislo, ktere dostane pristi otevreny rozsah (nikdy se nevraci)
  int next_var_id; // Posledni prideleny unique_id promenne
};

// Vytvori novou tabulku symbolu a inicializuje zasobnik rozsahu pro globalni kontext
SymTable *symtable_new(void) {
  SymTable *table = xcalloc(1, sizeof *table);

  table->scope_capacity = INITIAL_SCOPE_CAPACITY;
  table->scope_stack =
      xmalloc((size_t)table->scope_capacity * sizeof *table->scope_stack);

  // Na zacatku jsme v globalnim rozsahu
  table->scope_stack[0] = GLOBAL_SCOPE;
  table->scope_depth = 1;
  table->next_scope_id = GLOBAL_SCOPE + 1;
  table->next_var_id = 0;

  return table;
}

// Uvolni tabulku symbolu z pameti vcetne stromu a zasobniku rozsahu
void symtable_free(SymTable *table) {
  if (table == NULL) {
    return;
  }

  avl_free(table->root);
  free(table->scope_stack);
  free(table);
}

// Vstoupi do noveho rozsahu, v pripade potreby zvetsi kapacitu zasobniku a priradi nove ID
void symtable_enter_scope(SymTable *table) {
  // Kdyz je zasobnik plny, zvetsi se na dvojnasobek
  if (table->scope_depth == table->scope_capacity) {
    table->scope_capacity *= 2;
    table->scope_stack =
        xrealloc(table->scope_stack,
                 (size_t)table->scope_capacity * sizeof *table->scope_stack);
  }

  // Novy rozsah dostane vzdy nove cislo, aby se nepletl s drivejsimi sourozeneckymi bloky
  table->scope_stack[table->scope_depth] = table->next_scope_id;
  table->scope_depth++;
  table->next_scope_id++;
}

// Opusti aktualni rozsah snizenim hloubky zasobniku, globalni rozsah nezavira
void symtable_exit_scope(SymTable *table) {
  // Globalni rozsah se nikdy nezavira
  if (table->scope_depth > 1) {
    table->scope_depth--;
  }
  // Strom se nemeni, symboly zustavaji az do symtable_free
}

//--------------------------------------------------------------------------------//
// Vyhledavani
//--------------------------------------------------------------------------------//

// Hleda symbol v aktualnim rozsahu na vrcholu zasobniku
Symbol *symtable_lookup_in_current_scope(const SymTable *table,
                                         const char *name) {
  int current_scope = table->scope_stack[table->scope_depth - 1];

  AvlNode *node = avl_find(table->root, name, current_scope);
  return node != NULL ? node->symbol : NULL;
}

// Hleda symbol od nejvnitrnejsiho rozsahu smerem ven vcetne podpory zastineni
Symbol *symtable_lookup(const SymTable *table, const char *name) {
  // Od nejvnitrnejsiho rozsahu smerem ven, prvni nalez vyhrava (tim funguje zastineni)
  for (int depth = table->scope_depth - 1; depth >= 0; depth--) {
    AvlNode *node = avl_find(table->root, name, table->scope_stack[depth]);
    if (node != NULL) {
      return node->symbol;
    }
  }

  return NULL;
}

//--------------------------------------------------------------------------------//
// Definice symbolu
//--------------------------------------------------------------------------------//

// Zabali symbol do uzlu a vlozi ho do stromu pod klicem (jmeno symbolu, scope)
// Tabulka tim symbol prebira do vlastnictvi, vrati ho zpet pro pohodli volajiciho
static Symbol *insert_symbol(SymTable *table, Symbol *symbol, int scope) {
  AvlNode *node = xmalloc(sizeof *node);
  node->symbol = symbol;
  node->scope_id = scope;
  node->left = NULL;
  node->right = NULL;
  node->height = 1;

  table->root = avl_insert(table->root, node);
  return symbol;
}

// Definuje novou promennou v aktualnim rozsahu, overi existenci a priradi jedinecne ID
Symbol *symtable_define_var(SymTable *table, const char *name, const Type *type,
                            bool is_mutable) {
  int current_scope = table->scope_stack[table->scope_depth - 1];

  // Jmeno uz v aktualnim rozsahu existuje -> redefinice (chyba 4 se hlasi u volajiciho)
  if (avl_find(table->root, name, current_scope) != NULL) {
    return NULL;
  }

  Symbol *symbol = xcalloc(1, sizeof *symbol);
  symbol->kind = SYM_VAR;
  symbol->name = xstrdup(name);
  symbol->as.var.type = (type != NULL) ? type_clone(type) : NULL;
  symbol->as.var.is_mutable = is_mutable;

  table->next_var_id++;
  symbol->as.var.unique_id = table->next_var_id;

  return insert_symbol(table, symbol, current_scope);
}

// Definuje novou funkci v globalnim rozsahu, overi existenci a zkopiruje typy
Symbol *symtable_define_func(SymTable *table, const char *name, FuncDef *def) {
  // Funkce jsou vzdy v globalnim rozsahu, bez ohledu na to, kde prave jsme
  // nelze definovat vnorenou funkci uvnitr jine funkce
  if (avl_find(table->root, name, GLOBAL_SCOPE) != NULL) {
    return NULL;
  }

  Symbol *symbol = xcalloc(1, sizeof *symbol);
  symbol->kind = SYM_FUNC;
  symbol->name = xstrdup(name);

  symbol->as.func.def = def; // Nevlastni, patri AST
  symbol->as.func.param_count = def->param_count;
  symbol->as.func.is_builtin = false; // Jde o uzivatelskou funkci, proto false
  symbol->as.func.being_inferred = false;

  // Typy parametru se opisou z definice (kazdy symbol ma svoje vlastni kopie)
  if (def->param_count > 0) {
    symbol->as.func.param_types =
        xmalloc((size_t)def->param_count * sizeof *symbol->as.func.param_types);
    for (int index = 0; index < def->param_count; index++) {
      symbol->as.func.param_types[index] = type_clone(def->params[index].type);
    }
  } else {
    symbol->as.func.param_types = NULL;
  }

  // Navratovy typ muze chybet (NULL = zatim neznamy, odvodi se az pozdeji)
  symbol->as.func.return_type =
      (def->return_type != NULL) ? type_clone(def->return_type) : NULL;

  return insert_symbol(table, symbol, GLOBAL_SCOPE);
}

// Definuje vestavenou funkci v globalnim rozsahu, overi existenci a zkopiruje typy
Symbol *symtable_define_builtin(SymTable *table, const char *name,
                                int param_count, const Type *const *param_types,
                                const Type *return_type) {
  // Vestavene funkce jsou take globalni
  if (avl_find(table->root, name, GLOBAL_SCOPE) != NULL) {
    return NULL;
  }

  Symbol *symbol = xcalloc(1, sizeof *symbol);
  symbol->kind = SYM_FUNC;
  symbol->name = xstrdup(name);

  symbol->as.func.def = NULL; // Vestavena funkce nema definici v AST
  symbol->as.func.param_count = param_count;
  symbol->as.func.is_builtin = true;
  symbol->as.func.being_inferred = false;

  // NULL prvek v param_types znamena parametr typu Any, ten zustane NULL
  if (param_count > 0) {
    symbol->as.func.param_types =
        xmalloc((size_t)param_count * sizeof *symbol->as.func.param_types);
    for (int index = 0; index < param_count; index++) {
      symbol->as.func.param_types[index] =
          (param_types[index] != NULL) ? type_clone(param_types[index]) : NULL;
    }
  } else {
    symbol->as.func.param_types = NULL;
  }

  symbol->as.func.return_type =
      (return_type != NULL) ? type_clone(return_type) : NULL;

  return insert_symbol(table, symbol, GLOBAL_SCOPE);
}

//--------------------------------------------------------------------------------//
// Uprava symbolu
//--------------------------------------------------------------------------------//

// Nastavi nebo zmeni navratovy typ funkce
void symbol_set_return_type(Symbol *symbol, const Type *type) {
  if (symbol->kind != SYM_FUNC) {
    return;
  }

  // Nejdriv vytvorit novou kopii a az pak uvolnit stary typ
  Type *new_type = (type != NULL) ? type_clone(type) : NULL;
  type_free(symbol->as.func.return_type);
  symbol->as.func.return_type = new_type;
}

//--------------------------------------------------------------------------------//
// Vestavene funkce
//--------------------------------------------------------------------------------//

void symtable_register_builtins(SymTable *table) {
  // Typy pouzite v signaturach
  // symtable_define_builtin si typy zkopiruje, proto staci obycejne lokalni promenne
  const Type type_int = {.kind = TYPE_INT, .element = NULL};
  const Type type_double = {.kind = TYPE_DOUBLE, .element = NULL};
  const Type type_string = {.kind = TYPE_STRING, .element = NULL};
  const Type type_unit = {.kind = TYPE_UNIT, .element = NULL};
  const Type type_boolean = {.kind = TYPE_BOOLEAN, .element = NULL};

  // Pole typu parametru, NULL prvek znamena Any
  const Type *const params_any[] = {NULL};
  const Type *const params_double[] = {&type_double};
  const Type *const params_string[] = {&type_string};
  const Type *const params_string_int[] = {&type_string, &type_int};
  const Type *const params_string_int_int[] = {&type_string, &type_int,
                                               &type_int};
  const Type *const params_int[] = {&type_int};

  // print(value: Any): Unit
  symtable_define_builtin(table, "print", 1, params_any, &type_unit);

  // Funkce pro cteni ze vstupu, bez parametru
  symtable_define_builtin(table, "readLine", 0, NULL, &type_string);
  symtable_define_builtin(table, "readInt", 0, NULL, &type_int);
  symtable_define_builtin(table, "readDouble", 0, NULL, &type_double);
  symtable_define_builtin(table, "readBoolean", 0, NULL,
                          &type_boolean); // rozsireni BOOLTHEN

  // toStr(value: Any): String
  symtable_define_builtin(table, "toStr", 1, params_any, &type_string);

  // toInt(value: Double): Int
  symtable_define_builtin(table, "toInt", 1, params_double, &type_int);

  // strLength(value: String): Int
  symtable_define_builtin(table, "strLength", 1, params_string, &type_int);

  // subStr(value: String, start: Int, end: Int): String
  symtable_define_builtin(table, "subStr", 3, params_string_int_int,
                          &type_string);

  // ord(value: String, index: Int): Int
  symtable_define_builtin(table, "ord", 2, params_string_int, &type_int);

  // chr(code: Int): String
  symtable_define_builtin(table, "chr", 1, params_int, &type_string);
}
