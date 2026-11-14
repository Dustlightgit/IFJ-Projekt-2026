/*
    IFJ projekt 2026

    semantic_types.h

    Puvodni autor: xbalaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - IDContextType zobecnen a prejmenovan na ContextAnnotation
          (hodnoty CONTEXT_*), SemanticCallback prijima kontextovou anotaci.

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
    Prompt pro zobecneni IDContextType na ContextAnnotation, predavani
    anotace callbackum a kopirovani anotace ve statickych produkcich. Stejny
    prompt zpusobil zmeny take v parser_common.h, parser_common.c,
    semantic_actions.h, semantic_actions.c, production.h, production.c,
    predictive_parser.h a predictive_parser.c, jeho zneni je pouze zde.

    Prompt pro Claude:
        Dobrá, další věc, co bude potřeba upravit, je IDContextType. Původně
        jsem zamýšlel, že tento enum umožní budoucímu callbacku pro zpracování
        ID rozhodnout, co má udělat. Nejspíše ale bude potřeba tento enum
        zobecnit na ContextAnnotation, který není vázaný pouze na ID, avšak
        bude připojitelný k libovolnému terminálu či neterminálu, aby šlo
        všeobecně každému callbacku sdělit, jak má postupovat.
        Proto tento enum přejmenuj na ContextAnnotation, jeho "výchozí"
        prázdná hodnota bude CONTEXT_NONE. Ostatní hodnoty budou doplněny dle
        potřeb stavby AST, prozatím můžeš ponechat stávající ilustrativní
        hodnoty bez prefixu ID_.
        Kromě tohoto enumu budou potřeba následující úpravy:

        * úprava zásobníkového záznamu parseru a jeho metod tak, aby nesl tento
          zobecněný enum namísto starého.
        * každé callbackové pravidlo bude přijímat i argument
          ContextAnnotation.
        * uvnitř callbacku bude možné kontextovou anotaci přiložit i k
          neterminálu vkládanému , ne pouze k terminálu.
        * vložení statické produkce na zásobník bude kromě sémantického
          pointeru kopírovat i kontextovou anotaci. Zadokumentuj toto nové
          pravidlo do komentářů.

        V tomto zobecněném systému každý callback obdrží pointer a kontextovou
        anotaci ze svého rozvíjeného neterminálu, aby věděl, co má provést a
        věděl, kam to uložit. Na zásobník bude vkládat symboly své pravé strany
        spolu s pointery a sémantickou anotací, aby jim sdělila, co mají udělat
        a kam to uložit. Kopírování pointeru a anotace ve statických pravidlech
        zajistí šíření těchto informací, aniž by bylo nutné z nich dělat
        callbacky. Kopírování ve statickém callbacku je vždy do všech prvků
        produkce.
*/

/**
 * @brief Kontextova anotace pripojena k symbolu na zasobniku parseru.
 *
 * Anotaci lze pripojit k libovolnemu terminalu i neterminalu. Callback
 * rozvadeneho neterminalu podle ni pozna, co ma provest a kam vysledek ulozit,
 * a stejnou cestou anotuje symboly vkladane na zasobnik. Hodnoty budou
 * doplneny dle potreb stavby AST, prozatim jsou ilustrativni.
 */
typedef enum {
  CONTEXT_NONE,                      ///< Vychozi hodnota, symbol bez anotace
  CONTEXT_FUNCTION_DECLARATION_NAME, ///< Jmeno deklarovane funkce
  CONTEXT_DECLARATION_NAME,          ///< Jmeno deklarovane promenne
  CONTEXT_ASSIGNMENT_TARGET,         ///< Cil prirazeni
  CONTEXT_EXPRESSION_REFERENCE,      ///< Reference na ID ve vyrazu
} ContextAnnotation;

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
 * Akce se provede pri rozvoji neterminalu timto pravidlem. Callback dostane
 * ukazatel a kontextovou anotaci rozvadeneho neterminalu, aby vedel, co ma
 * provest a kam vysledek ulozit. Symboly sve prave strany vklada na zasobnik
 * spolu s ukazateli a anotacemi, ktere jim sdeluji totez. Staticke produkce
 * kopiruji ukazatel a anotaci neterminalu na leve strane vzdy do vsech symbolu
 * produkce, aniz by musely byt callbacky.
 *
 * @param ctx Semanticky kontext sdileny mezi akcemi.
 * @param semantic_pointer Ukazatel ulozeny na zasobniku u neterminalu,
 *        ktery je timto pravidlem rozvadeny. Vyznam zavisi na parseru:
 *        - V LL parseru je to ukazatel na misto, kam ma byt vlozen uzel AST,
 *          pripadne na atribut, ktery ma byt zmenen. Zmenu provede
 *          semanticka akce nahrazujici dany neterminal.
 *        - Ve vyrazovem parseru je to obvykle ukazatel na prislusny
 *          dokonceny vyrazovy podstrom.
 * @param annotation Kontextova anotace ulozena na zasobniku u neterminalu,
 *        ktery je timto pravidlem rozvadeny.
 * @param stack Zasobnik parseru, akce na nej muze pridat produkci
 *        a k ni rovnou atributy (ukazatele, kontextove anotace apod.).
 */
typedef void (*SemanticCallback)(SemanticContext *ctx, void *semantic_pointer,
                                 ContextAnnotation annotation, Stack *stack);

#endif // SEMANTIC_TYPES_H
