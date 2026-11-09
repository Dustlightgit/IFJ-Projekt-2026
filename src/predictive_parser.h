
/*
    IFJ projekt 2026

    predictive_parser.h

    Puvodni autor: xbalaka00 za konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - pridana deklarace funkce predictive_parser za asistence
          Claude, prompt je v predictive_parser.c.

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