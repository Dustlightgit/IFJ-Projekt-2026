/*
    IFJ projekt 2026

    precedence_parser.h

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - pridana deklarace funkce precedence_parser (prompt je
          zde).
        - xbalaka00 - upravena deklarace precedence_parser
        - xbalaka00 - pridana deklarace a dokumentace funkce
          parser_execute_directive (prompt je zde).
        - xbalaka00 - ReductionResult prejmenovan na AnnotatedSymbol (prompt
          je v semantic_types.h).

    Hlavickovy soubor pro precedencni parser vyrazu (zalozeny na precedencni
    tabulce) a jeho pomocne funkce. Zatim obsahuje provedeni redukce,
    provedeni direktivy a deklaraci jadra parseru.
*/

#ifndef PRECEDENCE_PARSER_H
#define PRECEDENCE_PARSER_H

#include "ast.h"
#include "parser_common.h"
#include "semantic_types.h"
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
 *    AnnotatedSymbol symbol, semanticky ukazatel a anotaci noveho zaznamu.
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

/*
    Prompt pro kontrolu rucne napsaneho jadra precedencniho parseru,
    doxygen dokumentaci funkce parser_execute_directive a opravy v souborech
    precedence_parser.c, precedence_table.c, precedence_table.h
    a special_actions.c/.h. Jeho zneni je pouze zde.

    Prompt pro Claude:
        Zde jsou aktualizované soubory s mojí ruční implementací jádra
        precedenčního parseru - můžeš jimi aktualizovat své soubory. Snad jsem
        všechny záznamy a komentáře doplnil správně; myslím, že jsem však
        neudělal doxygen komentář k funkci parser_execute_directive.
        Zkontroluješ, že jsem někde na nic nezapomněl? Nějaké kontroly jsem do
        kódu přidal, avšak pokud myslíš, že by se někde hodila další, klidně
        ji doplň.

        parser_execute_directive dostane kromě kontextu a stacku i příznak,
        zda má využít specifikovaný terminál, poté argument s terminálem
        (který se ignoruje, pokud je příznak false) a sémantický pointer,
        který se má vložit do záznamu, pokud by nějaký záznam měl být vložen.

        Funkce prakticky implementuje provedení direktiv. Pokud není
        specifikovaný terminál, funkce použije ten vstupní, a rovnou zavolá
        další, pokud to tedy direktiva přikazuje. Pokud je terminál
        specifikován, pak se na vstupní token nesahá. Sémantickou anotaci
        nakonec dává none, protože podle mě nebude potřeba, a i kdyby
        náhodou, tak se doplní později.

        No a jádro parseru jsem napsal tak, jak bych ti ho stejně zdlouhavě
        popsal textem. Nejdřív připraví stack a přízaky, potom zpracuje
        specifikovaný vstupní terminál, jako kdyby byl vstupní, následně
        probíhá hlavní smyčka parseru, dokud není detekován ukončovač.
        Pokud je na vstupu if, tak se zpracuje ve zvláštní větvi, která
        převezme odpovědnost za celý stav parseru (zajistí správnou změnu
        redukční značky, příznaku parser_running i prediktoru; bude u nich
        totiž záviset, jakým způsobem byl ternární if zpracován (pravidlo 53
        nebo 54)). Jinak se provede direktiva a vyhodnotí prediktor.

        Při ukončování parseru se opět provádí kroky dle nařízení
        precedenční tabulky, avšak místo vstupního token je použit
        TOKEN_DOLLAR, protože ukončovač se nesmí změnit - je to již první
        token vyšší instance precedenčního parseru. Precedenční tabulka při
        správném návrhu pro tento terminál přikazuje pouze redukce. Na konci
        se ověří, že stack skončil tak, jak je předpokládané ($ E) a vrátí se
        sémantický pointer z E (vytvořený podstrom, až ho callbacky budou
        skládat).

        Větev pro ternární if ještě není hotová, stejně tak speciální akci
        pro preprocessing (a dokumentaci k ní) doplníme za chvíli.
*/

