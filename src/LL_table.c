/*
    IFJ projekt 2026

    xbalaka00 Artur Balak

    LL_table.c

    Puvodni autor: xbalaka00 za asistence Claude a konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - staticke produkce presunuty do production.c a doplneny
          o staticka pravidla cele gramatiky.
        - xbalaka00 - LL tabulka vyplnena podle tabulky gramatiky
          (LL_gramatika.xlsx), odstranena testovaci pravidla a produkce
          krome <expr> => * a <expr> => ID.
        - xbalaka00 - TOKEN_STAR jako docasny zastupce sloupce *dalsi,
          odstraneno testovaci pravidlo <expr> => ID.
        - xbalaka00 - pridana funkce ll_table_lookup.
        - xbalaka00 - poznamky o <exprNoID> upraveny (neterminal byl odstranen).
        - xbalaka00 - odkazy na excel s gramatikou zmeneny na
          parser_formalisms.xlsx.
        - xbalaka00 - velikost tabulky je NONTERM_LL_TABLE_COUNT radku,
          ll_table_lookup kontroluje rozsah dle novych oddelovacu (prompt je
          v nonterminal.h). Docasny zastupce sloupce *dalsi (TOKEN_STAR) a
          testovaci pravidlo <expr> => * zustavaji v tabulce.
        - xbalaka00 - odstraneny vsechny zaznamy sloupce TOKEN_STAR (docasny
          zastupce sloupce *dalsi, ktery je nyni v jadre parseru) vcetne
          testovaciho pravidla <expr> => * v radku NONTERM_EXPR, ktery je
          tedy prazdny. TOKEN_STAR uz LL tabulku neindexuje (prompt je v
          token.h).
        - xbalaka00 - cisla pravidel v komentarich maji tvar L<n> po vzoru
          E<n> u precedencnich pravidel (prompt je v token.h).

    Tento soubor obsahuje LL tabulku prediktivniho parseru vyplnenou podle
    tabulky gramatiky (parser_formalisms.xlsx), makra pro zapis jejich polozek
    a funkci pro vyhledani pravidla v tabulce.
    Staticke produkce, na ktere se tabulka odkazuje, jsou v production.c,
    semanticke akce v semantic_actions.c. Sloupec *dalsi a neterminal v
    odpovednosti jadra parseru (<IDcommand>) v tabulce nejsou. Radek <expr>
    je prazdny, prechod do vyrazove analyzy resi jadro parseru.
    Pravidla jsou cislovana s prefixem L (napr. L8) po vzoru pravidel E8
    vyrazove analyzy, cisla odpovidaji tabulce gramatiky.

*/

#include "LL_table.h"

#include <stdio.h>

#include "error.h"
#include "nonterminal.h"
#include "production.h"
#include "semantic_actions.h"
#include "token.h"

/*
    Prompt pro odstraneni TOKEN_STAR z LL tabulky a zmenu formatu cisel
    pravidel na L<n> je v souboru token.h.
*/

/*
    Prompt pro zmenu velikosti tabulky na NONTERM_LL_TABLE_COUNT a upravu
    kontrol v ll_table_lookup je v souboru nonterminal.h.
*/

/*
    Prompty pro funkci ll_table_lookup jsou v souboru predictive_parser.c. Prompt pro
    prejmenovani odkazu na excel je v souboru production.c.
*/

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

