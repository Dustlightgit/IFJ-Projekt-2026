/*
    IFJ projekt 2026

    xbalaka00 Artur Balak

    grammar_symbol.h

    Puvodni autor: xbalaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - GrammarSymbol a makra GRAMMAR_TERMINAL
          a GRAMMAR_NONTERMINAL vyclenena z production.h.

    Tento soubor obsahuje symbol gramatiky (terminal nebo neterminal)
    a makra pro jeho inicializaci. Je spolecny pro staticke produkce
    (production.h) i pro vyhledavaci prvek tabulky redukci (reduction_table.h).
*/

#ifndef GRAMMAR_SYMBOL_H
#define GRAMMAR_SYMBOL_H

#include <stdbool.h>

#include "nonterminal.h"
#include "token.h"

/*
    Prompt pro vycleneni tohoto souboru je v souboru reduction_table.h.
    Prompt pro vychozi podobu GrammarSymbol je v souboru production.h, prompt
    pro makra GRAMMAR_TERMINAL a GRAMMAR_NONTERMINAL je v souboru LL_table.c.
*/

/**
 * @brief Symbol gramatiky, terminal nebo neterminal.
 */
typedef struct {
  bool is_terminal; ///< Odliseni terminalu od neterminalu
  union {
    TokenType terminal;          ///< Platne, pokud je is_terminal true
    NonterminalType nonterminal; ///< Platne, pokud je is_terminal false
  };
} GrammarSymbol;

/**
 * @brief Inicializator GrammarSymbol pro terminal.
 *
 * Pouziva se pri inicializaci promenne nebo pole, napr.
 * GrammarSymbol symbol = GRAMMAR_TERMINAL(TOKEN_ID);
 */
#define GRAMMAR_TERMINAL(t)                                                    \
  { .is_terminal = true, .terminal = (t) }

/**
 * @brief Inicializator GrammarSymbol pro neterminal.
 *
 * Pouziva se pri inicializaci promenne nebo pole, napr.
 * GrammarSymbol symbol = GRAMMAR_NONTERMINAL(NONTERM_TYPE);
 */
#define GRAMMAR_NONTERMINAL(n)                                                 \
  { .is_terminal = false, .nonterminal = (n) }

#endif // GRAMMAR_SYMBOL_H
