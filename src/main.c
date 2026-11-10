#include <stdio.h>

#include "nonterminal.h"
#include "parser_common.h"
#include "predictive_parser.h"

int main() {

  // Docasne, kvuli testovani

  // printf("cau\n");

  ParserContext parser_ctx;

  parser_context_init(&parser_ctx);

  predictive_parser(&parser_ctx, NONTERM_COMMAND_LIST, NULL);

  return 0;
}