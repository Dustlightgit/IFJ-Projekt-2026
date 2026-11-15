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
        - xbalaka00 - odstranen callback pravidla 12 (testovaci pozustatek,
          pravidlo je staticke).
        - xbalaka00 - callbacky prijimaji ContextAnnotation, push_terminal
          a push_nonterminal predavaji kontextovou anotaci.
        - xbalaka00 - callbacky prijimaji cislo radku, trace_rule ho vypisuje.
        - xbalaka00 - akce pravidla 31 anotuje ID a predava ukazatel
          neterminalu <IDcommand>.
        - xbalaka00 - push_terminal a push_nonterminal vkladaji zaznamy bez
          znacky redukce (prompt je v parser_common.c).
        - xbalaka00 - include grammar_symbol.h pro makra GRAMMAR_TERMINAL a GRAMMAR_NONTERMINAL (GrammarSymbol presunut do grammar_symbol.h,
          prompt je v reduction_table.h).

    Tento soubor obsahuje semanticke akce pravidel LL parseru. Akce zatim
    pouze vypisuji cislo pravidla na stderr a vkladaji pravou stranu pravidla
    na zasobnik, atributy pro stavbu stromu zatim nepridavaji.

*/

#include "semantic_actions.h"

#include "ast.h"
#include "error.h"
#include <stdio.h>

#include "grammar_symbol.h"
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
        potřeb sémantiky. Zatím však všude dej NULL, resp. CONTEXT_NONE na
        místa sémantického pointeru, resp. IDContextType.

        Opět drž styl jednotný a doplň prompt a změnové záznamy.

        LL tabulku prozatím neměň, ta se doplní v příštím kroku na základě LL
        tabulky, kterou ještě musím vypočítat (chtěl bych to udělat ručně).

        Zde je deklarace parser_stack_push:
        <zde jsem prekopiroval doxygen deklaraci funkce parser_stack_push>

    Prompty pro aktualizaci dle nove verze gramatiky a pro upravu komentaru na
    ceskou terminologii jsou v souboru production.c. Prompt pro vyplneni LL
    tabulky a odstraneni testovacich akci je v souboru LL_table.c. Prompty pro
    predani kontextove anotace a cisla radku callbackum jsou v souboru
    semantic_types.h. Prompt pro chovani jadra pro <IDcommand>, anotaci CONTEXT_ID_COMMAND
    a specialni akci pro ID je v souboru predictive_parser.c.

    Prompt pro vraceni wrapperu push_terminal a push_nonterminal.

    Prompt pro Claude:
        Dobrá, ty dlouhé přetypování a makra nejsou příliš pěkné. Možná je
        přece jen vrať jako wrapper funkce pro terminál a pro neterminál, ale
        ať mají argument pro každou hodnotu, která do stacku může být vložena.
        Tyto wrappery schovají prakticky jenom to makro a přetypování, ale
        bude to s nimi stravitelnější.
*/

// Vypise cislo pravidla a cislo radku na stderr (ladici vystup)
static void trace_rule(int rule_number, int line) {
  fprintf(stderr, "Pravidlo %d (radek %d)\n", rule_number, line);
}

// Vlozi na zasobnik terminal. Funkce pouze schovava makro a pretypovani,
// predavaji se vsechny hodnoty zaznamu na zasobniku.
static void push_terminal(Stack *stack, TokenType type, void *semantic_pointer,
                          ContextAnnotation annotation) {
  parser_stack_push(stack, (GrammarSymbol)GRAMMAR_TERMINAL(type),
                    semantic_pointer, annotation, false);
}

// Vlozi na zasobnik neterminal. Stejne jako u terminalu se predavaji vsechny
// hodnoty zaznamu na zasobniku, kontextovou anotaci lze pripojit i k neterminalu.
static void push_nonterminal(Stack *stack, NonterminalType type,
                             void *semantic_pointer,
                             ContextAnnotation annotation) {
  parser_stack_push(stack, (GrammarSymbol)GRAMMAR_NONTERMINAL(type),
                    semantic_pointer, annotation, false);
}

