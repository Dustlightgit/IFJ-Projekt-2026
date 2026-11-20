/*
    IFJ projekt 2026

    xbalaka00 Artur Balak
    xnovacm00 Michal Novacek

    semantic_types.h

    Puvodni autor: xbalaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - IDContextType zobecnen a prejmenovan na ContextAnnotation
          (hodnoty CONTEXT_*), SemanticCallback prijima kontextovou anotaci.
        - xbalaka00 - PredictiveSemanticAction prijima cislo radku.
        - xbalaka00 - pridana anotace CONTEXT_ID_COMMAND a polozka id_expr
          v SemanticContext.
        - xnovacm00 - pridany nove hodnoty ContextAnnotation za ucelem stavby AST
        - xbalaka00 - pridany typy ReductionResult a PrecedenceSemanticAction
          (semanticka akce precedencniho parseru), include grammar_symbol.h.
        - xbalaka00 - ReductionResult prejmenovan na AnnotatedSymbol, protoze
          uz neni jen vysledkem redukce, ale obecne symbol s semantickym
          ukazatelem a anotaci (prompt je zde).
        - xbalaka00 - pridana anotace CONTEXT_RANGE_WITHOUT_STEP (prompt je
          v precedence_semantic_actions.c).

    Tento soubor obsahuje spolecne datove typy pro semanticke akce: kontextovou
    anotaci prvku na zasobniku parseru, semanticky kontext a typy semantickych akci.
*/

#ifndef SEMANTIC_TYPES_H
#define SEMANTIC_TYPES_H

#include "ast.h"
#include "grammar_symbol.h"
#include "stack.h"

/*
    Prompty pro vytvoreni tohoto souboru a presun typu do nej. Stejne prompty
    zpusobily zmeny take v parser_common.h, predictive_semantic_actions.h a LL_table.h,
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
    predictive_semantic_actions.h, predictive_semantic_actions.c, production.h, production.c,
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

/*
    Prompt pro chovani jadra pro <IDcommand>, anotaci CONTEXT_ID_COMMAND
    a specialni akci pro ID je v souboru predictive_parser.c.
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
  CONTEXT_ID_COMMAND, ///< ID na zacatku prikazu, special_action_id vytvori uzel

  CONTEXT_PROLOGUE_CHECK,       ///< ID v prologu, kontrola ifj26
  CONTEXT_FUNCTION_DEF,         ///< ID je jmeno definovane funkce
  CONTEXT_FUNCTION_RETURN_TYPE, ///< Typ je navratovy typ funkce

  CONTEXT_PARAM_NAME,   ///< ID je jmeno parametru funkce
  CONTEXT_PARAM_TYPE,   ///< Typ je typ naposledy pridaneho parametru
  CONTEXT_LIST_ELEMENT, ///< Typ je typ prvku seznamu (pointer je Type *)

  CONTEXT_BLOCK_STMT,    ///< Pointer je Block *, prikaz se prida do nej
  CONTEXT_FUNCTION_BODY, ///< Pointer je FuncDef *, blok je telo funkce
  CONTEXT_WHILE_BODY,    ///< Pointer je Stmt * (while), blok je telo
  CONTEXT_FOR_BODY,      ///< Pointer je Stmt * (for), blok je telo
  CONTEXT_VARDEF_TYPE,   ///< Typ je deklarovany typ promenne (Stmt *)
  CONTEXT_VARDEF_INIT,   ///< Vyraz je inicializace promenne (Stmt *)
  CONTEXT_ASSIGN_VALUE,  ///< Vyraz je hodnota prirazeni (Stmt *)
  CONTEXT_RETURN_VALUE,  ///< Vyraz je hodnota returnu (Stmt *)

  CONTEXT_WHILE_COND, ///< Vyraz je podminka while (Stmt *)
  CONTEXT_FOR_VAR,    ///< ID je ridici promenna for (Stmt *)
  CONTEXT_FOR_SOURCE, ///< Vyraz je zdroj for (Stmt *)
  CONTEXT_FOR_GUARD,  ///< Vyraz je straz for (Stmt *)

  CONTEXT_IF_THEN, ///< Blok je vetev then (Expr * if)
  CONTEXT_IF_ELSE, ///< Blok je vetev else (Expr * if)
  CONTEXT_IF_COND, ///< Vyraz je podminka if (Expr * if)

  /// Neterminal E je rozsah bez kroku (Expr * EXPR_RANGE vytvoreny pravidlem
  /// "to" nebo "until"), pripustny levy operand infixoveho "by"
  CONTEXT_RANGE_WITHOUT_STEP,
} ContextAnnotation;

/*
    Prompt pro vychozi kostru SemanticContext a PredictiveSemanticAction (puvodne
    v predictive_semantic_actions.h).

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

    Prompt pro upravu PredictiveSemanticAction (parser context se callbacku nepredava).

    Prompt pro Claude:
        Myslím, že nakonec parser context nakonec nebude potřeba předávat
        callbacku. Bude existovat, ale budou v něm jen data společná pro
        všechny instance parseru.

    Prompt pro predani cisla radku semantickym callbackum. Stejny prompt
    zpusobil zmeny take v predictive_semantic_actions.h, predictive_semantic_actions.c
    a predictive_parser.c, jeho zneni je pouze zde.

    Prompt pro Claude:
        Zdá se, že u stavby AST se bude do nodes doplňovat i číslo řádku kvůli
        chybovým výpisům a obecně by bylo užitečné, kdyby sémantické callbacky
        měly přístup k číslu řádku. Doplň tedy k sémantickému callbacku
        argument čísla řádku; můžeš přidat jeho výpis i do trace_rule. V jádře
        u volání callbacku předávej nejspíše číslo řádku aktuálního vstupního
        terminálu, protože ten nejspíše bude nejblíž skutečnému umístění
        konstrukce ve vstupním kódu. Do speciálního callbacku pro ID to nutné
        není, protože ten má přístup k celému tokenu, kde je i číslo řádku.
*/