/*
    Prompt pro vyplneni LL tabulky z tabulky gramatiky a odstraneni testovacich
    pravidel. Stejny prompt zpusobil zmeny take v semantic_actions.c
    a semantic_actions.h, jeho zneni je pouze zde.

    Prompt pro Claude:
        Dobrá, nyní bych přešel na převedení LL tabulky z excelu do kódu.
        Vyplň/přepiš LL tabulku na základě excelu po vzoru současného vyplnění
        testovacími pravidly v souboru LL_table.c callbacky a statickými
        produkcemi ze souborů static_production a využij příslušné makra a
        pomocné funkce.

        Sloupec s *další a chování neterminálů, které jsou odpovědností jádra
        prozatím neber v úvahu; pouze vyplň LL tabulku. Smaž metody a statické
        produkce atp., které byly ponechány pouze pro účely testovací
        gramatiky. Ponechej však testovací pravidla <expr> -> * a <expr> -> ID
        a ponech jim také záznamy v LL tabulce; výrazová analýza zatím není
        napsaná, proto je nutné prozatím testovat bez ní.

    Prompt pro docasneho zastupce sloupce *dalsi a odstraneni pravidla
    <expr> => ID. Stejny prompt zpusobil zmeny take v production.c
    a production.h, jeho zneni je pouze zde.

    Prompt pro Claude:
        Hmm, to se zástupcem *další je pravda. Sloupeček *další bude ve
        finální verzi řešit jádro, protože to bude prakticky fallback pro
        vstupní terminály, které nejsou indexátory LL tabulky, ale může jimi
        začínat výraz - pak bude provedeno příslušné pravidlo ze sloupce dle
        neterminálu na vrcholu zásobníku, případně chyba, pokud by na vrcholu
        nebyl jeden z těchto neterminálů. Možná tedy hvězdičku jako zástupce za
        *další přidej, aby byla prozatím všechna pravidla, která mají na prvním
        místě produkce <expr> nebo by k tomuto směřovaly, provedena a
        korektně se dokonvergovalo se k terminálu <expr>, který však prozatím
        bude pouze přepsán na hvězdičku testovacím pravidlem. Tím pádem také
        odstraň pravidlo <expr> -> ID; bude to přehlednější, když všude, kde je
        očekáván výraz, prozatím bude napevno hvězdička pro testovací účely. Až
        se bude doplňovat funkcionalita výrazového parseru, tak se tento
        dočasný zástupce za *další a dočasné pravidlo <expr> -> * odstraní a
        bude zavedeno finální řešení do jádra. Před plným testováním LL
        gramatiky pak bude stačit doplnit funkcionalitu jádrových pravidel
        souvisejících s <IDcommand> (to ale počká po dokončení LL tabulky).
*/

// Zapis polozky tabulky se statickou produkci
#define LL_PROD(p)                                                             \
  { .type = LL_RULE_STATIC, .production = &(p) }

// Zapis polozky tabulky se semantickou akci (callbackem)
#define LL_CALLBACK(f)                                                         \
  { .type = LL_RULE_SEMANTIC, .callback = (f) }

