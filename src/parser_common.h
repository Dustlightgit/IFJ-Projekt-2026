/*
    IFJ projekt 2026

    parser_common.h

    Puvodni autor: xbalaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - IDContextType (vcetne prejmenovani hodnot na
          ID_CONTEXT_*) presunut do semantic_types.h.
        - xbalaka00 - pridani funkci parser_stack_init, parser_stack_push,
          parser_stack_pop a parser_stack_top.
        - xbalaka00 - pridani funkce parser_context_init za asistence Claude,
          prompt je v predictive_parser.c.
        - xbalaka00 - ParserStackEntry a parser_stack_push nesou
          ContextAnnotation misto IDContextType.
        - xbalaka00 - dokumentace parser_advance doplnena o overeni typu
          tokenu (prompt je v nonterminal.h).
        - xbalaka00 - ParserStackEntry a parser_stack_push nesou priznak
          reduction_marker (prompt je v parser_common.c).
        - xbalaka00 - pridana funkce parser_stack_peek (prompt je v
          semantic_types.h).
        - xbalaka00 - include grammar_symbol.h pro GrammarSymbol (GrammarSymbol presunut do grammar_symbol.h,
          prompt je v reduction_table.h).
        - xbalaka00 - pridana funkce trace_rule (prompt je v parser_common.c).
        - xbalaka00 - upraven popis funkce parser_advance

    Tento soubor definuje veci spolecne pro oba parsery: zaznam ukladany
    na zasobniku parseru, kontext sdileny mezi parsery, funkce pro praci
    se zasobnikem parseru a spolecny ladici vypis pravidel.
*/

#ifndef PARSER_COMMON_H
#define PARSER_COMMON_H

#include "grammar_symbol.h"
#include "production.h"
#include "semantic_types.h"
#include "stack.h"
#include "token.h"

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

/*
    Prompt pro zobecneni IDContextType na ContextAnnotation je v souboru
    semantic_types.h. Prompt pro priznak reduction_marker je v souboru
    parser_common.c.
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

  /**
     * Kontextova anotace symbolu, sdeluje callbacku, jak ma s symbolem
     * postupovat. Lze ji pripojit k terminalu i k neterminalu.
     */
  ContextAnnotation annotation;

  /**
     * Priznak znacky redukce pro precedencni parser. Je-li true, bezprostredne
     * za timto zaznamem (smerem k vrcholu zasobniku) nasleduje symbol <,
     * ktery v matematickem modelu oznacuje zacatek redukce. Handle tvori
     * vsechny zaznamy od vrcholu zasobniku az po prvni zaznam s timto
     * priznakem, tento zaznam do handle nepatri. Vychozi hodnota je false,
     * prediktivni parser priznak nevyuziva.
     */
  bool reduction_marker;
} ParserStackEntry;

/**
 * @brief Kontext sdileny mezi vsemi parsery.
 *
 * Obsahuje aktualni terminal ze vstupu.
 */
typedef struct {
  Token current_token;           ///< Aktualni token na vstupu programu
  bool has_current_token;        ///< Priznak validity vstupniho tokenu
  bool previous_token_was_comma; ///< Priznak pro kontrolu koncove carky
  SemanticContext
      semantic_context; ///< Struktura s daty pro interni potrebu semantickych akci
  // Pokud nebude vyuzita, bude odstranena.
} ParserContext;

/**
 * @brief Umoznuje zjistit, zda je na vstupu konkretni token.
 * 
 * Funkce je ciste zjistovaci, nemeni stav kontextu parseru.
 *
 * @param ctx Spolecny context parseru.
 * @param expected Porovnavany typ token.
 */
bool parser_check_token(const ParserContext *ctx, TokenType expected);

/**
 * @brief Posun ve vstupnim proudu dale. Kontroluje take koncovou carku.
 * 
 * Uvolni soucasny token a zavola pritele scannera, aby dodal dalsi. Veskere zpracovani tokenu
 * tedy musi probehnout jeste pred zavolanim teto funkce. Typ noveho tokenu
 * overi funkci token_type_validate (neplatny typ je interni chyba).
 * 
 * Funkce take kontroluje zakaz dvou po sobe jdoucich carek a koncovou carku v seznamu.
 * Kontrola je tedy spolecna pro oba parsery.
 *
 * @param ctx Spolecny context parseru.
 */
void parser_advance(ParserContext *ctx);

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
 * @param annotation Kontextova anotace symbolu, bez anotace CONTEXT_NONE.
 * @param reduction_marker Priznak znacky redukce, bezne false (viz
 *        ParserStackEntry).
 */
void parser_stack_push(Stack *stack, GrammarSymbol symbol,
                       void *semantic_pointer, ContextAnnotation annotation,
                       bool reduction_marker);

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

/**
 * @brief Vrati zaznam na zasobniku parseru v dane hloubce.
 *
 * @param stack Zasobnik parseru.
 * @param depth Hloubka, 0 je vrchol, 1 je zaznam tesne pod nim.
 * @return Ukazatel na zaznam, nebo NULL, pokud takovy zaznam neni.
 */
ParserStackEntry *parser_stack_peek(const Stack *stack, size_t depth);

/*
    Prompt pro funkci parser_stack_peek je v souboru semantic_types.h.
    Prompt pro nasledujici deklaraci funkce je v souboru predictive_parser.c.
*/

/**
 * @brief Inicializuje kontext parseru a nacte prvni token ze vstupu.
 *
 * Vsechny hodnoty nastavi na nulove, potom zavola parser_advance, ktery
 * ziska prvni vstupni token. Scanner musi byt pripraveny pred volanim.
 *
 * @param ctx Kontext parseru k inicializaci.
 */
void parser_context_init(ParserContext *ctx);

/*
    Prompt pro nasledujici deklaraci funkce je v souboru parser_common.c.
*/

/**
 * @brief Vypise pouzite pravidlo a cislo radku na stderr (ladici vystup).
 *
 * Format vystupu urcuje parametr is_predictive_parser. Pravidla prediktivniho
 * (LL) parseru se vypisuji jako "LL Parser: Pravidlo L<cislo> (radek <radek>)",
 * pravidla precedencniho parseru jako
 * "Precedencni parser: Pravidlo E<cislo> (radek <radek>)".
 *
 * @param is_predictive_parser True pro pravidlo prediktivniho (LL) parseru,
 *        false pro pravidlo precedencniho parseru.
 * @param rule_number Cislo pravidla.
 * @param line Cislo radku vstupu.
 */
void trace_rule(bool is_predictive_parser, int rule_number, int line);

#endif // PARSER_COMMON_H
