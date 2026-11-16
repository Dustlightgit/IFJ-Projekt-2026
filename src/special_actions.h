/*
    IFJ projekt 2026

    special_actions.h

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - special_action_id vytvari AST uzel pro ID pri anotaci
          CONTEXT_ID_COMMAND.
        - xbalaka00 - pridana prozatim prazdna deklarace pro specialni akci predzpracovani terminalu

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

#endif // SPECIAL_ACTIONS_H

/* Popis a implementace budou doplneny pozdeji */
void *special_action_precedence_preprocessing(Token *input_token);