// Symboly prave strany se vkladaji od posledniho, aby na vrcholu zasobniku
// zustal prvni symbol. Semanticky ukazatel (NULL) a kontextova anotace
// (CONTEXT_NONE) se zatim u zadneho symbolu nepouzivaji, pozdeji se zde
// nahradi hodnotami podle potreby stavby AST. Ukazatel a anotace rozvadeneho
// neterminalu (parametry callbacku) se zatim nevyuzivaji.

// Pravidlo 0 (Prolog): <program> => import ID . * EOL <defList> EOF
// Pozn.: Semanticky pointer je koren AST (Program *), preda se seznamu funkci.
void semantic_action_prolog(SemanticContext *ctx, void *semantic_pointer,
                            ContextAnnotation annotation, int line,
                            Stack *stack) {
  (void)ctx;
  (void)annotation;
  trace_rule(0, line);

  push_terminal(stack, TOKEN_EOF, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_DEF_LIST, semantic_pointer, CONTEXT_NONE);
  push_terminal(stack, TOKEN_EOL, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_STAR, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_DOT, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_ID, NULL, CONTEXT_PROLOGUE_CHECK);
  push_terminal(stack, TOKEN_KW_IMPORT, NULL, CONTEXT_NONE);
}

// Pravidlo 3 (Deklarace funkce): <def> => <main> def ID ( <paramAdd> ) <optionalType> = <blockOrExpr>
// Pozn.: Semanticky pointer je Program *. Vytvori se FuncDef, pripoji se
// k programu a jeho ukazatel dostanou deti, ktere funkci doplnuji.
void semantic_action_function_definition(SemanticContext *ctx,
                                         void *semantic_pointer,
                                         ContextAnnotation annotation, int line,
                                         Stack *stack) {
  (void)ctx;
  (void)annotation;
  trace_rule(3, line);

  Program *program = (Program *)semantic_pointer;
  FuncDef *func =
      funcdef_new(NULL, false, line); // Jmeno doplni special_action_id
  program_add_func(program, func);

  push_nonterminal(stack, NONTERM_BLOCK_OR_EXPR, func, CONTEXT_FUNCTION_BODY);
  push_terminal(stack, TOKEN_ASSIGN, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_OPTIONAL_TYPE, func,
                   CONTEXT_FUNCTION_RETURN_TYPE);
  push_terminal(stack, TOKEN_RPAREN, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_PARAM_LIST, func, CONTEXT_NONE);
  push_terminal(stack, TOKEN_LPAREN, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_ID, func, CONTEXT_FUNCTION_DEF);
  push_terminal(stack, TOKEN_KW_DEF, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_MAIN_ANNOTATION, func, CONTEXT_NONE);
}

// Pravidlo 4 (Anotace main): <main> => @main <optionalEOL>
// Pozn.: Semanticky pointer je FuncDef *, oznaci se jako vstupni funkce.
void semantic_action_main_annotation(SemanticContext *ctx,
                                     void *semantic_pointer,
                                     ContextAnnotation annotation, int line,
                                     Stack *stack) {
  (void)ctx;
  (void)annotation;
  trace_rule(4, line);

  funcdef_set_main((FuncDef *)semantic_pointer, true);

  push_nonterminal(stack, NONTERM_OPTIONAL_EOL, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_ANNOT_MAIN, NULL, CONTEXT_NONE);
}

