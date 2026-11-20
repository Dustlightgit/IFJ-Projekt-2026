/*
    IFJ projekt 2026

    xbalaka00 Artur Balak

    special_actions.h

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - special_action_id vytvari AST uzel pro ID pri anotaci
          CONTEXT_ID_COMMAND.
        - xbalaka00 - pridana prozatim prazdna deklarace pro specialni akci predzpracovani terminalu
        - xbalaka00 - doplnena dokumentace a funkcionalita akce
          special_action_precedence_preprocessing (prompt je zde).
        - xbalaka00 - pridana special_action_execute_expr_rule a presunuta
          funkce ast_attach_expr ze predictive_semantic_actions.h, prejmenovana na
          special_action_expr (prompt je zde), include parser_common.h.
        - xbalaka00 - funkce target_block a store_type sem byly presunuty z predictive_semantic_actions.*
          a prejmenovany na special_action_expr a special_action_store_type

    Tento soubor obsahuje deklarace specialnich akci parseru, tedy akci,
    ktere nejsou vazane na precedencni nebo prediktivni pravidlo a maji
    vlastni rozhrani (napr. akce pro token ID, ktera dostava cely vstupni token).
*/

#ifndef SPECIAL_ACTIONS_H
#define SPECIAL_ACTIONS_H

#include "parser_common.h"
#include "semantic_types.h"
#include "stack.h"
#include "token.h"

/*
    Prompt pro chovani akce pro CONTEXT_ID_COMMAND je v souboru
    predictive_parser.c.

    Prompt pro specialni akci pro ID a vznik tohoto souboru. Stejny prompt
    zpusobil zmeny take v special_actions.c a predictive_parser.c, jeho zneni
    je pouze zde.

    Prompt pro Claude:
        Dobrá, teď bych do kódu zavedl speciální callback pro ID. Ten bude v
        jádře volán na tom místě, kde je teď výpis potkaného ID tokenu.
        Provede se v situaci, kdy je na vrcholu ID a na vstupu je také ID
        (jinak by to stejně byla při volání funkce parser_match chyba takže
        není nutné nic dělat). Tento callback zpracuje každý token ID včetně
        jeho atributu předtím, než bude terminál porovnán a uvolněn funkcí
        match. Tento callback přijme za argument sémantický pointer a anotaci
        ze symbolu ID na zásobníku, a také ukazatel na celý vstupní token ze
        scanneru, aby měl přístup ke všem jeho atributům. Prozatím chci jen
        kostru po vzoru ostatních sémantických callbacků, do které se
        funkcionalita dodá až se bude propojovat s parserem - přesuň do něj
        tedy zatím jen současný výpis z jádřa, že bylo potkáno ID.

        Jeho obvyklá akce bude podle anotace provést připojení názvu tohoto ID
        k node specifikovaného v pointeru (například název funkce nebo čítače
        for cyklu atp.), případně vytvoření uzlu AST pro ID.

        Přemýšlím o umístění. Pokud to struktura importů dovolí, mohl by být v
        semantic actions, ačkoliv možná by i bez problému s importy bylo
        přehlednější umístit je do nového páru special_actions.*, kam později
        mohou být doplněny i další speciální callbacky, budou-li potřeba.
        Přinejmenším jeden další, který by tam mohl později přijít v souvislosti
        s výrazovým parserem mě napadá.
*/

/**
 * @brief Specialni akce pro token ID.
 *
 * Jadro ji vola, kdyz je na vrcholu zasobniku terminal ID a na vstupu je
 * take ID, a to pred porovnanim a uvolnenim tokenu funkci parser_match.
 * Zpracuje tedy kazdy token ID vcetne jeho atributu. Obvykla akce bude podle
 * anotace pripojit nazev ID k uzlu urcenemu ukazatelem (napr. nazev funkce
 * nebo citace cyklu for), pripadne vytvorit uzel AST pro ID.
 *
 * Vzdy vypise informaci o potkanem ID na stderr. Podle anotace pak:
 * - CONTEXT_ID_COMMAND: vytvori AST uzel pro ID (Expr typu EXPR_VARIABLE)
 *   a ulozi ukazatel na nej do ctx->id_expr, odkud ho vezme jadro parseru.
 * - ostatni anotace: zatim nic dalsiho (pozdeji nejspis interni chyba).
 *
 * @param ctx Semanticky kontext, do nej se uklada vytvoreny AST uzel.
 * @param semantic_pointer Ukazatel ulozeny u symbolu ID na zasobniku.
 * @param annotation Kontextova anotace ulozena u symbolu ID na zasobniku.
 * @param token Vstupni token ID ze scanneru. Token po skonceni akce uvolni
 *        parser_match, akce proto musi text, ktery chce uchovat, zkopirovat.
 */
