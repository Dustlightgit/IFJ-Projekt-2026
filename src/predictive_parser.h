/*
    IFJ projekt 2026

    xbalaka00 Artur Balak

    predictive_parser.h

    Puvodni autor: xbalaka00 za konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - pridana deklarace funkce predictive_parser za asistence
          Claude, prompt je v predictive_parser.c.
        - xbalaka00 - pridana deklarace funkce parser_push_static_production
          za asistence Claude, prompt je v predictive_parser.c.
        - xbalaka00 - parser_push_static_production kopiruje i kontextovou
          anotaci.

    Hlavickovy soubor pro parser zalozeny na LL gramatice a jeho pomocne funkce.
*/

#ifndef PREDICTIVE_PARSER_H
#define PREDICTIVE_PARSER_H

#include "nonterminal.h"
#include "parser_common.h"

/**
 * @brief Porovnani vstupniho tokenu s vrcholem zasobniku.
 * 
 * Pri shode jsou vstupni token i vrchol spotrebovany. Pri odlisnosti je vyvolana chyba 2.
 *
 * @param ctx Spolecny context parseru.
 * @param stack Zasobnik parseru.
 */
void parser_match(ParserContext *ctx, Stack *stack);

/*
    Prompt pro funkci parser_push_static_production je v souboru
    predictive_parser.c. Prompt pro kopirovani kontextove anotace je v souboru
    semantic_types.h.
*/

/**
 * @brief Vlozi symboly staticke produkce na zasobnik parseru.
 *
 * Symboly se vkladaji od posledniho, aby byl na vrcholu prvni symbol
 * produkce. Semanticky ukazatel i kontextova anotace se zkopiruji do vsech
 * vlozenych symbolu, vzdy, bez vyjimky. Pro produkci delky 0 se nic nevlozi.
 *
 * @param stack Zasobnik parseru.
 * @param production Staticka produkce, nesmi byt NULL.
 * @param semantic_pointer Semanticky ukazatel rozvadeneho neterminalu.
 * @param annotation Kontextova anotace rozvadeneho neterminalu.
 */
void parser_push_static_production(Stack *stack,
                                   const StaticProduction *production,
                                   void *semantic_pointer,
                                   ContextAnnotation annotation);

/**
 * @brief Jadro prediktivniho parseru rizeneho LL tabulkou.
 *
 * Funkce si vytvori vlastni zasobnik, takze se muze rekurzivne volat.
 * Vstupni proud je sdileny pres ctx a ctx musi mit nacteny prvni token
 * (viz parser_context_init). Pri chybe ukonci program pres error_exit.
 *
 * @param ctx Spolecny kontext parseru.
 * @param starting_nonterm Pocatecni neterminal, ktery se bude rozvadet.
 * @param semantic_pointer Semanticky ukazatel pocatecniho neterminalu.
 */
void predictive_parser(ParserContext *ctx, NonterminalType starting_nonterm,
                       void *semantic_pointer);

#endif // PREDICTIVE_PARSER_H