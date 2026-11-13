/*
    IFJ projekt 2026

    semantic_actions.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - pridany callbacky vsech pravidel gramatiky se
          semantickou akci.
        - xbalaka00 - funkce push_terminal a push_nonterminal vraceny jako
          wrappery se vsemi atributy zaznamu na zasobniku.
        - xbalaka00 - odstranen callback pravidla 32 (nyni v jadre parseru),
          doplnene poznamky k pravidlum dle nove verze gramatiky.

    Tento soubor obsahuje semanticke akce pravidel LL parseru. Akce zatim
    pouze vypisuji cislo pravidla na stderr a vkladaji pravou stranu pravidla
    na zasobnik, atributy pro stavbu stromu zatim nepridavaji.

*/

#include "semantic_actions.h"

#include <stdio.h>

#include "nonterminal.h"
#include "parser_common.h"
#include "production.h"
#include "token.h"

/*
    Prompt pro semanticke akce je v souboru LL_table.c.

    Prompt pro doplneni callbacku vsech pravidel se semantickou akci a pro
    odstraneni funkci push_terminal a push_nonterminal (viz nasledujici prompt
    o jejich vraceni). Stejny prompt zpusobil
    zmeny take v semantic_actions.h, nonterminal.h a nonterminal.c, jeho zneni
    je pouze zde.

    Prompt pro Claude:
        Dobrá, nyní můžeme přejít na ty sémantické callbacky. Přikládám
        ostatní relevantní soubory.

        Callbacky prozatím budou pouze dávat věci na stack po vzoru testovacích
        pravidel. Pro testovací účely budou také vypisovat pravidlo, které bylo
        použito. Později v nich bude budování AST a hrstka dalších funkcí, ale
        prozatím chci kostry, do kterých bude možné doplňovat.

        Na rozdíl od testovacích pravidel bych možná opustil funkce
        push_terminal a push_nonterminal jako wrappery nad parser_stack_push a
        dával věci na stack přímo; důvodem je, aby bylo později jednoduché
        měnit atributy, které se doplní k neterminálům a někdy i terminálům dle
        potřeb sémantiky. Zatím však všude dej NULL, resp. ID_CONTEXT_NONE na
        místa sémantického pointeru, resp. IDContextType.

        Opět drž styl jednotný a doplň prompt a změnové záznamy.

        LL tabulku prozatím neměň, ta se doplní v příštím kroku na základě LL
        tabulky, kterou ještě musím vypočítat (chtěl bych to udělat ručně).

        Zde je deklarace parser_stack_push:
        <zde jsem prekopiroval doxygen deklaraci funkce parser_stack_push>

    Prompty pro aktualizaci dle nove verze gramatiky a pro upravu komentaru na
    ceskou terminologii jsou v souboru production.c.

    Prompt pro vraceni wrapperu push_terminal a push_nonterminal.

    Prompt pro Claude:
        Dobrá, ty dlouhé přetypování a makra nejsou příliš pěkné. Možná je
        přece jen vrať jako wrapper funkce pro terminál a pro neterminál, ale
        ať mají argument pro každou hodnotu, která do stacku může být vložena.
        Tyto wrappery schovají prakticky jenom to makro a přetypování, ale
        bude to s nimi stravitelnější.
*/

// Vypise cislo pravidla na stderr (ladici vystup)
static void trace_rule(int rule_number) {
  fprintf(stderr, "Pravidlo %d\n", rule_number);
}

// Vlozi na zasobnik terminal. Funkce pouze schovava makro a pretypovani,
// predavaji se vsechny hodnoty zaznamu na zasobniku.
static void push_terminal(Stack *stack, TokenType type, void *semantic_pointer,
                          IDContextType id_type) {
  parser_stack_push(stack, (GrammarSymbol)GRAMMAR_TERMINAL(type),
                    semantic_pointer, id_type);
}

