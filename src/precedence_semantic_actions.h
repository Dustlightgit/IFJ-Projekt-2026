/*
    IFJ projekt 2026

    xbalaka00 Artur Balak

    precedence_semantic_actions.h

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - dokumentace akci E7 a E8 upravena, operatorem handle je
          TOKEN_PLUS_UNARY resp. TOKEN_MINUS_UNARY (prompt je v token.h).
        - xbalaka00 - pridany deklarace akci pravidel E32 a E33, akce puvodniho
          pravidla E32 prejmenovana na ...multiple_args (nyni E34), pravidla
          E33 az E39 precislovana na E35 az E41 (prompt je v
          precedence_semantic_actions.c).
        - xbalaka00 - dokumentace akci upravena podle napojeni na AST (prompt je
          v precedence_semantic_actions.c).
        - xbalaka00 - pridano pravidlo E30 (clenska notace bez parametru),
          pravidla E30 az E41 precislovana na E31 az E42 (prompt je v
          precedence_semantic_actions.c).

    Tento soubor obsahuje deklarace semantickych akci pravidel precedencniho
    parseru. Akce odpovidaji pravidlum z redukcni tabulky (E0 az E42), vypisuji
    cislo pravidla, stavi uzly AST z podstromu ulozenych na zasobniku a urcuji
    neterminal, na ktery se handle redukuje. Datove typy akci jsou v semantic_types.h, prompt, ze ktereho
    soubor vznikl, je v souboru precedence_semantic_actions.c.
    Semanticke kontroly jsou prevazne zajistovany analyzatorem v souboru semantic_analyzer.*
*/

#ifndef PRECEDENCE_SEMANTIC_ACTIONS_H
#define PRECEDENCE_SEMANTIC_ACTIONS_H

#include "semantic_types.h"

/*
    Prompt pro vznik tohoto souboru a souboru precedence_semantic_actions.c je
    v souboru precedence_semantic_actions.c.
*/

