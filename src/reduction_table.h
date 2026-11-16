/*
    IFJ projekt 2026

    reduction_table.h

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - pridana funkce reduction_table_lookup (prompt je v
          reduction_table.h).
        - xbalaka00 - pridana polozka ReductionTableEntry, konstanta
          REDUCTION_TABLE_SIZE a deklarace tabulky REDUCTION_TABLE (prompt je
          v semantic_types.h).
        - xbalaka00 - tabulka REDUCTION_TABLE je vyplnena (prompt je
          v reduction_table.c), upravena dokumentace.

    Tento soubor obsahuje tabulku redukcnich pravidel (pravidel vyrazove
    gramatiky), ktera mapuje handle jednotlivych pravidel na jejich semanticke
    akce. Obsahuje vyhledavaci prvek tabulky (handle), polozku tabulky
    a deklaraci tabulky.
*/

#ifndef REDUCTION_TABLE_H
#define REDUCTION_TABLE_H

#include <stddef.h>

#include "grammar_symbol.h"
#include "semantic_types.h"

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

/*
    Prompt pro polozku ReductionTableEntry a tabulku REDUCTION_TABLE je
    v souboru semantic_types.h. Prompt pro vyplneni tabulky je v souboru
    reduction_table.c.
*/

/**
 * @brief Pocet radku (pravidel) redukcni tabulky.
 */
#define REDUCTION_TABLE_SIZE 40

/**
 * @brief Polozka redukcni tabulky, jedno pravidlo vyrazove gramatiky.
 */
typedef struct {
  /// Semanticka akce pravidla, provede se pri redukci handle
  PrecedenceSemanticAction action;
  /// Handle pravidla, tedy symboly jeho prave strany
  const SearchHandle handle;
} ReductionTableEntry;

/**
 * @brief Tabulka redukcnich pravidel vyrazove gramatiky.
 *
 * Mapuje handle pravidla na jeho semantickou akci. Polozky jsou serazeny
 * vzestupne podle delky handle (poradi neodpovida cislum pravidel), na jejich
 * poradi se spoleha funkce reduction_table_lookup. Pripadne nevyuzite polozky
 * s action NULL jsou az na konci tabulky. Tabulka obsahuje vsech 40 pravidel
 * vyrazove gramatiky (E0 az E39), nevyuzite polozky tedy zatim zadne nejsou.
 */
extern const ReductionTableEntry REDUCTION_TABLE[REDUCTION_TABLE_SIZE];

/*
    Prompt pro vyhledavaci funkci reduction_table_lookup. Stejny prompt
    zpusobil zmeny take v reduction_table.c, jeho zneni je pouze zde.

    Prompt pro Claude:
        OK, teď bych udělal tu vyhledávací funkci. Je to prakticky vyhledávací
        tabulka, která ale má pevně daný a neměnný počet položek. Proto bude
        bohatě stačit O(n) vyhledávání. V tabulce bude search handle klíčem,
        podle kterého bude vyhledávána položka pointeru na sémantickou akci.

        Vyhledávací funkce reduction_table_lookup dostane jako argument
        SearchHandle, které jádro vyplní symboly ze stacku dle příslušné
        konvence, a funkce bude vracet callback uložený na této pozici
        (případně sama vyhodí chybu).

        Položky v tabulce budou seřazeny přednostně podle délky handle
        (navzdory tomu, že to neodpovídá číslům pravidel).

        Prvně se bude vyhledávat podle délky handle - tedy první záznamy se
        pouze prosviští jednoduchou kontrolou, dokud se nedojde k posloupnosti
        stejně dlouhých handle. Tyto handle se pak po jednom budou srovnávat s
        klíčem, podle kterého hledáme; při první neshodě se postoupí o index
        seznamu ReductionTableEntry dále. Takto se prohledají všechny záznamy
        se stejnou délkou handle; jakmile se dojde k první položce s delším
        handle, víme, že můžeme skončit.

        Pokud chceš, můžeš udělat i předpoklad lexikografického seřazení
        pravidel dle handle (ať už primárně, nebo sekundárně), a vyhledávat
        handle lexikograficky, avšak mám dojem, že to nebude tvořit moc velký
        rozdíl. Pokud by však bylo použito lexikografické vyhledávání, je
        nutné zajistit, že celý vyhledávaný handle skutečně odpovídá celému
        handle nalezeného pravidla.

        Úspěšné vyhledání pravidla vrátí příslušný callback, aby jej jádro
        mohlo zavolat. Neúspěšné vyhledání vyhodí ERR_SYNTAX.
*/

/**
 * @brief Vyhleda v redukcni tabulce pravidlo podle handle.
 *
 * Tabulka je serazena podle delky handle, funkce proto preskoci kratsi
 * handle, porovna cely vyhledavany handle (delku i vsechny symboly) postupne
 * se vsemi handle stejne delky a pri prvnim delsim handle vyhledavani konci.
 * Delka vyhledavaneho handle musi byt v rozsahu 1 az HANDLE_MAX_LENGTH, jinak
 * jde o internich chybu (ERR_INTERNAL). Pokud pravidlo neexistuje, program
 * skonci syntaktickou chybou (ERR_SYNTAX) a na stderr vypise hledany handle.
 *
 * @param handle Vyhledavany handle, sestaveny jadrem ze symbolu zasobniku
 *        v prirozenem poradi (viz SearchHandle), nesmi byt NULL.
 * @param line Cislo radku vstupniho tokenu pro chybova hlaseni.
 * @return Semanticka akce nalezeneho pravidla, nikdy NULL.
 */
PrecedenceSemanticAction reduction_table_lookup(const SearchHandle *handle,
                                                int line);

#endif // REDUCTION_TABLE_H
