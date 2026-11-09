/*
    IFJ projekt 2026

    predictive_parser.h

    Puvodni autor: xbalaka00 za konzultace s ChatGPT
    Seznam zmen:
        - 

    Implementace prediktivniho parseru zalozeneho na LL tabulce a nekterych jeho pomocnych funkci.
*/

#include "predictive_parser.h"
#include "error.h"
#include <stdio.h>

void parser_match(ParserContext *ctx, Stack *stack) {

  // Overeni, ze vstup ma validni terminal
  if (!ctx->has_current_token)
    error_exit(ERR_INTERNAL, 0,
               "LL Parser: Spatne pouziti parser_match(): Neni nacten token ze "
               "vstupu.");

  // Overeni, ze zasobnik neni prazdny
  if (stack_is_empty(stack))
    error_exit(
        ERR_INTERNAL, ctx->current_token.line,
        "LL Parser: Spatne pouziti parser_match(): Zasobnik je prazdny.");

  // Ziskani vrcholu
  ParserStackEntry *top = parser_stack_top(stack);

  // Pokud vrchol neni terminal, tato funkce nemela byt zavolana
  if (!top->symbol.is_terminal)
    error_exit(ERR_INTERNAL, ctx->current_token.line,
               "LL Parser: Spatne pouziti parser_match(): Na vrcholu zasobniku "
               "neni terminal.");

  // Porovnani a pripadne vyvolani chyby
  if (!parser_check_token(ctx, top->symbol.terminal)) {
    // Token na zasobniku a na vstupu jsou odlisne
    fprintf(stderr,
            "LL Parser: Soucasny vstupni terminal: %s\nSoucasny vrchol "
            "zasobniku: %s\n",
            token_type_name(ctx->current_token.type),
            token_type_name(top->symbol.terminal));
    error_exit(ERR_SYNTAX, ctx->current_token.line,
               "Odlisny terminal na vstupu a na vrcholu zasobniku.");
  }

  // Terminaly jsou v poradku.

  // Odstraneni vrcholu zasobniku.
  parser_stack_pop(stack, NULL);

  // Pokud terminal nebyl EOF, ziskej dalsi token
  if (ctx->current_token.type != TOKEN_EOF)
    parser_advance(ctx);
}