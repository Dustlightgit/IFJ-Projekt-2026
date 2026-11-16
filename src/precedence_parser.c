/*
    IFJ projekt 2026

    precedence_parser.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - pridana prazdna hlavicka funkce precedence_parser
          (prompt je v precedence_parser.h).

    Tento soubor obsahuje implementaci precedencniho parseru vyrazu a jeho
    pomocnych funkci. Prompt pro vznik souboru je v souboru
    precedence_parser.h.
*/

#include "precedence_parser.h"

#include "error.h"
#include "nonterminal.h"
#include "precedence_helpers.h"
#include "reduction_table.h"
#include "semantic_types.h"

void parser_execute_reduction(const ParserContext *ctx, Stack *stack) {
  int line = ctx->current_token.line;

  // Sestaveni handle ze zasobniku a vyhledani semanticke akce pravidla
  SearchHandle handle;
  parser_build_handle(stack, &handle, line);
  PrecedenceSemanticAction action = reduction_table_lookup(&handle, line);

  // Provedeni akce, handle je v tu chvili stale na zasobniku
  ReductionResult result = {0};
  action(stack, line, &result);

  // Redukci musi vzniknout neterminal, jinak je chyba v akci
  if (result.symbol.is_terminal)
    error_exit(ERR_INTERNAL, line,
               "Precedencni parser: Semanticka akce nevratila neterminal.");
  nonterm_type_validate(result.symbol.nonterminal, line);

  // Vyklizeni handle ze zasobniku
  for (size_t i = 0; i < handle.length; i++)
    parser_stack_pop(stack, NULL);

  // Smazani znacky redukce, redukce, kterou ohranicovala, byla provedena
  ParserStackEntry *top = parser_stack_top(stack);
  if (top == NULL)
    error_exit(ERR_INTERNAL, line,
               "Precedencni parser: Po vyklizeni handle je zasobnik prazdny.");
  top->reduction_marker = false;

  // Vlozeni redukovaneho neterminalu
  parser_stack_push(stack, result.symbol, result.semantic_pointer,
                    result.annotation, false);
}

Expr *precedence_parser(ParserContext *ctx,
                        ReductionResult *starting_stack_entry) {
  (void)ctx;
  (void)starting_stack_entry;

  // Prozatim bez stavby stromu
  return NULL;
}
