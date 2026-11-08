/*
    IFJ projekt 2026

    semantic_actions.h

    Puvodni autor: xbakaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        -

    Tento soubor obsahuje deklaraci tvaru ukazatelu na semanticke akce, ktere budou soucasti LL tabulky.
    Dale obsahuje dalsi pomocne struktury, napr sdileny semanticky kontext.

*/


/*
    Prompt pro vychozi kostru souboru se SemanticContext a SemanticCallback

    Prompt pro Claude:
        Nevím jestli jsem to již nezmínil, ale budou dva parsery - prediktivní parser řízený LL tabulkou a potom výrazový parser. Sémantické akce (možná by lepší název byl parser akce?) budou stavět AST; sémantické kontroly pro kompatibility typů apod.  proběhnou až nad kompletním AST.

        Zde je hrubá verze deklarací, které budou v souboru semantic_actions.h. Zde budou deklarovány všechny sémantické akce u pravidel, které sémantickou akci mají. Možná se rozhraní ještě upraví, ale prozatím může zůstat takto jako první verze. Sémantický kontext bude obsahovat informace, které je potřeba, aby sémantické akce sdílely (pokud tedy budou vůbec nějaké potřeba, případně se to později odstraní).

        Sémantická akce tedy obdrží sémantický kontext. Dále obdrží ukazatel, který je uložen na stacku u neterminálu, který je tímto pravidlem rozváděn; může se jednat třeba o dokončený podstrom výrazu v případě výrazového parseru, nebo o ukazatel na místo AST, kam má být vložen další kus stromu nebo zjištěná informace (nebo na node, ke kterému se má napojit...). Výrazový parser potom bude pravděpodobně mít odlišnou hlavičku svých callbacků, protože ten bude nejspíše používat celý handle.
        Stack pak bude dostávat proto, aby callback mohl na stack přidat produkci a k ní rovnou přidávat atributy (pointery dle své potřeby, případně jiné atributy např. pro kontext ID).

        #include "stack.h"

        typedef struct {
            int temp;    // Prozatim nevim, co bude obsahovat. Bude se jednat o veskere informace, ktere si jednotlive semanticke akce budou potrebovat udrzovat mezi sebou, budou-li nejake.
        } SemanticContext;

        typedef void (*SemanticCallback)(SemanticContext*ctx, void *semantic_pointer, Stack *stack);

        /*
            ast_pointer bude vyznamny predevsim pro neterminaly a ma nasledujici vyznam:
                - v LL parseru to bude ukazatel na misto, kam ma byt vlozen uzel ast, pripadne na atribut, jez ma byt zmenen,
                pricemz toto provede semanticka funkce nahrazujici dany neterminal.
                - ve vyrazovem parseru to bude obvykle ukazatel na prislusny dokonceny vyrazovy podstrom.
        * /
*/

#ifndef SEMANTIC_ACTIONS_H
#define SEMANTIC_ACTIONS_H
 
#include "stack.h"
 
/**
 * @brief Semanticky kontext sdileny mezi semantickymi akcemi.
 *
 * Obsahuje vse, co si jednotlive akce potrebuji udrzovat mezi sebou,
 * pokud nejake takove informace budou.
 */
typedef struct {
    int temp; ///< Prozatim bez vyznamu
} SemanticContext;
 
/**
 * @brief Semanticka akce pravidla.
 *
 * @param ctx Semanticky kontext sdileny mezi akcemi.
 * @param semantic_pointer Ukazatel ulozeny na zasobniku u neterminalu,
 *        ktery je timto pravidlem rozvadeny. Vyznam zavisi na parseru:
 *        - V LL parseru je to ukazatel na misto, kam ma byt vlozen uzel AST,
 *          pripadne na atribut, ktery ma byt zmenen. Zmenu provede
 *          semanticka akce nahrazujici dany neterminal.
 *        - Ve vyrazovem parseru je to obvykle ukazatel na prislusny
 *          dokonceny vyrazovy podstrom.
 * @param stack Zasobnik parseru, akce na nej muze pridat produkci
 *        a k ni rovnou atributy (ukazatele, kontext ID apod.).
 */
typedef void (*SemanticCallback)(SemanticContext *ctx, void *semantic_pointer, Stack *stack);
 
// Deklarace semantickych akci
 
#endif // SEMANTIC_ACTIONS_H
