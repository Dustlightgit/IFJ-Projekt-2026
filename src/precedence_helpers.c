/*
    IFJ projekt 2026

    precedence_helpers.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        -

    Tento soubor obsahuje implementaci podpurnych funkci jadra precedencniho
    parseru. Prompt pro vznik souboru je v souboru precedence_helpers.h.
*/

#include "precedence_helpers.h"

#include "error.h"

ParserStackEntry *parser_top_terminal(const Stack *stack, int line) {

  // Prochazeni od vrcholu hloubeji, na dne je vzdy terminal $
  for (size_t depth = 0;; depth++) {
    ParserStackEntry *entry = parser_stack_peek(stack, depth);

    // Dno zasobniku bez terminalu, chybi terminal $
    if (entry == NULL)
      error_exit(ERR_INTERNAL, line,
                 "Precedencni parser: Na zasobniku neni zadny terminal.");

    if (entry->symbol.is_terminal)
      return entry;
  }
}

void parser_build_handle(const Stack *stack, SearchHandle *handle, int line) {

  // Zjisteni delky handle, tedy poctu zaznamu pred prvni znackou redukce
  size_t length = 0;
  for (;; length++) {
    const ParserStackEntry *entry = parser_stack_peek(stack, length);

    // Dno zasobniku bez znacky redukce
    if (entry == NULL)
      error_exit(ERR_INTERNAL, line,
                 "Precedencni parser: Na zasobniku neni znacka redukce.");

    if (entry->reduction_marker)
      break;

    // Zaznam v hloubce HANDLE_MAX_LENGTH by byl uz (HANDLE_MAX_LENGTH + 1).
    // symbolem handle
    if (length >= HANDLE_MAX_LENGTH)
      error_exit(ERR_SYNTAX, line,
                 "Precedencni parser: Handle je prilis dlouhy.");
  }

  // Skladani v prirozenem poradi, symbol na vrcholu je v handle posledni
  handle->length = length;
  for (size_t i = 0; i < length; i++)
    handle->symbols[length - 1 - i] = parser_stack_peek(stack, i)->symbol;
}

PredictorValue parser_evaluate_predictor(Token *token,
                                         PredictorValue predictor) {

  // ID na miste operatoru se prebarvi na ID_OPERATOR, po operatoru nasleduje
  // operand
  if (token->type == TOKEN_ID && predictor == EXPECT_OPERATOR) {
    token->type = TOKEN_ID_OPERATOR;
    return EXPECT_OPERAND;
  }

  // Po ukoncovaci zavorce a operandech nasleduje operator, jinak operand
  switch (token->type) {
  case TOKEN_RPAREN:
  case TOKEN_ID:
  case TOKEN_INT_LIT:
  case TOKEN_DOUBLE_LIT:
  case TOKEN_STRING_LIT:
  case TOKEN_KW_TRUE:
  case TOKEN_KW_FALSE:
    return EXPECT_OPERATOR;
  default:
    return EXPECT_OPERAND;
  }
}

bool parser_is_expression_terminator(const Token *token) {
  switch (token->type) {
  case TOKEN_EOL:
  case TOKEN_SEMICOLON:
  case TOKEN_KW_DO:
  case TOKEN_KW_THEN:
  case TOKEN_KW_ELSE:
    return true;
  default:
    return false;
  }
}
