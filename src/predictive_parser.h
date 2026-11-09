
/*
    IFJ projekt 2026

    predictive_parser.h

    Puvodni autor: xbalaka00 za konzultace s ChatGPT
    Seznam zmen:
        - 

    Hlavickovy soubor pro parser zalozeny na LL gramatice a jeho pomocne funkce.
*/

#ifndef PREDICTIVE_PARSER_H
#define PREDICTIVE_PARSER_H

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

#endif // PREDICTIVE_PARSER_H