/**
 * @brief Semanticky kontext sdileny mezi semantickymi akcemi.
 *
 * Obsahuje vse, co si jednotlive akce potrebuji udrzovat mezi sebou,
 * pokud nejake takove informace budou.
 */
typedef struct {
  int temp; ///< Prozatim bez vyznamu
  /// AST uzel ID na zacatku prikazu (vytvari ho special_action_id pri
  /// anotaci CONTEXT_ID_COMMAND, zpracuje a uvolni jadro parseru). NULL, pokud
  /// zadny takovy uzel prave neexistuje.
  Expr *id_expr;
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
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu,
 *        ktery je nejblize skutecnemu umisteni konstrukce ve vstupu. Slouzi
 *        napr. pro cislo radku v uzlech AST a pro chybova hlaseni.
 * @param stack Zasobnik parseru, akce na nej muze pridat produkci
 *        a k ni rovnou atributy (ukazatele, kontextove anotace apod.).
 */
typedef void (*PredictiveSemanticAction)(SemanticContext *ctx, void *semantic_pointer,
                                 ContextAnnotation annotation, int line,
                                 Stack *stack);

/*
    Prompt pro semantickou akci precedencniho parseru (PrecedenceSemanticAction
    a ReductionResult, pozdeji prejmenovan na AnnotatedSymbol), polozku
    redukcni tabulky, tabulku redukci
    v reduction_table.* a funkci parser_stack_peek. Stejny prompt zpusobil
    zmeny take v reduction_table.h, reduction_table.c, parser_common.h
    a parser_common.c, jeho zneni je pouze zde. V promptu je uvedeno
    PredictiveSemanticAction a predictive_predictive_semantic_actions.*, myslen je tim
    PrecedenceSemanticAction a precedence_predictive_semantic_actions.*.

    Prompt pro Claude:
        Dobrá, teď bych navrhnul ten callback. Rovnou řeknu, že jim tu budu
        říkat callback, ale v komentářích je pojmenovávej česky bez
        diakritiky. Oficiální název je sémantická akce precedenčního parseru,
        v kódu PrecedenceSemanticAction.

        Callback reprezentuje pravidlo a koná stavbu stromu. Zde asi nebudu
        rozlišovat dva druhy pravidel, protože téměř všechna pravidla
        výrazovky potřebují provést užitečnou akci - vzít podstromy z
        jednotlivých uzlů, složit je do nového uzlu a ten nechat v
        neterminálu, na který se pravidlo redukuje.

        Pokud je pravidlo zavoláno, pak to bylo na základě toho, že byl jeho
        handle vyhledán v redukční tabulce, a SearchHandle bude sestavován z
        prvků zásobníku. Callback tedy v moment zavolání má zaručeno, že
        prvních i prvků stacku jsou přesně symboly odpovídající jeho pravé
        straně. Aby nebylo potřeba do callbacku dávat směšně velké množství
        informací, bude tedy callback mít právo používat parser_stack_peek a
        nahlížet na data symbolů jeho handle přímo na zásobníku. Argumentem
        callbacku tedy bude stack.

        Vyklízení handle ze zásobníku a vložení neterminálu, na který byla
        produkce redukovaná, bude odpovědností jádra (protože handle v době
        zavolání callbacku musí být stále na stacku, a handle musí být
        vyklizen dříve, než je vložen redukovaný neterminál). Callback tedy
        nebude měnit zásobník.

        Jádru bude muset předat symbol, sémantický pointer a sémantickou
        anotaci, kterou chce vložit do nového ParserStackEntry. Protože to je
        větší množství informací, budou vraceny přes pointerový argument
        místo returnu.

        Prozatím jen přidej deklaraci PredictiveSemanticAction do
        semantic_types.h; samotné callbacky a jejich deklarace pro účely
        importů budou v páru predictive_semantic_actions.*
        Prozatím však callbacky nevytvářej.

        Rovnou můžeme vytvořit i zbytek redukční tabulky (ta bude v
        reduction_table.h).
        ReductionTableEntry bude tvořen pointerem na výše uvedenou sémantickou
        akci, a konstantním SearchHandle. V search handle bude později
        zapsán handle pravidla v podobě symbolů.

        Samotná tabulka pak bude
        extern static ReductionTableEntry REDUCTION_TABLE[REDUCTION_TABLE_SIZE];

        S tím že velikost bude
        #define REDUCTION_TABLE_SIZE 40

        K tabulce bude přistupováno vyhledávací funkcí, kterou ale asi
        navrhneme až v dalším kroku. Ještě přemýšlím, jestli je potřeba, aby
        byla tabulka veřejně viditelná v hlavičkovém souboru, když se k ní
        stejně vždy bude přistupovat přes tu vyhledávací metodu; totéž možná
        platí o všech tabulkách, ale zatím to asi uděláme takto a necháme to
        být. Zatím myslím všude do hlavičky dávám všechno, včetně věcí, které
        nejspíš nikdo mimo danou funkci nevyužije, ale případně se to předělá
        až ke konci, jestli to bude potřeba.

        Tabulka bude vyplněna a callbacky budou vytvořeny až později.
*/

/*
    Prompt pro prejmenovani ReductionResult na AnnotatedSymbol. Stejny prompt
    zpusobil zmeny take v precedence_parser.h a precedence_parser.c, jeho
    zneni je pouze zde. Nazev AnnotatedSymbol vybral Claude. Deklarace typu
    v tomto souboru jiz byla, k presunu proto nedoslo.

    Prompt pro Claude:
        Poslední věc, kterou jsem udělal, je vytvoření kódu jádra
        precedenčního parseru. Využil jsem u toho strukturu ReductionResult,
        která původně byla myšlena jako návratový typ callbacku precedenčního
        parseru, ve kterém akce určí, na který neterminál má být handle
        redukován. Použil jsem ho však i jako vstup precedenční analýzy pro
        specifikování prvotního terminálu na vstupu, aby bylo možné pracovat
        se speciálním chováním pravidel rozlišujících přiřazení a výraz
        začínající na ID.

        Proto myslím, že název ReductionResult možná už není výstižný, protože
        už nespecifikuje pouze výsledek redukce, tedy neterminál. Beze změny
        funkcionality kódu bych ho přejmenoval na SymbolWithMetadata nebo na
        SymbolWithContext nebo AnnotatedSymbol (asi vyber, co zní nejlépe) a
        jeho deklaraci přesunul do semantic_types.h. Proveď tuto změnu a uprav
        příslušně importy v projektu tak, aby tento přesun zohlednily.
        Struktura tedy bude obecněji pojatá jako symbol, u kterého jsou
        uložena i sémantická anotace a pointer, a slouží spíše pro snížení
        množství parametrů funkcí při přesunu dat. Využit bude však přesně na
        stejných místech jako stávající ReductionResult.
*/

/**
 * @brief Symbol gramatiky doplneny o semanticky ukazatel a kontextovou
 * anotaci.
 *
 * Slouzi ke snizeni poctu parametru funkci pri predavani dat budouciho zaznamu
 * zasobniku. Pouziva se na dvou mistech: jako vystup semanticke akce
 * precedencniho parseru (neterminal, na ktery se pravidlo redukuje) a jako
 * volitelny pocatecni terminal vstupu precedencniho parseru. Z techto hodnot
 * jadro sestavi novy ParserStackEntry. Priznak reduction_marker nastavuje
 * jadro samo.
 */
typedef struct {
  GrammarSymbol symbol;         ///< Symbol, u vysledku redukce neterminal
  void *semantic_pointer;       ///< Semanticky ukazatel noveho zaznamu, obvykle
                                ///< hotovy podstrom
  ContextAnnotation annotation; ///< Kontextova anotace noveho zaznamu
} AnnotatedSymbol;

/**
 * @brief Semanticka akce precedencniho parseru, reprezentuje redukcni pravidlo.
 *
 * Akce se zavola, kdyz byl handle pravidla vyhledan v redukcni tabulce.
 * Handle se sklada primo z prvku zasobniku, akce tedy ma zaruceno, ze
 * prvnich length symbolu na zasobniku (od vrcholu) jsou presne symboly prave
 * strany pravidla, kde length je delka handle pravidla. Data techto symbolu
 * si akce prohlizi pomoci parser_stack_peek (hloubka 0 je vrchol, tedy posledni
 * symbol handle). Vezme podstromy z jednotlivych symbolu, slozi je do noveho
 * uzlu a ten preda jadru pres result jako semanticky ukazatel redukovaneho
 * neterminalu.
 *
 * Akce zasobnik nemeni. Vyklizeni handle a vlozeni redukovaneho neterminalu
 * je odpovednosti jadra, protoze handle musi byt v dobe volani akce stale na
 * zasobniku a musi byt vyklizen drive, nez se vlozi redukovany neterminal.
 *
 * @param stack Zasobnik parseru, akce z nej cte, ale nemeni ho.
 * @param line Cislo radku, obvykle radek aktualniho vstupniho terminalu,
 *        ktery je nejblize skutecnemu umisteni konstrukce ve vstupu. Slouzi
 *        napr. pro cislo radku v uzlech AST a pro chybova hlaseni.
 * @param result Vystup, symbol, semanticky ukazatel a anotace, ktere jadro
 *        vlozi do noveho zaznamu na zasobniku.
 */
typedef void (*PrecedenceSemanticAction)(const Stack *stack, int line,
                                         AnnotatedSymbol *result);

#endif // SEMANTIC_TYPES_H