void special_action_id(SemanticContext *ctx, void *semantic_pointer,
                       ContextAnnotation annotation, const Token *token);

/**
 * @brief Vrati blok, do ktereho se ma pridat prikaz nebo pripoji prikaz k vlastnikovi bloku.
 * 
 * Pokud pointer ukazuje na vlastnika tela (funkce, cyklus), blok se vytvori a pripoji.
 * Pouziva se uvnitr semantickych akci prediktivniho parseru.
 * 
 * @param semantic_pointer Ukazatel ulozeny u neterminalu pravidla na zasobniku.
 * @param annotation Kontextova anotace ulozena u neterminalu pravidla na zasobniku.
 * @param line Cislo radku pro chybove vystupy.
 */
Block *special_action_block(void *ptr, ContextAnnotation annotation, int line);


/**
 * @brief Ulozi hotovy typ na misto urcene ukazatelem a anotaci (prebira vlastnictvi typu).
 * 
 * @param semantic_pointer Ukazatel urcujici, kam se ma typ vlozit.
 * @param annotation Kontextova anotace urcujici, jaka funkce se ma pouzit pro vlozeni.
 * @param type Typ, ktery se ma ulozit na dane misto.
 * @param line Cislo radku pro chybove vystupy.
 */
void special_action_store_type(void *semantic_pointer, ContextAnnotation annotation,
                       Type *type, int line);

/*
    Prompt pro specialni akci predzpracovani terminalu precedencniho parseru.
    Stejny prompt zpusobil zmeny take v special_actions.c, jeho zneni je
    pouze zde.

    Prompt pro Claude:
        Dobrá, nyní bych vyplnil speciální akci
        special_action_precedence_preprocessing, aby tam nebyla jen prázdná
        deklarace bez dokumentace.

        Akce přijme ukazatel na token, a provede následující:

        * pokud se jedná o token s atributem, tzn. ID/ID_OPERATOR nebo literál
          string, double nebo int, tak pro něj vytvoří příslušný uzel AST a
          vrátí ukazatel na něj. Jak pro ID, tak pro ID_OPERATOR se vytvoří
          uzel pomocí expr_new_variable. Pro ostatní se použije
          expr_new_[int|double|string].
        * pro ostatní tokeny vrátí NULL.

        Důvodem je, že je nutné zpracovat atribut tokenu ještě před jeho
        uvolněním, a takto bude atribut rovnou uchován v uzlu AST, nebude tedy
        nutné si název ID apod. někam kopírovat... U literálu a ID na místě
        běžného operandu by stejně proběhla tvorba AST uzlu, takto se jen
        provede ještě před redukcí na E; tam, kde ID nebo ID_OPERATOR nakonec
        nebude využito jako listová proměnná (např. název funkce u volání
        funkce, nebo ID_OPERATOR, tedy ID použité jako infixový operátor) se
        využije název uložený v tomto uzlu AST pro vytvoření skutečně
        potřebného uzlu a ten pomocný se uvolní.
        Bude tedy existovat pravidlo, že terminály s atributem na stack
        precedenčního parseru putují již jako AST node a callback pro redukci
        literálu/ID na E pointer pouze předá. Chování parseru ve speciálním
        případě jádrových pravidel 32 a 33 pro rozlišení přiřazení a výrazu,
        který začíná na ID, s tímto již počítá a pokud tedy prediktivní
        parser zavolá precedenční se specifikovaným prvním terminálem, předá
        také ukazatel na uzel AST jako jeho semantic_pointer - proto se tento
        preprocessing neprovádí i na tomto specifikovaném vstupním tokenu.

        U ostatních symbolů podobné předvytvoření AST uzlu nutné není, proto
        na stack poputují se sémantickým ukazatelem NULL a AST se vytvoří
        normálně v sémantických akcích precedenčního parseru.
*/

