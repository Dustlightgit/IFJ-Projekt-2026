/*
    IFJ projekt 2026

    precedence_helpers.h

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        -

    Tento soubor obsahuje podpurne funkce jadra precedencniho parseru: hledani
    terminalu na zasobniku, sestaveni handle ze zasobniku, predikci operatoru
    a operandu a detekci ukonceni vyrazu. Jsou vyclenene z jadra, protoze jich
    je vice a vyuziva je jen jadro precedencniho parseru.
*/

#ifndef PRECEDENCE_HELPERS_H
#define PRECEDENCE_HELPERS_H

#include <stdbool.h>

#include "parser_common.h"
#include "reduction_table.h"
#include "stack.h"
#include "token.h"

/*
    Prompt pro podpurne funkce jadra precedencniho parseru (vznik tohoto
    souboru a souboru precedence_helpers.c) a pro opravu token_free pro
    TOKEN_ID_OPERATOR. Stejny prompt zpusobil zmeny take v token.h a token.c,
    jeho zneni je pouze zde. Nazev souboru zvolil Claude.

    Prompt pro Claude:
        Dobrá, už je snad připravená infrastruktura okolo. Připravil bych
        podpůrné funkce pro jádro. Jen přemýšlím, jestli je ponechat v kódu
        jádra stejně jako třeba parser_match - ta je v souboru jádra LL
        parseru proto, že to není sdílená akce pro oba parsery (ta by byla v
        parser_common) a využije ji konkrétně jen jádro LL parseru.
        Precedenční parser funkcí pravděpodobně bude mít více, takže můžeš
        rozhodnout, jestli je vyčlenit do samostatného souboru, nebo ne,
        podle jejich množství, ale možná to potřeba nebude.

        Bude potřeba funkce parser_top_terminal, která pomocí
        parser_stack_peek projde stack od vrcholu hlouběji, a vrátí ukazatel
        na první ParserStackEntry, který je terminálem. Tento záznam bude
        potom použit pro indexaxi precedenční tabulky, a v případě potřeby se
        do tohoto záznamu vepíše značka redukce. Funkce by nejspíše nikdy
        neměla selhat, protože na dně zásobníku bude vždy terminál $ neboli
        terminátor podobně jako v matematickém modelu, ale klidně přidej
        ošetření dna zásobníku na interní chybu.

        Dále bude potřeba funkce parser_build_handle, která dostane pointer
        na handle, který má sestavit (bude to asi prostě jen lokální
        proměnná v jádře); projde stack od vrcholu hlouběji až po první
        záznam s redukční značkou a zjistí jeho délku; pokud je delší než
        maximální délka handle, může rovnou vyhodit syntax error (ne interní,
        protože toto by mohlo nastat třeba i kvůli neštastnému pořadí symbolů
        na vstupu, alespoň myslím, ale je třeba také možné, že to
        precedenční tabulka je schopná vynutit... mám aspoň námět pro
        nastudování, ale asi tipuji, že v obecnosti to platit nebude). Pokud
        je délka v pořádku, pak seskládá handle v pořadí, v jakém ho bude
        očekávat redukční tabulka. Symbol na vrcholu zásobníku je vždy ten,
        který je nejvíc vlevo ve větné formě.

        Dále to bude pomocná funkce, která implementuje mechanismus
        predikce, jestli následuje operátor nebo operand, který v případě
        potřeby změní vstupní ID na ID_OPERATOR. Mechanismus je popsán v
        excelu, ale ve zkratce bude fungovat tak, že v jádře bude příznak
        (nejspíše bude stačit booleanový, ale jestli chceš, můžeš zavést i
        dvojhodnotový enum EXPECT_OPERAND a EXPECT_OPERATOR) predikce, který
        "předpoví", jestli příští symbol bude operátor, nebo operand. Jediné
        jeho využití je, že pokud je na vstupu ID a je očekáván operátor, tak
        se ID zamění za ID_OPERATOR a dále se s ním nakládá jako s tímto
        terminálem (tedy především bude mít odlišné záznamy v precedenční
        tabulce než ID).
        Funkce dostane ukazatel na vstupní token a současnou hodnotu příznaku,
        a vrátí novou hodnotu příznaku.
        Jak jsem tedy říkal, predikce operátoru a ID na vstupu způsobí, že se
        ID změní na ID_OPERATOR. Poté se nastaví nová predikce takto: Pokud je
        vstupní terminál  jeden z ( ID INT_LIT DOUBLE_LIT STRING_LIT true
        false, pak se předpoví operátor, jinak se předpoví operand. (Pro
        jistotu upřesním, že pokud je na vstupu ID, které bylo přepsáno jako
        ID_OPERATOR, nová predikce bude operand; pokud vstup byl ID který
        nebyl změněn, nový prediktor bude operátor (protože ID je
        "operandové")).
        Funkce se může jmenovat parser_evaluate_predictor.

        A do čtveřice všeho dobrého také funkce, která akorát detekuje
        ukončení výrazové analýzy. Dostane vstupní token, a vrátí true, pokud
        je to jeden z ukončovačů EOL ; do then else. Toto se uloží do
        příslušného příznaku.
*/

