/*
    IFJ projekt 2026

    reduction_table.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - pridana funkce reduction_table_lookup (prompt je v
          reduction_table.h).
        - xbalaka00 - vyplnena tabulka REDUCTION_TABLE vsemi pravidly E0 az
          E39 a pomocna makra pro zapis polozek (prompt je zde).
        - xbalaka00 - handle pravidel E7 a E8 pouzivaji TOKEN_PLUS_UNARY
          a TOKEN_MINUS_UNARY (prompt je v token.h).
        - xbalaka00 - pridana pravidla E32 a E33 (staticka funkce seznamu bez
          parametru a s parametrem), pravidla E33 az E39 precislovana na E35
          az E41 (prompt je v precedence_semantic_actions.c).

    Tento soubor obsahuje tabulku redukcnich pravidel vyrazove gramatiky,
    ktera mapuje handle jednotlivych pravidel na jejich semanticke akce.
    Prompt pro vznik tabulky je v souboru semantic_types.h.
*/

#include "reduction_table.h"

#include <stdio.h>

#include "error.h"
#include "grammar_symbol.h"
#include "nonterminal.h"
#include "precedence_semantic_actions.h"
#include "token.h"

/*
    Prompt pro vyplneni tabulky REDUCTION_TABLE a pomocna makra pro zapis
    jejich polozek. Stejny prompt zpusobil zmeny take v reduction_table.h,
    jeho zneni je pouze zde. Pozn.: V predchozim kroku vytvorene akce jsou
    v souboru precedence_semantic_actions.c.

    Prompt pro Claude:
        Dobrá, teď redukční tabulka. Každá položka bude obsahovat
        callback z minulého kroku, a SearchHandle, podle kterého bude
        vyhledávána. Pravida v ní musí být seřazena alespoň dle délky
        handle vzestupně (předpoklad vyhledávací funkce). Handle je
        tvořen GrammarSymbol a symboly jsou seřazeny v přirozeném
        pořadí. Pokud to pomůže, smíš napsat makra pro zjednodušení
        zápisu položek.
*/

// Zkracene zapisy symbolu handle, schovavaji makra GRAMMAR_TERMINAL
// a GRAMMAR_NONTERMINAL
#define SYM_TERM(t) GRAMMAR_TERMINAL(t)
#define SYM_NONTERM(n) GRAMMAR_NONTERMINAL(n)

// Zapis polozky tabulky: semanticka akce a symboly handle v prirozenem poradi
// zleva doprava. Delka handle se spocita z poctu predanych symbolu, nelze ji
// tedy zapsat chybne.
#define REDUCTION_RULE(action_function, ...)                                   \
  {                                                                            \
    .action = (action_function), .handle = {                                   \
      .length =                                                                \
          sizeof((GrammarSymbol[]){__VA_ARGS__}) / sizeof(GrammarSymbol),      \
      .symbols = {__VA_ARGS__}                                                 \
    }                                                                          \
  }