/**
 * @brief Provede direktivu precedencni tabulky.
 *
 * Najde terminal nejblize vrcholu zasobniku (parser_top_terminal) a vyhleda
 * direktivu v precedencni tabulce pro dvojici tento terminal a vstupni
 * terminal. Podle direktivy:
 * - DIRECTIVE_REDUCTION_EXECUTE: provede redukci (parser_execute_reduction),
 *   vstup se nemeni.
 * - DIRECTIVE_REDUCTION_MARKER: nastavi znacku redukce u terminalu nejblize
 *   vrcholu a pak postupuje jako u DIRECTIVE_SHIFT_SYMBOL.
 * - DIRECTIVE_SHIFT_SYMBOL: vlozi vstupni terminal na zasobnik se
 *   semantickym ukazatelem a anotaci CONTEXT_NONE. Pokud neni pouzit
 *   specifikovany terminal, posune se take vstup (parser_advance).
 *
 * Neplatna kombinace terminalu (DIRECTIVE_ERROR) skonci chybou uz pri
 * vyhledani v precedencni tabulce.
 *
 * @param ctx Spolecny kontext parseru, vstupni token je uz nacteny.
 * @param stack Zasobnik precedencniho parseru.
 * @param use_specified_terminal Je-li true, pouzije se misto vstupniho tokenu
 *        terminal v argumentu terminal a vstup se nemeni.
 * @param terminal Terminal pouzity misto vstupniho tokenu, ignoruje se, je-li
 *        use_specified_terminal false.
 * @param semantic_pointer Semanticky ukazatel noveho zaznamu, pokud se nejaky
 *        vklada na zasobnik.
 * @return True, pokud byl spotrebovan vstupni token (vstup byl posunut), jinak
 *         false (redukce nebo pouzit specifikovany terminal).
 */
bool parser_execute_directive(ParserContext *ctx, Stack *stack,
                              bool use_specified_terminal, TokenType terminal,
                              void *semantic_pointer);

/*
    Prompt pro prejmenovani ReductionResult na AnnotatedSymbol je v souboru
    semantic_types.h.
    Prompt pro funkci precedence_parser je nize.
*/

/*
    Prompt pro deklaraci a prazdnou hlavicku funkce precedence_parser. Stejny
    prompt zpusobil zmeny take v precedence_parser.c, jeho zneni je pouze zde.

    Prompt pro Claude:
        Dobrá, nyní asi můžeme přejít k samotnému jádru výrazového parseru.

        Expr *precedence_parser(ParserContext *ctx, ReductionResult
        *starting_stack_entry);

        Precedenční parser obdrží parser context se společným vstupním tokenem
        ze scanneru a také volitelný počáteční stack entry, který se využije
        pouze ve speciální situaci jádrových pravidel 32 a 33 LL gramatiky -
        rozlišení přiřazení od výrazu, který začíná ID. starting_stack_entry
        tedy ve většině případů bude NULL. Parser vrací ukazatel na vytvořený
        podstrom.

        Stavba stromu však ještě probíhat nebude, proto prozatím budeme vracet
        NULL (protože callbacky do všech redukcí prozatím dají NULL jako
        sémantický pointer).

        Víš ty co, možná ho zkusím napsat ručně, ať nezakrním v psaní kódu.
        Vytvoř pro něj tedy jen deklaraci a prázdnou hlavičku funkce v
        precedence_parser.*
*/

/**
 * @brief Jadro precedencniho parseru vyrazu.
 *
 * Zpracuje vyraz ze vstupniho proudu sdileneho pres ctx a vrati ukazatel na
 * vytvoreny podstrom. Pri chybe ukonci program pres error_exit. Funkce si
 * vytvori vlastni zasobnik, takze se muze rekurzivne volat (napr. pro
 * ternarni if).
 *
 * Volitelny pocatecni zaznam se pouziva jen ve specialnim pripade jadrovych
 * pravidel 32 a 33 LL gramatiky, tedy pri rozliseni prirazeni od vyrazu
 * zacinajiciho ID, kdy je ID uz zpracovano LL parserem. Ve vetsine pripadu je
 * NULL.
 *
 * Stavba stromu zatim neprobiha, funkce prozatim vraci NULL, protoze NULL vraci i callbacky.
 *
 * @param ctx Spolecny kontext parseru, vstupni token je uz nacteny.
 * @param starting_terminal Volitelny pocatecni zaznam zasobniku (symbol,
 *        semanticky ukazatel a anotace), nebo NULL.
 * @return Ukazatel na vytvoreny podstrom vyrazu, prozatim NULL.
 */
Expr *precedence_parser(ParserContext *ctx, AnnotatedSymbol *starting_terminal);

#endif // PRECEDENCE_PARSER_H
