/*
    IFJ projekt 2026

    xbalaka00 Artur Balak
    xnovacm00 Michal Novacek

    special_actions.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - special_action_id vytvari AST uzel pro ID pri anotaci
          CONTEXT_ID_COMMAND.
        - xnovacm00 - vyplnena funkce special_action_id
        - xbalaka00 - pridana prozatim prazdna funkce pro specialni akci predzpracovani terminalu
        - xbalaka00 - implementace special_action_precedence_preprocessing
          (prompt je v special_actions.h).
        - xbalaka00 - pridana special_action_execute_expr_rule a presunuta
          funkce ast_attach_expr z predictive_semantic_actions.c, prejmenovana na
          special_action_expr (prompt je v special_actions.h).
        - xbalaka00 - funkce target_block a store_type sem byly presunuty z predictive_semantic_actions.*
          a prejmenovany na special_action_expr a special_action_store_type,
          sjednocen format cisel pravidel u special_action_execute_expr_rule.
        - xbalaka00 - zmenen argument funkce special_action_execute_expr_rule, protoze misto pro schovani
          uzlu ID pro pravidla L32 a L33 bylo presunuto do ParserContext.

    Tento soubor obsahuje specialni akce parseru, napriklad akce pro stavbu AST,
    ktere ale nejsou samy pravidlem.
*/

#include "special_actions.h"

#include "ast.h"
#include "error.h"
#include "nonterminal.h"
#include "predictive_parser.h"
#include "predictive_semantic_actions.h"
#include "production.h"
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

void special_action_id(ParserContext *ctx, void *semantic_pointer,
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

Block *special_action_block(void *ptr, ContextAnnotation annotation, int line) {
  Block *block;
  switch (annotation) {
  case CONTEXT_BLOCK_STMT:
    return (Block *)ptr;
  case CONTEXT_FUNCTION_BODY:
    block = block_new();
    funcdef_set_body((FuncDef *)ptr, block);
    return block;
  case CONTEXT_WHILE_BODY:
    block = block_new();
    stmt_while_set_body((Stmt *)ptr, block);
    return block;
  case CONTEXT_FOR_BODY:
    block = block_new();
    stmt_for_set_body((Stmt *)ptr, block);
    return block;
  case CONTEXT_IF_THEN:
    block = block_new();
    expr_if_set_then((Expr *)ptr, block);
    return block;
  case CONTEXT_IF_ELSE:
    block = block_new();
    expr_if_set_else((Expr *)ptr, block);
    return block;
  default:
    error_exit(ERR_INTERNAL, line, "Prikaz nema urceno, kam se ma pridat");
  }
  return NULL;
}

void special_action_store_type(void *semantic_pointer,
                               ContextAnnotation annotation, Type *type,
                               int line) {
  switch (annotation) {
  case CONTEXT_FUNCTION_RETURN_TYPE:
    funcdef_set_return_type((FuncDef *)semantic_pointer, type);
    break;
  case CONTEXT_PARAM_TYPE: {
    // Typ patri parametru, ktery byl pridan jako posledni
    FuncDef *func = (FuncDef *)semantic_pointer;
    funcdef_param_set_type(func, func->param_count - 1, type);
    break;
  }
  case CONTEXT_LIST_ELEMENT:
    type_list_set_element((Type *)semantic_pointer, type);
    break;

  case CONTEXT_VARDEF_TYPE:
    stmt_vardef_set_type((Stmt *)semantic_pointer, type);
    break;

  default:
    type_free(type);
    error_exit(ERR_INTERNAL, line, "Typ nema urceno, kam se ma ulozit");
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
    Block *block = special_action_block(ptr, annotation, line);
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
  case NONTERM_BLOCK_OR_EXPR: // Pravidlo L20
    parser_push_static_production(stack, &PRODUCTION_BLOCK_OR_EXPR_EXPR, ptr,
                                  annotation);
    break;

  case NONTERM_COMMAND: // Pravidlo L45
    parser_push_static_production(stack, &PRODUCTION_COMMAND_SIMPLE, ptr,
                                  annotation);
    break;

  case NONTERM_COMMAND_LIST: // Pravidlo L23
    parser_push_static_production(stack, &PRODUCTION_COMMAND_LIST_NEXT, ptr,
                                  annotation);
    break;

  case NONTERM_COMMAND_SIMPLE: // Pravidlo L30
    parser_push_static_production(stack, &PRODUCTION_COMMAND_SIMPLE_EXPR, ptr,
                                  annotation);
    break;

  case NONTERM_RETURN_VALUE: // Pravidlo L35
    parser_push_static_production(stack, &PRODUCTION_RETURN_VALUE_EXPR, ptr,
                                  annotation);
    break;

  case NONTERM_BLOCK_OR_COMMAND: // Pravidlo L44
    parser_push_static_production(stack, &PRODUCTION_BLOCK_OR_COMMAND_COMMAND,
                                  ptr, annotation);
    break;

  case NONTERM_THEN_BRANCH: // Pravidlo L56
    parser_push_static_production(stack, &PRODUCTION_THEN_BRANCH_SIMPLE, ptr,
                                  annotation);
    break;

  case NONTERM_OPTIONAL_ELSE: // Pravidlo L49
    parser_push_static_production(stack, &PRODUCTION_OPTIONAL_ELSE_ABSENT, ptr,
                                  annotation);
    break;

  case NONTERM_EXPR_OR_INDENT_EXPR: // Pravidlo L52
    parser_push_static_production(stack, &PRODUCTION_EXPR_OR_INDENT_EXPR_PLAIN,
                                  ptr, annotation);
    break;

  case NONTERM_ELSE_TERNARY_IF: // Pravidlo L54
    parser_push_static_production(stack, &PRODUCTION_ELSE_TERNARY_IF_ABSENT,
                                  ptr, annotation);
    break;

  default:
    error_exit(ERR_SYNTAX, ctx->current_token.line,
               "LL Parser: Neni mozne zvolit pravidlo.");
  }
}