/**
 * @brief Semanticka akce pravidla E0 (Term ID).
 *
 * Pravidlo: E <= ID
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E.
 * Terminal s atributem je na zasobniku uz uzel AST (EXPR_VARIABLE) vytvoreny
 * akci special_action_precedence_preprocessing, akce proto jen preda jeho
 * ukazatel jako vyrazovy podstrom E. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_term_id(const Stack *stack, int line,
                               AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E1 (Term integer).
 *
 * Pravidlo: E <= INT_LIT
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E.
 * Terminal s atributem je na zasobniku uz uzel AST (EXPR_LITERAL typu int)
 * vytvoreny akci special_action_precedence_preprocessing, akce proto jen
 * preda jeho ukazatel jako vyrazovy podstrom E. Anotace vysledku je
 * CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_term_int(const Stack *stack, int line,
                                AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E2 (Term double).
 *
 * Pravidlo: E <= DOUBLE_LIT
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E.
 * Terminal s atributem je na zasobniku uz uzel AST (EXPR_LITERAL typu
 * double) vytvoreny akci special_action_precedence_preprocessing, akce proto
 * jen preda jeho ukazatel jako vyrazovy podstrom E. Anotace vysledku je
 * CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_term_double(const Stack *stack, int line,
                                   AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E3 (Term string).
 *
 * Pravidlo: E <= STRING_LIT
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E.
 * Terminal s atributem je na zasobniku uz uzel AST (EXPR_LITERAL typu
 * string) vytvoreny akci special_action_precedence_preprocessing, akce proto
 * jen preda jeho ukazatel jako vyrazovy podstrom E. Anotace vysledku je
 * CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_term_string(const Stack *stack, int line,
                                   AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E4 (Term true).
 *
 * Pravidlo: E <= true
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Klicove
 * slovo atribut nema, akce proto vytvori literal pres expr_new_bool(true).
 * Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_term_true(const Stack *stack, int line,
                                 AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E5 (Term false).
 *
 * Pravidlo: E <= false
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Klicove
 * slovo atribut nema, akce proto vytvori literal pres expr_new_bool(false).
 * Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_term_false(const Stack *stack, int line,
                                  AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E6 (Term unit).
 *
 * Pravidlo: E <= ( )
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * literal unit pres expr_new_unit. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_term_unit(const Stack *stack, int line,
                                 AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E7 (Operator + unarni).
 *
 * Pravidlo: E <= + E (operator je terminal TOKEN_PLUS_UNARY)
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * unarni uzel (expr_new_unary, UNOP_PLUS) z operandu pod operatorem.
 * Operator atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_unary_plus(const Stack *stack, int line,
                                  AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E8 (Operator - unarni).
 *
 * Pravidlo: E <= - E (operator je terminal TOKEN_MINUS_UNARY)
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * unarni uzel (expr_new_unary, UNOP_NEG) z operandu pod operatorem. Operator
 * atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_unary_minus(const Stack *stack, int line,
                                   AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E9 (Operator ! unarni).
 *
 * Pravidlo: E <= ! E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * unarni uzel (expr_new_unary, UNOP_NOT) z operandu pod operatorem. Operator
 * atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_unary_not(const Stack *stack, int line,
                                 AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E10 (Operator *).
 *
 * Pravidlo: E <= E * E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * binarni uzel (expr_new_binary, BINOP_MUL) z leveho a praveho operandu.
 * Operator atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_multiply(const Stack *stack, int line,
                                AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E11 (Operator /).
 *
 * Pravidlo: E <= E / E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * binarni uzel (expr_new_binary, BINOP_DIV) z leveho a praveho operandu.
 * Operator atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_divide(const Stack *stack, int line,
                              AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E12 (Operator +).
 *
 * Pravidlo: E <= E + E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * binarni uzel (expr_new_binary, BINOP_ADD) z leveho a praveho operandu.
 * Operator atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_add(const Stack *stack, int line,
                           AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E13 (Operator -).
 *
 * Pravidlo: E <= E - E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * binarni uzel (expr_new_binary, BINOP_SUB) z leveho a praveho operandu.
 * Operator atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_subtract(const Stack *stack, int line,
                                AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E14 (Operator ++).
 *
 * Pravidlo: E <= E ++ E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * binarni uzel (expr_new_binary, BINOP_CONCAT_LIST) z leveho a praveho
 * operandu. Operator atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_concat(const Stack *stack, int line,
                              AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E15 (Operator <).
 *
 * Pravidlo: E <= E < E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * binarni uzel (expr_new_binary, BINOP_LT) z leveho a praveho operandu.
 * Operator atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_less(const Stack *stack, int line,
                            AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E16 (Operator <=).
 *
 * Pravidlo: E <= E <= E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * binarni uzel (expr_new_binary, BINOP_LE) z leveho a praveho operandu.
 * Operator atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_less_equal(const Stack *stack, int line,
                                  AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E17 (Operator >).
 *
 * Pravidlo: E <= E > E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * binarni uzel (expr_new_binary, BINOP_GT) z leveho a praveho operandu.
 * Operator atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_greater(const Stack *stack, int line,
                               AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E18 (Operator >=).
 *
 * Pravidlo: E <= E >= E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * binarni uzel (expr_new_binary, BINOP_GE) z leveho a praveho operandu.
 * Operator atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_greater_equal(const Stack *stack, int line,
                                     AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E19 (Operator ==).
 *
 * Pravidlo: E <= E == E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * binarni uzel (expr_new_binary, BINOP_EQ) z leveho a praveho operandu.
 * Operator atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_equal(const Stack *stack, int line,
                             AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E20 (Operator !=).
 *
 * Pravidlo: E <= E != E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * binarni uzel (expr_new_binary, BINOP_NE) z leveho a praveho operandu.
 * Operator atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_not_equal(const Stack *stack, int line,
                                 AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E21 (Operator &&).
 *
 * Pravidlo: E <= E && E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * binarni uzel (expr_new_binary, BINOP_AND) z leveho a praveho operandu.
 * Operator atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_and(const Stack *stack, int line,
                           AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E22 (Operator ||).
 *
 * Pravidlo: E <= E || E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * binarni uzel (expr_new_binary, BINOP_OR) z leveho a praveho operandu.
 * Operator atribut nema. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_or(const Stack *stack, int line,
                          AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E23 (Volani funkce bez parametru).
 *
 * Pravidlo: E <= ID ( )
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Z
 * pomocneho uzlu ID vezme jmeno, vytvori volani funkce bez argumentu
 * (expr_new_call) a pomocny uzel uvolni. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_call_no_args(const Stack *stack, int line,
                                    AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E24 (Volani funkce s parametrem).
 *
 * Pravidlo: E <= ID ( E )
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Z
 * pomocneho uzlu ID vezme jmeno, vytvori volani funkce (expr_new_call) a
 * prida mu argument. Indexace seznamu ma stejny tvar, proto se vzdy vytvari
 * volani funkce a rozlisi ho semanticky analyzator. Anotace vysledku je
 * CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_call_one_arg(const Stack *stack, int line,
                                    AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E25 (Volani funkce s vice parametry).
 *
 * Pravidlo: E <= ID ( ARGS )
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * volani funkce se jmenem z pomocneho uzlu ID. ARGS je na zasobniku pomocny
 * uzel EXPR_LIST_LITERAL (viz E36), jehoz prvky se presunou do volani
 * (expr_call_take_args), schranka se uvolni. Anotace vysledku je
 * CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_call_multiple_args(const Stack *stack, int line,
                                          AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E26 (Prazdny seznam).
 *
 * Pravidlo: E <= List ( )
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * prazdny seznam (expr_new_list_literal). Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_list_empty(const Stack *stack, int line,
                                  AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E27 (Jednoprvkovy seznam).
 *
 * Pravidlo: E <= List ( E )
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * seznam (expr_new_list_literal) s jednim prvkem. Anotace vysledku je
 * CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_list_one_element(const Stack *stack, int line,
                                        AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E28 (Viceprvkovy seznam).
 *
 * Pravidlo: E <= List ( ARGS )
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. ARGS je
 * na zasobniku pomocny uzel EXPR_LIST_LITERAL (viz E36), ktery je rovnou
 * hledanym seznamem, akce jej proto jen preda jako vyrazovy podstrom E.
 * Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_list_multiple_elements(const Stack *stack, int line,
                                              AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E29 (Clenska notace).
 *
 * Pravidlo: E <= E . ID
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * clen bez argumentu (expr_new_member), jmeno vezme z pomocneho uzlu ID,
 * ktery uvolni. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_member_access(const Stack *stack, int line,
                                     AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E30 (Clenska notace bez parametru).
 *
 * Pravidlo: E <= E . ID ( )
 *
 * Vypise cislo pravidla na stderr a skonci chybou ERR_UNDEFINED, protoze
 * v zvolenych rozsirenich neexistuje zadna clenska metoda bez parametru.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, akce ho nenastavuje (konci chybou).
 */
