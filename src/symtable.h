// xnovacm00
// symtable.h - hlavickovy soubor pro tabulku symbolu

#ifndef SYMTABLE_H
#define SYMTABLE_H
 
#include <stdbool.h>
 
#include "ast.h"
 
 

// Tabulka vlastni vsechny symboly a uvolni je az funkci symtable_free
// Ukazatel na Symbol zustava platny az do symtable_free, i po opusteni rozsahu
// Opusteni rozsahu jen skryje jeho jmena, symboly se neuvolnuji
// Jmena a typy se kopiruji, volajici si svoje originaly uvolnuje sam
// Pri nedostatku pameti program konci pres error_exit s kodem ERR_INTERNAL
// Funkce symtable_free akceptuje NULL
 
 
//--------------------------------------------------------------------------------//
// Symbol
//--------------------------------------------------------------------------------//
 
// Druh symbolu
typedef enum {
    SYM_VAR,  // Promenna (val, var) nebo parametr funkce
    SYM_FUNC  // Funkce uzivatelska nebo vestavena
} SymbolKind;
 
// Symbol: vse, co prekladac vi o jednom jmene v programu
typedef struct Symbol Symbol;
 
struct Symbol {
    SymbolKind kind; // Urcuje, kterou polozku z as cist
    char *name;      // Jmeno, vlastni kopie
    union {
        struct {
            Type *type;      // Typ promenne, vlastni kopie
            bool is_mutable; // True pro var, false pro val a parametry
            int unique_id;   // Jedinecne cislo, aby se zastinene promenne nepletly
        } var;               // Pro SYM_VAR
        struct {
            FuncDef *def;        // Definice v AST (nevlastni), NULL pro vestavenou funkci
            int param_count;     // Pocet parametru
            Type **param_types;  // Typy parametru (vlastni kopie), NULL prvek znamena Any
            Type *return_type;   // Navratovy typ (vlastni kopie), NULL dokud neni znamy
            bool is_builtin;     // True pro vestavene funkce (print, readInt...)
            bool being_inferred; // True, kdyz se navratovy typ prave odvozuje (detekce rekurze)
        } func;                  // Pro SYM_FUNC
    } as;
};


//--------------------------------------------------------------------------------//
// Tabulka symbolu
//--------------------------------------------------------------------------------//


// Tabulka symbolu
typedef struct SymTable SymTable;


// Vytvori tabulku s jednim (globalnim) rozsahem
SymTable *symtable_new(void);

// Uvolni tabulku a vsechny symboly, ktere obsahuje
void symtable_free(SymTable *table);


// Rozsahy platnosti
 
// Otevre novy rozsah (vstup do bloku, funkce nebo tela cyklu)
void symtable_enter_scope(SymTable *table);
 
// Zavre nejvnitrnejsi rozsah, jeho jmena prestanou byt videt
void symtable_exit_scope(SymTable *table);


// Definice symbolu
 
// Definuje promennou v aktualnim rozsahu, typ se zkopiruje
// Vrati NULL, kdyz jmeno v aktualnim rozsahu uz existuje (chyba 4)
// Promenne patri jen do lokalnich rozsahu, nikdy do globalniho
Symbol *symtable_define_var(SymTable *table, const char *name, const Type *type, bool is_mutable);


// Definuje uzivatelskou funkci v globalnim rozsahu (bez ohledu na aktualni rozsah)
// Parametry a navratovy typ se opisou z definice, NULL return_type znamena neznamy
// Vrati NULL, kdyz funkce stejneho jmena uz existuje (chyba 4)
// Volat pro vsechny funkce driv, nez se zacnou analyzovat jejich tela
Symbol *symtable_define_func(SymTable *table, const char *name, FuncDef *def);


// Definuje vestavenou funkci v globalnim rozsahu, typy se zkopiruji
// NULL prvek v param_types znamena parametr typu Any
Symbol *symtable_define_builtin(SymTable *table, const char *name, int param_count, const Type *const *param_types, const Type *return_type);


// Zaregistruje vsechny vestavene funkce (print, readLine, readInt, readDouble,
// readBoolean, toStr, toInt, strLength, subStr, ord, chr)
void symtable_register_builtins(SymTable *table);

// Vyhledavani
 
// Najde symbol podle jmena od nejvnitrnejsiho rozsahu ven, NULL kdyz neexistuje (chyba 3)
Symbol *symtable_lookup(const SymTable *table, const char *name);


// Najde symbol jen v aktualnim rozsahu, NULL kdyz tam neexistuje
Symbol *symtable_lookup_in_current_scope(const SymTable *table, const char *name);


// Uprava symbolu
 
// Nastavi navratovy typ funkce (zkopiruje ho, puvodni typ uvolni)
void symbol_set_return_type(Symbol *symbol, const Type *type);

#endif
