/*
    IFJ projekt 2026

    precedence_parser.h

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        -

    Hlavickovy soubor pro precedencni parser vyrazu (zalozeny na precedencni
    tabulce) a jeho pomocne funkce. Zatim obsahuje jen provedeni redukce.
*/

#ifndef PRECEDENCE_PARSER_H
#define PRECEDENCE_PARSER_H

#include "parser_common.h"
#include "stack.h"

/*
    Prompt pro funkci parser_execute_reduction (vznik tohoto souboru
    a souboru precedence_parser.c). Stejny prompt zpusobil zmeny pouze v techto
    dvou souborech, jeho zneni je pouze zde.

    Prompt pro Claude:
        Myslím, že bych implementoval ještě funkci provedení redukce, která
        už ale rozhodně tvoří kód jádra. Vyčleňuji ji bokem jednak kvůli
        přehlednosti, ale druhak také proto, že potenciálně bude potřeba
        volat ze dvou míst, pokud konec výrazové analýzy ošetřím v jiné větvi
        než běžné provedení redukce. Jmenovat se bude parser_execute_reduction
        a bude v precedence_parser.*
        Precedenční tabulka přikázala, že se má provést redukce, proto je
        zavolána tato funkce. Funkce obdrží parser context a stack.

        Funkce vytvoří lokální proměnnou typu SearchHandle, tu předá funkci
        pro vytvoření handle, kterou jsme právě vytvořili; dále ji použije
        jako argument v reduction_table_lookup pro vyhledání callbacku.
        Vytvoří ReductionResult, aby mohla převzít její návratové hodnoty, a
        zavolá callback s příslušnými argumenty. Callback má díky vyhledání v
        redukční tabulce zaručeno, že na stacku je přesně jeho handle, se
        kterým provede cokoliv potřebuje, a vrátí symbol, jež má být umístěn
        na stack.

        Jakmile je callback ukončen, vyklidí se jeho stack handle (pojmenování
        pro handle na stacku pro odlišení od search handle, které je tvořeno
        pouze symboly) zavoláním parser_stack_pop tolikrát, jaká je délka
        handle.

        Získá se ukazatel na vrchol pomocí parser_stack_top a z něj je
        smazána redukční značka, protože redukce, kterou ohraničovala, právě
        byla provedena. Na zásobník je následně vložen záznam pro symbol a
        sémantické informace, které vrátil callback.

        Dobrá, možná nakonec parser context ani nepotřebuje; využila by z něj
        jen číslo řádku na vstupu.
*/

/**
 * @brief Provede redukci na zasobniku precedencniho parseru.
 *
 * Vola se, kdyz precedencni tabulka prikazala provest redukci. Funkce:
 * 1. Sestavi handle ze zasobniku (parser_build_handle).
 * 2. Vyhleda v redukcni tabulce semantickou akci (reduction_table_lookup).
 * 3. Zavola akci, handle je v tu chvili jeste na zasobniku. Akce vrati pres
 *    ReductionResult symbol, semanticky ukazatel a anotaci noveho zaznamu.
 * 4. Vyklidi handle ze zasobniku (tolikrat parser_stack_pop, kolik je delka
 *    handle).
 * 5. Smaze znacku redukce ze zaznamu na vrcholu, protoze redukce, kterou
 *    ohranicovala, byla provedena.
 * 6. Vlozi na zasobnik zaznam s redukovanym neterminalem a informacemi od
 *    akce, bez znacky redukce.
 *
 * Chyby handle a vyhledani resi volane funkce (ERR_SYNTAX pri prilis dlouhem
 * handle nebo neexistujicim pravidle). Pokud akce nevrati neterminal, jde
 * o internich chybu (ERR_INTERNAL).
 *
 * @param ctx Kontext parseru, pouziva se z nej cislo radku vstupniho tokenu.
 * @param stack Zasobnik precedencniho parseru.
 */
void parser_execute_reduction(const ParserContext *ctx, Stack *stack);

#endif // PRECEDENCE_PARSER_H