// LL tabulka podle tabulky gramatiky (parser_formalisms.xlsx). Polozky bez
// zaznamu jsou LL_RULE_ERROR (hodnota 0). Sloupec *dalsi a neterminal <IDcommand>
// resi jadro parseru, v tabulce proto nejsou. Neterminal <exprNoID> z tabulky
// gramatiky v implementaci neexistuje, nahrazuje ho <expr>. Komentare L<n>
// uvadeji cisla pravidel z tabulky gramatiky.
const LLTableEntry LL_TABLE[NONTERM_LL_TABLE_COUNT][TOKEN_LL_TERM_COUNT] = {
    // L0
    [NONTERM_PROGRAM] =
        {
            [TOKEN_KW_IMPORT] = LL_CALLBACK(semantic_action_prolog),
        },

    // L1, L2
    [NONTERM_DEF_LIST] =
        {
            [TOKEN_KW_DEF] = LL_PROD(PRODUCTION_DEF_LIST_NEXT),
            [TOKEN_ANNOT_MAIN] = LL_PROD(PRODUCTION_DEF_LIST_NEXT),
            [TOKEN_EOF] = LL_PROD(PRODUCTION_DEF_LIST_END),
        },

    // L3
    [NONTERM_DEF] =
        {
            [TOKEN_KW_DEF] = LL_CALLBACK(semantic_action_function_definition),
            [TOKEN_ANNOT_MAIN] =
                LL_CALLBACK(semantic_action_function_definition),
        },

    // L4, L5
    [NONTERM_MAIN_ANNOTATION] =
        {
            [TOKEN_ANNOT_MAIN] = LL_CALLBACK(semantic_action_main_annotation),
            [TOKEN_KW_DEF] = LL_PROD(PRODUCTION_MAIN_ABSENT),
        },

    // L6, L7
    [NONTERM_OPTIONAL_EOL] =
        {
            [TOKEN_EOL] = LL_PROD(PRODUCTION_OPTIONAL_EOL_PRESENT),
            [TOKEN_KW_DEF] = LL_PROD(PRODUCTION_OPTIONAL_EOL_ABSENT),
        },

    // L8, L10
    [NONTERM_PARAM_LIST] =
        {
            [TOKEN_ID] = LL_CALLBACK(semantic_action_param_add),
            [TOKEN_RPAREN] = LL_PROD(PRODUCTION_PARAM_LIST_END),
        },

    // L9, L11
    [NONTERM_PARAM_LIST_TAIL] =
        {
            [TOKEN_COMMA] = LL_PROD(PRODUCTION_PARAM_LIST_TAIL_NEXT),
            [TOKEN_RPAREN] = LL_PROD(PRODUCTION_PARAM_LIST_TAIL_END),
        },

    // L12, L13
    [NONTERM_OPTIONAL_TYPE] =
        {
            [TOKEN_COLON] = LL_PROD(PRODUCTION_OPTIONAL_TYPE_PRESENT),
            [TOKEN_ASSIGN] = LL_PROD(PRODUCTION_OPTIONAL_TYPE_ABSENT),
        },

    // L14-L19
    [NONTERM_TYPE] =
        {
            [TOKEN_KW_INT] = LL_CALLBACK(semantic_action_type_int),
            [TOKEN_KW_DOUBLE] = LL_CALLBACK(semantic_action_type_double),
            [TOKEN_KW_STRING] = LL_CALLBACK(semantic_action_type_string),
            [TOKEN_KW_UNIT] = LL_CALLBACK(semantic_action_type_unit),
            [TOKEN_KW_BOOLEAN] = LL_CALLBACK(semantic_action_type_boolean),
            [TOKEN_KW_LIST] = LL_CALLBACK(semantic_action_type_list),
        },

    // L20, L21
    [NONTERM_BLOCK_OR_EXPR] =
        {
            [TOKEN_EOL] = LL_PROD(PRODUCTION_BLOCK_OR_EXPR_BLOCK),
            [TOKEN_ID] = LL_PROD(PRODUCTION_BLOCK_OR_EXPR_EXPR),
            [TOKEN_KW_IF] = LL_PROD(PRODUCTION_BLOCK_OR_EXPR_EXPR),
            [TOKEN_KW_LIST] = LL_PROD(PRODUCTION_BLOCK_OR_EXPR_EXPR),
            [TOKEN_LPAREN] = LL_PROD(PRODUCTION_BLOCK_OR_EXPR_EXPR),
        },

    // L22
    [NONTERM_BLOCK] =
        {
            [TOKEN_EOL] = LL_CALLBACK(semantic_action_block),
        },

    // L23, L24
    [NONTERM_COMMAND_LIST] =
        {
            [TOKEN_DEDENT] = LL_PROD(PRODUCTION_COMMAND_LIST_END),
            [TOKEN_ID] = LL_PROD(PRODUCTION_COMMAND_LIST_NEXT),
            [TOKEN_KW_IF] = LL_PROD(PRODUCTION_COMMAND_LIST_NEXT),
            [TOKEN_KW_RETURN] = LL_PROD(PRODUCTION_COMMAND_LIST_NEXT),
            [TOKEN_KW_WHILE] = LL_PROD(PRODUCTION_COMMAND_LIST_NEXT),
            [TOKEN_KW_FOR] = LL_PROD(PRODUCTION_COMMAND_LIST_NEXT),
            [TOKEN_KW_VAL] = LL_PROD(PRODUCTION_COMMAND_LIST_NEXT),
            [TOKEN_KW_VAR] = LL_PROD(PRODUCTION_COMMAND_LIST_NEXT),
            [TOKEN_KW_LIST] = LL_PROD(PRODUCTION_COMMAND_LIST_NEXT),
            [TOKEN_LPAREN] = LL_PROD(PRODUCTION_COMMAND_LIST_NEXT),
        },

    // L25, L26
    [NONTERM_COMMAND_END] =
        {
            [TOKEN_EOL] = LL_PROD(PRODUCTION_COMMAND_END_EOL),
            [TOKEN_SEMICOLON] = LL_PROD(PRODUCTION_COMMAND_END_SEMICOLON),
        },

    // L45, L46
    [NONTERM_COMMAND] =
        {
            [TOKEN_ID] = LL_PROD(PRODUCTION_COMMAND_SIMPLE),
            [TOKEN_KW_RETURN] = LL_PROD(PRODUCTION_COMMAND_SIMPLE),
            [TOKEN_KW_VAL] = LL_PROD(PRODUCTION_COMMAND_SIMPLE),
            [TOKEN_KW_VAR] = LL_PROD(PRODUCTION_COMMAND_SIMPLE),
            [TOKEN_KW_LIST] = LL_PROD(PRODUCTION_COMMAND_SIMPLE),
            [TOKEN_LPAREN] = LL_PROD(PRODUCTION_COMMAND_SIMPLE),
            [TOKEN_KW_IF] = LL_PROD(PRODUCTION_COMMAND_CONTROL),
            [TOKEN_KW_WHILE] = LL_PROD(PRODUCTION_COMMAND_CONTROL),
            [TOKEN_KW_FOR] = LL_PROD(PRODUCTION_COMMAND_CONTROL),
        },

    // L55, L56, L57
    [NONTERM_THEN_BRANCH] =
        {
            [TOKEN_EOL] = LL_PROD(PRODUCTION_THEN_BRANCH_BLOCK),
            [TOKEN_ID] = LL_PROD(PRODUCTION_THEN_BRANCH_SIMPLE),
            [TOKEN_KW_RETURN] = LL_PROD(PRODUCTION_THEN_BRANCH_SIMPLE),
            [TOKEN_KW_VAL] = LL_PROD(PRODUCTION_THEN_BRANCH_SIMPLE),
            [TOKEN_KW_VAR] = LL_PROD(PRODUCTION_THEN_BRANCH_SIMPLE),
            [TOKEN_KW_LIST] = LL_PROD(PRODUCTION_THEN_BRANCH_SIMPLE),
            [TOKEN_LPAREN] = LL_PROD(PRODUCTION_THEN_BRANCH_SIMPLE),
            [TOKEN_KW_IF] = LL_PROD(PRODUCTION_THEN_BRANCH_CONTROL),
            [TOKEN_KW_WHILE] = LL_PROD(PRODUCTION_THEN_BRANCH_CONTROL),
            [TOKEN_KW_FOR] = LL_PROD(PRODUCTION_THEN_BRANCH_CONTROL),
        },

    // L58, L59
    [NONTERM_ELSE_OR_COMMAND_END] =
        {
            [TOKEN_SEMICOLON] =
                LL_PROD(PRODUCTION_ELSE_OR_COMMAND_END_COMMAND_END),
            [TOKEN_EOL] = LL_PROD(PRODUCTION_ELSE_OR_COMMAND_END_COMMAND_END),
            [TOKEN_KW_ELSE] =
                LL_CALLBACK(semantic_action_else_or_command_end_else),
        },

    // L27, L30, L31, L34 (jednoduche prikazy)
    [NONTERM_COMMAND_SIMPLE] =
        {
            [TOKEN_ID] = LL_CALLBACK(semantic_action_id_command),
            [TOKEN_KW_RETURN] = LL_CALLBACK(semantic_action_return_command),
            [TOKEN_KW_VAL] = LL_CALLBACK(semantic_action_declaration_command),
            [TOKEN_KW_VAR] = LL_CALLBACK(semantic_action_declaration_command),
            [TOKEN_KW_LIST] = LL_PROD(PRODUCTION_COMMAND_SIMPLE_EXPR),
            [TOKEN_LPAREN] = LL_PROD(PRODUCTION_COMMAND_SIMPLE_EXPR),
        },

    // L37, L38, L47 (ridici prikazy)
    [NONTERM_COMMAND_CONTROL] =
        {
            [TOKEN_KW_IF] = LL_CALLBACK(semantic_action_if_command),
            [TOKEN_KW_WHILE] = LL_CALLBACK(semantic_action_while_command),
            [TOKEN_KW_FOR] = LL_CALLBACK(semantic_action_for_command),
        },

    // L28, L29
    [NONTERM_VAL_VAR] =
        {
            [TOKEN_KW_VAL] = LL_CALLBACK(semantic_action_val),
            [TOKEN_KW_VAR] = LL_PROD(PRODUCTION_VAR),
        },

    // L35, L36
    [NONTERM_RETURN_VALUE] =
        {
            [TOKEN_EOL] = LL_PROD(PRODUCTION_RETURN_VALUE_EMPTY),
            [TOKEN_SEMICOLON] = LL_PROD(PRODUCTION_RETURN_VALUE_EMPTY),
            [TOKEN_KW_ELSE] = LL_PROD(PRODUCTION_RETURN_VALUE_EMPTY),
            [TOKEN_ID] = LL_PROD(PRODUCTION_RETURN_VALUE_EXPR),
            [TOKEN_KW_IF] = LL_PROD(PRODUCTION_RETURN_VALUE_EXPR),
            [TOKEN_KW_LIST] = LL_PROD(PRODUCTION_RETURN_VALUE_EXPR),
            [TOKEN_LPAREN] = LL_PROD(PRODUCTION_RETURN_VALUE_EXPR),
        },

    // L43, L44
    [NONTERM_BLOCK_OR_COMMAND] =
        {
            [TOKEN_EOL] = LL_PROD(PRODUCTION_BLOCK_OR_COMMAND_BLOCK),
            [TOKEN_ID] = LL_PROD(PRODUCTION_BLOCK_OR_COMMAND_COMMAND),
            [TOKEN_KW_IF] = LL_PROD(PRODUCTION_BLOCK_OR_COMMAND_COMMAND),
            [TOKEN_KW_RETURN] = LL_PROD(PRODUCTION_BLOCK_OR_COMMAND_COMMAND),
            [TOKEN_KW_WHILE] = LL_PROD(PRODUCTION_BLOCK_OR_COMMAND_COMMAND),
            [TOKEN_KW_FOR] = LL_PROD(PRODUCTION_BLOCK_OR_COMMAND_COMMAND),
            [TOKEN_KW_VAL] = LL_PROD(PRODUCTION_BLOCK_OR_COMMAND_COMMAND),
            [TOKEN_KW_VAR] = LL_PROD(PRODUCTION_BLOCK_OR_COMMAND_COMMAND),
            [TOKEN_KW_LIST] = LL_PROD(PRODUCTION_BLOCK_OR_COMMAND_COMMAND),
            [TOKEN_LPAREN] = LL_PROD(PRODUCTION_BLOCK_OR_COMMAND_COMMAND),
        },

    // L39, L40
    [NONTERM_FOR_HEADER] =
        {
            [TOKEN_EOL] = LL_CALLBACK(semantic_action_for_header_block),
            [TOKEN_ID] = LL_CALLBACK(semantic_action_for_header_inline),
        },

    // L41, L42
    [NONTERM_GUARD_LIST] =
        {
            [TOKEN_DEDENT] = LL_PROD(PRODUCTION_GUARD_LIST_END),
            [TOKEN_KW_IF] = LL_CALLBACK(semantic_action_guard_next),
        },

    // L48, L49
    [NONTERM_OPTIONAL_ELSE] =
        {
            [TOKEN_KW_ELSE] =
                LL_CALLBACK(semantic_action_optional_else_present),
            [TOKEN_DEDENT] = LL_PROD(PRODUCTION_OPTIONAL_ELSE_ABSENT),
            [TOKEN_ID] = LL_PROD(PRODUCTION_OPTIONAL_ELSE_ABSENT),
            [TOKEN_KW_RETURN] = LL_PROD(PRODUCTION_OPTIONAL_ELSE_ABSENT),
            [TOKEN_KW_VAL] = LL_PROD(PRODUCTION_OPTIONAL_ELSE_ABSENT),
            [TOKEN_KW_VAR] = LL_PROD(PRODUCTION_OPTIONAL_ELSE_ABSENT),
            [TOKEN_KW_LIST] = LL_PROD(PRODUCTION_OPTIONAL_ELSE_ABSENT),
            [TOKEN_LPAREN] = LL_PROD(PRODUCTION_OPTIONAL_ELSE_ABSENT),
            [TOKEN_KW_IF] = LL_PROD(PRODUCTION_OPTIONAL_ELSE_ABSENT),
            [TOKEN_KW_WHILE] = LL_PROD(PRODUCTION_OPTIONAL_ELSE_ABSENT),
            [TOKEN_KW_FOR] = LL_PROD(PRODUCTION_OPTIONAL_ELSE_ABSENT),
        },

    // L50
    [NONTERM_TERNARY_IF] =
        {
            [TOKEN_KW_IF] = LL_CALLBACK(semantic_action_ternary_if),
        },

    // L51, L52
    [NONTERM_EXPR_OR_INDENT_EXPR] =
        {
            [TOKEN_EOL] = LL_PROD(PRODUCTION_EXPR_OR_INDENT_EXPR_INDENTED),
            [TOKEN_ID] = LL_PROD(PRODUCTION_EXPR_OR_INDENT_EXPR_PLAIN),
            [TOKEN_KW_IF] = LL_PROD(PRODUCTION_EXPR_OR_INDENT_EXPR_PLAIN),
            [TOKEN_KW_LIST] = LL_PROD(PRODUCTION_EXPR_OR_INDENT_EXPR_PLAIN),
            [TOKEN_LPAREN] = LL_PROD(PRODUCTION_EXPR_OR_INDENT_EXPR_PLAIN),
        },

    // L53, L54
    [NONTERM_ELSE_TERNARY_IF] =
        {
            [TOKEN_EOL] = LL_CALLBACK(semantic_action_else_ternary_if_indented),
            [TOKEN_ID] = LL_PROD(PRODUCTION_ELSE_TERNARY_IF_ABSENT),
            [TOKEN_KW_IF] = LL_PROD(PRODUCTION_ELSE_TERNARY_IF_ABSENT),
            [TOKEN_KW_ELSE] = LL_PROD(PRODUCTION_ELSE_TERNARY_IF_ABSENT),
            [TOKEN_KW_THEN] = LL_PROD(PRODUCTION_ELSE_TERNARY_IF_ABSENT),
            [TOKEN_KW_DO] = LL_PROD(PRODUCTION_ELSE_TERNARY_IF_ABSENT),
            [TOKEN_KW_LIST] = LL_PROD(PRODUCTION_ELSE_TERNARY_IF_ABSENT),
            [TOKEN_LPAREN] = LL_PROD(PRODUCTION_ELSE_TERNARY_IF_ABSENT),
            [TOKEN_SEMICOLON] = LL_PROD(PRODUCTION_ELSE_TERNARY_IF_ABSENT),
        }
};