// Tabulka je serazena vzestupne podle delky handle (na tom stavi funkce
// reduction_table_lookup), uvnitr skupiny stejne delky podle cisla pravidla.
// Cislo pravidla je v komentari u kazde polozky. Tabulka je zaplnena celou svou
// velikosti.
const ReductionTableEntry REDUCTION_TABLE[REDUCTION_TABLE_SIZE] = {
    // Handle delky 1
    // E0: E <= ID
    REDUCTION_RULE(precedence_action_term_id, SYM_TERM(TOKEN_ID)),
    // E1: E <= INT_LIT
    REDUCTION_RULE(precedence_action_term_int, SYM_TERM(TOKEN_INT_LIT)),
    // E2: E <= DOUBLE_LIT
    REDUCTION_RULE(precedence_action_term_double, SYM_TERM(TOKEN_DOUBLE_LIT)),
    // E3: E <= STRING_LIT
    REDUCTION_RULE(precedence_action_term_string, SYM_TERM(TOKEN_STRING_LIT)),
    // E4: E <= true
    REDUCTION_RULE(precedence_action_term_true, SYM_TERM(TOKEN_KW_TRUE)),
    // E5: E <= false
    REDUCTION_RULE(precedence_action_term_false, SYM_TERM(TOKEN_KW_FALSE)),

    // Handle delky 2
    // E6: E <= ( )
    REDUCTION_RULE(precedence_action_term_unit, SYM_TERM(TOKEN_LPAREN),
                   SYM_TERM(TOKEN_RPAREN)),
    // E7: E <= + E
    REDUCTION_RULE(precedence_action_unary_plus, SYM_TERM(TOKEN_PLUS_UNARY),
                   SYM_NONTERM(NONTERM_EXPR)),
    // E8: E <= - E
    REDUCTION_RULE(precedence_action_unary_minus, SYM_TERM(TOKEN_MINUS_UNARY),
                   SYM_NONTERM(NONTERM_EXPR)),
    // E9: E <= ! E
    REDUCTION_RULE(precedence_action_unary_not, SYM_TERM(TOKEN_NOT),
                   SYM_NONTERM(NONTERM_EXPR)),
    // E37: ARGS <= E ,
    REDUCTION_RULE(precedence_action_args_trailing_comma_expr,
                   SYM_NONTERM(NONTERM_EXPR), SYM_TERM(TOKEN_COMMA)),
    // E38: ARGS <= ARGS ,
    REDUCTION_RULE(precedence_action_args_trailing_comma_args,
                   SYM_NONTERM(NONTERM_ARGS), SYM_TERM(TOKEN_COMMA)),
    // E40: E <= TIF E
    REDUCTION_RULE(precedence_action_ternary_else, SYM_NONTERM(NONTERM_TIF),
                   SYM_NONTERM(NONTERM_EXPR)),

    // Handle delky 3
    // E10: E <= E * E
    REDUCTION_RULE(precedence_action_multiply, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_STAR), SYM_NONTERM(NONTERM_EXPR)),
    // E11: E <= E / E
    REDUCTION_RULE(precedence_action_divide, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_SLASH), SYM_NONTERM(NONTERM_EXPR)),
    // E12: E <= E + E
    REDUCTION_RULE(precedence_action_add, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_PLUS), SYM_NONTERM(NONTERM_EXPR)),
    // E13: E <= E - E
    REDUCTION_RULE(precedence_action_subtract, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_MINUS), SYM_NONTERM(NONTERM_EXPR)),
    // E14: E <= E ++ E
    REDUCTION_RULE(precedence_action_concat, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_PLUSPLUS), SYM_NONTERM(NONTERM_EXPR)),
    // E15: E <= E < E
    REDUCTION_RULE(precedence_action_less, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_LT), SYM_NONTERM(NONTERM_EXPR)),
    // E16: E <= E <= E
    REDUCTION_RULE(precedence_action_less_equal, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_LE), SYM_NONTERM(NONTERM_EXPR)),
    // E17: E <= E > E
    REDUCTION_RULE(precedence_action_greater, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_GT), SYM_NONTERM(NONTERM_EXPR)),
    // E18: E <= E >= E
    REDUCTION_RULE(precedence_action_greater_equal, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_GE), SYM_NONTERM(NONTERM_EXPR)),
    // E19: E <= E == E
    REDUCTION_RULE(precedence_action_equal, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_EQ), SYM_NONTERM(NONTERM_EXPR)),
    // E20: E <= E != E
    REDUCTION_RULE(precedence_action_not_equal, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_NE), SYM_NONTERM(NONTERM_EXPR)),
    // E21: E <= E && E
    REDUCTION_RULE(precedence_action_and, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_AND), SYM_NONTERM(NONTERM_EXPR)),
    // E22: E <= E || E
    REDUCTION_RULE(precedence_action_or, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_OR), SYM_NONTERM(NONTERM_EXPR)),
    // E23: E <= ID ( )
    REDUCTION_RULE(precedence_action_call_no_args, SYM_TERM(TOKEN_ID),
                   SYM_TERM(TOKEN_LPAREN), SYM_TERM(TOKEN_RPAREN)),
    // E26: E <= List ( )
    REDUCTION_RULE(precedence_action_list_empty, SYM_TERM(TOKEN_KW_LIST),
                   SYM_TERM(TOKEN_LPAREN), SYM_TERM(TOKEN_RPAREN)),
    // E29: E <= E . ID
    REDUCTION_RULE(precedence_action_member_access, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_DOT), SYM_TERM(TOKEN_ID)),
    // E35: ARGS <= E , E
    REDUCTION_RULE(precedence_action_args_create, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_COMMA), SYM_NONTERM(NONTERM_EXPR)),
    // E36: ARGS <= ARGS , E
    REDUCTION_RULE(precedence_action_args_append, SYM_NONTERM(NONTERM_ARGS),
                   SYM_TERM(TOKEN_COMMA), SYM_NONTERM(NONTERM_EXPR)),
    // E39: E <= ( E )
    REDUCTION_RULE(precedence_action_parenthesized_expr, SYM_TERM(TOKEN_LPAREN),
                   SYM_NONTERM(NONTERM_EXPR), SYM_TERM(TOKEN_RPAREN)),
    // E41: E <= E ID_OPERATOR E
    REDUCTION_RULE(precedence_action_infix_method, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_ID_OPERATOR), SYM_NONTERM(NONTERM_EXPR)),

    // Handle delky 4
    // E24: E <= ID ( E )
    REDUCTION_RULE(precedence_action_call_one_arg, SYM_TERM(TOKEN_ID),
                   SYM_TERM(TOKEN_LPAREN), SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_RPAREN)),
    // E25: E <= ID ( ARGS )
    REDUCTION_RULE(precedence_action_call_multiple_args, SYM_TERM(TOKEN_ID),
                   SYM_TERM(TOKEN_LPAREN), SYM_NONTERM(NONTERM_ARGS),
                   SYM_TERM(TOKEN_RPAREN)),
    // E27: E <= List ( E )
    REDUCTION_RULE(precedence_action_list_one_element, SYM_TERM(TOKEN_KW_LIST),
                   SYM_TERM(TOKEN_LPAREN), SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_RPAREN)),
    // E28: E <= List ( ARGS )
    REDUCTION_RULE(precedence_action_list_multiple_elements,
                   SYM_TERM(TOKEN_KW_LIST), SYM_TERM(TOKEN_LPAREN),
                   SYM_NONTERM(NONTERM_ARGS), SYM_TERM(TOKEN_RPAREN)),

    // Handle delky 5
    // E32: E <= List . ID ( )
    REDUCTION_RULE(precedence_action_list_static_function_no_args,
                   SYM_TERM(TOKEN_KW_LIST), SYM_TERM(TOKEN_DOT),
                   SYM_TERM(TOKEN_ID), SYM_TERM(TOKEN_LPAREN),
                   SYM_TERM(TOKEN_RPAREN)),

    // Handle delky 6
    // E30: E <= E . ID ( E )
    REDUCTION_RULE(precedence_action_method_one_arg, SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_DOT), SYM_TERM(TOKEN_ID),
                   SYM_TERM(TOKEN_LPAREN), SYM_NONTERM(NONTERM_EXPR),
                   SYM_TERM(TOKEN_RPAREN)),
    // E31: E <= E . ID ( ARGS )
    REDUCTION_RULE(precedence_action_method_multiple_args,
                   SYM_NONTERM(NONTERM_EXPR), SYM_TERM(TOKEN_DOT),
                   SYM_TERM(TOKEN_ID), SYM_TERM(TOKEN_LPAREN),
                   SYM_NONTERM(NONTERM_ARGS), SYM_TERM(TOKEN_RPAREN)),
    // E33: E <= List . ID ( E )
    REDUCTION_RULE(precedence_action_list_static_function_one_arg,
                   SYM_TERM(TOKEN_KW_LIST), SYM_TERM(TOKEN_DOT),
                   SYM_TERM(TOKEN_ID), SYM_TERM(TOKEN_LPAREN),
                   SYM_NONTERM(NONTERM_EXPR), SYM_TERM(TOKEN_RPAREN)),
    // E34: E <= List . ID ( ARGS )
    REDUCTION_RULE(precedence_action_list_static_function_multiple_args,
                   SYM_TERM(TOKEN_KW_LIST), SYM_TERM(TOKEN_DOT),
                   SYM_TERM(TOKEN_ID), SYM_TERM(TOKEN_LPAREN),
                   SYM_NONTERM(NONTERM_ARGS), SYM_TERM(TOKEN_RPAREN)),
};

