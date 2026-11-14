/*
    IFJ projekt 2026

    special_actions.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - special_action_id vytvari AST uzel pro ID pri anotaci
          CONTEXT_ID_COMMAND.

    Tento soubor obsahuje specialni akce parseru, ktere nejsou vazane na
    pravidlo LL tabulky. Zatim jen kostry, funkcionalita se doplni pri
    propojeni s AST.

*/

#include "special_actions.h"

#include <stdio.h>

/*
    Prompt pro specialni akci pro ID je v souboru special_actions.h. Prompt
    pro chovani akce pro CONTEXT_ID_COMMAND je v souboru predictive_parser.c.
*/

void special_action_id(SemanticContext *ctx, void *semantic_pointer,
                       ContextAnnotation annotation, const Token *token) {
  (void)semantic_pointer;

  // Potkani ID se vypise vzdy
  fprintf(stderr, "LL Parser: Potkan token ID, atribut: %s\n",
          token->value.text);

  switch (annotation) {
  case CONTEXT_ID_COMMAND:
    // ID na zacatku prikazu: vytvori se AST uzel, ke kteremu ma pristup jadro
    // parseru (zpracuje ho v pravidlech 32 a 33 a uvolni ho)
    ctx->id_expr = expr_new_variable(token->value.text, token->line);
    break;

  default:
    // Ostatni anotace se zatim nezpracovavaji, pozdeji zde pribude zpracovani
    // dalsich kontextu (u neznameho kontextu nejspis interni chyba)
    break;
  }
}