/**
 * @brief Predikce, co nasleduje na vstupu vyrazu.
 *
 * Pouziva se jen k "prebarveni" ID na ID_OPERATOR v kontextech, kdy je
 * ocekavan operator. Pravidla predikce jsou v excelu parser_formalisms.xlsx
 * (druhy list, tabulka prediktoru).
 */
typedef enum {
  EXPECT_OPERAND,  ///< Nasleduje operand, tedy zacatek podvyrazu
  EXPECT_OPERATOR, ///< Nasleduje operator
} PredictorValue;

/**
 * @brief Najde terminal nejblize vrcholu zasobniku.
 *
 * Prochazi zasobnik od vrcholu hloubeji pomoci parser_stack_peek a vrati
 * prvni zaznam, ktery je terminal. Tento zaznam se pouzije k indexaci
 * precedencni tabulky a v pripade potreby se do nej vepise znacka redukce.
 * Na dne zasobniku je vzdy terminal $, funkce proto nema selhat; pokud
 * zadny terminal nenajde, jde o internich chybu (ERR_INTERNAL).
 *
 * @param stack Zasobnik parseru.
 * @param line Cislo radku pro chybova hlaseni.
 * @return Ukazatel na zaznam s terminalem nejblize vrcholu, nikdy NULL.
 */
ParserStackEntry *parser_top_terminal(const Stack *stack, int line);

/**
 * @brief Sestavi handle ze zasobniku.
 *
 * Projde zasobnik od vrcholu hloubeji az po prvni zaznam se znackou redukce
 * (reduction_marker); tento zaznam do handle nepatri. Pokud je handle delsi
 * nez HANDLE_MAX_LENGTH, program skonci syntaktickou chybou (ERR_SYNTAX), pokud
 * se znacka na zasobniku vubec nenajde, jde o internich chybu (ERR_INTERNAL).
 * Symboly se do handle ukladaji v prirozenem poradi zleva doprava, tedy od
 * nejhloubeji ulozeneho symbolu (handle->symbols[0]) po symbol na vrcholu
 * zasobniku, v poradi, v jakem je ocekava redukcni tabulka. Prazdny handle
 * (znacka primo na vrcholu) funkce nekontroluje, resi ho reduction_table_lookup.
 *
 * @param stack Zasobnik parseru.
 * @param handle Handle, ktery se sestavi, nesmi byt NULL.
 * @param line Cislo radku pro chybova hlaseni.
 */
void parser_build_handle(const Stack *stack, SearchHandle *handle, int line);

/**
 * @brief Vyhodnoti predikci operatoru a operandu pro vstupni token.
 *
 * Pokud je na vstupu ID a je predpovidan operator, zmeni se typ tokenu na
 * TOKEN_ID_OPERATOR (atribut tokenu zustava). Nova predikce se pak urci
 * podle tokenu: po ), ID, INT_LIT, DOUBLE_LIT, STRING_LIT, true a false se
 * predpovida operator, jinak operand. ID prepsane na ID_OPERATOR je operator,
 * takze po nem nasleduje operand; nezmenene ID je operand, po nem nasleduje
 * operator.
 *
 * @param token Vstupni token, muze byt zmenen z TOKEN_ID na TOKEN_ID_OPERATOR.
 * @param predictor Soucasna hodnota predikce.
 * @return Nova hodnota predikce.
 */
PredictorValue parser_evaluate_predictor(Token *token,
                                         PredictorValue predictor);

/**
 * @brief Zjisti, zda je vstupni token ukoncovacem vyrazu.
 *
 * Ukoncovace jsou EOL, ;, do, then a else.
 *
 * @param token Vstupni token.
 * @return True, pokud token ukoncuje vyraz, jinak false.
 */
bool parser_is_expression_terminator(const Token *token);

#endif // PRECEDENCE_HELPERS_H
