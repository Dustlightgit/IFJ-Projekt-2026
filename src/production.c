/*
    IFJ projekt 2026

    production.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        -

    Tento soubor obsahuje staticke produkce LL gramatiky, tedy pravidla bez
    semanticke akce, a to jak s predanim semantickeho ukazatele (ptr), tak bez
    nej. Cisla pravidel odpovidaji tabulce gramatiky (LL_gramatika.xlsx).
    Pravidla se semantickou akci ("ano") jsou zpracovana callbacky.

*/

#include "production.h"

#include "nonterminal.h"
#include "token.h"

/*
    Prompt pro vychozi obsah souboru se statickymi produkcemi. Stejny prompt
    zpusobil zmeny take v production.h a LL_table.c, jeho zneni je pouze zde.

    Prompt pro Claude:
        Ahoj. Mám udělané jádro prediktivního parseru založeného na LL tabulce.
        Prozatím jsem do něj vygeneroval pouze testovací pravidla, avšak nastal
        čas přidat tam mnou navrženou gramatiku, která je v přiloženém excelu.

        Vše o formátu pravidel by mělo být v hlavičkových souborech. Pravidla
        existují dvojí - statická a callbacková. Pro začátek vygenerujeme pouze
        statická pravidla.

        V souboru LL_table.c se můžeš inspirovat, jakým způsobem se pravidla
        vkládají. Jedná se o konstanty typu StaticProduction.

        Pravidla pro přehlednost vygeneruj v novém souboru production.c
        odpovídajícího hlavičkovému souboru production.h. K jednotlivým
        pravidlům vytvoř i záznam v .h tak, aby je šlo importovat v LL_table.c
        pro použití v LL tabulce. Tento import rovnou můžeš přidat a smíš smazat
        stará testovací statická pravidla ze souboru LL_table.c. Jinak ale nic
        jiného v kódu neměň.

        Zatím generujeme pouze tvar statických pravidel, nikoliv LL tabulku
        samotnou.

        Drž se stylu v ostatních hlavičkových souborech a dodržuj naming
        conventions včetně komentářů v češtině bez diakritiky a rozlišení
        doxygen/jednoduchých. Dodrž formát hlavičky souboru a přidej změnový
        záznam. Po vzoru ostatních souborů přidej také tento prompt na
        nejvhodnější místo.

        Vygeneruj zatím pouze statická pravidla, které jsou v kolonce Sém. akce
        anotována jako "ano ptr" nebo mají prázdné políčko (obě jsou statické
        pravidla, prakticky v nich není rozdíl). Ostatní pravidla přijdou na
        řadu později.

        Symboly do produkcí dej v "přirozeném pořadí" - parser je bude na stack
        dávat v pořadí snižujícího se indexu.

        Tabulka neterminálů by mohla být zastaralá, protože pravidla jsem od
        jejího vytvoření upravoval a některá přibyla. Pokud to budeš potřebovat,
        přidej si do něj chybějící neterminály z tabulky.

        Prozatím ponechej také testovací pravidlo <expr> -> * protože výrazový
        parser zatím není hotový.

        Vygenerováný kód můžeš zkusit zkompilovat, ale netestuj ho - testování
        pak proběhne na našem parseru.

        Zkus pravidla vhodně anotovat po vzoru testovacích pravidel na základě
        excelu.

        Pokud něco není jasné, zeptej se, než začneš generovat.
*/

// Symboly jsou v produkcich v prirozenem poradi (zleva doprava), parser je
// na zasobnik vklada od posledniho. Pravidla oznacena (ptr) predavaji
// semanticky ukazatel z neterminalu na leve strane do symbolu produkce.

// Pravidlo 1 (Seznam deklaraci) (ptr): <defList> => <def> <defList>
const StaticProduction PRODUCTION_DEF_LIST_NEXT = {
    .length = 2,
    .production =
        {
            GRAMMAR_NONTERMINAL(NONTERM_DEF),
            GRAMMAR_NONTERMINAL(NONTERM_DEF_LIST),
        },
};

// Pravidlo 2 (Konec deklaraci): <defList> => epsilon
const StaticProduction PRODUCTION_DEF_LIST_END = {.length = 0};

// Pravidlo 5 (Anotace neni): <main> => epsilon
const StaticProduction PRODUCTION_MAIN_ABSENT = {.length = 0};

