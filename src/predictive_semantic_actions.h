/*
    IFJ projekt 2026

    xbalaka00 Artur Balak
    xnovacm00 Michal Novacek

    predictive_semantic_actions.h

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
        - xnovacm00 - pridany deklarace pomocnych funkci pro stavbu AST
        - xbalaka00 - funkce ast_attach_expr presunuta do special_actions.h
          a prejmenovana na special_action_expr (prompt je tam).
        - xbalaka00 - funkce ast_attach_expr presunuta do special_actions.c
          a prejmenovana na special_action_expr
        - xbalaka00 - funkce target_block a store_type presunuty do special_actions.c
          a prejmenovany na special_action_expr a special_action_store_type,
          sjednocen format cisel pravidel a prejmenovany akce na predictive_action_*
          Soubor byl prejmenovan na predictive_semantic_actions.h
        - xbalaka00 - odstranen SemanticContext z akci, protoze akce jej nevyuzivaly.

    Tento soubor obsahuje deklarace semantickych akci pravidel LL parseru.
    Akce stavi AST, semanticke kontroly (napr. kompatibilita typu) probihaji
    az nad kompletnim AST. Datove typy akci a prompty, ze kterych vznikly,
    jsou v semantic_types.h.
    Semanticke kontroly jsou prevazne zajistovany analyzatorem v souboru semantic_analyzer.*
*/

#ifndef PREDICTIVE_SEMANTIC_ACTIONS_H
#define PREDICTIVE_SEMANTIC_ACTIONS_H

#include "semantic_types.h"

/*
    Prompt pro semanticke akce je v souboru LL_table.c. Prompt pro doplneni
    akci vsech pravidel je v souboru precedence_semantic_actions.c, prompt pro aktualizaci
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
 * @brief Semanticka akce pravidla L0 (prolog).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_prolog(void *semantic_pointer,
                            ContextAnnotation annotation, int line,
                            Stack *stack);

/**
 * @brief Semanticka akce pravidla L3 (deklarace funkce).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_function_definition(void *semantic_pointer,
                                         ContextAnnotation annotation, int line,
                                         Stack *stack);

/**
 * @brief Semanticka akce pravidla L4 (anotace main).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_main_annotation(void *semantic_pointer,
                                     ContextAnnotation annotation, int line,
                                     Stack *stack);

/**
 * @brief Semanticka akce pravidla L8 (pridani parametru).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_param_add(void *semantic_pointer,
                               ContextAnnotation annotation, int line,
                               Stack *stack);

/**
 * @brief Semanticka akce pravidla L14 (typ integer).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_type_int(void *semantic_pointer,
                              ContextAnnotation annotation, int line,
                              Stack *stack);

/**
 * @brief Semanticka akce pravidla L15 (typ double).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_type_double(void *semantic_pointer,
                                 ContextAnnotation annotation, int line,
                                 Stack *stack);

/**
 * @brief Semanticka akce pravidla L16 (typ string).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_type_string(void *semantic_pointer,
                                 ContextAnnotation annotation, int line,
                                 Stack *stack);

/**
 * @brief Semanticka akce pravidla L17 (typ unit).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_type_unit(void *semantic_pointer,
                               ContextAnnotation annotation, int line,
                               Stack *stack);

/**
 * @brief Semanticka akce pravidla L18 (typ boolean).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_type_boolean(void *semantic_pointer,
                                  ContextAnnotation annotation, int line,
                                  Stack *stack);

/**
 * @brief Semanticka akce pravidla L19 (typ list).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_type_list(void *semantic_pointer,
                               ContextAnnotation annotation, int line,
                               Stack *stack);

/**
 * @brief Semanticka akce pravidla L22 (blok a prvni prikaz).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_block(void *semantic_pointer,
                           ContextAnnotation annotation, int line,
                           Stack *stack);

/**
 * @brief Semanticka akce pravidla L27 (prikaz deklarace).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_declaration_command(void *semantic_pointer,
                                         ContextAnnotation annotation, int line,
                                         Stack *stack);

/**
 * @brief Semanticka akce pravidla L28 (deklarace nemodifikovatelne promenne).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_val(void *semantic_pointer,
                         ContextAnnotation annotation, int line, Stack *stack);

/**
 * @brief Semanticka akce pravidla L31 (prikaz zacinajici id).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Terminalu ID priradi anotaci CONTEXT_ID_COMMAND, aby ho zpracovala
 * special_action_id, a neterminalu <IDcommand> preda svuj semanticky ukazatel
 * (kam ulozit prirazeni nebo vyraz). Dalsi atributy stavba stromu nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_id_command(void *semantic_pointer,
                                ContextAnnotation annotation, int line,
                                Stack *stack);

/**
 * @brief Semanticka akce pravidla L34 (prikaz return).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_return_command(void *semantic_pointer,
                                    ContextAnnotation annotation, int line,
                                    Stack *stack);

/**
 * @brief Semanticka akce pravidla L37 (cyklus while).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_while_command(void *semantic_pointer,
                                   ContextAnnotation annotation, int line,
                                   Stack *stack);

/**
 * @brief Semanticka akce pravidla L38 (cyklus for).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_for_command(void *semantic_pointer,
                                 ContextAnnotation annotation, int line,
                                 Stack *stack);

/**
 * @brief Semanticka akce pravidla L39 (jednoradkova hlavicka for).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_for_header_inline(void *semantic_pointer,
                                       ContextAnnotation annotation, int line,
                                       Stack *stack);

/**
 * @brief Semanticka akce pravidla L40 (viceradkova hlavicka for).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_for_header_block(void *semantic_pointer,
                                      ContextAnnotation annotation, int line,
                                      Stack *stack);

/**
 * @brief Semanticka akce pravidla L41 (dalsi straz).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_guard_next(void *semantic_pointer,
                                ContextAnnotation annotation, int line,
                                Stack *stack);

/**
 * @brief Semanticka akce pravidla L47 (podminka).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_if_command(void *semantic_pointer,
                                ContextAnnotation annotation, int line,
                                Stack *stack);

/**
 * @brief Semanticka akce pravidla L48 (nepovinna cast else).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_optional_else_present(void *semantic_pointer,
                                           ContextAnnotation annotation,
                                           int line, Stack *stack);

/**
 * @brief Semanticka akce pravidla L50 (ternarni podminka).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_ternary_if(void *semantic_pointer,
                                ContextAnnotation annotation, int line,
                                Stack *stack);

/**
 * @brief Semanticka akce pravidla L53 (odsazena vetev else).
 *
 * Vypise cislo pravidla na stderr a vlozi pravou stranu pravidla na zasobnik.
 * Atributy pro stavbu stromu zatim nepridava.
 *
 * @param semantic_pointer Ukazatel ulozeny u rozvadeneho neterminalu.
 * @param annotation Kontextova anotace ulozena u rozvadeneho neterminalu.
 * @param line Cislo radku aktualniho vstupniho terminalu.
 * @param stack Zasobnik parseru.
 */
void predictive_action_else_ternary_if_indented(void *semantic_pointer,
                                              ContextAnnotation annotation,
                                              int line, Stack *stack);

// Semanticka akce pravidla L59
void predictive_action_else_or_command_end_else(void *semantic_pointer,
                                              ContextAnnotation annotation,
                                              int line, Stack *stack);

#endif // PREDICTIVE_SEMANTIC_ACTIONS_H
