/*
    IFJ projekt 2026

    xsantar00 Radovan Santavy
    xnovacm00 Michal Novacek
    xbalaka00 Artur Balak

    main.c

    Vstupni funkce programu, ktera orchestruje chovani celeho prekladace.
*/

#include <stdio.h>

#include "ast.h"
#include "generator.h"
#include "nonterminal.h"
#include "parser_common.h"
#include "predictive_parser.h"
#include "semantic.h"
#include "symtable.h"

int main() {
  // Inicializace kontextu parseru
  ParserContext parser_ctx;
  parser_context_init(&parser_ctx);

  // Priprava AST
  Program *program = program_new();

  // Spusteni parseru a stavby AST
  predictive_parser(&parser_ctx, NONTERM_PROGRAM, program);

  // Spusteni semantickeho analyzatoru
  SymTable *table = semantic_analyze(program);

  // Generator kodu
  generate_program(program, stdout);

  // Uklid
  symtable_free(table);
  program_free(program);
  return 0;
}
