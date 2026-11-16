/*
    IFJ projekt 2026

    special_actions.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - special_action_id vytvari AST uzel pro ID pri anotaci
          CONTEXT_ID_COMMAND.
        - xbalaka00 - pridana prozatim prazdna funkce pro specialni akci predzpracovani terminalu

    Tento soubor obsahuje specialni akce parseru, ktere nejsou vazane na
    pravidlo LL tabulky. Zatim jen kostry, funkcionalita se doplni pri
    propojeni s AST.

*/

#include "special_actions.h"

#include "ast.h"
#include "error.h"
#include <stdio.h>
#include <string.h>

/*
    Prompt pro specialni akci pro ID je v souboru special_actions.h. Prompt
    pro chovani akce pro CONTEXT_ID_COMMAND je v souboru predictive_parser.c.
*/

void special_action_id(SemanticContext *ctx, void *semantic_pointer,
                       ContextAnnotation annotation, const Token *token) {

  // Potkani ID se vypise vzdy
  fprintf(stderr, "LL Parser: Potkan token ID, atribut: %s\n",
          token->value.text);

  switch (annotation) {
  case CONTEXT_ID_COMMAND:
    ctx->id_expr = expr_new_variable(token->value.text, token->line);
    break;

  case CONTEXT_PROLOGUE_CHECK:
    // Prolog musi byt presne import ifj26.*
    if (strcmp(token->value.text, "ifj26") != 0)
      error_exit(ERR_SYNTAX, token->line, "Prolog musi byt import ifj26.*");
    break;

  case CONTEXT_FUNCTION_DEF:
    // funcdef_set_name si text zkopiruje, token uvolni parser_match
    funcdef_set_name((FuncDef *)semantic_pointer, token->value.text);
    break;

  case CONTEXT_PARAM_NAME:
    // Typ se doplni pozdeji, az se rozvede <type> (CONTEXT_PARAM_TYPE)
    funcdef_add_param((FuncDef *)semantic_pointer, token->value.text, NULL);
    break;

  case CONTEXT_DECLARATION_NAME:
    stmt_vardef_set_name((Stmt *)semantic_pointer, token->value.text);
    break;

  case CONTEXT_FOR_VAR:
    stmt_for_set_var_name((Stmt *)semantic_pointer, token->value.text);
    break;

  default:
    break;
  }
}

// Bude doplneno pozdeji
void *special_action_precedence_preprocessing(Token *input_token) {
  (void)input_token;
  return NULL;
}