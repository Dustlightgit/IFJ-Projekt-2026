/*
    IFJ projekt 2026

    special_actions.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - special_action_id vytvari AST uzel pro ID pri anotaci
          CONTEXT_ID_COMMAND.
        - xbalaka00 - pridana prozatim prazdna funkce pro specialni akci predzpracovani terminalu
        - xbalaka00 - implementace special_action_precedence_preprocessing
          (prompt je v special_actions.h).
        - xbalaka00 - pridana special_action_execute_expr_rule a presunuta
          funkce ast_attach_expr z semantic_actions.c, prejmenovana na
          special_action_expr (prompt je v special_actions.h).

    Tento soubor obsahuje specialni akce parseru, ktere nejsou vazane na
    pravidlo LL tabulky. Zatim jen kostry, funkcionalita se doplni pri
    propojeni s AST.

*/

#include "special_actions.h"

#include "ast.h"
#include "error.h"
#include "nonterminal.h"
#include "predictive_parser.h"
#include "production.h"
#include "semantic_actions.h"
#include <stdio.h>
#include <string.h>

/*
    Prompt pro specialni akci pro ID je v souboru special_actions.h. Prompt
    pro chovani akce pro CONTEXT_ID_COMMAND je v souboru predictive_parser.c.
    Prompt pro specialni akci predzpracovani terminalu precedencniho parseru
    je v souboru special_actions.h. Prompt pro specialni akci provedeni
    pravidla vyrazu a pro presun funkce special_action_expr je take v souboru
    special_actions.h.
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

void *special_action_precedence_preprocessing(Token *input_token) {
  switch (input_token->type) {
  case TOKEN_ID:
  case TOKEN_ID_OPERATOR:
    // ID i ID_OPERATOR nesou jmeno, rozhodnuti o skutecnem uzlu padne az pri
    // redukci
    return expr_new_variable(input_token->value.text, input_token->line);

  case TOKEN_INT_LIT:
    // Atribut tokenu je long long, uzel AST uchovava int
    return expr_new_int((int)input_token->value.int_val, input_token->line);

  case TOKEN_DOUBLE_LIT:
    return expr_new_double(input_token->value.double_val, input_token->line);

  case TOKEN_STRING_LIT:
    return expr_new_string(input_token->value.text, input_token->line);

  // Ostatni tokeny atribut nemaji
  default:
    return NULL;
  }
}

// Ulozi hotovy vyraz podle anotace (vola ji vyrazovy parser)
void special_action_expr(void *ptr, ContextAnnotation annotation, Expr *expr,
                         int line) {
  switch (annotation) {
  case CONTEXT_RETURN_VALUE:
    stmt_return_set_value((Stmt *)ptr, expr);
    break;
  case CONTEXT_VARDEF_INIT:
    stmt_vardef_set_init((Stmt *)ptr, expr);
    break;
  case CONTEXT_ASSIGN_VALUE:
    stmt_assign_set_value((Stmt *)ptr, expr);
    break;

  case CONTEXT_WHILE_COND:
    stmt_while_set_cond((Stmt *)ptr, expr);
    break;
  case CONTEXT_FOR_SOURCE:
    stmt_for_set_source((Stmt *)ptr, expr);
    break;
  case CONTEXT_FOR_GUARD:
    stmt_for_add_guard((Stmt *)ptr, expr);
    break;

  case CONTEXT_IF_COND:
    expr_if_set_cond((Expr *)ptr, expr);
    break;

  default: {
    // Samostatny vyraz (prikaz nebo jednoradkove telo)
    Block *block = target_block(ptr, annotation, line);
    block_add(block, stmt_new_expr(expr, line));
  }
  }
}

void special_action_execute_expr_rule(ParserContext *ctx, Stack *stack,
                                      const AnnotatedSymbol *top) {
  // Neterminal z vrcholu uz byl odebran ze zasobniku, pravidlo proto vklada
  // pravou stranu rovnou s jeho ukazatelem a anotaci
  void *ptr = top->semantic_pointer;
  ContextAnnotation annotation = top->annotation;

  switch (top->symbol.nonterminal) {
  case NONTERM_BLOCK_OR_EXPR: // Pravidlo 20
    parser_push_static_production(stack, &PRODUCTION_BLOCK_OR_EXPR_EXPR, ptr,
                                  annotation);
    break;

  case NONTERM_COMMAND: // Pravidlo 45
    parser_push_static_production(stack, &PRODUCTION_COMMAND_SIMPLE, ptr,
                                  annotation);
    break;

  case NONTERM_COMMAND_LIST: // Pravidlo 23
    parser_push_static_production(stack, &PRODUCTION_COMMAND_LIST_NEXT, ptr,
                                  annotation);
    break;

  case NONTERM_COMMAND_SIMPLE: // Pravidlo 30
    parser_push_static_production(stack, &PRODUCTION_COMMAND_SIMPLE_EXPR, ptr,
                                  annotation);
    break;

  case NONTERM_RETURN_VALUE: // Pravidlo 35
    parser_push_static_production(stack, &PRODUCTION_RETURN_VALUE_EXPR, ptr,
                                  annotation);
    break;

  case NONTERM_BLOCK_OR_COMMAND: // Pravidlo 44
    parser_push_static_production(stack, &PRODUCTION_BLOCK_OR_COMMAND_COMMAND,
                                  ptr, annotation);
    break;

  case NONTERM_THEN_BRANCH: // Pravidlo 56
    parser_push_static_production(stack, &PRODUCTION_THEN_BRANCH_SIMPLE, ptr,
                                  annotation);
    break;

  case NONTERM_OPTIONAL_ELSE: // Pravidlo 49
    parser_push_static_production(stack, &PRODUCTION_OPTIONAL_ELSE_ABSENT, ptr,
                                  annotation);
    break;

  case NONTERM_EXPR_OR_INDENT_EXPR: // Pravidlo 52
    parser_push_static_production(stack, &PRODUCTION_EXPR_OR_INDENT_EXPR_PLAIN,
                                  ptr, annotation);
    break;

  case NONTERM_ELSE_TERNARY_IF: // Pravidlo 54
    parser_push_static_production(stack, &PRODUCTION_ELSE_TERNARY_IF_ABSENT,
                                  ptr, annotation);
    break;

  default:
    error_exit(ERR_SYNTAX, ctx->current_token.line,
               "LL Parser: Neni mozne zvolit pravidlo.");
  }
}
