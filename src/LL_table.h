/*
    IFJ projekt 2026

    LL_table.h

    Puvodni autor: xbalaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - include semantic_actions.h nahrazen semantic_types.h.

    Tento soubor obsahuje deklarace pro LL tabulku, kterou bude vyuzivat
    prediktivni parser.
*/

#ifndef LL_TABLE_H
#define LL_TABLE_H

/*
    Zmena includu (semantic_actions.h nahrazen semantic_types.h) byla
    zpusobena prompty, jejichz zneni je v souboru semantic_types.h.
*/

#include "nonterminal.h"
#include "production.h"
#include "semantic_types.h"
#include "token.h"

/*
    Prompt pro vychozi kostru souboru s LL tabulkou.

    Prompt pro Claude:
        Tento soubor ll_table.h bude obshaovat deklarace pro samotnou LL
        tabulku. V .c tohoto souboru bude tato tabulka deklarovaná a zaplněná
        jako globální proměnná. Tabulka bude vždy míť pravidlo buď v statické,
        nebo callbackové verzi. Statickou produkci parser nahází na stack sám, u
        callbacku to bude dělat callback, aby mohl rovnou připojit i své
        atributy dle své potřeby.

        typedef enum {
            LL_RULE_ERROR = 0,
            LL_RULE_STATIC,
            LL_RULE_SEMANTIC
        } LLRuleType;

        typedef struct {
            LLRuleType type;
            union {
                StaticProduction *production;
                SemanticCallback callback;
            };
        } LLTableEntry;

        extern const LLTableEntry LL_table[NONTERM_COUNT][TOKEN_LL_TERM_COUNT];
*/

/**
 * @brief Typ polozky LL tabulky.
 */
typedef enum {
  LL_RULE_ERROR = 0, ///< Pro dvojici neterminal a terminal neexistuje pravidlo
  LL_RULE_STATIC,    ///< Staticka produkce, parser ji na zasobnik vlozi sam
  LL_RULE_SEMANTIC   ///< Callback, ktery produkci na zasobnik vlozi sam
} LLRuleType;

/**
 * @brief Polozka LL tabulky.
 *
 * U callbacku vklada produkci na zasobnik akce sama, aby mohla rovnou
 * pripojit i sve atributy podle potreby.
 */
typedef struct {
  LLRuleType type; ///< Urcuje, ktery clen unie je platny
  union {
    const StaticProduction *production; ///< Platne pro LL_RULE_STATIC
    SemanticCallback callback;          ///< Platne pro LL_RULE_SEMANTIC
  };
} LLTableEntry;

/**
 * @brief LL tabulka indexovana neterminalem (radek) a terminalem (sloupec).
 */
extern const LLTableEntry LL_TABLE[NONTERM_COUNT][TOKEN_LL_TERM_COUNT];

#endif // LL_TABLE_H
