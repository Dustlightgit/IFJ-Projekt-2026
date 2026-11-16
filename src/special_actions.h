/*
    IFJ projekt 2026

    special_actions.h

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - special_action_id vytvari AST uzel pro ID pri anotaci
          CONTEXT_ID_COMMAND.
        - xbalaka00 - pridana prozatim prazdna deklarace pro specialni akci predzpracovani terminalu
        - xbalaka00 - doplnena dokumentace a funkcionalita akce
          special_action_precedence_preprocessing (prompt je zde).

    Tento soubor obsahuje deklarace specialnich akci parseru, tedy callbacku,
    ktere nejsou vazane na pravidlo LL tabulky a maji vlastni rozhrani
    (napr. akce pro token ID, ktera dostava cely vstupni token). Zatim jsou
    to jen kostry, funkcionalita se doplni pri propojeni s AST.
*/

#ifndef SPECIAL_ACTIONS_H
#define SPECIAL_ACTIONS_H

#include "semantic_types.h"
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

#endif // SPECIAL_ACTIONS_H
