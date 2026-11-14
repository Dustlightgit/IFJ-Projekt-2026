/*
    IFJ projekt 2026

    special_actions.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        -

    Tento soubor obsahuje specialni akce parseru, ktere nejsou vazane na
    pravidlo LL tabulky. Zatim jen kostry, funkcionalita se doplni pri
    propojeni s AST.

*/

#include "special_actions.h"

#include <stdio.h>

/*
    Prompt pro specialni akci pro ID je v souboru special_actions.h.
*/

void special_action_id(SemanticContext *ctx, void *semantic_pointer,
                       ContextAnnotation annotation, const Token *token) {
  (void)ctx;
  (void)semantic_pointer;
  (void)annotation;

  // Zatim jen vypis, pozdeji zde pribude zpracovani ID podle anotace
  fprintf(stderr, "LL Parser: Potkan token ID, atribut: %s\n",
          token->value.text);
}