// Pravidlo 6 (EOL je): <optionalEOL> => EOL
const StaticProduction PRODUCTION_OPTIONAL_EOL_PRESENT = {
    .length = 1,
    .production = {GRAMMAR_TERMINAL(TOKEN_EOL)},
};

// Pravidlo 7 (EOL neni): <optionalEOL> => epsilon
const StaticProduction PRODUCTION_OPTIONAL_EOL_ABSENT = {.length = 0};

// Pravidlo 9 (Pridani carky) (ptr): <paramComma> => , <paramAdd>
const StaticProduction PRODUCTION_PARAM_LIST_TAIL_NEXT = {
    .length = 2,
    .production =
        {
            GRAMMAR_TERMINAL(TOKEN_COMMA),
            GRAMMAR_NONTERMINAL(NONTERM_PARAM_LIST),
        },
};

// Pravidlo 10 (Konec parametru A): <paramAdd> => epsilon
const StaticProduction PRODUCTION_PARAM_LIST_END = {.length = 0};

// Pravidlo 11 (Konec parametru B): <paramComma> => epsilon
const StaticProduction PRODUCTION_PARAM_LIST_TAIL_END = {.length = 0};

// Pravidlo 12 (Navratovy typ je) (ptr): <optionalType> => : <type>
const StaticProduction PRODUCTION_OPTIONAL_TYPE_PRESENT = {
    .length = 2,
    .production =
        {
            GRAMMAR_TERMINAL(TOKEN_COLON),
            GRAMMAR_NONTERMINAL(NONTERM_TYPE),
        },
};

// Pravidlo 13 (Navratovy typ neni): <optionalType> => epsilon
const StaticProduction PRODUCTION_OPTIONAL_TYPE_ABSENT = {.length = 0};

// Pravidlo 20 (Inline expr misto bloku) (ptr): <blockOrExpr> => <expr> EOL
const StaticProduction PRODUCTION_BLOCK_OR_EXPR_EXPR = {
    .length = 2,
    .production =
        {
            GRAMMAR_NONTERMINAL(NONTERM_EXPR),
            GRAMMAR_TERMINAL(TOKEN_EOL),
        },
};

// Pravidlo 21 (Pouziti bloku) (ptr): <blockOrExpr> => <block>
const StaticProduction PRODUCTION_BLOCK_OR_EXPR_BLOCK = {
    .length = 1,
    .production = {GRAMMAR_NONTERMINAL(NONTERM_BLOCK)},
};

// Pravidlo 23 (Dalsi prikaz) (ptr): <commandList> => <command> <commandList>
const StaticProduction PRODUCTION_COMMAND_LIST_NEXT = {
    .length = 2,
    .production =
        {
            GRAMMAR_NONTERMINAL(NONTERM_COMMAND),
            GRAMMAR_NONTERMINAL(NONTERM_COMMAND_LIST),
        },
};

// Pravidlo 24 (Konec seznamu prikazu): <commandList> => epsilon
const StaticProduction PRODUCTION_COMMAND_LIST_END = {.length = 0};

// Pravidlo 25 (Konec prikazu - strednik): <commandEnd> => ; <optionalEOL>
const StaticProduction PRODUCTION_COMMAND_END_SEMICOLON = {
    .length = 2,
    .production =
        {
            GRAMMAR_TERMINAL(TOKEN_SEMICOLON),
            GRAMMAR_NONTERMINAL(NONTERM_OPTIONAL_EOL),
        },
};

// Pravidlo 26 (Konec prikazu - EOL): <commandEnd> => EOL
const StaticProduction PRODUCTION_COMMAND_END_EOL = {
    .length = 1,
    .production = {GRAMMAR_TERMINAL(TOKEN_EOL)},
};

// Pravidlo 29 (Deklarace variable): <valVar> => var
const StaticProduction PRODUCTION_VAR = {
    .length = 1,
    .production = {GRAMMAR_TERMINAL(TOKEN_KW_VAR)},
};

// Pravidlo 30 (Prikaz vyrazu) (ptr): <command> => <exprNoID> <commandEnd>
const StaticProduction PRODUCTION_COMMAND_EXPR = {
    .length = 2,
    .production =
        {
            GRAMMAR_NONTERMINAL(NONTERM_EXPR_NO_ID),
            GRAMMAR_NONTERMINAL(NONTERM_COMMAND_END),
        },
};

