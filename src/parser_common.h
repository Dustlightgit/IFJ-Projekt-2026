/*
    IFJ projekt 2026

    parser_common.h

    Puvodni autor: xbalaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - IDContextType (vcetne prejmenovani hodnot na
          ID_CONTEXT_*) presunut do semantic_types.h.
        - xbalaka00 - pridani funkci parser_stack_init, parser_stack_push,
          parser_stack_pop a parser_stack_top.

    Tento soubor definuje veci spolecne pro oba parsery: zaznam ukladany
    na zasobniku parseru, kontext sdileny mezi parsery a funkce pro praci
    se zasobnikem parseru.
*/

#ifndef PARSER_COMMON_H
#define PARSER_COMMON_H

#include "production.h"
#include "semantic_types.h"
#include "stack.h"

/*
    Prompt pro vychozi kostru souboru (IDContextType byl pozdeji presunut do
    semantic_types.h, kde je i prompt, ktery presun zpusobil).

    Prompt pro Claude:
        Tento soubor parser_common.h definuje věci společné pro oba parsery.
        Prozatím vím, že to bude záznam ukládaný ve stacku, a nějaký společný
        kontext (zatím si nejsem jistý, co bude uvnitř). Záznam stacku bude
        obsahovat pointer, který do něj vloží obvykle sémantická akce k
        neterminálu. Bude to většinou ukazatel na uzel, ke kteremu se má
        připojit další díl stromu v LL parseru nebo výrazový podstrom ve
        výrazovém parseru.

        #include "stack.h"

        typedef enum {
            LL_NOT_ID,
            LL_ID_FUNCTION_DECLARATION_NAME,
            LL_ID_DECLARATION_NAME,
            LL_ID_ASSIGNMENT_TARGET,
            LL_ID_EXPRESSION_REFERENCE,
        } IDContextType;       // Bude pozdeji obsahovat kontext, podle ktereho
                ma pozdeji tento token ID byt zpracovan, prozatim nevim, jestli
                je uplny

        typedef struct {
            GrammarSymbol symbol;       // Symbol na zasobniku
            void *semantic_pointer;     // Ukazatel pro stavbu stromu a
                    semantiku.
            IDContextType id_type;
        } ParserStackEntry;

        // Kontext sdileny mezi vsemi parsery, napriklad s aktualnim terminalem
                ze vstupu atp.
        typedef struct {
            int idk;   // Prozatim nevim, co bude obsahovat
        } ParserContext;

    Prompt pro prejmenovani hodnot IDContextType.

    Prompt pro Claude:
        To s LL_ je pravda, můžeš to přejmenovat. Přesnou podobu toho structu
        nejspíše budu ještě měnit, ale pravděpodobně bude ID zpracováváno
        nějakým speciálním callbackem, který bude potřebovat znát, v jakém
        kontextu se toto ID vyskytlo ve stromu. Tento callback však navrhnu
        asi až později.
*/

/**
 * @brief Zaznam ulozeny na zasobniku parseru.
 */
typedef struct {
    GrammarSymbol symbol; ///< Symbol na zasobniku

    /**
     * Ukazatel pro stavbu stromu, obvykle ho vklada semanticka akce
     * neterminalu. V LL parseru ukazuje na uzel, ke kteremu se pripoji
     * dalsi cast stromu, ve vyrazovem parseru na vyrazovy podstrom.
     */
    void *semantic_pointer;

    IDContextType id_type; ///< Kontext vyskytu tokenu ID ve strome
} ParserStackEntry;

/**
 * @brief Kontext sdileny mezi vsemi parsery.
 *
 * Bude obsahovat napriklad aktualni terminal ze vstupu.
 */
typedef struct {
    int idk; ///< Prozatim bez vyznamu
} ParserContext;

/*
    Prompt pro nasledujici deklarace funkci je v souboru parser_common.c.
*/

/**
 * @brief Inicializuje zasobnik parseru.
 *
 * Velikost prvku je sizeof(ParserStackEntry).
 *
 * @param stack Zasobnik k inicializaci.
 */
void parser_stack_init(Stack *stack);

/**
 * @brief Vlozi na zasobnik parseru novy zaznam.
 *
 * Zaznam se sestavi z predanych hodnot, zasobnik si ho zkopiruje.
 *
 * @param stack Zasobnik parseru.
 * @param symbol Symbol gramatiky.
 * @param semantic_pointer Ukazatel pro stavbu stromu.
 * @param id_type Kontext vyskytu tokenu ID, pro ostatni symboly
 *        ID_CONTEXT_NONE.
 */
void parser_stack_push(Stack *stack, GrammarSymbol symbol,
                       void *semantic_pointer, IDContextType id_type);

/**
 * @brief Odebere zaznam z vrcholu zasobniku parseru.
 *
 * @param stack Zasobnik parseru.
 * @param out Kam se odebrany zaznam zkopiruje, pokud neni NULL.
 * @return False, pokud je zasobnik prazdny, jinak true.
 */
bool parser_stack_pop(Stack *stack, ParserStackEntry *out);

/**
 * @brief Vrati zaznam na vrcholu zasobniku parseru.
 *
 * @param stack Zasobnik parseru.
 * @return Ukazatel na zaznam na vrcholu, nebo NULL, je-li zasobnik prazdny.
 */
ParserStackEntry *parser_stack_top(const Stack *stack);

#endif // PARSER_COMMON_H