const LLTableEntry *ll_table_lookup(NonterminalType nonterm, TokenType terminal,
                                    int line) {

  // Do LL tabulky lze indexovat jen neterminaly pred NONTERM_LL_TABLE_COUNT
  if (nonterm >= NONTERM_LL_TABLE_COUNT)
    error_exit(ERR_INTERNAL, line,
               "LL Parser: Spatne pouziti ll_table_lookup(): Neplatny "
               "neterminal.");

  // Do LL tabulky lze indexovat jen tokeny pred TOKEN_LL_TERM_COUNT
  if (terminal >= TOKEN_LL_TERM_COUNT)
    error_exit(ERR_INTERNAL, line,
               "LL Parser: Spatne pouziti ll_table_lookup(): Terminal neni "
               "indexem LL tabulky.");

  // Vyhledani pravidla
  const LLTableEntry *entry = &LL_TABLE[nonterm][terminal];

  if (entry->type == LL_RULE_ERROR) {
    fprintf(stderr,
            "LL Parser: Soucasny vstupni terminal: %s\nSoucasny vrchol "
            "zasobniku: %s\n",
            token_type_name(terminal), nonterm_type_name(nonterm));

    error_exit(ERR_SYNTAX, line, "LL Parser: Neni mozne zvolit pravidlo.");
  }

  // Kontrola platnosti zaznamu podle typu pravidla
  if (entry->type == LL_RULE_STATIC) {
    if (entry->production == NULL ||
        entry->production->length > STATIC_PRODUCTION_MAX_LENGTH)
      error_exit(ERR_INTERNAL, line,
                 "LL Parser: Neplatna staticka produkce v LL tabulce.");
  }

  else if (entry->type == LL_RULE_SEMANTIC) {
    if (entry->callback == NULL)
      error_exit(ERR_INTERNAL, line,
                 "LL Parser: Callback v LL tabulce je NULL.");
  }

  else {
    // Neznamy typ pravidla
    error_exit(ERR_INTERNAL, line,
               "LL Parser: Neznamy typ pravidla v LL tabulce.");
  }

  return entry;
}