// Pravidlo 33 (Vyraz zacinajici ID) (ptr): <IDcommand> => <expr>
const StaticProduction PRODUCTION_ID_COMMAND_EXPR = {
    .length = 1,
    .production = {GRAMMAR_NONTERMINAL(NONTERM_EXPR)},
};

// Pravidlo 35 (Return vyrazu) (ptr): <returnValue> => <expr>
const StaticProduction PRODUCTION_RETURN_VALUE_EXPR = {
    .length = 1,
    .production = {GRAMMAR_NONTERMINAL(NONTERM_EXPR)},
};

// Pravidlo 36 (Prazdny return): <returnValue> => epsilon
const StaticProduction PRODUCTION_RETURN_VALUE_EMPTY = {.length = 0};

// Pravidlo 42 (Konec guardu): <guardList> => epsilon
const StaticProduction PRODUCTION_GUARD_LIST_END = {.length = 0};

// Pravidlo 43 (Nasleduje blok) (ptr): <blockOrCommand> => <block>
const StaticProduction PRODUCTION_BLOCK_OR_COMMAND_BLOCK = {
    .length = 1,
    .production = {GRAMMAR_NONTERMINAL(NONTERM_BLOCK)},
};

// Pravidlo 44 (Nasleduje prikaz) (ptr): <blockOrCommand> => <command> <optionalSemicolon> EOL
const StaticProduction PRODUCTION_BLOCK_OR_COMMAND_COMMAND = {
    .length = 3,
    .production =
        {
            GRAMMAR_NONTERMINAL(NONTERM_COMMAND),
            GRAMMAR_NONTERMINAL(NONTERM_OPTIONAL_SEMICOLON),
            GRAMMAR_TERMINAL(TOKEN_EOL),
        },
};

// Pravidlo 45 (Na konci je strednik): <optionalSemicolon> => ;
const StaticProduction PRODUCTION_OPTIONAL_SEMICOLON_PRESENT = {
    .length = 1,
    .production = {GRAMMAR_TERMINAL(TOKEN_SEMICOLON)},
};

// Pravidlo 46 (Na konci neni strednik): <optionalSemicolon> => epsilon
const StaticProduction PRODUCTION_OPTIONAL_SEMICOLON_ABSENT = {.length = 0};

// Pravidlo 49 (Vynechane else): <optionalElse> => epsilon
const StaticProduction PRODUCTION_OPTIONAL_ELSE_ABSENT = {.length = 0};

// Pravidlo 51 (Odsazeni vyrazu v ifu) (ptr): <exprOrIndentExpr> => EOL INDENT <expr> EOL DEDENT
const StaticProduction PRODUCTION_EXPR_OR_INDENT_EXPR_INDENTED = {
    .length = 5,
    .production =
        {
            GRAMMAR_TERMINAL(TOKEN_EOL),
            GRAMMAR_TERMINAL(TOKEN_INDENT),
            GRAMMAR_NONTERMINAL(NONTERM_EXPR),
            GRAMMAR_TERMINAL(TOKEN_EOL),
            GRAMMAR_TERMINAL(TOKEN_DEDENT),
        },
};

// Pravidlo 52 (Neodsazeny vyraz v ifu) (ptr): <exprOrIndentExpr> => <expr>
const StaticProduction PRODUCTION_EXPR_OR_INDENT_EXPR_PLAIN = {
    .length = 1,
    .production = {GRAMMAR_NONTERMINAL(NONTERM_EXPR)},
};

// Pravidlo 54 (Nedokoncena else vetev): <elseTernaryIf> => epsilon
const StaticProduction PRODUCTION_ELSE_TERNARY_IF_ABSENT = {.length = 0};

// Testovaci pravidla mimo gramatiku, v hotovem parseru je <expr> prechod
// do vyrazoveho parseru

// Testovaci pravidlo mimo gramatiku: <expr> => *
const StaticProduction PRODUCTION_TEST_EXPR_STAR = {
    .length = 1,
    .production = {GRAMMAR_TERMINAL(TOKEN_STAR)},
};

// Testovaci pravidlo mimo gramatiku: <expr> => ID
const StaticProduction PRODUCTION_TEST_EXPR_ID = {
    .length = 1,
    .production = {GRAMMAR_TERMINAL(TOKEN_ID)},
};
