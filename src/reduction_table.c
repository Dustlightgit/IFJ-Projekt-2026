/*
    IFJ projekt 2026

    reduction_table.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        -

    Tento soubor obsahuje tabulku redukcnich pravidel vyrazove gramatiky.
    Tabulka je prozatim prazdna, pravidla a jejich semanticke akce budou
    doplneny pozdeji. Prompt pro vznik tabulky je v souboru semantic_types.h.
*/

#include "reduction_table.h"

// Tabulka je prozatim prazdna, vsechny polozky maji action NULL a handle
// delky 0.
const ReductionTableEntry REDUCTION_TABLE[REDUCTION_TABLE_SIZE] = {
    {.action = NULL},
};
