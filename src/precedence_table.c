/*
    IFJ projekt 2026

    precedence_table.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        -

    Tento soubor obsahuje precedencni tabulku precedencniho parseru vyrazu
    a funkce pro prevod terminalu na jeji index a vyhledani direktivy.
    Tabulka je prozatim prazdna. Prompt pro vznik souboru je v souboru
    precedence_table.h.
*/

#include "precedence_table.h"

#include <stdio.h>

#include "error.h"
#include "token.h"

// Pocet terminalu v souvislem rozsahu enumu TokenType mezi TOKEN_LL_TERM_COUNT
// a TOKEN_EXPR_TERM_DELIMITER bez oddelovace TOKEN_EXPRESSION_BEGINNING_DELIMITER
#define PRECEDENCE_RANGE_COUNT                                                 \
  ((size_t)TOKEN_EXPR_TERM_DELIMITER - (size_t)TOKEN_LL_TERM_COUNT - 2)

// Pocet terminalu precedencni tabulky, ktere jsou v enumu TokenType mezi
// indexatory LL tabulky a v precedence_table_index je resi switch
// (KW_LIST, (, ), carka, ID a docasne take TOKEN_STAR).
#define PRECEDENCE_SWITCH_COUNT 6

// Pokud se TOKEN_STAR presune z indexatoru LL tabulky mezi ostatni terminaly
// vyrazove analyzy, tato kontrola selze. Je pak treba odstranit TOKEN_STAR ze
// switch v precedence_table_index a snizit PRECEDENCE_SWITCH_COUNT.
_Static_assert(PRECEDENCE_RANGE_COUNT + PRECEDENCE_SWITCH_COUNT ==
                   PRECEDENCE_TABLE_SIZE,
               "Pocet indexatoru precedencni tabulky nesedi s "
               "PRECEDENCE_TABLE_SIZE.");

// Precedencni tabulka je prozatim prazdna, vsechny polozky jsou
// DIRECTIVE_ERROR (hodnota 0).
const PrecedenceDirective
    PRECEDENCE_TABLE[PRECEDENCE_TABLE_SIZE][PRECEDENCE_TABLE_SIZE] = {
        {DIRECTIVE_ERROR},
};

size_t precedence_table_index(TokenType terminal, int line) {

  // Overeni, ze se jedna o skutecny token
  token_type_validate(terminal, line);

  // Terminaly v souvislem rozsahu mezi TOKEN_LL_TERM_COUNT
  // a TOKEN_EXPR_TERM_DELIMITER, indexy zacinaji od 0
  if (terminal > TOKEN_LL_TERM_COUNT && terminal < TOKEN_EXPR_TERM_DELIMITER) {
    size_t index = (size_t)terminal - (size_t)TOKEN_LL_TERM_COUNT - 1;

    // Oddelovac TOKEN_EXPRESSION_BEGINNING_DELIMITER neni terminal
    if (terminal > TOKEN_EXPRESSION_BEGINNING_DELIMITER)
      index--;

    return index;
  }

  // Terminaly zamichane mezi indexatory LL tabulky, indexy za souvislym
  // rozsahem
  switch (terminal) {
  case TOKEN_KW_LIST:
    return PRECEDENCE_RANGE_COUNT + 0;
  case TOKEN_LPAREN:
    return PRECEDENCE_RANGE_COUNT + 1;
  case TOKEN_RPAREN:
    return PRECEDENCE_RANGE_COUNT + 2;
  case TOKEN_COMMA:
    return PRECEDENCE_RANGE_COUNT + 3;
  case TOKEN_ID:
    return PRECEDENCE_RANGE_COUNT + 4;
  case TOKEN_STAR: // Docasne, TOKEN_STAR se v budoucnu presune do rozsahu
    return PRECEDENCE_RANGE_COUNT + 5;
  default:
    break;
  }

  error_exit(ERR_INTERNAL, line,
             "Precedencni parser: Spatne pouziti precedence_table_index(): "
             "Token neni indexem precedencni tabulky.");
  return 0;
}

PrecedenceDirective precedence_table_lookup(TokenType stack_terminal,
                                            TokenType input_terminal,
                                            int line) {

  size_t row = precedence_table_index(stack_terminal, line);
  size_t column = precedence_table_index(input_terminal, line);

  PrecedenceDirective directive = PRECEDENCE_TABLE[row][column];

  if (directive == DIRECTIVE_ERROR) {
    fprintf(stderr,
            "Precedencni parser: Terminal na zasobniku: %s\nTerminal na "
            "vstupu: %s\n",
            token_type_name(stack_terminal), token_type_name(input_terminal));

    char message[128];
    snprintf(message, sizeof message,
             "Precedencni parser: Kombinace terminalu neni platna (index "
             "zasobniku %zu, index vstupu %zu).",
             row, column);
    error_exit(ERR_SYNTAX, line, message);
  }

  // Kontrola platnosti zaznamu
  if (directive != DIRECTIVE_REDUCTION_MARKER &&
      directive != DIRECTIVE_REDUCTION_EXECUTE &&
      directive != DIRECTIVE_SHIFT_SYMBOL)
    error_exit(ERR_INTERNAL, line,
               "Precedencni parser: Neznama direktiva v precedencni tabulce.");

  return directive;
}