#undef REDUCTION_RULE
#undef SYM_NONTERM
#undef SYM_TERM

// Porovna dva symboly gramatiky, shodne musi byt druh symbolu i jeho hodnota
static bool grammar_symbols_equal(const GrammarSymbol *a,
                                  const GrammarSymbol *b) {
  if (a->is_terminal != b->is_terminal)
    return false;

  if (a->is_terminal)
    return a->terminal == b->terminal;

  return a->nonterminal == b->nonterminal;
}

// Porovna dva handle stejne delky, shodne musi byt vsechny symboly
static bool handles_equal(const SearchHandle *a, const SearchHandle *b) {
  if (a->length != b->length)
    return false;

  for (size_t i = 0; i < a->length; i++) {
    if (!grammar_symbols_equal(&a->symbols[i], &b->symbols[i]))
      return false;
  }
  return true;
}

// Vypise hledany handle na stderr pro chybove hlaseni
static void print_handle(const SearchHandle *handle) {
  fprintf(stderr, "Precedencni parser: Hledany handle:");
  for (size_t i = 0; i < handle->length; i++) {
    const GrammarSymbol *symbol = &handle->symbols[i];
    fprintf(stderr, " %s",
            symbol->is_terminal ? token_type_name(symbol->terminal)
                                : nonterm_type_name(symbol->nonterminal));
  }
  fprintf(stderr, "\n");
}

PrecedenceSemanticAction reduction_table_lookup(const SearchHandle *handle,
                                                int line) {

  if (handle->length < 1 || handle->length > HANDLE_MAX_LENGTH)
    error_exit(ERR_INTERNAL, line,
               "Precedencni parser: Neplatna delka hledaneho handle.");

  for (size_t i = 0; i < REDUCTION_TABLE_SIZE; i++) {
    const ReductionTableEntry *entry = &REDUCTION_TABLE[i];

    // Nevyuzite polozky jsou az na konci tabulky
    if (entry->action == NULL)
      break;

    // Kratsi handle nemohou odpovidat, preskoceni
    if (entry->handle.length < handle->length)
      continue;

    // Tabulka je serazena podle delky, delsi handle uz nenasleduje shodny
    if (entry->handle.length > handle->length)
      break;

    // Handle stejne delky, porovnani vsech symbolu
    if (handles_equal(&entry->handle, handle))
      return entry->action;
  }

  print_handle(handle);
  error_exit(ERR_SYNTAX, line,
             "Precedencni parser: Pro handle neexistuje redukcni pravidlo.");
  return NULL;
}
