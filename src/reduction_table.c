/*
    IFJ projekt 2026

    reduction_table.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - pridana funkce reduction_table_lookup (prompt je v
          reduction_table.h).

    Tento soubor obsahuje tabulku redukcnich pravidel vyrazove gramatiky.
    Tabulka je prozatim prazdna, pravidla a jejich semanticke akce budou
    doplneny pozdeji. Prompt pro vznik tabulky je v souboru semantic_types.h.
*/

#include "reduction_table.h"

#include <stdio.h>

#include "error.h"
#include "grammar_symbol.h"
#include "nonterminal.h"
#include "token.h"

// Tabulka je prozatim prazdna, vsechny polozky maji action NULL a handle
// delky 0.
const ReductionTableEntry REDUCTION_TABLE[REDUCTION_TABLE_SIZE] = {
    {.action = NULL},
};

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