// Vrati blok, do ktereho se ma pridat prikaz. Pokud pointer ukazuje na
// vlastnika tela (funkce, cyklus), blok se vytvori a pripoji.
Block *target_block(void *ptr, ContextAnnotation annotation, int line) {
  Block *block;
  switch (annotation) {
  case CONTEXT_BLOCK_STMT:
    return (Block *)ptr;
  case CONTEXT_FUNCTION_BODY:
    block = block_new();
    funcdef_set_body((FuncDef *)ptr, block);
    return block;
  case CONTEXT_WHILE_BODY:
    block = block_new();
    stmt_while_set_body((Stmt *)ptr, block);
    return block;
  case CONTEXT_FOR_BODY:
    block = block_new();
    stmt_for_set_body((Stmt *)ptr, block);
    return block;
  case CONTEXT_IF_THEN:
    block = block_new();
    expr_if_set_then((Expr *)ptr, block);
    return block;
  case CONTEXT_IF_ELSE:
    block = block_new();
    expr_if_set_else((Expr *)ptr, block);
    return block;
  default:
    error_exit(ERR_INTERNAL, line, "Prikaz nema urceno, kam se ma pridat");
  }
  return NULL;
}

// Ulozi hotovy vyraz podle anotace (vola ji vyrazovy parser)
void ast_attach_expr(void *ptr, ContextAnnotation annotation, Expr *expr,
                     int line) {
  switch (annotation) {
  case CONTEXT_RETURN_VALUE:
    stmt_return_set_value((Stmt *)ptr, expr);
    break;
  case CONTEXT_VARDEF_INIT:
    stmt_vardef_set_init((Stmt *)ptr, expr);
    break;
  case CONTEXT_ASSIGN_VALUE:
    stmt_assign_set_value((Stmt *)ptr, expr);
    break;

  case CONTEXT_WHILE_COND:
    stmt_while_set_cond((Stmt *)ptr, expr);
    break;
  case CONTEXT_FOR_SOURCE:
    stmt_for_set_source((Stmt *)ptr, expr);
    break;
  case CONTEXT_FOR_GUARD:
    stmt_for_add_guard((Stmt *)ptr, expr);
    break;

  case CONTEXT_IF_COND:
    expr_if_set_cond((Expr *)ptr, expr);
    break;

  default: {
    // Samostatny vyraz (prikaz nebo jednoradkove telo)
    Block *block = target_block(ptr, annotation, line);
    block_add(block, stmt_new_expr(expr, line));
  }
  }
}

// Ulozi hotovy typ na misto urcene ukazatelem a anotaci (prebira vlastnictvi typu)
static void store_type(void *semantic_pointer, ContextAnnotation annotation,
                       Type *type, int line) {
  switch (annotation) {
  case CONTEXT_FUNCTION_RETURN_TYPE:
    funcdef_set_return_type((FuncDef *)semantic_pointer, type);
    break;
  case CONTEXT_PARAM_TYPE: {
    // Typ patri parametru, ktery byl pridan jako posledni
    FuncDef *func = (FuncDef *)semantic_pointer;
    funcdef_param_set_type(func, func->param_count - 1, type);
    break;
  }
  case CONTEXT_LIST_ELEMENT:
    type_list_set_element((Type *)semantic_pointer, type);
    break;

  case CONTEXT_VARDEF_TYPE:
    stmt_vardef_set_type((Stmt *)semantic_pointer, type);
    break;

  default:
    type_free(type);
    error_exit(ERR_INTERNAL, line, "Typ nema urceno, kam se ma ulozit");
  }
}

// Pravidlo 8 (Pridani parametru): <paramAdd> => ID : <type> <paramComma>
void semantic_action_param_add(SemanticContext *ctx, void *semantic_pointer,
                               ContextAnnotation annotation, int line,
                               Stack *stack) {
  (void)ctx;
  (void)annotation;
  trace_rule(8, line);

  push_nonterminal(stack, NONTERM_PARAM_LIST_TAIL, semantic_pointer,
                   CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_TYPE, semantic_pointer, CONTEXT_PARAM_TYPE);
  push_terminal(stack, TOKEN_COLON, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_ID, semantic_pointer, CONTEXT_PARAM_NAME);
}

