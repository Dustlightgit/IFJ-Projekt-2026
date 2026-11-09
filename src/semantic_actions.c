/*
    IFJ projekt 2026

    semantic_actions.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        -

    Tento soubor obsahuje semanticke akce pravidel LL parseru. Akce zatim
    pouze vypisuji cislo pravidla na stderr a vkladaji pravou stranu pravidla
    na zasobnik, atributy pro stavbu stromu zatim nepridavaji.

*/

#include "semantic_actions.h"

#include <stdio.h>

#include "nonterminal.h"
#include "parser_common.h"
#include "production.h"
#include "token.h"

/*
    Prompt pro semanticke akce je v souboru LL_table.c.
*/

// Vypise cislo pravidla na stderr (ladici vystup)
static void trace_rule(int rule_number) {
  fprintf(stderr, "Pravidlo %d\n", rule_number);
}

// Vlozi na zasobnik terminal bez atributu pro stavbu stromu
static void push_terminal(Stack *stack, TokenType type) {
  GrammarSymbol symbol = GRAMMAR_TERMINAL(type);
  parser_stack_push(stack, symbol, NULL, ID_CONTEXT_NONE);
}

// Vlozi na zasobnik neterminal bez atributu pro stavbu stromu
static void push_nonterminal(Stack *stack, NonterminalType type) {
  GrammarSymbol symbol = GRAMMAR_NONTERMINAL(type);
  parser_stack_push(stack, symbol, NULL, ID_CONTEXT_NONE);
}

// Symboly prave strany se vkladaji od posledniho, aby na vrcholu zasobniku
// zustal prvni symbol.

// Pravidlo 12: <optionalType> => : <type>
void semantic_action_optional_type_present(SemanticContext *ctx,
                                           void *semantic_pointer,
                                           Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(12);

  push_nonterminal(stack, NONTERM_TYPE);
  push_terminal(stack, TOKEN_COLON);
}

// Pravidlo 27: <command> => <valVar> ID <optionalType> = <expr> <commandEnd>
void semantic_action_declaration_command(SemanticContext *ctx,
                                         void *semantic_pointer, Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(27);

  push_nonterminal(stack, NONTERM_COMMAND_END);
  push_nonterminal(stack, NONTERM_EXPR);
  push_terminal(stack, TOKEN_ASSIGN);
  push_nonterminal(stack, NONTERM_OPTIONAL_TYPE);
  push_terminal(stack, TOKEN_ID);
  push_nonterminal(stack, NONTERM_VAL_VAR);
}

// Pravidlo 34: <command> => return <returnValue> <commandEnd>
void semantic_action_return_command(SemanticContext *ctx,
                                    void *semantic_pointer, Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(34);

  push_nonterminal(stack, NONTERM_COMMAND_END);
  push_nonterminal(stack, NONTERM_RETURN_VALUE);
  push_terminal(stack, TOKEN_KW_RETURN);
}
