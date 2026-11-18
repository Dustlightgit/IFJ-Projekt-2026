/*
    IFJ projekt 2026

    semantic_actions.h

    Puvodni autor: xbalaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - SemanticContext a SemanticCallback presunuty do
          semantic_types.h.
        - xbalaka00 - pridany semanticke akce pro pravidla 12, 27 a 34.
        - xbalaka00 - pridany semanticke akce pro vsechna ostatni pravidla
          gramatiky se semantickou akci.
        - xbalaka00 - odstranena akce pravidla 32 (nyni v jadre parseru).
        - xbalaka00 - odstranena akce pravidla 12 (testovaci pozustatek,
          pravidlo je staticke).
        - xbalaka00 - callbacky prijimaji kontextovou anotaci
          (ContextAnnotation).
        - xbalaka00 - callbacky prijimaji cislo radku.
        - xbalaka00 - akce pravidla 31 anotuje ID a predava ukazatel
          neterminalu <IDcommand>.
        - xbalaka00 - funkce ast_attach_expr presunuta do special_actions.h
          a prejmenovana na special_action_expr (prompt je tam).

    Tento soubor obsahuje deklarace semantickych akci pravidel LL parseru.
    Akce stavi AST, semanticke kontroly (napr. kompatibilita typu) probihaji
    az nad kompletnim AST. Datove typy akci a prompty, ze kterych vznikly,
    jsou v semantic_types.h.
*/

#ifndef SEMANTIC_ACTIONS_H
#define SEMANTIC_ACTIONS_H

#include "semantic_types.h"

/*
    Prompt pro semanticke akce je v souboru LL_table.c. Prompt pro doplneni
    akci vsech pravidel je v souboru semantic_actions.c, prompt pro aktualizaci
    dle nove verze gramatiky a prompt pro upravu komentaru na ceskou
    terminologii v souboru production.c. Prompt pro vyplneni LL tabulky
    a odstraneni testovacich akci je v souboru LL_table.c. Prompty pro predani
    kontextove anotace a cisla radku callbackum jsou v souboru
    semantic_types.h. Prompt pro chovani jadra pro <IDcommand>, anotaci CONTEXT_ID_COMMAND
    a specialni akci pro ID je v souboru predictive_parser.c. Prompt pro
    presun funkce ast_attach_expr do special_actions.h a jeji prejmenovani na
    special_action_expr je v souboru special_actions.h.
*/

