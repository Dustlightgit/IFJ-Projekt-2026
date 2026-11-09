/*
    IFJ projekt 2026

    LL_table.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        -

    Tento soubor obsahuje definici LL tabulky pro prediktivni parser.
    Prozatim je tabulka prazdna, aby bylo mozne projekt prelozit a slinkovat;
    pozdeji bude doplnena pravidly gramatiky.

*/

#include "LL_table.h"

/*
    Prompt pro Claude:
        Myslíš, že je toto způsobeno tím, že LL_TABLE prozatím není definovaná
        v souboru .c a je jen definovaná? Pokud ano, vygeneruj dočasný
        LL_table.c, který jen deklaruje prázdnou tabulku, aby šlo udělat
        commit bez kompilačních problémů. Později bude doplněna/nahrazena
        samotnými pravidly. Ideálně prozatím tedy parser sice nepůjde spustit,
        avšak půjde zkompilovat.

        (K promptu byl priložen výpis linkeru s chybou undefined reference to
        LL_TABLE.)
*/

/**
 * @brief LL tabulka, prozatim prazdna.
 *
 * Vsechny polozky maji typ LL_RULE_ERROR (hodnota 0), takze parser pri
 * rozvoji jakehokoliv neterminalu skonci chybou syntaxe. Neuvedene prvky
 * pole jsou v C automaticky vynulovany.
 */
const LLTableEntry LL_TABLE[NONTERM_COUNT][TOKEN_LL_TERM_COUNT] = {
    {{.type = LL_RULE_ERROR}},
};
