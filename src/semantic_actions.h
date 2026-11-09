/*
    IFJ projekt 2026

    semantic_actions.h

    Puvodni autor: xbalaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - SemanticContext a SemanticCallback presunuty do
          semantic_types.h.
        - xbalaka00 - pridany semanticke akce pro pravidla 12, 27 a 34.

    Tento soubor obsahuje deklarace semantickych akci pravidel LL parseru.
    Akce stavi AST, semanticke kontroly (napr. kompatibilita typu) probihaji
    az nad kompletnim AST. Datove typy akci a prompty, ze kterych vznikly,
    jsou v semantic_types.h.
*/

#ifndef SEMANTIC_ACTIONS_H
#define SEMANTIC_ACTIONS_H

#include "semantic_types.h"

/*
    Prompt pro semanticke akce je v souboru LL_table.c.
*/

/**
 * @brief Semanticka akce pravidla 12 (navratovy typ je).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_optional_type_present(SemanticContext *ctx,
                                           void *semantic_pointer,
                                           Stack *stack);

/**
 * @brief Semanticka akce pravidla 27 (prikaz deklarace).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_declaration_command(SemanticContext *ctx,
                                         void *semantic_pointer, Stack *stack);

/**
 * @brief Semanticka akce pravidla 34 (prikaz return).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_return_command(SemanticContext *ctx,
                                    void *semantic_pointer, Stack *stack);

#endif // SEMANTIC_ACTIONS_H