// Vlozi na zasobnik neterminal. Kontext ID je pro neterminaly vzdy
// ID_CONTEXT_NONE, proto se nepredava.
static void push_nonterminal(Stack *stack, NonterminalType type,
                             void *semantic_pointer) {
  parser_stack_push(stack, (GrammarSymbol)GRAMMAR_NONTERMINAL(type),
                    semantic_pointer, ID_CONTEXT_NONE);
}

// Symboly prave strany se vkladaji od posledniho, aby na vrcholu zasobniku
// zustal prvni symbol. Semanticky ukazatel (NULL) a kontext ID
// (ID_CONTEXT_NONE) se zatim u zadneho symbolu nepouzivaji, pozdeji se zde
// nahradi hodnotami podle potreby stavby AST.

// Pravidlo 0 (Prolog): <program> => import ID . * EOL <defList> EOF
// Pozn.: Semanticka akce u ID zkontroluje, ze text ID je ifj26.
void semantic_action_prolog(SemanticContext *ctx, void *semantic_pointer,
                            Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(0);

  push_terminal(stack, TOKEN_EOF, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_DEF_LIST, NULL);
  push_terminal(stack, TOKEN_EOL, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_STAR, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_DOT, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_ID, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_KW_IMPORT, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 3 (Deklarace funkce): <def> => <main> def ID ( <paramAdd> ) <optionalType> = <blockOrExpr>
// Pozn.: EOL/INDENT je reseno v pravidle pro blok.
void semantic_action_function_definition(SemanticContext *ctx,
                                         void *semantic_pointer, Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(3);

  push_nonterminal(stack, NONTERM_BLOCK_OR_EXPR, NULL);
  push_terminal(stack, TOKEN_ASSIGN, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_OPTIONAL_TYPE, NULL);
  push_terminal(stack, TOKEN_RPAREN, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_PARAM_LIST, NULL);
  push_terminal(stack, TOKEN_LPAREN, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_ID, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_KW_DEF, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_MAIN_ANNOTATION, NULL);
}

// Pravidlo 4 (Anotace main): <main> => @main <optionalEOL>
// Pozn.: @main muze byt pred kazdou funkci.
void semantic_action_main_annotation(SemanticContext *ctx,
                                     void *semantic_pointer, Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(4);

  push_nonterminal(stack, NONTERM_OPTIONAL_EOL, NULL);
  push_terminal(stack, TOKEN_ANNOT_MAIN, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 8 (Pridani parametru): <paramAdd> => ID : <type> <paramComma>
void semantic_action_param_add(SemanticContext *ctx, void *semantic_pointer,
                               Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(8);

  push_nonterminal(stack, NONTERM_PARAM_LIST_TAIL, NULL);
  push_nonterminal(stack, NONTERM_TYPE, NULL);
  push_terminal(stack, TOKEN_COLON, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_ID, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 12 (Navratovy typ je): <optionalType> => : <type>
// Pozn.: pravidlo 12 je v gramatice nyni ptr (staticke, production.c), callback
// je ponechan jen dokud na nej odkazuje LL_table.c.
void semantic_action_optional_type_present(SemanticContext *ctx,
                                           void *semantic_pointer,
                                           Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(12);

  push_nonterminal(stack, NONTERM_TYPE, NULL);
  push_terminal(stack, TOKEN_COLON, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 14 (Typ Integer): <type> => Int
void semantic_action_type_int(SemanticContext *ctx, void *semantic_pointer,
                              Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(14);

  push_terminal(stack, TOKEN_KW_INT, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 15 (Typ Double): <type> => Double
void semantic_action_type_double(SemanticContext *ctx, void *semantic_pointer,
                                 Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(15);

  push_terminal(stack, TOKEN_KW_DOUBLE, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 16 (Typ String): <type> => String
void semantic_action_type_string(SemanticContext *ctx, void *semantic_pointer,
                                 Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(16);

  push_terminal(stack, TOKEN_KW_STRING, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 17 (Typ Unit): <type> => Unit
void semantic_action_type_unit(SemanticContext *ctx, void *semantic_pointer,
                               Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(17);

  push_terminal(stack, TOKEN_KW_UNIT, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 18 (Typ Boolean): <type> => Boolean
void semantic_action_type_boolean(SemanticContext *ctx, void *semantic_pointer,
                                  Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(18);

  push_terminal(stack, TOKEN_KW_BOOLEAN, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 19 (Typ List): <type> => List [ <type> ]
// Pozn.: Seznamy seznamu jsou povoleny.
void semantic_action_type_list(SemanticContext *ctx, void *semantic_pointer,
                               Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(19);

  push_terminal(stack, TOKEN_RBRACKET, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_TYPE, NULL);
  push_terminal(stack, TOKEN_LBRACKET, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_KW_LIST, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 22 (Blok a prvni prikaz): <block> => EOL INDENT <command> <commandList> DEDENT
// Pozn.: Musi byt pritomen alespon jeden prikaz.
void semantic_action_block(SemanticContext *ctx, void *semantic_pointer,
                           Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(22);

  push_terminal(stack, TOKEN_DEDENT, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_COMMAND_LIST, NULL);
  push_nonterminal(stack, NONTERM_COMMAND, NULL);
  push_terminal(stack, TOKEN_INDENT, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_EOL, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 27 (Prikaz deklarace): <command> => <valVar> ID <optionalType> = <expr> <commandEnd>
// Pozn.: U deklarace se predpoklada var, pravidlo 28 to pripadne prepise.
void semantic_action_declaration_command(SemanticContext *ctx,
                                         void *semantic_pointer, Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(27);

  push_nonterminal(stack, NONTERM_COMMAND_END, NULL);
  push_nonterminal(stack, NONTERM_EXPR, NULL);
  push_terminal(stack, TOKEN_ASSIGN, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_OPTIONAL_TYPE, NULL);
  push_terminal(stack, TOKEN_ID, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_VAL_VAR, NULL);
}

// Pravidlo 28 (Deklarace nemodifikovatelne promenne): <valVar> => val
void semantic_action_val(SemanticContext *ctx, void *semantic_pointer,
                         Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(28);

  push_terminal(stack, TOKEN_KW_VAL, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 31 (Prikaz zacinajici ID): <command> => ID <IDcommand> <commandEnd>
// Pozn.: Neterminal <IDcommand> (pravidla 32 a 33) je reseny v jadre parseru,
// ne v LL tabulce: pri vstupnim = se provede prirazeni, jinak se do vyrazove
// analyzy preda ID.
void semantic_action_id_command(SemanticContext *ctx, void *semantic_pointer,
                                Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(31);

  push_nonterminal(stack, NONTERM_COMMAND_END, NULL);
  push_nonterminal(stack, NONTERM_ID_COMMAND, NULL);
  push_terminal(stack, TOKEN_ID, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 34 (Prikaz return): <command> => return <returnValue> <commandEnd>
void semantic_action_return_command(SemanticContext *ctx,
                                    void *semantic_pointer, Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(34);

  push_nonterminal(stack, NONTERM_COMMAND_END, NULL);
  push_nonterminal(stack, NONTERM_RETURN_VALUE, NULL);
  push_terminal(stack, TOKEN_KW_RETURN, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 37 (Cyklus while): <command> => while <expr> do <blockOrCommand>
void semantic_action_while_command(SemanticContext *ctx, void *semantic_pointer,
                                   Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(37);

  push_nonterminal(stack, NONTERM_BLOCK_OR_COMMAND, NULL);
  push_terminal(stack, TOKEN_KW_DO, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR, NULL);
  push_terminal(stack, TOKEN_KW_WHILE, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 38 (Cyklus for): <command> => for <forHeader> do <blockOrCommand>
void semantic_action_for_command(SemanticContext *ctx, void *semantic_pointer,
                                 Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(38);

  push_nonterminal(stack, NONTERM_BLOCK_OR_COMMAND, NULL);
  push_terminal(stack, TOKEN_KW_DO, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_FOR_HEADER, NULL);
  push_terminal(stack, TOKEN_KW_FOR, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 39 (Jednoradkova hlavicka for): <forHeader> => ID <- <expr>
void semantic_action_for_header_inline(SemanticContext *ctx,
                                       void *semantic_pointer, Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(39);

  push_nonterminal(stack, NONTERM_EXPR, NULL);
  push_terminal(stack, TOKEN_LARROW, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_ID, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 40 (Viceradkova hlavicka for): <forHeader> => EOL INDENT ID <- <expr> EOL <guardList> DEDENT
// Pozn.: Je treba rozlisit jednoradkovy a viceradkovy zapis hlavicky for.
void semantic_action_for_header_block(SemanticContext *ctx,
                                      void *semantic_pointer, Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(40);

  push_terminal(stack, TOKEN_DEDENT, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_GUARD_LIST, NULL);
  push_terminal(stack, TOKEN_EOL, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR, NULL);
  push_terminal(stack, TOKEN_LARROW, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_ID, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_INDENT, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_EOL, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 41 (Dalsi straz): <guardList> => if <expr> EOL <guardList>
// Pozn.: Jednoradkova hlavicka neumoznuje straze.
void semantic_action_guard_next(SemanticContext *ctx, void *semantic_pointer,
                                Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(41);

  push_nonterminal(stack, NONTERM_GUARD_LIST, NULL);
  push_terminal(stack, TOKEN_EOL, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR, NULL);
  push_terminal(stack, TOKEN_KW_IF, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 47 (Podminka): <command> => if <expr> then <blockOrCommand> <optionalElse>
// Pozn.: Konstrukce if else je pokryta jednoradkovym blokem s prikazem if.
void semantic_action_if_command(SemanticContext *ctx, void *semantic_pointer,
                                Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(47);

  push_nonterminal(stack, NONTERM_OPTIONAL_ELSE, NULL);
  push_nonterminal(stack, NONTERM_BLOCK_OR_COMMAND, NULL);
  push_terminal(stack, TOKEN_KW_THEN, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR, NULL);
  push_terminal(stack, TOKEN_KW_IF, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 48 (Nepovinna cast else): <optionalElse> => else <blockOrCommand>
// Pozn.: Visici else patri k nejblizsimu if, konflikt v LL tabulce je vyresen
// rucne.
void semantic_action_optional_else_present(SemanticContext *ctx,
                                           void *semantic_pointer,
                                           Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(48);

  push_nonterminal(stack, NONTERM_BLOCK_OR_COMMAND, NULL);
  push_terminal(stack, TOKEN_KW_ELSE, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 50 (Ternarni podminka): <ternaryIf> => if <expr> then <exprOrIndentExpr> else <elseTernaryIf>
// Pozn.: Pravidlo se zacne zpracovavat, kdyz vyrazovy parser zavola LL parser s
// pocatecnim neterminalem <ternaryIf> a ukazatelem na zaznam, ktery chce
// vyrazovka umistit na svuj zasobnik.
void semantic_action_ternary_if(SemanticContext *ctx, void *semantic_pointer,
                                Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(50);

  push_nonterminal(stack, NONTERM_ELSE_TERNARY_IF, NULL);
  push_terminal(stack, TOKEN_KW_ELSE, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR_OR_INDENT_EXPR, NULL);
  push_terminal(stack, TOKEN_KW_THEN, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR, NULL);
  push_terminal(stack, TOKEN_KW_IF, NULL, ID_CONTEXT_NONE);
}

// Pravidlo 53 (Odsazena vetev else): <elseTernaryIf> => EOL INDENT <expr> EOL DEDENT
// Pozn.: Mimo zavorky je nutne podporovat odsazeny vyraz. Provedenim pravidla
// se vrati vyraz s hotovou ternarni podminkou a volajici vyrazovka se rovnou
// ukonci. Pravidla 51 a 53 maji shodnou pravou stranu, ale lisi se chovanim.
void semantic_action_else_ternary_if_indented(SemanticContext *ctx,
                                              void *semantic_pointer,
                                              Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  trace_rule(53);

  push_terminal(stack, TOKEN_DEDENT, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_EOL, NULL, ID_CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR, NULL);
  push_terminal(stack, TOKEN_INDENT, NULL, ID_CONTEXT_NONE);
  push_terminal(stack, TOKEN_EOL, NULL, ID_CONTEXT_NONE);
}
