/*
    IFJ projekt 2026

    semantic_types.h

    Puvodni autor: xbalaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        -

    Tento soubor obsahuje spolecne datove typy pro semanticke akce: kontext
    tokenu ID, semanticky kontext a typ callbacku semanticke akce. Je urcen
    k zahrnuti z parser_common.h, semantic_actions.h i LL_table.h bez vzniku
    cyklickych zavislosti, proto nezahrnuje nic krome stack.h.
*/

#ifndef SEMANTIC_TYPES_H
#define SEMANTIC_TYPES_H

#include "stack.h"

/*
    Prompty pro vytvoreni tohoto souboru a presun typu do nej. Stejne prompty
    zpusobily zmeny take v parser_common.h, semantic_actions.h a LL_table.h,
    jejich zneni je pouze zde.

    Prompt pro Claude:
        Kdybych IDContextType přesunul do sémantiky, byly by nějaké problémy?
        On totiž teoreticky spadá spíš pod sémantiku protože ta určuje, jaký
        kontext má daný terminál ID při vkládání na stack; stejně jej tedy
        bude i používat. Jestli by závislost vznikla, můžeme vytvořit nový
        soubor, kde budou všechny sémantické deklarace enumů a drobných věcí
        (třeba i se semantic context), a pak samotný semantic_actions by už
        měl samotné callbacky a jejich definice i s využitím funkcí např. z
        parser_common, které nejspíš bude potřebovat, protože callbacky mimo
        jiné budou samy dávat na stack produkci tohoto pravidla s
        příslušnými atributy/pointery.

    Prompt pro Claude:
        Myslím, že callback pro ID nejspíše bude mít vlastní typ. Na rozdíl
        od "pravidlových callbacků" bude totiž potřebovat přinejmenším
        atribut ID, možná bych té funkci dal celý stack entry se všema
        informacema o tom ID naráz.

        semantic_types.c zní dobře.

        Možná by i ten void *semantic_pointer chtěl vlastní enum, jen v tuto
        chvíli nemám představu, co všechno by muselo v tom enumu být,
        potenciálně toho bude obrovská hromada a bude to záviset i na tom,
        jak bude vypadat AST. I tak to možná bude lepší mít jako enum, který
        nejspíše patří do semantic_types.h.

        Než začneš generovat, bude nejlepší, když uvidíš soubor už v podobě,
        v jaké ho nejspíš budeme chtít. Možná by za hlavička mohla taky být
        doxygen, ale už jsme to začali dělat takhle... tak bych to už asi
        neměnil. Nevím jestli umíš prompty sám přidávat, ale bylo by to také
        super. Jen teda nevím, jak by to vypadalo, když by jeden prompt měnil
        více souborů...

        Přikládám současnou verzi LL_table.h pro představu.
*/

/*
    Prompty pro IDContextType jsou v parser_common.h, odkud byl tento typ
    presunut.
*/

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
  ID_CONTEXT_EXPRESSION_REFERENCE,      ///< Reference na ID ve vyrazu
} IDContextType;

/*
    Prompt pro vychozi kostru SemanticContext a SemanticCallback (puvodne
    v semantic_actions.h).

    Prompt pro Claude:
        Nevím jestli jsem to již nezmínil, ale budou dva parsery - prediktivní
        parser řízený LL tabulkou a potom výrazový parser. Sémantické akce
        (možná by lepší název byl parser akce?) budou stavět AST; sémantické
        kontroly pro kompatibility typů apod.  proběhnou až nad kompletním
        AST.

        Zde je hrubá verze deklarací, které budou v souboru
        semantic_actions.h. Zde budou deklarovány všechny sémantické akce u
        pravidel, které sémantickou akci mají. Možná se rozhraní ještě upraví,
        ale prozatím může zůstat takto jako první verze. Sémantický kontext
        bude obsahovat informace, které je potřeba, aby sémantické akce
        sdílely (pokud tedy budou vůbec nějaké potřeba, případně se to později
        odstraní).

        Sémantická akce tedy obdrží sémantický kontext. Dále obdrží ukazatel,
        který je uložen na stacku u neterminálu, který je tímto pravidlem
        rozváděn; může se jednat třeba o dokončený podstrom výrazu v případě
        výrazového parseru, nebo o ukazatel na místo AST, kam má být vložen
        další kus stromu nebo zjištěná informace (nebo na node, ke kterému se
        má napojit...). Výrazový parser potom bude pravděpodobně mít odlišnou
        hlavičku svých callbacků, protože ten bude nejspíše používat celý
        handle.
        Stack pak bude dostávat proto, aby callback mohl na stack přidat
        produkci a k ní rovnou přidávat atributy (pointery dle své potřeby,
        případně jiné atributy např. pro kontext ID).

        #include "stack.h"

        typedef struct {
            int temp;    // Prozatim nevim, co bude obsahovat. Bude se jednat o
                    veskere informace, ktere si jednotlive semanticke akce budou
                    potrebovat udrzovat mezi sebou, budou-li nejake.
        } SemanticContext;

        typedef void (*SemanticCallback)(SemanticContext*ctx, void
                *semantic_pointer, Stack *stack);

            ast_pointer bude vyznamny predevsim pro neterminaly a ma nasledujici
                    vyznam:
                - v LL parseru to bude ukazatel na misto, kam ma byt vlozen uzel
                        ast, pripadne na atribut, jez ma byt zmenen,
                pricemz toto provede semanticka funkce nahrazujici dany
                        neterminal.
                - ve vyrazovem parseru to bude obvykle ukazatel na prislusny
                        dokonceny vyrazovy podstrom.

    Prompt pro upravu SemanticCallback (parser context se callbacku nepredava).

    Prompt pro Claude:
        Myslím, že nakonec parser context nakonec nebude potřeba předávat
        callbacku. Bude existovat, ale budou v něm jen data společná pro
        všechny instance parseru.
*/

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
 * Akce se provede pri rozvoji neterminalu timto pravidlem.
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
typedef void (*SemanticCallback)(SemanticContext *ctx, void *semantic_pointer,
                                 Stack *stack);

#endif // SEMANTIC_TYPES_H
