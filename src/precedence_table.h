/*
    IFJ projekt 2026

    precedence_table.h

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - upravena dokumentace precedence_table_lookup, vstupni
          token mimo tabulku je syntakticka chyba (prompt je v
          precedence_parser.h).
        - xbalaka00 - PRECEDENCE_TABLE_SIZE zvetsena z 27 na 29 kvuli novym
          tokenum TOKEN_PLUS_UNARY a TOKEN_MINUS_UNARY (prompt je v token.h).

    Tento soubor obsahuje deklarace pro precedencni tabulku, kterou bude
    vyuzivat precedencni parser vyrazu, a funkce pro prevod terminalu na
    index tabulky a pro vyhledani direktivy v ni.
*/

#ifndef PRECEDENCE_TABLE_H
#define PRECEDENCE_TABLE_H

#include <stddef.h>

#include "token.h"

/*
    Prompt pro vznik tohoto souboru a souboru precedence_table.c.

    Prompt pro Claude:
        Dobrá, nyní vytvoříme hlavičkový soubor pro precedenční tabulku.
        Vytvoř pár precedence_table.* s tím že .c prozatím bude obsahovat
        pouze prázdnou tabulku a případně implementaci pomocných funkcí.

        Soubor .h bude obsahovat následující deklarace:

        #define PRECEDENCE_TABLE_SIZE 27

        typedef enum {
              DIRECTIVE_ERROR = 0,     // prazdne pole precedencni tabulky
              DIRECTIVE_REDUCTION_MARKER,   // polozka < tabulky
              DIRECTIVE_REDUCTION_EXECUTE,  // polozka > tabulky
              DIRECTIVE_SHIFT_SYMBOL,      // polozka = tabulky
        } PrecedenceDirective;

        unsigned precedence_table_index(TokenType terminal, int line);

        PrecedenceDirective precedence_table_lookup(TokenType stackValue,
              TokenType inputValue, int line);

        extern const PrecedenceDirective
              precedence_table[PRECEDENCE_TABLE_SIZE][PRECEDENCE_TABLE_SIZE];

        Hlavičkový soubor vygeneruj s příslušnou dokumentací a v souladu s
        ostatními soubory, tedy např. aby precedence_table byla deklarována v
        souladu s LL_tabulkou apod. pokud jsem někde zapomněl klíčové slovo
        nebo tak. Případně uprav i typ unsigned na vhodnější, pokud to uznáš
        za vhodné.

        Funkce budou dělat následující:

        * precedence_table_index - podle tokenu na vstupu vrátí index do
          tabulky. Nejprve zvaliduje token přes funkci, kterou jsme vytvořili
          v minulém kroku. Pro terminály mezi TOKEN_LL_TERM_COUNT a
          TOKEN_EXPR_TERM_DELIMITER půjde použít jednoduchý přepočet, který
          při tokenu větším než TOKEN_EXPRESSION_BEGINNING_DELIMITER odečte
          jedničku, a potom odečte TOKEN_LL_TERM_COUNT a případně další
          jedničku tak, aby tyto tokeny byly v rozsahu 0 až n. Ostatní
          tokeny, které jsou intexátory precedenční tabulky, ale zároveň jsou
          někde zamíchané mezi indexátory LL tabulky, jako například kulaté
          závorky, dostanou přiřazený index z rozsahu n+1 až 27 přes switch
          case. Token, který není indexátorem precedenční tabulky způsobí
          interní chybu. Indexátory jsou následující terminály: List ( ) , .
          ID INT_LIT DOUBLE_LIT STRING_LIT true false + - * / ++ < <= > >= ==
          != ! && || ID_OPERATOR $
        * precedence_table_lookup zavolá výše uvedenou funkci na oba tokeny,
          zaindexuje s hodnotami precedenční tabulku, pro DIRECTIVE_ERROR
          rovnou vyhodí SYNTAX_ERROR s příslušnou zprávou precedenčního
          parseru, že kombinace terminálů není platná a výpisem použitých
          indexů, a vrátí vytaženou direktivu. Direktiva je to proto, že
          přikáže precedenčnímu parseru, co má udělat za akci.
*/

/**
 * @brief Pocet radku i sloupcu precedencni tabulky.
 *
 * Odpovida poctu terminalu vyrazove analyzy, ktere indexuji precedencni
 * tabulku (viz TokenType v token.h).
 */
#define PRECEDENCE_TABLE_SIZE 29

/**
 * @brief Direktiva precedencni tabulky.
 *
 * Prikazuje precedencnimu parseru, jakou akci ma provest.
 */
typedef enum {
  DIRECTIVE_ERROR = 0,         ///< Prazdne pole tabulky, kombinace neni platna
  DIRECTIVE_REDUCTION_MARKER,  ///< Polozka <, vlozeni znacky redukce
  DIRECTIVE_REDUCTION_EXECUTE, ///< Polozka >, provedeni redukce
  DIRECTIVE_SHIFT_SYMBOL,      ///< Polozka =, presun vstupniho symbolu
} PrecedenceDirective;

/**
 * @brief Precedencni tabulka indexovana terminalem na zasobniku (radek)
 * a terminalem na vstupu (sloupec).
 *
 * Indexy terminalu urcuje funkce precedence_table_index. Polozky bez zaznamu
 * jsou DIRECTIVE_ERROR (hodnota 0).
 */
extern const PrecedenceDirective PRECEDENCE_TABLE[PRECEDENCE_TABLE_SIZE]
                                                 [PRECEDENCE_TABLE_SIZE];

/**
 * @brief Vrati index terminalu do precedencni tabulky.
 *
 * Nejprve overi token funkci token_type_validate (neplatny typ je interni
 * chyba). Terminaly mezi TOKEN_LL_TERM_COUNT a TOKEN_EXPR_TERM_DELIMITER
 * dostanou index vypoctem z jejich hodnoty v enumu a zabiraji souvisly rozsah
 * od 0. Ostatni terminaly precedencni tabulky, ktere jsou v enumu
 * TokenType mezi indexatory LL tabulky (napr. zavorky), dostanou index za
 * timto rozsahem. Token, ktery precedencni tabulku neindexuje, zpusobi
 * interni chybu (ERR_INTERNAL).
 *
 * @param terminal Typ terminalu.
 * @param line Cislo radku pro chybova hlaseni, <= 0 pokud neni znam.
 * @return Index v rozsahu 0 az PRECEDENCE_TABLE_SIZE - 1.
 */
size_t precedence_table_index(TokenType terminal, int line);

/**
 * @brief Vyhleda direktivu v precedencni tabulce.
 *
 * Zaindexuje tabulku terminalem na zasobniku (ten je vzdy indexem tabulky,
 * jinak jde o internich chybu) a terminalem na vstupu. Vstupni terminal, ktery
 * precedencni tabulku neindexuje, je chyba ve vstupnim programu a program
 * skonci syntaktickou chybou (ERR_SYNTAX). Pokud je pole tabulky prazdne
 * (DIRECTIVE_ERROR), kombinace terminalu neni platna a program skonci take
 * syntaktickou chybou (ERR_SYNTAX) s vypisem pouzitych indexu.
 *
 * @param stack_terminal Terminal na zasobniku (radek tabulky).
 * @param input_terminal Terminal na vstupu (sloupec tabulky).
 * @param line Cislo radku vstupniho tokenu pro chybova hlaseni.
 * @return Direktiva, ktera neni DIRECTIVE_ERROR.
 */
PrecedenceDirective precedence_table_lookup(TokenType stack_terminal,
                                            TokenType input_terminal, int line);

#endif // PRECEDENCE_TABLE_H
