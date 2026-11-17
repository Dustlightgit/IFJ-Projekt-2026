/*
    IFJ projekt 2026

    precedence_semantic_actions.h

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - dokumentace akci E7 a E8 upravena, operatorem handle je
          TOKEN_PLUS_UNARY resp. TOKEN_MINUS_UNARY (prompt je v token.h).
        - xbalaka00 - pridany deklarace akci pravidel E32 a E33, akce puvodniho
          pravidla E32 prejmenovana na ...multiple_args (nyni E34), pravidla
          E33 az E39 precislovana na E35 az E41 (prompt je v
          precedence_semantic_actions.c).

    Tento soubor obsahuje deklarace semantickych akci pravidel precedencniho
    parseru. Akce odpovidaji pravidlum z redukcni tabulky (E0 az E41), zatim
    pouze vypisuji cislo pravidla a urcuji neterminal, na ktery se handle
    redukuje. Datove typy akci jsou v semantic_types.h, prompt, ze ktereho
    soubor vznikl, je v souboru precedence_semantic_actions.c.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
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
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_list_multiple_elements(const Stack *stack, int line,
                                              AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E29 (Clenska notace bez parametru).
 *
 * Pravidlo: E <= E . ID
 *
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_member_access(const Stack *stack, int line,
                                     AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E30 (Clenska metoda s parametrem).
 *
 * Pravidlo: E <= E . ID ( E )
 *
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_method_one_arg(const Stack *stack, int line,
                                      AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E31 (Clenska metoda s vice parametry).
 *
 * Pravidlo: E <= E . ID ( ARGS )
 *
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_method_multiple_args(const Stack *stack, int line,
                                            AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E32 (Staticka funkce seznamu bez parametru).
 *
 * Pravidlo: E <= List . ID ( )
 *
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_list_static_function_no_args(const Stack *stack,
                                                    int line,
                                                    AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E33 (Staticka funkce seznamu s parametrem).
 *
 * Pravidlo: E <= List . ID ( E )
 *
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_list_static_function_one_arg(const Stack *stack,
                                                    int line,
                                                    AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E34 (Staticka funkce seznamu s vice
 * parametry).
 *
 * Pravidlo: E <= List . ID ( ARGS )
 *
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_list_static_function_multiple_args(
    const Stack *stack, int line, AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E35 (Tvorba seznamu parametru).
 *
 * Pravidlo: ARGS <= E , E
 *
 * Vypise cislo pravidla na stderr a nastavi neterminal ARGS, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_args_create(const Stack *stack, int line,
                                   AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E36 (Konsolidace parametru).
 *
 * Pravidlo: ARGS <= ARGS , E
 *
 * Vypise cislo pravidla na stderr a nastavi neterminal ARGS, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_args_append(const Stack *stack, int line,
                                   AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E37 (Likvidace koncove carky (za vyrazem)).
 *
 * Pravidlo: ARGS <= E ,
 *
 * Vypise cislo pravidla na stderr a nastavi neterminal ARGS, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_args_trailing_comma_expr(const Stack *stack, int line,
                                                AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E38 (Likvidace koncove carky (za seznamem)).
 *
 * Pravidlo: ARGS <= ARGS ,
 *
 * Vypise cislo pravidla na stderr a nastavi neterminal ARGS, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_args_trailing_comma_args(const Stack *stack, int line,
                                                AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E39 (Redukce uzavorkovanych vyrazu).
 *
 * Pravidlo: E <= ( E )
 *
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_parenthesized_expr(const Stack *stack, int line,
                                          AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E40 (Pripojeni else vetve).
 *
 * Pravidlo: E <= TIF E
 *
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_ternary_else(const Stack *stack, int line,
                                    AnnotatedSymbol *result);

/**
 * @brief Semanticka akce pravidla E41 (Infixova metoda).
 *
 * Pravidlo: E <= E ID_OPERATOR E
 *
 * Vypise cislo pravidla na stderr a nastavi neterminal E, na ktery se
 * handle redukuje. Stavbu stromu zatim neprovadi, semanticky ukazatel
 * a kontextova anotace vysledku jsou NULL a CONTEXT_NONE.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu.
 * @param result Vystup, symbol a atributy noveho zaznamu na zasobniku.
 */
void precedence_action_infix_method(const Stack *stack, int line,
                                    AnnotatedSymbol *result);

#endif // PRECEDENCE_SEMANTIC_ACTIONS_H
