/*
    IFJ projekt 2026

    parser_common.h

    Puvodni autor: xbakaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        -

    Tento soubor obsahuje spolecne deklarace, ktere budou pouzivat oba parsery,
   napriklad struktura prvku zasobniku a spolecny kontext.

*/

/*
    Prompt pro vychozi kostru souboru s IDContextType, ParserStackEntry a
   ParserContext

    Prompt pro Claude:
        Tento soubor parser_common.h definuje věci společné pro oba parsery.
   Prozatím vím, že to bude záznam ukládaný ve stacku, a nějaký společný kontext
   (zatím si nejsem jistý, co bude uvnitř). Záznam stacku bude obsahovat
   pointer, který do něj vloží obvykle sémantická akce k neterminálu. Bude to
   většinou ukazatel na uzel, ke kteremu se má připojit další díl stromu v LL
   parseru nebo výrazový podstrom ve výrazovém parseru.

        #include "stack.h"

        typedef enum {
            LL_NOT_ID,
            LL_ID_FUNCTION_DECLARATION_NAME,
            LL_ID_DECLARATION_NAME,
            LL_ID_ASSIGNMENT_TARGET,
            LL_ID_EXPRESSION_REFERENCE,
        } IDContextType;       // Bude pozdeji obsahovat kontext, podle ktereho
   ma pozdeji tento token ID byt zpracovan, prozatim nevim, jestli je uplny

        typedef struct {
            GrammarSymbol symbol;       // Symbol na zasobniku
            void *semantic_pointer;     // Ukazatel pro stavbu stromu a
   semantiku. IDContextType id_type; } ParserStackEntry;

        // Kontext sdileny mezi vsemi parsery, napriklad s aktualnim terminalem
   ze vstupu atp. typedef struct { int idk;   // Prozatim nevim, co bude
   obsahovat } ParserContext;
*/

#ifndef PARSER_COMMON_H
#define PARSER_COMMON_H

#include "production.h"

/**
 * @brief Kontext, ve kterem se token ID vyskytl ve strome.
 *
 * Podle nej se ID zpracuje specialnim callbackem. Bude doplnen,
 * prozatim nemusi byt uplny.
 */
typedef enum {
  ID_CONTEXT_NONE,                      ///< Symbol neni identifikator
  ID_CONTEXT_FUNCTION_DECLARATION_NAME, ///< Jmeno deklarovane funkce
  ID_CONTEXT_DECLARATION_NAME,          ///< Jmeno deklarovane promenne
  ID_CONTEXT_ASSIGNMENT_TARGET,         ///< Cil prirazeni
  ID_CONTEXT_EXPRESSION_REFERENCE, ///< Reference na identifikator ve vyrazu
} IDContextType;

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

#endif // PARSER_COMMON_H
