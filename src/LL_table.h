/*
    IFJ projekt 2026

    xbalaka00 Artur Balak

    LL_table.h

    Puvodni autor: xbalaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - include semantic_actions.h nahrazen semantic_types.h.
        - xbalaka00 - pridana deklarace funkce ll_table_lookup a poznamka
          o zname chybe v navrhu gramatiky.
        - xbalaka00 - velikost tabulky je NONTERM_LL_TABLE_COUNT radku,
          upravena dokumentace ll_table_lookup (prompt je v nonterminal.h).
          TOKEN_STAR zatim docasne zustava indexem tabulky.

    Tento soubor obsahuje deklarace pro LL tabulku, kterou bude vyuzivat
    prediktivni parser, a funkci pro vyhledani pravidla v ni.
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

/*
    Prompty pro funkci ll_table_lookup a poznamku o chybe gramatiky jsou
    v souboru predictive_parser.c.
*/

/*
    Prompt pro zmenu velikosti tabulky na NONTERM_LL_TABLE_COUNT je v souboru
    nonterminal.h.
*/

/*
    Zname chyba navrhu gramatiky (pravidla zatim nejsou opravena):
    Rizeni konstrukce if s jednoradkovym telem then a pritomnym telem else
    vyzaduje, aby prikaz uvnitr jednoradkoveho then nebyl terminovan, protoze
    jeho ukonceni zajisti else. Pravidla ale prikaz v <blockOrCommand> terminuji
    pomoci <commandEnd>, takze takovy zapis nelze odvodit. Chyba nevznikla
    pri prevodu gramatiky do kodu, je dana navrhem gramatiky. Gramatika bude
    prepracovana a pravidla v LL_table.c, production.c a semantic_actions.c
    nasledne prepsana; do te doby LL tabulka tuto chybu obsahuje.
*/

/**
 * @brief LL tabulka indexovana neterminalem (radek) a terminalem (sloupec).
 */
extern const LLTableEntry LL_TABLE[NONTERM_LL_TABLE_COUNT][TOKEN_LL_TERM_COUNT];

/**
 * @brief Vyhleda pravidlo v LL tabulce pro dvojici neterminal a terminal.
 *
 * Funkce overi rozsah indexu, vyhleda zaznam a zkontroluje jeho platnost
 * (znamy typ pravidla, u statickeho pravidla platnou produkci, u semantickeho
 * nenulovy callback). Vracena pravidla jsou tedy vzdy staticka nebo semanticka.
 * Volajici musi zajistit, ze neterminal je mensi nez NONTERM_LL_TABLE_COUNT
 * a terminal mensi nez TOKEN_LL_TERM_COUNT, jinak jde o interni chybu
 * (ERR_INTERNAL). Pokud pro
 * dvojici neexistuje pravidlo, ukonci program syntaktickou chybou (ERR_SYNTAX).
 *
 * @param nonterm Neterminal (radek tabulky).
 * @param terminal Typ vstupniho terminalu (sloupec tabulky).
 * @param line Cislo radku vstupniho tokenu pro chybova hlaseni.
 * @return Ukazatel na zaznam, ktery neni prazdnym pravidlem a je platny.
 */
const LLTableEntry *ll_table_lookup(NonterminalType nonterm, TokenType terminal,
                                    int line);

#endif // LL_TABLE_H
