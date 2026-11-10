/*
    IFJ projekt 2026

    LL_table.c

    Puvodni autor: xbalaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - staticke produkce presunuty do production.c a doplneny
          o staticka pravidla cele gramatiky.

    Tento soubor obsahuje LL tabulku prediktivniho parseru a staticke produkce,
    na ktere se tabulka odkazuje. Prozatim je vyplnena jen testovaci podmnozina
    gramatiky: prikazy val/var a return, typy Int a Double a zastupny vyraz.
    Cisla pravidel odpovidaji tabulce gramatiky (LL_gramatika.xlsx).

*/

#include "LL_table.h"

#include "nonterminal.h"
#include "production.h"
#include "semantic_actions.h"
#include "token.h"

/*
    Prompt pro presun statickych produkci do production.c a doplneni pravidel
    gramatiky je v souboru production.c.
*/

/*
    Prompt pro vychozi testovaci obsah LL tabulky, staticke produkce, makra
    LL_PROD a LL_CALLBACK a semanticke akce v semantic_actions.c. Stejny prompt
    zpusobil zmeny take v semantic_actions.h, semantic_actions.c a production.h,
    jeho zneni je pouze zde.

    Prompt pro Claude:
        Dobrá, teď už můžeme udělat samotná testovací pravidla. Zatím bych je
        nedělal z celé gramatiky, jak ji máš v excelu - ten stejně ještě musím
        trochu doladit a převést na LL tabulku. Můžeme pro testovací účely
        vytvořit smyšlenou gramatiku vycházející z podmnožiny IFJ26.

        Dejme tomu, že začínat budeme od <commandList> (kvůli ternárnímu ifu
        budu parser psát tak, aby mohl začínat na libovolném neterminálu a
        končil vyprázdněním zásobníku), a zatím jako příkazy budeme mít pouze
        val/var deklaraci a return. Vygeneruj tedy cvičně pravidla 23-29 a
        34-36, a k nim budou potřeba ještě pravidla 12-15 kvůli typům. Testy
        tedy budou probíhat na prográmcích složených pouze z příkazů val/var a
        return, které budou stát samy o sobě. Testovací prostředí pro jednotku
        parser doplníme později, spíše chci mít představu, jak bude vypadat
        tabulka pravidel už s konkrétními vyplněnými místy.

        Prozatím bych se vyhnul výrazům, protože ještě není hotový výrazový
        parser - můžeš přidat třeba pravidlo <expr> -> * a <expr> -> ID, aby na
        místě výrazu bylo buď * nebo ID samo o sobě. V hotovém parseru bude
        <expr> na vrcholu zásobníku obvykle značit přechod do výrazového
        parseru.

        Ve finální podobě to tak nejspíše nebude, ale prozatím vygeneruj
        pravidla pro deklaraci, return a optional type, které se přepíše na typ
        (ne na epsilon) jako callbackové a ostatní jako jednoduché statické
        produkce. Statická produkce v sobě obsahuje pravou stranu pravidla
        přímo, kterou parser pouze vloží na stack sám.

        Statické produkce budou jako globální proměnné definovány a
        inicializovány prozatím přímo v souboru LL-table.c, později se možná
        dají do production.c, aby to bylo přehledněji rozděleno, ale to by
        vyžadovalo spoustu hlaviček. Možná to ale můžeš udělat rovnou v
        production.c, jestli chceš. Callbacky budou v semantic_actions.c s
        deklarací v .h, a prozatím bude callback vypadat tak, že vypíše číslo
        pravidla na stderr (aby šlo vidět, co parser dělá) a poté pomocí funkce
        parser_stack_push na zásobník přidá pravou stranu pravidla. V tomto
        kroku by přidal i své atributy pro stavbu stromu, ale ty prozatím nech
        třeba na NULL a ID_CONTEXT_NONE.

        No a soubor LL_table.c bude obsahovat samotnou LL tabulku zaplněnou
        callbacky a statickými produkcemi z ostatních souborů. Položky bez
        záznamu by měly už samy o sobě být prázdné díky tomu, že v enumu
        LLRuleType je explicitně LL_RULE_ERROR = 0, tedy by mělo stačit
        inicializovat pouze záznamy se skutečnou hodnotou.

        ChatGPT pro to vypsal následující příklad:
        const LLTableEntry LL_table[NONTERM_COUNT][TOKEN_LL_TERM_COUNT] = {
            [NONTERM_PROGRAM] = {
                [TOKEN_ID] = LL_CALLBACK(callback_program_id),
                [TOKEN_IF] = LL_PROD(production_program_if),
            },

            [NONTERM_STATEMENT] = {
                [TOKEN_ID] = LL_PROD(production_statement_id),
                [TOKEN_IF] = LL_PROD(production_statement_if),
            },
        };

        Kde LL_PROD a LL_CALLBACK jsou makra pro zpřehlednění zápisu. Tyto makra
        můžeš rovněž vytvořit, měl by jsi mít dost typových informací. Chat GPT
        pro ně uvedl příklad:
        #define LL_PROD(p) \
            { .type = LL_ENTRY_PRODUCTION, .production = &(p) }

        #define LL_CALLBACK(f) \
            { .type = LL_ENTRY_CALLBACK, .callback = (f) }

        Takto velká změna bude samostatný commit - přidej tedy i záznamy o
        změnách.
*/

// Zapis polozky tabulky se statickou produkci
#define LL_PROD(p)                                                             \
  { .type = LL_RULE_STATIC, .production = &(p) }

// Zapis polozky tabulky se semantickou akci (callbackem)
#define LL_CALLBACK(f)                                                         \
  { .type = LL_RULE_SEMANTIC, .callback = (f) }