/**
 * @brief Specialni akce predzpracovani vstupniho terminalu precedencniho
 * parseru.
 *
 * Jadro precedencniho parseru ji vola jednou pro kazdy vstupni token, a to
 * po vyhodnoceni prediktoru (token je tedy pripadne uz prebarveny z TOKEN_ID
 * na TOKEN_ID_OPERATOR) a pred jeho zpracovanim a uvolnenim. Atribut tokenu
 * je tak zpracovan driv, nez ho parser_advance uvolni.
 *
 * Pro tokeny s atributem vytvori uzel AST a vrati ukazatel na nej:
 * - TOKEN_ID a TOKEN_ID_OPERATOR: expr_new_variable (jmeno z atributu),
 * - TOKEN_INT_LIT: expr_new_int,
 * - TOKEN_DOUBLE_LIT: expr_new_double,
 * - TOKEN_STRING_LIT: expr_new_string.
 * Pro ostatni tokeny vrati NULL.
 *
 * Vraceny ukazatel jadro ulozi jako semanticky ukazatel terminalu na zasobniku
 * precedencniho parseru, terminaly s atributem tedy na zasobniku jsou uz
 * uzlem AST a semanticka akce redukce literalu nebo ID na E ukazatel pouze
 * preda. Pokud se ID nebo ID_OPERATOR nakonec nepouzije jako listova
 * promenna (napr. nazev volane funkce nebo ID pouzite jako infixovy operator),
 * vezme se z uzlu jmeno, vytvori se skutecne potrebny uzel a pomocny uzel se
 * uvolni. Tento zpusob umoznuje, aby atribut tokenu zustal uchovan v uzlu AST
 * a nebylo ho treba nikam jinam kopirovat.
 *
 * Akce se neprovadi na pocatecnim terminalu, ktery prediktivni parser predava
 * precedencnimu parseru (jadrova pravidla 32 a 33); u nej prediktivni parser
 * predava uzel AST rovnou jako semanticky ukazatel.
 *
 * @param input_token Vstupni token, nesmi byt NULL. Token se nemeni, akce si
 *        text atributu predava funkci expr_new_*, token ho uvolni pozdeji
 *        parser_advance.
 * @return Ukazatel na vytvoreny uzel AST (Expr *), nebo NULL pro token bez
 *         atributu.
 */
void *special_action_precedence_preprocessing(Token *input_token);

/*
    Prompt pro specialni akci provedeni pravidla vyrazu a pro presun funkce
    ast_attach_expr z predictive_semantic_actions.* a jeji prejmenovani na
    special_action_expr. Stejny prompt zpusobil zmeny take v special_actions.c,
    predictive_semantic_actions.h a predictive_semantic_actions.c, jeho zneni je pouze zde.

    Prompt pro Claude:
        Dělám parser založený na hybridu prediktivní a precedenční analýzy.
        Přikládám relevantní soubory. Pokud by ti některý soubor chyběl,
        vyžádej si jej, než začneš pracovat. Kontext návrhu a význam kódu si
        přečti v komentářích a deklarovaných promptech. Dodržuj jmenné
        konvence (včetně komentářů v češtině bez diakritiky - používej české
        názvy všech věcí i přesto, že v promptech někdy věci pojmenuji anglicky;
        prompty ale cituj doslovně po vzoru souborů), formát hlaviček souborů s
        popisem souboru, dle vzoru přiložených souborů také deklaruj prompty a
        aktualizuj změnové seznamy. Kód nekompiluj ani netestuj.

        Mohl by jsi do special_actions.* implementovat novou funkci
        special_action_execute_expr_rule, která provede určité pravidlo v
        situaci, kdy je na vstupu terminál, který neindexuje LL tabulku, ale
        kterým smí začínat výraz? Při zavolání funkce bylo již ověřeno, že tato
        situace nastala, terminál tedy není nutné jakkoliv kontrolovat. Funkce
        bude mít parametry typu ParserContext a AnnotatedSymbol s neterminálem
        na vrcholu a jeho metadaty. Podle neterminálu přes switch case vybere a
        provede příslušné pravidlo dle mapování, které ti dám na konec promptu.
        Vždy se buď zavolá sémantická akce pravidla, nebo se zavolá funkce s
        deklarací

        void parser_push_static_production(Stack *stack,
                                           const StaticProduction *production,
                                           void *semantic_pointer,
                                           ContextAnnotation annotation);

        pro statické pravidlo. Pokud neterminál není jeden z uvedených, vyhodí
        se chyba zavoláním
        error_exit(ERR_SYNTAX, line, "LL Parser: Neni mozne zvolit pravidlo.");

        Dále také přesuň do special_actions.* funkci ast_attach_expr ze souboru
        semantic_actions.* a přejmenuj ji na special_action_expr. Tělo funkce
        neměň.

        Mapování neterminál na číslo pravidla:
        NONTERM_BLOCK_OR_EXPR 20
        NONTERM_COMMAND 45
        NONTERM_COMMAND_LIST 23
        NONTERM_COMMAND_SIMPLE 30
        NONTERM_RETURN_VALUE 35
        NONTERM_BLOCK_OR_COMMAND 44
        NONTERM_THEN_BRANCH 56
        NONTERM_OPTIONAL_ELSE 49
        NONTERM_EXPR_OR_INDENT_EXPR 52
        NONTERM_ELSE_TERNARY_IF 54

    Prompt pro Claude (doplneni parser_common.h):
        Zde je parser_common.h; `parser_push_static_production` je aktuálně v
        predictive_parser.h - pokud by měl vzniknout cyklus, tak ji ručně
        přesunu třeba do production.* kde také dává smysl, aby byla.
*/

