/*
    IFJ projekt 2026

    xnovacm00 Michal Novacek

    symtable.h

    Tento soubor obsahuje deklarace datovych typu a funkci pro tabulku symbolu.
    Tabulka vlastni vsechny symboly a uvolni je az funkci symtable_free.
    Ukazatel na Symbol zustava platny az do symtable_free, i po opusteni
    rozsahu. Opusteni rozsahu jen skryje jeho jmena, symboly se neuvolnuji.
    Jmena a typy se kopiruji, volajici si svoje originaly uvolnuje sam.
    Pri nedostatku pameti program konci pres error_exit s kodem ERR_INTERNAL.
    Funkce symtable_free akceptuje NULL.
*/

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>

#include "ast.h"

//--------------------------------------------------------------------------------//
// Symbol
//--------------------------------------------------------------------------------//

/**
 * @brief Druh symbolu.
 */
typedef enum {
  SYM_VAR,  ///< Promenna (val, var) nebo parametr funkce.
  SYM_FUNC  ///< Funkce uzivatelska nebo vestavena.
} SymbolKind;

/**
 * @brief Symbol: vse, co prekladac vi o jednom jmene v programu.
 */
typedef struct Symbol Symbol;

/**
 * @brief Definice struktury Symbol.
 *
 * Polozka kind urcuje, ktera varianta unie as je platna.
 */
struct Symbol {
  SymbolKind kind; ///< Urcuje, kterou polozku z as cist.
  char *name;      ///< Jmeno, vlastni kopie.
  union {
    /** Pro SYM_VAR. */
    struct {
      Type *type;      ///< Typ promenne, vlastni kopie.
      bool is_mutable; ///< True pro var, false pro val a parametry.
      int unique_id;   ///< Jedinecne cislo, aby se zastinene promenne nepletly.
    } var;
    /** Pro SYM_FUNC. */
    struct {
      FuncDef *def;        ///< Definice v AST (nevlastni), NULL pro vestavenou funkci.
      int param_count;     ///< Pocet parametru.
      Type **param_types;  ///< Typy parametru (vlastni kopie), NULL prvek znamena Any.
      Type *return_type;   ///< Navratovy typ (vlastni kopie), NULL dokud neni znamy.
      bool is_builtin;     ///< True pro vestavene funkce (print, readInt...).
      bool being_inferred; ///< True, kdyz se navratovy typ prave odvozuje (detekce rekurze).
    } func;
  } as;
};

//--------------------------------------------------------------------------------//
// Tabulka symbolu
//--------------------------------------------------------------------------------//

/**
 * @brief Tabulka symbolu.
 */
typedef struct SymTable SymTable;

/**
 * @brief Vytvori tabulku s jednim (globalnim) rozsahem.
 *
 * @return Ukazatel na novou tabulku.
 */
SymTable *symtable_new(void);

/**
 * @brief Uvolni tabulku a vsechny symboly, ktere obsahuje.
 *
 * Akceptuje NULL.
 *
 * @param table Tabulka k uvolneni.
 */
void symtable_free(SymTable *table);

//--------------------------------------------------------------------------------//
// Rozsahy platnosti
//--------------------------------------------------------------------------------//

/**
 * @brief Otevre novy rozsah (vstup do bloku, funkce nebo tela cyklu).
 *
 * @param table Tabulka symbolu.
 */
void symtable_enter_scope(SymTable *table);

/**
 * @brief Zavre nejvnitrnejsi rozsah, jeho jmena prestanou byt videt.
 *
 * Symboly se neuvolnuji, ukazatele na ne zustavaji platne.
 *
 * @param table Tabulka symbolu.
 */
void symtable_exit_scope(SymTable *table);

//--------------------------------------------------------------------------------//
// Definice symbolu
//--------------------------------------------------------------------------------//

/**
 * @brief Definuje promennou v aktualnim rozsahu.
 *
 * Promenne patri jen do lokalnich rozsahu, nikdy do globalniho.
 * Jmeno i typ se zkopiruji.
 *
 * @param table      Tabulka symbolu.
 * @param name       Jmeno promenne.
 * @param type       Typ promenne (zkopiruje se).
 * @param is_mutable True pro var, false pro val a parametry.
 * @return Novy symbol, nebo NULL, kdyz jmeno v aktualnim rozsahu uz existuje
 *         (chyba 4).
 */
Symbol *symtable_define_var(SymTable *table, const char *name, const Type *type,
                            bool is_mutable);

/**
 * @brief Definuje uzivatelskou funkci v globalnim rozsahu.
 *
 * Funkce se definuje do globalniho rozsahu bez ohledu na aktualni rozsah.
 * Parametry a navratovy typ se opisou z definice, NULL return_type znamena
 * neznamy. Volat pro vsechny funkce driv, nez se zacnou analyzovat jejich tela.
 *
 * @param table Tabulka symbolu.
 * @param name  Jmeno funkce.
 * @param def   Definice funkce v AST (nevlastni).
 * @return Novy symbol, nebo NULL, kdyz funkce stejneho jmena uz existuje
 *         (chyba 4).
 */
Symbol *symtable_define_func(SymTable *table, const char *name, FuncDef *def);

/**
 * @brief Definuje vestavenou funkci v globalnim rozsahu.
 *
 * Typy se zkopiruji. NULL prvek v param_types znamena parametr typu Any.
 *
 * @param table       Tabulka symbolu.
 * @param name        Jmeno funkce.
 * @param param_count Pocet parametru.
 * @param param_types Pole typu parametru (zkopiruje se).
 * @param return_type Navratovy typ (zkopiruje se).
 * @return Novy symbol, nebo NULL, kdyz funkce stejneho jmena uz existuje.
 */
Symbol *symtable_define_builtin(SymTable *table, const char *name,
                                int param_count, const Type *const *param_types,
                                const Type *return_type);

/**
 * @brief Zaregistruje vsechny vestavene funkce.
 *
 * Registruje: print, readLine, readInt, readDouble, readBoolean, toStr,
 * toInt, strLength, subStr, ord, chr.
 *
 * @param table Tabulka symbolu.
 */
void symtable_register_builtins(SymTable *table);

//--------------------------------------------------------------------------------//
// Vyhledavani
//--------------------------------------------------------------------------------//

/**
 * @brief Najde symbol podle jmena od nejvnitrnejsiho rozsahu ven.
 *
 * @param table Tabulka symbolu.
 * @param name  Hledane jmeno.
 * @return Nalezeny symbol, nebo NULL, kdyz neexistuje (chyba 3).
 */
Symbol *symtable_lookup(const SymTable *table, const char *name);

/**
 * @brief Najde symbol jen v aktualnim rozsahu.
 *
 * @param table Tabulka symbolu.
 * @param name  Hledane jmeno.
 * @return Nalezeny symbol, nebo NULL, kdyz tam neexistuje.
 */
Symbol *symtable_lookup_in_current_scope(const SymTable *table,
                                         const char *name);

//--------------------------------------------------------------------------------//
// Uprava symbolu
//--------------------------------------------------------------------------------//

/**
 * @brief Nastavi navratovy typ funkce.
 *
 * Novy typ se zkopiruje, puvodni typ se uvolni.
 *
 * @param symbol Symbol funkce.
 * @param type   Novy navratovy typ.
 */
void symbol_set_return_type(Symbol *symbol, const Type *type);

#endif