// Pravidlo 14 (Typ Integer): <type> => Int
void semantic_action_type_int(SemanticContext *ctx, void *semantic_pointer,
                              ContextAnnotation annotation, int line,
                              Stack *stack) {
  (void)ctx;
  trace_rule(14, line);
  store_type(semantic_pointer, annotation, type_new(TYPE_INT), line);
  push_terminal(stack, TOKEN_KW_INT, NULL, CONTEXT_NONE);
}

// Pravidlo 15 (Typ Double): <type> => Double
void semantic_action_type_double(SemanticContext *ctx, void *semantic_pointer,
                                 ContextAnnotation annotation, int line,
                                 Stack *stack) {
  (void)ctx;
  trace_rule(15, line);
  store_type(semantic_pointer, annotation, type_new(TYPE_DOUBLE), line);
  push_terminal(stack, TOKEN_KW_DOUBLE, NULL, CONTEXT_NONE);
}

// Pravidlo 16 (Typ String): <type> => String
void semantic_action_type_string(SemanticContext *ctx, void *semantic_pointer,
                                 ContextAnnotation annotation, int line,
                                 Stack *stack) {
  (void)ctx;
  trace_rule(16, line);
  store_type(semantic_pointer, annotation, type_new(TYPE_STRING), line);
  push_terminal(stack, TOKEN_KW_STRING, NULL, CONTEXT_NONE);
}

// Pravidlo 17 (Typ Unit): <type> => Unit
void semantic_action_type_unit(SemanticContext *ctx, void *semantic_pointer,
                               ContextAnnotation annotation, int line,
                               Stack *stack) {
  (void)ctx;
  trace_rule(17, line);
  store_type(semantic_pointer, annotation, type_new(TYPE_UNIT), line);
  push_terminal(stack, TOKEN_KW_UNIT, NULL, CONTEXT_NONE);
}

// Pravidlo 18 (Typ Boolean): <type> => Boolean
void semantic_action_type_boolean(SemanticContext *ctx, void *semantic_pointer,
                                  ContextAnnotation annotation, int line,
                                  Stack *stack) {
  (void)ctx;

  trace_rule(18, line);
  store_type(semantic_pointer, annotation, type_new(TYPE_BOOLEAN), line);
  push_terminal(stack, TOKEN_KW_BOOLEAN, NULL, CONTEXT_NONE);
}

// Pravidlo 19 (Typ List): <type> => List [ <type> ]
// Pozn.: Seznamy seznamu jsou povoleny.
void semantic_action_type_list(SemanticContext *ctx, void *semantic_pointer,
                               ContextAnnotation annotation, int line,
                               Stack *stack) {
  (void)ctx;
  trace_rule(19, line);

  Type *list = type_new_list(NULL); // Typ prvku se doplni pozdeji
  store_type(semantic_pointer, annotation, list, line);

  push_terminal(stack, TOKEN_RBRACKET, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_TYPE, list, CONTEXT_LIST_ELEMENT);
  push_terminal(stack, TOKEN_LBRACKET, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_KW_LIST, NULL, CONTEXT_NONE);
}

// Pravidlo 22 (Blok a prvni prikaz): <block> => EOL INDENT <command> <commandList> DEDENT
// Pozn.: Musi byt pritomen alespon jeden prikaz.
void semantic_action_block(SemanticContext *ctx, void *semantic_pointer,
                           ContextAnnotation annotation, int line,
                           Stack *stack) {
  (void)ctx;
  trace_rule(22, line);
  Block *block = target_block(semantic_pointer, annotation, line);

  push_terminal(stack, TOKEN_DEDENT, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_COMMAND_LIST, block, CONTEXT_BLOCK_STMT);
  push_nonterminal(stack, NONTERM_COMMAND, block, CONTEXT_BLOCK_STMT);
  push_terminal(stack, TOKEN_INDENT, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_EOL, NULL, CONTEXT_NONE);
}