/**
 * @brief Specialni akce provedeni pravidla pro vstupni terminal, kterym smi
 * zacinat vyraz.
 *
 * Jadro ji vola, kdyz je na vstupu terminal, ktery neindexuje LL tabulku, ale
 * muze jim zacinat vyraz. Tato situace je pri volani jiz overena, funkce
 * vstupni terminal nijak nekontroluje. Podle neterminalu z vrcholu zasobniku
 * vybere pravidlo, ktere se pro vyraz na vstupu pouzije, a provede ho vlozenim
 * prave strany staticke produkce na zasobnik (zadne z vybranych pravidel nema
 * semantickou akci). Neterminal musi byt v dobe volani uz odebrany ze
 * zasobniku, jeho semanticky ukazatel a anotaci predava jadro v top a funkce je
 * podle pravidla pro staticke produkce zkopiruje do vsech vlozenych symbolu.
 *
 * Mapovani neterminalu na pravidla gramatiky:
 * - NONTERM_BLOCK_OR_EXPR: pravidlo 20,
 * - NONTERM_COMMAND: pravidlo 45,
 * - NONTERM_COMMAND_LIST: pravidlo 23,
 * - NONTERM_COMMAND_SIMPLE: pravidlo 30,
 * - NONTERM_RETURN_VALUE: pravidlo 35,
 * - NONTERM_BLOCK_OR_COMMAND: pravidlo 44,
 * - NONTERM_THEN_BRANCH: pravidlo 56,
 * - NONTERM_OPTIONAL_ELSE: pravidlo 49,
 * - NONTERM_EXPR_OR_INDENT_EXPR: pravidlo 52,
 * - NONTERM_ELSE_TERNARY_IF: pravidlo 54.
 *
 * Pro jiny neterminal skonci program chybou ERR_SYNTAX s hlasenim "LL Parser:
 * Neni mozne zvolit pravidlo.".
 *
 * @param ctx Spolecny kontext parseru, z nej se bere cislo radku pro chybu.
 * @param stack Zasobnik prediktivniho parseru, na nej se vklada produkce.
 * @param top Neterminal odebrany z vrcholu zasobniku spolu s jeho semantickym
 *        ukazatelem a kontextovou anotaci.
 */
void special_action_execute_expr_rule(ParserContext *ctx, Stack *stack,
                                      const AnnotatedSymbol *top);

/**
 * @brief Ulozi hotovy vyraz podle kontextove anotace.
 *
 * Vola ji vyrazovy parser po dokonceni vyrazu. Podle anotace vyraz ulozi na
 * misto urcene ukazatelem (hodnota returnu, inicializace promenne, hodnota
 * prirazeni, podminka cyklu while nebo if, zdroj nebo straz cyklu for).
 * Pri ostatnich anotacich se vyraz prida jako samostatny prikaz do bloku
 * ziskaneho funkci special_action_block. Drive ast_attach_expr v predictive_semantic_actions.*.
 *
 * @param ptr Semanticky ukazatel ulozeny u neterminalu, jehoz vyraz se uklada.
 * @param annotation Kontextova anotace ulozena u tohoto neterminalu.
 * @param expr Hotovy vyraz, funkce prebira jeho vlastnictvi.
 * @param line Cislo radku pro uzly AST a chybova hlaseni.
 */
void special_action_expr(void *ptr, ContextAnnotation annotation, Expr *expr,
                         int line);

#endif // SPECIAL_ACTIONS_H