/**
 * @brief Semanticka akce pravidla 0 (prolog).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_prolog(SemanticContext *ctx, void *semantic_pointer,
                            ContextAnnotation annotation, int line,
                            Stack *stack);

/**
 * @brief Semanticka akce pravidla 3 (deklarace funkce).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_function_definition(SemanticContext *ctx,
                                         void *semantic_pointer,
                                         ContextAnnotation annotation, int line,
                                         Stack *stack);

/**
 * @brief Semanticka akce pravidla 4 (anotace main).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_main_annotation(SemanticContext *ctx,
                                     void *semantic_pointer,
                                     ContextAnnotation annotation, int line,
                                     Stack *stack);

/**
 * @brief Semanticka akce pravidla 8 (pridani parametru).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_param_add(SemanticContext *ctx, void *semantic_pointer,
                               ContextAnnotation annotation, int line,
                               Stack *stack);

/**
 * @brief Semanticka akce pravidla 14 (typ integer).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_type_int(SemanticContext *ctx, void *semantic_pointer,
                              ContextAnnotation annotation, int line,
                              Stack *stack);

/**
 * @brief Semanticka akce pravidla 15 (typ double).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_type_double(SemanticContext *ctx, void *semantic_pointer,
                                 ContextAnnotation annotation, int line,
                                 Stack *stack);

/**
 * @brief Semanticka akce pravidla 16 (typ string).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_type_string(SemanticContext *ctx, void *semantic_pointer,
                                 ContextAnnotation annotation, int line,
                                 Stack *stack);

/**
 * @brief Semanticka akce pravidla 17 (typ unit).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_type_unit(SemanticContext *ctx, void *semantic_pointer,
                               ContextAnnotation annotation, int line,
                               Stack *stack);

/**
 * @brief Semanticka akce pravidla 18 (typ boolean).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_type_boolean(SemanticContext *ctx, void *semantic_pointer,
                                  ContextAnnotation annotation, int line,
                                  Stack *stack);

/**
 * @brief Semanticka akce pravidla 19 (typ list).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_type_list(SemanticContext *ctx, void *semantic_pointer,
                               ContextAnnotation annotation, int line,
                               Stack *stack);

/**
 * @brief Semanticka akce pravidla 22 (blok a prvni prikaz).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_block(SemanticContext *ctx, void *semantic_pointer,
                           ContextAnnotation annotation, int line,
                           Stack *stack);

/**
 * @brief Semanticka akce pravidla 27 (prikaz deklarace).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_declaration_command(SemanticContext *ctx,
                                         void *semantic_pointer,
                                         ContextAnnotation annotation, int line,
                                         Stack *stack);

/**
 * @brief Semanticka akce pravidla 28 (deklarace nemodifikovatelne promenne).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_val(SemanticContext *ctx, void *semantic_pointer,
                         ContextAnnotation annotation, int line, Stack *stack);

/**
 * @brief Semanticka akce pravidla 31 (prikaz zacinajici id).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Terminalu ID priradi anotaci CONTEXT_ID_COMMAND, aby ho zpracovala
 * special_action_id, a neterminalu <IDcommand> preda svuj semanticky ukazatel
 * (kam ulozit prirazeni nebo vyraz). Dalsi atributy stavba stromu nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_id_command(SemanticContext *ctx, void *semantic_pointer,
                                ContextAnnotation annotation, int line,
                                Stack *stack);

/**
 * @brief Semanticka akce pravidla 34 (prikaz return).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_return_command(SemanticContext *ctx,
                                    void *semantic_pointer,
                                    ContextAnnotation annotation, int line,
                                    Stack *stack);

/**
 * @brief Semanticka akce pravidla 37 (cyklus while).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_while_command(SemanticContext *ctx, void *semantic_pointer,
                                   ContextAnnotation annotation, int line,
                                   Stack *stack);

/**
 * @brief Semanticka akce pravidla 38 (cyklus for).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_for_command(SemanticContext *ctx, void *semantic_pointer,
                                 ContextAnnotation annotation, int line,
                                 Stack *stack);

/**
 * @brief Semanticka akce pravidla 39 (jednoradkova hlavicka for).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_for_header_inline(SemanticContext *ctx,
                                       void *semantic_pointer,
                                       ContextAnnotation annotation, int line,
                                       Stack *stack);

/**
 * @brief Semanticka akce pravidla 40 (viceradkova hlavicka for).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_for_header_block(SemanticContext *ctx,
                                      void *semantic_pointer,
                                      ContextAnnotation annotation, int line,
                                      Stack *stack);

/**
 * @brief Semanticka akce pravidla 41 (dalsi straz).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_guard_next(SemanticContext *ctx, void *semantic_pointer,
                                ContextAnnotation annotation, int line,
                                Stack *stack);

/**
 * @brief Semanticka akce pravidla 47 (podminka).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_if_command(SemanticContext *ctx, void *semantic_pointer,
                                ContextAnnotation annotation, int line,
                                Stack *stack);

/**
 * @brief Semanticka akce pravidla 48 (nepovinna cast else).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_optional_else_present(SemanticContext *ctx,
                                           void *semantic_pointer,
                                           ContextAnnotation annotation,
                                           int line, Stack *stack);

/**
 * @brief Semanticka akce pravidla 50 (ternarni podminka).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_ternary_if(SemanticContext *ctx, void *semantic_pointer,
                                ContextAnnotation annotation, int line,
                                Stack *stack);

/**
 * @brief Semanticka akce pravidla 53 (odsazena vetev else).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param ctx Semanticky kontext.
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void semantic_action_else_ternary_if_indented(SemanticContext *ctx,
                                              void *semantic_pointer,
                                              ContextAnnotation annotation,
                                              int line, Stack *stack);

// Semanticka akce pravidla 59
void semantic_action_else_or_command_end_else(SemanticContext *ctx,
                                              void *semantic_pointer,
                                              ContextAnnotation annotation,
                                              int line, Stack *stack);

void semantic_action_test_expr(SemanticContext *ctx, void *semantic_pointer,
                               ContextAnnotation annotation, int line,
                               Stack *stack);

Block *target_block(void *ptr, ContextAnnotation annotation, int line);

#endif // SEMANTIC_ACTIONS_H