// Pravidlo 27 (Prikaz deklarace): <command> => <valVar> ID <optionalType> = <expr> <commandEnd>
// Pozn.: U deklarace se predpoklada var, pravidlo 28 to pripadne prepise.
void semantic_action_declaration_command(SemanticContext *ctx,
                                         void *semantic_pointer,
                                         ContextAnnotation annotation, int line,
                                         Stack *stack) {
  (void)ctx;
  trace_rule(27, line);
  Block *block = target_block(semantic_pointer, annotation, line);
  Stmt *stmt = stmt_new_vardef(true, NULL, NULL, NULL, line); // val to prepise
  block_add(block, stmt);

  push_nonterminal(stack, NONTERM_COMMAND_END, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR, stmt, CONTEXT_VARDEF_INIT);
  push_terminal(stack, TOKEN_ASSIGN, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_OPTIONAL_TYPE, stmt, CONTEXT_VARDEF_TYPE);
  push_terminal(stack, TOKEN_ID, stmt, CONTEXT_DECLARATION_NAME);
  push_nonterminal(stack, NONTERM_VAL_VAR, stmt, CONTEXT_NONE);
}

// Pravidlo 28 (Deklarace nemodifikovatelne promenne): <valVar> => val
void semantic_action_val(SemanticContext *ctx, void *semantic_pointer,
                         ContextAnnotation annotation, int line, Stack *stack) {
  (void)ctx;
  (void)annotation;
  trace_rule(28, line);

  stmt_vardef_set_mutable((Stmt *)semantic_pointer, false);
  push_terminal(stack, TOKEN_KW_VAL, NULL, CONTEXT_NONE);
}

// Pravidlo 31 (Prikaz zacinajici ID): <command> => ID <IDcommand> <commandEnd>
// Pozn.: Neterminal <IDcommand> (pravidla 32 a 33) je reseny v jadre parseru,
// ne v LL tabulce: pri vstupnim = se provede prirazeni, jinak se do vyrazove
// analyzy preda ID.
void semantic_action_id_command(SemanticContext *ctx, void *semantic_pointer,
                                ContextAnnotation annotation, int line,
                                Stack *stack) {
  (void)ctx;
  trace_rule(31, line);

  push_nonterminal(stack, NONTERM_COMMAND_END, NULL, CONTEXT_NONE);
  // Pointer a anotace se predaji jadru, ktere z nich pri prirazeni urci blok
  push_nonterminal(stack, NONTERM_ID_COMMAND, semantic_pointer, annotation);
  push_terminal(stack, TOKEN_ID, NULL, CONTEXT_ID_COMMAND);
}

// Pravidlo 34 (Prikaz return): <command> => return <returnValue> <commandEnd>
void semantic_action_return_command(SemanticContext *ctx,
                                    void *semantic_pointer,
                                    ContextAnnotation annotation, int line,
                                    Stack *stack) {
  (void)ctx;

  trace_rule(34, line);
  Block *block = target_block(semantic_pointer, annotation, line);
  Stmt *stmt = stmt_new_return(NULL, line);
  block_add(block, stmt);

  push_nonterminal(stack, NONTERM_COMMAND_END, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_RETURN_VALUE, stmt, CONTEXT_RETURN_VALUE);
  push_terminal(stack, TOKEN_KW_RETURN, NULL, CONTEXT_NONE);
}

// Pravidlo 37 (Cyklus while): <command> => while <expr> do <blockOrCommand>
void semantic_action_while_command(SemanticContext *ctx, void *semantic_pointer,
                                   ContextAnnotation annotation, int line,
                                   Stack *stack) {
  (void)ctx;
  trace_rule(37, line);
  Block *block = target_block(semantic_pointer, annotation, line);
  Stmt *stmt = stmt_new_while(NULL, NULL, line);
  block_add(block, stmt);

  push_nonterminal(stack, NONTERM_BLOCK_OR_COMMAND, stmt, CONTEXT_WHILE_BODY);
  push_terminal(stack, TOKEN_KW_DO, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR, stmt, CONTEXT_WHILE_COND);
  push_terminal(stack, TOKEN_KW_WHILE, NULL, CONTEXT_NONE);
}