// Staticke produkce. Pravou stranu pravidla parser vlozi na zasobnik sam.

// Pozn.: pravidla 14, 15 a 28 jsou v gramatice se semantickou akci, jejich
// produkce zde jsou jen testovaci pozustatek, dokud se LL tabulka neprepise
// na callbacky. Ostatni staticke produkce jsou v production.c.

// Pravidlo 14: <type> => Int
static const StaticProduction PRODUCTION_TYPE_INT = {
    .length = 1,
    .production = {GRAMMAR_TERMINAL(TOKEN_KW_INT)},
};

// Pravidlo 15: <type> => Double
static const StaticProduction PRODUCTION_TYPE_DOUBLE = {
    .length = 1,
    .production = {GRAMMAR_TERMINAL(TOKEN_KW_DOUBLE)},
};

// Pravidlo 28: <valVar> => val
static const StaticProduction PRODUCTION_VAL = {
    .length = 1,
    .production = {GRAMMAR_TERMINAL(TOKEN_KW_VAL)},
};

// LL tabulka. Polozky bez zaznamu jsou LL_RULE_ERROR (hodnota 0).
const LLTableEntry LL_TABLE[NONTERM_COUNT][TOKEN_LL_TERM_COUNT] = {
    // Pravidla 6, 7
    [NONTERM_OPTIONAL_EOL] =
        {
            [TOKEN_EOL] = LL_PROD(PRODUCTION_OPTIONAL_EOL_PRESENT),
            // Za EOL nasleduje dalsi prikaz nebo konec seznamu prikazu
            [TOKEN_KW_VAL] = LL_PROD(PRODUCTION_OPTIONAL_EOL_ABSENT),
            [TOKEN_KW_VAR] = LL_PROD(PRODUCTION_OPTIONAL_EOL_ABSENT),
            [TOKEN_KW_RETURN] = LL_PROD(PRODUCTION_OPTIONAL_EOL_ABSENT),
            [TOKEN_DEDENT] = LL_PROD(PRODUCTION_OPTIONAL_EOL_ABSENT),
            [TOKEN_EOF] = LL_PROD(PRODUCTION_OPTIONAL_EOL_ABSENT),
        },

    // Pravidla 12, 13
    [NONTERM_OPTIONAL_TYPE] =
        {
            [TOKEN_COLON] = LL_CALLBACK(semantic_action_optional_type_present),
            [TOKEN_ASSIGN] = LL_PROD(PRODUCTION_OPTIONAL_TYPE_ABSENT),
        },

    // Pravidla 14, 15
    [NONTERM_TYPE] =
        {
            [TOKEN_KW_INT] = LL_PROD(PRODUCTION_TYPE_INT),
            [TOKEN_KW_DOUBLE] = LL_PROD(PRODUCTION_TYPE_DOUBLE),
        },

    // Pravidla 23, 24
    [NONTERM_COMMAND_LIST] =
        {
            [TOKEN_KW_VAL] = LL_PROD(PRODUCTION_COMMAND_LIST_NEXT),
            [TOKEN_KW_VAR] = LL_PROD(PRODUCTION_COMMAND_LIST_NEXT),
            [TOKEN_KW_RETURN] = LL_PROD(PRODUCTION_COMMAND_LIST_NEXT),
            // Seznam konci DEDENT, v testech samostatny program konci EOF
            [TOKEN_DEDENT] = LL_PROD(PRODUCTION_COMMAND_LIST_END),
            [TOKEN_EOF] = LL_PROD(PRODUCTION_COMMAND_LIST_END),
        },

    // Pravidla 25, 26
    [NONTERM_COMMAND_END] =
        {
            [TOKEN_SEMICOLON] = LL_PROD(PRODUCTION_COMMAND_END_SEMICOLON),
            [TOKEN_EOL] = LL_PROD(PRODUCTION_COMMAND_END_EOL),
        },

    // Pravidla 27, 34 (zatim jen deklarace a return)
    [NONTERM_COMMAND] =
        {
            [TOKEN_KW_VAL] = LL_CALLBACK(semantic_action_declaration_command),
            [TOKEN_KW_VAR] = LL_CALLBACK(semantic_action_declaration_command),
            [TOKEN_KW_RETURN] = LL_CALLBACK(semantic_action_return_command),
        },

    // Pravidla 28, 29
    [NONTERM_VAL_VAR] =
        {
            [TOKEN_KW_VAL] = LL_PROD(PRODUCTION_VAL),
            [TOKEN_KW_VAR] = LL_PROD(PRODUCTION_VAR),
        },

    // Pravidla 35, 36
    [NONTERM_RETURN_VALUE] =
        {
            // Zacatek vyrazu, v testech jen ID nebo *
            [TOKEN_ID] = LL_PROD(PRODUCTION_RETURN_VALUE_EXPR),
            [TOKEN_STAR] = LL_PROD(PRODUCTION_RETURN_VALUE_EXPR),
            // Za prazdnym returnem nasleduje konec prikazu
            [TOKEN_SEMICOLON] = LL_PROD(PRODUCTION_RETURN_VALUE_EMPTY),
            [TOKEN_EOL] = LL_PROD(PRODUCTION_RETURN_VALUE_EMPTY),
        },

    // Testovaci pravidla mimo gramatiku, v hotovem parseru je <expr> prechod
    // do vyrazoveho parseru
    [NONTERM_EXPR] =
        {
            [TOKEN_STAR] = LL_PROD(PRODUCTION_TEST_EXPR_STAR),
            [TOKEN_ID] = LL_PROD(PRODUCTION_TEST_EXPR_ID),
        },
};
