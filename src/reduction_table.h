/*
    IFJ projekt 2026

    reduction_table.h

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        -

    Tento soubor bude obsahovat tabulku redukcnich pravidel (pravidel vyrazove
    gramatiky), ktera mapuje handle jednotlivych pravidel na jejich semanticky
    callback. Zatim obsahuje jen vyhledavaci prvek tabulky, tedy handle.
*/

#ifndef REDUCTION_TABLE_H
#define REDUCTION_TABLE_H

#include <stddef.h>

#include "grammar_symbol.h"

/*
    Prompt pro vycleneni GrammarSymbol do grammar_symbol.h, vznik tohoto
    souboru a zkraceni STATIC_PRODUCTION_MAX_LENGTH. Stejny prompt zpusobil
    zmeny take v grammar_symbol.h, production.h, production.c,
    parser_common.h, semantic_actions.c a predictive_parser.c, jeho zneni je
    pouze zde.

    Prompt pro Claude:
        Vyčleň GrammarSymbol a makra pro tvorbu jeho typů ven z production.h
        do vlastního hlavičkového souboru grammar_symbol.h - důvodem je, že
        vytvoříme nový souborový pár reduction_table.*, ve kterém bude tabulka
        redukčních pravidel (pravidel výrazové gramatiky). Tabulka bude
        mapovat handle jednotlivých pravidel na jejich sémantický callback.
        Tuto tabulku i ostatní věci doplníme později, prozatím navrhneme
        pouze to, čím se v ní bude vyhledávat.

        Vyhledávacím prvkem bude SearchHandle, což bude praktický protějšek
        StaticProduction - bude mít také délku a seznam GrammarSymbol pevné
        maximální délky, jehož maximální velikost bude definována konstantou
        #define HANDLE_MAX_LENGTH 6.

        Vyvstává otázka, jestli by nešlo použít tu samou strukturu, ale obě
        teoreticky reprezentují odlišnou věc. Maximální délka seznamu také asi
        bude odlišná, protože nejdelší statické pravidlo má 5 symbolů (tu
        konstantu vlastně můžeš aktualizovat když už jsme u toho) a nejdelší
        handle má 6 symbolů. Handle také nebude vždy jen statická konstanta -
        bude se skutečně sestavovat v jádře, aby se podle něj vyhledávalo v
        tabulce redukcí.

        Zatím tedy jen vyjmi GrammarSymbol ven z production, příslušně uprav
        importy, a vytvoř deklarace pro SearchHandle a konstantu pro jeho
        maximální délku v souboru reduction_table.h
*/

/**
 * @brief Maximalni pocet symbolu v jednom handle.
 *
 * Nejdelsi handle vyrazove gramatiky ma 6 symbolu.
 */
#define HANDLE_MAX_LENGTH 6

/**
 * @brief Handle, vyhledavaci prvek tabulky redukcnich pravidel.
 *
 * Prakticky protejsek StaticProduction, ktery ale predstavuje jinou vec:
 * handle neni prava strana pravidla ulozena jako konstanta, nybrz se za behu
 * sestavuje v jadre precedencniho parseru z prvku zasobniku (viz
 * reduction_marker v ParserStackEntry) a podle nej se vyhledava pravidlo
 * v tabulce redukci. Symboly jsou v prirozenem poradi (zleva doprava), tedy
 * od nejhloubeji ulozeneho symbolu k vrcholu zasobniku.
 */
typedef struct {
  size_t length; ///< Pocet symbolu
  /// Symboly, vyuzito je prvnich length
  GrammarSymbol symbols[HANDLE_MAX_LENGTH];
} SearchHandle;

#endif // REDUCTION_TABLE_H
