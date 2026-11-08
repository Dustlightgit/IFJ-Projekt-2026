/*
    IFJ projekt 2026

    semantic_types.h

    Puvodni autor: xbakaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        -

    Tento soubor obsahuje spolecne datove typy pro semanticke akce: kontext
    tokenu ID, semanticky kontext a typ callbacku semanticke akce. Je urcen
    k zahrnuti z parser_common.h, semantic_actions.h i LL_table.h bez vzniku
    cyklickych zavislosti, proto nezahrnuje nic krome stack.h.
*/

/*
    Puvodni prompty k obsahu tohoto souboru jsou v jinych souborech:
        - IDContextType: parser_common.h
        - SemanticContext a SemanticCallback: semantic_actions.h
*/

/*
    Prompt pro upravu SemanticCallback (parser context se callbacku nepredava).

    Prompt pro Claude:
        Myslím, že nakonec parser context nakonec nebude potřeba předávat
        callbacku. Bude existovat, ale budou v něm jen data společná pro
        všechny instance parseru.
*/

/*
    Prompt pro presun IDContextType, SemanticContext a SemanticCallback do
    tohoto souboru. Stejny prompt zpusobil zmeny take v parser_common.h,
    semantic_actions.h a LL_table.h, jejich zneni je pouze zde.

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
*/

/*
    Prompt pro vytvoreni tohoto souboru a podobu hlavicky souboru. Stejne
    jako predchozi prompt zasahl i do ostatnich souboru.

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

#ifndef SEMANTIC_TYPES_H
#define SEMANTIC_TYPES_H

#include "stack.h"

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