// Pravidlo 38 (Cyklus for): <command> => for <forHeader> do <blockOrCommand>
void semantic_action_for_command(SemanticContext *ctx, void *semantic_pointer,
                                 ContextAnnotation annotation, int line,
                                 Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  (void)annotation;
  trace_rule(38, line);
  Block *block = target_block(semantic_pointer, annotation, line);
  Stmt *stmt = stmt_new_for(NULL, NULL, line);
  block_add(block, stmt);

  push_nonterminal(stack, NONTERM_BLOCK_OR_COMMAND, stmt, CONTEXT_FOR_BODY);
  push_terminal(stack, TOKEN_KW_DO, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_FOR_HEADER, stmt, CONTEXT_NONE);
  push_terminal(stack, TOKEN_KW_FOR, NULL, CONTEXT_NONE);
}

// Pravidlo 39 (Jednoradkova hlavicka for): <forHeader> => ID <- <expr>
void semantic_action_for_header_inline(SemanticContext *ctx,
                                       void *semantic_pointer,
                                       ContextAnnotation annotation, int line,
                                       Stack *stack) {
  (void)ctx;
  (void)annotation;
  trace_rule(39, line);

  push_nonterminal(stack, NONTERM_EXPR, semantic_pointer, CONTEXT_FOR_SOURCE);
  push_terminal(stack, TOKEN_LARROW, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_ID, semantic_pointer, CONTEXT_FOR_VAR);
}

// Pravidlo 40 (Viceradkova hlavicka for): <forHeader> => EOL INDENT ID <- <expr> EOL <guardList> DEDENT
// Pozn.: Je treba rozlisit jednoradkovy a viceradkovy zapis hlavicky for.
void semantic_action_for_header_block(SemanticContext *ctx,
                                      void *semantic_pointer,
                                      ContextAnnotation annotation, int line,
                                      Stack *stack) {
  (void)ctx;
  (void)annotation;
  trace_rule(40, line);

  push_terminal(stack, TOKEN_DEDENT, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_GUARD_LIST, semantic_pointer, CONTEXT_NONE);
  push_terminal(stack, TOKEN_EOL, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR, semantic_pointer, CONTEXT_FOR_SOURCE);
  push_terminal(stack, TOKEN_LARROW, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_ID, semantic_pointer, CONTEXT_FOR_VAR);
  push_terminal(stack, TOKEN_INDENT, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_EOL, NULL, CONTEXT_NONE);
}

// Pravidlo 41 (Dalsi straz): <guardList> => if <expr> EOL <guardList>
// Pozn.: Jednoradkova hlavicka neumoznuje straze.
void semantic_action_guard_next(SemanticContext *ctx, void *semantic_pointer,
                                ContextAnnotation annotation, int line,
                                Stack *stack) {
  (void)ctx;
  (void)annotation;
  trace_rule(41, line);

  push_nonterminal(stack, NONTERM_GUARD_LIST, semantic_pointer, CONTEXT_NONE);
  push_terminal(stack, TOKEN_EOL, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR, semantic_pointer, CONTEXT_FOR_GUARD);
  push_terminal(stack, TOKEN_KW_IF, NULL, CONTEXT_NONE);
}