void precedence_action_method_no_args(const Stack *stack, int line,
                                      AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E31 (Clenska metoda s parametrem).
 *
 * Pravidlo: E <= E . ID ( E )
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * clen (expr_new_member) se jmenem z pomocneho uzlu ID a prida mu jeden
 * argument. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_method_one_arg(const Stack *stack, int line,
                                      AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E32 (Clenska metoda s vice parametry).
 *
 * Pravidlo: E <= E . ID ( ARGS )
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Vytvori
 * clen (expr_new_member) se jmenem z pomocneho uzlu ID. Prvky ARGS (pomocny
 * EXPR_LIST_LITERAL) presune do clenu (expr_member_take_args). Anotace
 * vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_method_multiple_args(const Stack *stack, int line,
                                            AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E33 (Staticka funkce seznamu bez parametru).
 *
 * Pravidlo: E <= List . ID ( )
 *
 * Vypise cislo pravidla na stderr. List.range ma v AST vlastni uzel (viz
 * E34) a obecna staticka funkce seznamu neexistuje, akce proto vzdy skonci
 * chybou ERR_ARGS (nespravny pocet parametru).
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_list_static_function_no_args(const Stack *stack,
                                                    int line,
                                                    AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E34 (Staticka funkce seznamu s parametrem).
 *
 * Pravidlo: E <= List . ID ( E )
 *
 * Vypise cislo pravidla na stderr. List.range ma v AST vlastni uzel (viz
 * E34) a obecna staticka funkce seznamu neexistuje, akce proto vzdy skonci
 * chybou ERR_ARGS (nespravny pocet parametru).
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_list_static_function_one_arg(const Stack *stack,
                                                    int line,
                                                    AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E35 (Staticka funkce seznamu s vice
 * parametry).
 *
 * Pravidlo: E <= List . ID ( ARGS )
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Jmeno v
 * ID musi byt presne range, jinak skonci chybou ERR_UNDEFINED. ARGS (pomocny
 * EXPR_LIST_LITERAL) musi mit 2 nebo 3 prvky, jinak skonci chybou ERR_ARGS.
 * Vytvori uzel expr_new_list_range (start, end, nepovinne step). Anotace
 * vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_list_static_function_multiple_args(
    const Stack *stack, int line, AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E36 (Tvorba seznamu parametru).
 *
 * Pravidlo: ARGS <= E , E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal ARGS.
 * Vytvori seznam argumentu, tedy pomocny uzel EXPR_LIST_LITERAL se dvema
 * prvky. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_args_create(const Stack *stack, int line,
                                   AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E37 (Konsolidace parametru).
 *
 * Pravidlo: ARGS <= ARGS , E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal ARGS.
 * Prida vyraz na konec seznamu argumentu (pomocneho uzlu EXPR_LIST_LITERAL),
 * ktery preda dale. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_args_append(const Stack *stack, int line,
                                   AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E38 (Likvidace koncove carky (za vyrazem)).
 *
 * Pravidlo: ARGS <= E ,
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal ARGS.
 * Koncova carka se zahodi, jediny vyraz se zabali do seznamu argumentu
 * (pomocneho uzlu EXPR_LIST_LITERAL), aby se ARGS na zasobniku dalo vzdy
 * poznat od vyrazu. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_args_trailing_comma_expr(const Stack *stack, int line,
                                                AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E39 (Likvidace koncove carky (za seznamem)).
 *
 * Pravidlo: ARGS <= ARGS ,
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal ARGS.
 * Koncova carka se zahodi, ukazatel na seznam argumentu se jen preda.
 * Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_args_trailing_comma_args(const Stack *stack, int line,
                                                AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E40 (Redukce uzavorkovanych vyrazu).
 *
 * Pravidlo: E <= ( E )
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Zavorky
 * se zahodi, ukazatel na vnitrni vyraz se jen preda. Zachova se i jeho
 * anotace, aby (1 to 5) by 2 fungovalo stejne jako 1 to 5 by 2. Anotace
 * vysledku je anotace vnitrniho vyrazu.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_parenthesized_expr(const Stack *stack, int line,
                                          AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E41 (Pripojeni else vetve).
 *
 * Pravidlo: E <= TIF E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. TIF je
 * rozpracovany uzel EXPR_IF bez else vetve. Akce z praveho vyrazu vytvori
 * blok (block_from_expr), pripoji ho jako else vetev (expr_if_set_else) a
 * uzel preda jako vyraz E. Anotace vysledku je CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_ternary_else(const Stack *stack, int line,
                                    AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E42 (Infixova metoda).
 *
 * Pravidlo: E <= E ID_OPERATOR E
 *
 * Vypise cislo pravidla na stderr a redukuje handle na neterminal E. Podle
 * jmena v ID_OPERATOR: "to" a "until" vytvori rozsah bez kroku
 * (expr_new_range) s anotaci CONTEXT_RANGE_WITHOUT_STEP; "by" doplni krok
 * (expr_range_set_step) k rozsahu v levem operandu, ktery musi mit tuto
 * anotaci, jinak skonci chybou ERR_OTHER; cokoliv jineho vytvori clen s
 * jednim argumentem (expr_new_member), jehoz platnost overi semanticky
 * analyzator. Anotace vysledku je CONTEXT_NONE, jen pro "to" a "until"
 * CONTEXT_RANGE_WITHOUT_STEP.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_infix_method(const Stack *stack, int line,
                                    AnnotatedSymbol *result);

#endif // PRECEDENCE_SEMANTIC_ACTIONS_H