// Pravidlo 47 (Podminka): <command> => if <expr> then <blockOrCommand> <optionalElse>
// Pozn.: Konstrukce if else je pokryta jednoradkovym blokem s prikazem if.
void semantic_action_if_command(SemanticContext *ctx, void *semantic_pointer,
                                ContextAnnotation annotation, int line,
                                Stack *stack) {
  (void)ctx;
  trace_rule(47, line);
  Block *block = target_block(semantic_pointer, annotation, line);
  Expr *ifx = expr_new_if(NULL, NULL, NULL, line);
  block_add(block, stmt_new_expr(ifx, line));

  push_nonterminal(stack, NONTERM_OPTIONAL_ELSE, ifx, CONTEXT_IF_ELSE);
  push_nonterminal(stack, NONTERM_BLOCK_OR_COMMAND, ifx, CONTEXT_IF_THEN);
  push_terminal(stack, TOKEN_KW_THEN, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR, ifx, CONTEXT_IF_COND);
  push_terminal(stack, TOKEN_KW_IF, NULL, CONTEXT_NONE);
}

// Pravidlo 48 (Nepovinna cast else): <optionalElse> => else <blockOrCommand>
// Pozn.: Visici else patri k nejblizsimu if, konflikt v LL tabulce je vyresen
// rucne.
void semantic_action_optional_else_present(SemanticContext *ctx,
                                           void *semantic_pointer,
                                           ContextAnnotation annotation,
                                           int line, Stack *stack) {
  (void)ctx;
  (void)annotation;
  trace_rule(48, line);

  push_nonterminal(stack, NONTERM_BLOCK_OR_COMMAND, semantic_pointer,
                   CONTEXT_IF_ELSE);
  push_terminal(stack, TOKEN_KW_ELSE, NULL, CONTEXT_NONE);
}

// Pravidlo 50 (Ternarni podminka): <ternaryIf> => if <expr> then <exprOrIndentExpr> else <elseTernaryIf>
// Pozn.: Pravidlo se zacne zpracovavat, kdyz vyrazovy parser zavola LL parser s
// pocatecnim neterminalem <ternaryIf> a ukazatelem na zaznam, ktery chce
// vyrazovka umistit na svuj zasobnik.
void semantic_action_ternary_if(SemanticContext *ctx, void *semantic_pointer,
                                ContextAnnotation annotation, int line,
                                Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  (void)annotation;
  trace_rule(50, line);

  push_nonterminal(stack, NONTERM_ELSE_TERNARY_IF, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_KW_ELSE, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR_OR_INDENT_EXPR, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_KW_THEN, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_KW_IF, NULL, CONTEXT_NONE);
}

// Pravidlo 53 (Odsazena vetev else): <elseTernaryIf> => EOL INDENT <expr> EOL DEDENT
// Pozn.: Mimo zavorky je nutne podporovat odsazeny vyraz. Provedenim pravidla
// se vrati vyraz s hotovou ternarni podminkou a volajici vyrazovka se rovnou
// ukonci. Pravidla 51 a 53 maji shodnou pravou stranu, ale lisi se chovanim.
void semantic_action_else_ternary_if_indented(SemanticContext *ctx,
                                              void *semantic_pointer,
                                              ContextAnnotation annotation,
                                              int line, Stack *stack) {
  (void)ctx;
  (void)semantic_pointer;
  (void)annotation;
  trace_rule(53, line);

  push_terminal(stack, TOKEN_DEDENT, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_EOL, NULL, CONTEXT_NONE);
  push_nonterminal(stack, NONTERM_EXPR, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_INDENT, NULL, CONTEXT_NONE);
  push_terminal(stack, TOKEN_EOL, NULL, CONTEXT_NONE);
}

// Docasne pravidlo mimo gramatiku: <expr> => *, vytvori literal 0
void semantic_action_test_expr(SemanticContext *ctx, void *semantic_pointer,
                               ContextAnnotation annotation, int line,
                               Stack *stack) {
  (void)ctx;
  trace_rule(999, line);
  ast_attach_expr(semantic_pointer, annotation, expr_new_int(0, line), line);
  push_terminal(stack, TOKEN_STAR, NULL, CONTEXT_NONE);
}
