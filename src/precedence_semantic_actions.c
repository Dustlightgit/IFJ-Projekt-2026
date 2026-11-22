/*
    IFJ projekt 2026

    xbalaka00 Artur Balak

    precedence_semantic_actions.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - akce pravidel E7 a E8 pocitaji s operatorem
          TOKEN_PLUS_UNARY resp. TOKEN_MINUS_UNARY (prompt je v token.h).
        - xbalaka00 - pridany akce pravidel E32 a E33 (staticka funkce
          seznamu bez parametru a s parametrem), akce puvodniho pravidla E32
          prejmenovana na ...multiple_args a pravidla E33 az E39 precislovana
          na E35 az E41 (prompt je zde).
        - xbalaka00 - akce pravidel E0 az E41 stavi uzly AST z podstromu na
          zasobniku, seznam argumentu ARGS je pomocny uzel EXPR_LIST_LITERAL
          (prompt je zde).
        - xbalaka00 - pridano pravidlo E30 (clenska notace bez parametru),
          pravidla E30 az E41 precislovana na E31 az E42 (prompt je v
          precedence_semantic_actions.c).

    Tento soubor obsahuje semanticke akce pravidel precedencniho parseru. Akce
    vypisuji cislo pravidla na stderr, stavi uzly AST z podstromu ulozenych na zasobniku
    a nastavuji neterminal, na ktery se handle redukuje.
    Semanticke kontroly jsou prevazne zajistovany analyzatorem v souboru semantic_analyzer.*

*/

#include "precedence_semantic_actions.h"

#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "error.h"
#include "grammar_symbol.h"
#include "nonterminal.h"
#include "parser_common.h"
#include "token.h"

/*
    Prompt pro semanticke akce precedencniho parseru. Stejny prompt zpusobil
    vznik take souboru precedence_semantic_actions.h, jeho zneni je pouze
    zde. V promptu je uvedeno PredictiveSemanticAction, myslen je tim
    PrecedenceSemanticAction (viz semantic_types.h). Typ AnnotatedSymbol je
    take deklarovan v semantic_types.h.

    Prompt pro Claude:
        Dobrá, myslím, že je vše v jádře i okolní infrastruktuře připraveno. Přejdeme na generování pravidel na základě parser_formalisms.xlsx - prozatím jen kostry akcí, nebudeme ještě vyplňovat tabulky ani doplňovat stavbu stromu. Soubor ti dávat nebudu, místo toho ti sem zkopíruji tabulku, která je pro tebe relevantní, v textové podobě. Tuto textovou podobu nemusíš zahrnovat do promptu.

        <zde jsem prekopiroval tabulku pravidel z parser_formalisms.xlsx>

        Callbacky budou tvořeny dle typu PredictiveSemanticAction a jejich chování je následující:

        * Po zavolání má funkce garantováno, že prvních n položek na stacku tvoří její handle - důvodem je, že aby callback vůbec mohl být proveden, musí být nalezen v redukční tabulce na základě SearchHandle.
        * Callback tedy má právo používat parser_stack_peek a nahlížet na svoje záznamy a brát z nich informace, které potřebuje.
        * Callback provede stavbu stromu (prozatím nedoplňujeme).
        * Callback vrací prostředníctvím AnnotatedSymbol neterminál, který má být vložen na zásobník - tedy specifikuje, jaký neterminál to je, sémantický pointer (obvykle to bude výrazový podstrom) a anotaci. Sémantickou anotaci a pointer prozatím bude vracet NULL a CONTEXT_NONE.
        * Callback narozdíl od sémantických akcí precedenčního parseru neupravuje zásobník - vyklizení handle i vložení neterminálu, na který byl redukován, je v odpovědnosti jádra.


        Prozatím tedy akce budou jen kostry, které budou konat dost na to, aby fungovala čistá syntaktická analýza. Callbacky vypíší číslo pravidla. Kostry správně nastaví typ neterminálu, na který se redukují. Můžeš do koster připravit zakomentované řádky ve tvaru
        // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
        Které získají pointer na jednotlivé záznamy handle dle pravé strany pravidla a uloží je do smysluplně pojmenované proměnné, aby později nebylo potřeba přemýšlet nad tím, v jakém pořadí jsou symboly na stacku umístěny - při doplnění stavby AST se pak využijí, případně smažou, nebudou-li potřeba.

        Pokud by ti chyběl nějaký soubor, řekni si o něj, než začneš pracovat.
        Callbacky a jejich deklarace dej do nového páru precedence_semantic_actions.*
        Callbacky LL parseru nechť slouží jako vzor, jak by to mohlo vypadat.
*/

/*
    Prompt pro pridani pravidel E32 a E33 (staticka funkce seznamu bez
    parametru a s jednim parametrem) a prislusne precislovani. Stejny prompt
    zpusobil zmeny take v precedence_semantic_actions.h, reduction_table.c
    a reduction_table.h, jeho zneni je pouze zde.

    Prompt pro Claude:
        Přidal jsem nová pravidla precedenční gramatiky. Konkrétně je nové pravidlo 32 a 33. Mohl by jsi je zavést do kódu? To znamená, vytvořit jim po vzoru ostatních sémantickou akci (resp. její kostru) v precedence_semantic_actions.* a přidat záznamy do redukční tabulky v reduction_table.*
        Ostatní pravidla jsou beze změny, pouze se přečíslovaly pravidla, která následují po 33 - přečísluj je prosím také.

        <zde jsem prekopiroval tabulku pravidel z parser_formalisms.xlsx>
*/

/*
    Prompt pro napojeni akci precedencniho parseru na AST (pravidla E0 az E41),
    pomocne funkce expr_range_set_step, expr_call_take_args,
    expr_member_take_args a expr_list_take_items v ast.* a anotaci
    CONTEXT_RANGE_WITHOUT_STEP v semantic_types.h. Stejny prompt zpusobil
    zmeny take v precedence_semantic_actions.h, ast.h, ast.c
    a semantic_types.h, jeho zneni je pouze zde.

    Prompt pro Claude:
        Dobrá, vypadá to dobře. Možná bych se teď vrhnul na napojení callbacků na AST.

        Je zavedeno pravidlo, že tokeny s atributem budou okamžitě po příchodu do precedenčního parseru přetvořeny na AST uzel, a tedy symboly ID, ID_OPERATOR a literály budou již v symbolové podobě obsahovat na pozici sémantického pointeru ukazatel na AST uzel. Pravidla 0-3 tedy pouze předají tento ukazatel dále.

        Jednoduché bude také většina dalších pravidel, kde obvykle půjde jenom o to, vytvořit uzel dle významu pravidla, naskládat do něj ze zásobníku podvýrazy apod. a ukazatel na uzel dát do výsledku.

        Výjímkami budou tato pravidla:

        * 25, 31, 34-36 budou muset nějak vyřešit předávání více parametrů mezi záznamy. Napadá mě, že by se na to mohla vytvořit pomocná struktura, která má délku pole a dynamicky zvětšující se seznam. V pravidle 35 by byl tento seznam prvně naalokován se dvěma prvky, v pravidle 36 by se rozšiřoval o nové prvky, a pravidla 25, 31, 34 tento seznam naskládají do uzlu volání funkce, resp. seznamu apod. a pomocnou strukturu uvolní. Pokud tě napadá způsob, jak toto vyřešit lépe, ideálně bez nutnosti použít alokovanou paměť, můžeš to udělat i vlastním způsobem.
        * Statická metoda List.range(start, end, [step]) má v AST vyhrazený uzel, nejedná se o obecnou statickou funkci. V pravidlech 32 a 33 proto bude stačit, když vyhodíš sémantickou chybu nesprávného počtu parametrů. V pravidle 34 kontroluj, že ID je přesně range, jinak vyhoď sémantickou chybu nedefinovaná funkce. Vytvoř uzel přes expr_new_list_range a doplň do něj podvýrazy start, end, nepovinně také step. U více argumentů vyhoď chybu nesprávného počtu argumentů.
        * 37-39 pouze předají pointer, protože ty pouze likvidují koncovou čárku nebo je to redukce závorek.
        * pravidlo 40 je speciální, protože ternární if je obecně speciální chování obou jader. Výrazový parser v jádře zavolá LL parser, aby jej zpracoval, a v jedné ze dvou situací parser vrátí rozpracovaný if, kterému chybí else větev - tato redukce else větev připojí. TIF je v tuto chvíli neterminál (reprezentující if bez else větve), možná z toho udělám terminál, ale to nijak chování této metody neovlivní.
        * Pravidlo 41 bylo zavedeno především jako způsob vyřešení notace generátoru 5 to 10 by 2 jako ze scaly. KDyž už jsme však zavedli infixové operátory obecně, proč je nenapojit i na tečkovou notaci, ačkoliv to bude využívat pouze malá hrstka funkcí. Toto pravidlo tady provede následující:
           * pokud je ID_operator přesně to nebo until, vytvoří se uzel generátoru přes expr_new_range bez kroku a nastaveným příznakem inkluzivity.
           * pokud je ID_operator přesně by, pak příjemce této metody musí být uzel range z výše uvedené metody, jinak je to sémantická chyba. K range se doplní krok. Na stacku můžeš využít sémantické anotace k odlišení uložených hodnot.
           * pokud je to cokoliv jiného, pak se vytvoří nový uzel členské notace přes expr_new_member s jedním argumentem. Kontrolu, zda jde o legitimní člen zde obstará sémantický analyzátor (ten je spuštěn nad sestaveným stromem).


        Volání funkce a indexace seznamu má stejný tvar a možná to má v AST odlišné uzly. Neodlišuj to - vždy vytvoř volání funkce a sémantický analyzátor to vyřeší.

        AST ideálně neměň. Smíš si však doplnit funkce, které například doplní nějakou informaci k existujícímu uzlu, pokud by taková někde chyběla. Pokud využiješ dynamickou paměť pro ty delší seznamy argumentů, vyčleň si případně pomocné funkce pro práci s pamětí z ast.* do nového souboru memory.* - neměň však za žádných okolností samotné struktury reprezentující AST, protože to by znamenalo rozhození všech navazujících stupňů překladače.
*/

/*
    Prompt pro pridani pravidla E30 (clenska notace bez parametru) a prislusne
    precislovani. Stejny prompt zpusobil zmeny take v
    precedence_semantic_actions.h, reduction_table.c a reduction_table.h, jeho
    zneni je pouze zde.

    Prompt pro Claude:
        To s tím pravidlem E.ID() je pravda. Všiml jsem si toho také a plánoval jsem to řešit. Pravidlo jsem už zavedl do excelu. Pravidlo má číslo 30, pravidla, které po něm následují, se přečíslovala.

        Pro jistotu, tedy je výňatek z upravené tabulky:
        <zde jsem prekopiroval vynatek z upravene tabulky>

        Zaveď prosím toto nové pravidlo jako sémantickou akci a zapiš jej do redukční tabulky. Akce vyhodí sémantickou chybu ERR_UNDEFINED, protože žádná taková metoda v námi zvolených rozšířeních není.
*/

// Nastavi vysledek redukce. Funkce pouze schovava makro a pretypovani, predavaji
// se vsechny hodnoty zaznamu, ktery jadro vlozi na zasobnik. Semanticky ukazatel
// (NULL) a kontextova anotace (CONTEXT_NONE) se zatim nepouzivaji, pozdeji se
// zde nahradi hodnotami podle potreby stavby AST.
static void set_result(AnnotatedSymbol *result, NonterminalType type,
                       void *semantic_pointer, ContextAnnotation annotation) {
  result->symbol = (GrammarSymbol)GRAMMAR_NONTERMINAL(type);
  result->semantic_pointer = semantic_pointer;
  result->annotation = annotation;
}

// Akce berou zaznamy handle ze zasobniku. Hloubka 0 je vrchol zasobniku, tedy
// posledni symbol prave strany pravidla, nejlevejsi symbol prave strany je
// v hloubce o jednu mensi, nez je delka handle.
//
// Pravidla stavby AST: terminaly s atributem (ID, ID_OPERATOR, literaly) jsou
// na zasobniku uz uzly AST (viz special_action_precedence_preprocessing),
// neterminal E nese hotovy vyrazovy podstrom a neterminal ARGS pomocny uzel
// EXPR_LIST_LITERAL se seznamem argumentu. Akce preberou vlastnictvi
// podstromu z handle, jadro je ze zasobniku vyklizi bez uvolnovani uzlu.

// Vrati vyrazovy podstrom ulozeny u zaznamu zasobniku v dane hloubce (0 je
// vrchol). Chybejici podstrom je chyba jadra, protoze terminaly s atributem
// i vsechny redukovane neterminaly nesou ukazatel na uzel AST.
static Expr *stack_expr(const Stack *stack, size_t depth, int line) {
  ParserStackEntry *entry = parser_stack_peek(stack, depth);
  if (entry == NULL || entry->semantic_pointer == NULL)
    error_exit(ERR_INTERNAL, line,
               "Precedencni parser: Na zasobniku chybi uzel AST.");
  return entry->semantic_pointer;
}

// Totez jako stack_expr, ale vyzaduje seznam argumentu (neterminal ARGS). Ten
// je na zasobniku ulozen jako pomocny uzel EXPR_LIST_LITERAL, ze ktereho se
// prvky pozdeji presunou do volani funkce, clenu nebo seznamu.
static Expr *stack_args(const Stack *stack, size_t depth, int line) {
  Expr *args = stack_expr(stack, depth, line);
  if (args->kind != EXPR_LIST_LITERAL)
    error_exit(ERR_INTERNAL, line,
               "Precedencni parser: ARGS na zasobniku neni seznam argumentu.");
  return args;
}

// Overi, ze uzel je pomocny uzel EXPR_VARIABLE, ktery vytvoril preprocessing
// pro token ID nebo ID_OPERATOR
static void require_id_node(const Expr *node, int line) {
  if (node->kind != EXPR_VARIABLE)
    error_exit(ERR_INTERNAL, line,
               "Precedencni parser: Ocekavan uzel AST pro ID.");
}

// Vytvori volani funkce, jmeno vezme z pomocneho uzlu ID, ktery uvolni
static Expr *call_from_id(Expr *id_node, int line) {
  require_id_node(id_node, line);
  Expr *call = expr_new_call(id_node->as.variable.name, id_node->line);
  expr_free(id_node);
  return call;
}

// Vytvori clen (metodu) objektu, jmeno vezme z pomocneho uzlu ID, ktery uvolni
static Expr *member_from_id(Expr *object, Expr *id_node, int line) {
  require_id_node(id_node, line);
  Expr *member =
      expr_new_member(object, id_node->as.variable.name, id_node->line);
  expr_free(id_node);
  return member;
}

// Spolecna cast akci unarnich operatoru: operand je pod operatorem, ten
// atribut nema
static void reduce_unary(UnaryOp op, const Stack *stack, int line,
                         AnnotatedSymbol *result) {
  Expr *operand_expr = stack_expr(stack, 0, line);
  set_result(result, NONTERM_EXPR, expr_new_unary(op, operand_expr, line),
             CONTEXT_NONE);
}

// Spolecna cast akci binarnich operatoru: levy operand je v hloubce 2,
// operator (bez atributu) v hloubce 1 a pravy operand na vrcholu
static void reduce_binary(BinaryOp op, const Stack *stack, int line,
                          AnnotatedSymbol *result) {
  Expr *left_expr = stack_expr(stack, 2, line);
  Expr *right_expr = stack_expr(stack, 0, line);
  set_result(result, NONTERM_EXPR,
             expr_new_binary(op, left_expr, right_expr, line), CONTEXT_NONE);
}

// Pravidlo E0 (Term ID): E <= ID
void precedence_action_term_id(const Stack *stack, int line,
                               AnnotatedSymbol *result) {
  trace_rule(false, 0, line);

  // ID je na zasobniku uz uzel AST, jen se preda
  Expr *id_node = stack_expr(stack, 0, line);

  set_result(result, NONTERM_EXPR, id_node, CONTEXT_NONE);
}

// Pravidlo E1 (Term integer): E <= INT_LIT
void precedence_action_term_int(const Stack *stack, int line,
                                AnnotatedSymbol *result) {
  trace_rule(false, 1, line);

  // Literal je na zasobniku uz uzel AST, jen se preda
  Expr *int_literal = stack_expr(stack, 0, line);

  set_result(result, NONTERM_EXPR, int_literal, CONTEXT_NONE);
}

// Pravidlo E2 (Term double): E <= DOUBLE_LIT
void precedence_action_term_double(const Stack *stack, int line,
                                   AnnotatedSymbol *result) {
  trace_rule(false, 2, line);

  // Literal je na zasobniku uz uzel AST, jen se preda
  Expr *double_literal = stack_expr(stack, 0, line);

  set_result(result, NONTERM_EXPR, double_literal, CONTEXT_NONE);
}

// Pravidlo E3 (Term string): E <= STRING_LIT
void precedence_action_term_string(const Stack *stack, int line,
                                   AnnotatedSymbol *result) {
  trace_rule(false, 3, line);

  // Literal je na zasobniku uz uzel AST, jen se preda
  Expr *string_literal = stack_expr(stack, 0, line);

  set_result(result, NONTERM_EXPR, string_literal, CONTEXT_NONE);
}

// Pravidlo E4 (Term true): E <= true
void precedence_action_term_true(const Stack *stack, int line,
                                 AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 4, line);

  // Klicove slovo true atribut nema, uzel se vytvori az zde
  Expr *true_literal = expr_new_bool(true, line);

  set_result(result, NONTERM_EXPR, true_literal, CONTEXT_NONE);
}

// Pravidlo E5 (Term false): E <= false
void precedence_action_term_false(const Stack *stack, int line,
                                  AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 5, line);

  Expr *false_literal = expr_new_bool(false, line);

  set_result(result, NONTERM_EXPR, false_literal, CONTEXT_NONE);
}

// Pravidlo E6 (Term unit): E <= ( )
void precedence_action_term_unit(const Stack *stack, int line,
                                 AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 6, line);

  Expr *unit_literal = expr_new_unit(line);

  set_result(result, NONTERM_EXPR, unit_literal, CONTEXT_NONE);
}

// Pravidlo E7 (Operator + unarni): E <= + E (TOKEN_PLUS_UNARY)
void precedence_action_unary_plus(const Stack *stack, int line,
                                  AnnotatedSymbol *result) {
  trace_rule(false, 7, line);

  // Operator je terminal TOKEN_PLUS_UNARY, operand je na vrcholu
  reduce_unary(UNOP_PLUS, stack, line, result);
}

// Pravidlo E8 (Operator - unarni): E <= - E (TOKEN_MINUS_UNARY)
void precedence_action_unary_minus(const Stack *stack, int line,
                                   AnnotatedSymbol *result) {
  trace_rule(false, 8, line);

  // Operator je terminal TOKEN_MINUS_UNARY, operand je na vrcholu
  reduce_unary(UNOP_NEG, stack, line, result);
}

// Pravidlo E9 (Operator ! unarni): E <= ! E
void precedence_action_unary_not(const Stack *stack, int line,
                                 AnnotatedSymbol *result) {
  trace_rule(false, 9, line);

  reduce_unary(UNOP_NOT, stack, line, result);
}

// Pravidlo E10 (Operator *): E <= E * E
void precedence_action_multiply(const Stack *stack, int line,
                                AnnotatedSymbol *result) {
  trace_rule(false, 10, line);

  reduce_binary(BINOP_MUL, stack, line, result);
}

// Pravidlo E11 (Operator /): E <= E / E
void precedence_action_divide(const Stack *stack, int line,
                              AnnotatedSymbol *result) {
  trace_rule(false, 11, line);

  reduce_binary(BINOP_DIV, stack, line, result);
}

// Pravidlo E12 (Operator +): E <= E + E
void precedence_action_add(const Stack *stack, int line,
                           AnnotatedSymbol *result) {
  trace_rule(false, 12, line);

  reduce_binary(BINOP_ADD, stack, line, result);
}

// Pravidlo E13 (Operator -): E <= E - E
void precedence_action_subtract(const Stack *stack, int line,
                                AnnotatedSymbol *result) {
  trace_rule(false, 13, line);

  reduce_binary(BINOP_SUB, stack, line, result);
}

// Pravidlo E14 (Operator ++): E <= E ++ E
void precedence_action_concat(const Stack *stack, int line,
                              AnnotatedSymbol *result) {
  trace_rule(false, 14, line);

  reduce_binary(BINOP_CONCAT_LIST, stack, line, result);
}

// Pravidlo E15 (Operator <): E <= E < E
void precedence_action_less(const Stack *stack, int line,
                            AnnotatedSymbol *result) {
  trace_rule(false, 15, line);

  reduce_binary(BINOP_LT, stack, line, result);
}

// Pravidlo E16 (Operator <=): E <= E <= E
void precedence_action_less_equal(const Stack *stack, int line,
                                  AnnotatedSymbol *result) {
  trace_rule(false, 16, line);

  reduce_binary(BINOP_LE, stack, line, result);
}

// Pravidlo E17 (Operator >): E <= E > E
void precedence_action_greater(const Stack *stack, int line,
                               AnnotatedSymbol *result) {
  trace_rule(false, 17, line);

  reduce_binary(BINOP_GT, stack, line, result);
}

// Pravidlo E18 (Operator >=): E <= E >= E
void precedence_action_greater_equal(const Stack *stack, int line,
                                     AnnotatedSymbol *result) {
  trace_rule(false, 18, line);

  reduce_binary(BINOP_GE, stack, line, result);
}

// Pravidlo E19 (Operator ==): E <= E == E
void precedence_action_equal(const Stack *stack, int line,
                             AnnotatedSymbol *result) {
  trace_rule(false, 19, line);

  reduce_binary(BINOP_EQ, stack, line, result);
}

// Pravidlo E20 (Operator !=): E <= E != E
void precedence_action_not_equal(const Stack *stack, int line,
                                 AnnotatedSymbol *result) {
  trace_rule(false, 20, line);

  reduce_binary(BINOP_NE, stack, line, result);
}

// Pravidlo E21 (Operator &&): E <= E && E
void precedence_action_and(const Stack *stack, int line,
                           AnnotatedSymbol *result) {
  trace_rule(false, 21, line);

  reduce_binary(BINOP_AND, stack, line, result);
}

// Pravidlo E22 (Operator ||): E <= E || E
void precedence_action_or(const Stack *stack, int line,
                          AnnotatedSymbol *result) {
  trace_rule(false, 22, line);

  reduce_binary(BINOP_OR, stack, line, result);
}

// Pravidlo E23 (Volani funkce bez parametru): E <= ID ( )
void precedence_action_call_no_args(const Stack *stack, int line,
                                    AnnotatedSymbol *result) {
  trace_rule(false, 23, line);

  Expr *id_node = stack_expr(stack, 2, line);

  Expr *call = call_from_id(id_node, line);

  set_result(result, NONTERM_EXPR, call, CONTEXT_NONE);
}

// Pravidlo E24 (Volani funkce s parametrem): E <= ID ( E )
void precedence_action_call_one_arg(const Stack *stack, int line,
                                    AnnotatedSymbol *result) {
  trace_rule(false, 24, line);

  // Volani funkce a indexace seznamu maji stejny tvar, rozlisi je semanticky
  // analyzator nad hotovym stromem
  Expr *id_node = stack_expr(stack, 3, line);
  Expr *argument_expr = stack_expr(stack, 1, line);

  Expr *call = call_from_id(id_node, line);
  expr_call_add_arg(call, argument_expr);

  set_result(result, NONTERM_EXPR, call, CONTEXT_NONE);
}

// Pravidlo E25 (Volani funkce s vice parametry): E <= ID ( ARGS )
void precedence_action_call_multiple_args(const Stack *stack, int line,
                                          AnnotatedSymbol *result) {
  trace_rule(false, 25, line);

  Expr *id_node = stack_expr(stack, 3, line);
  Expr *args = stack_args(stack, 1, line);

  Expr *call = call_from_id(id_node, line);
  expr_call_take_args(call, args);

  set_result(result, NONTERM_EXPR, call, CONTEXT_NONE);
}

// Pravidlo E26 (Prazdny seznam): E <= List ( )
void precedence_action_list_empty(const Stack *stack, int line,
                                  AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 26, line);

  Expr *list = expr_new_list_literal(line);

  set_result(result, NONTERM_EXPR, list, CONTEXT_NONE);
}

// Pravidlo E27 (Jednoprvkovy seznam): E <= List ( E )
void precedence_action_list_one_element(const Stack *stack, int line,
                                        AnnotatedSymbol *result) {
  trace_rule(false, 27, line);

  Expr *element_expr = stack_expr(stack, 1, line);

  Expr *list = expr_new_list_literal(line);
  expr_list_add_item(list, element_expr);

  set_result(result, NONTERM_EXPR, list, CONTEXT_NONE);
}

// Pravidlo E28 (Viceprvkovy seznam): E <= List ( ARGS )
void precedence_action_list_multiple_elements(const Stack *stack, int line,
                                              AnnotatedSymbol *result) {
  trace_rule(false, 28, line);

  // Pomocny uzel s argumenty je rovnou hledany seznam, jen se preda
  Expr *args = stack_args(stack, 1, line);

  set_result(result, NONTERM_EXPR, args, CONTEXT_NONE);
}

// Pravidlo E29 (Clenska notace): E <= E . ID
void precedence_action_member_access(const Stack *stack, int line,
                                     AnnotatedSymbol *result) {
  trace_rule(false, 29, line);

  Expr *object_expr = stack_expr(stack, 2, line);
  Expr *member_name = stack_expr(stack, 0, line);

  Expr *member = member_from_id(object_expr, member_name, line);

  set_result(result, NONTERM_EXPR, member, CONTEXT_NONE);
}

// Pravidlo E30 (Clenska notace bez parametru): E <= E . ID ( )
void precedence_action_method_no_args(const Stack *stack, int line,
                                      AnnotatedSymbol *result) {
  (void)result;
  trace_rule(false, 30, line);

  // Expr *object_expr = stack_expr(stack, 4, line);
  Expr *method_name = stack_expr(stack, 2, line);

  // V zvolenych rozsirenich neexistuje zadna metoda bez parametru
  require_id_node(method_name, line);
  error_exit(ERR_UNDEFINED, method_name->line,
             "Neexistujici clenska metoda bez parametru.");
}

// Pravidlo E31 (Clenska metoda s parametrem): E <= E . ID ( E )
void precedence_action_method_one_arg(const Stack *stack, int line,
                                      AnnotatedSymbol *result) {
  trace_rule(false, 31, line);

  Expr *object_expr = stack_expr(stack, 5, line);
  Expr *method_name = stack_expr(stack, 3, line);
  Expr *argument_expr = stack_expr(stack, 1, line);

  Expr *member = member_from_id(object_expr, method_name, line);
  expr_member_add_arg(member, argument_expr);

  set_result(result, NONTERM_EXPR, member, CONTEXT_NONE);
}

// Pravidlo E32 (Clenska metoda s vice parametry): E <= E . ID ( ARGS )
void precedence_action_method_multiple_args(const Stack *stack, int line,
                                            AnnotatedSymbol *result) {
  trace_rule(false, 32, line);

  Expr *object_expr = stack_expr(stack, 5, line);
  Expr *method_name = stack_expr(stack, 3, line);
  Expr *args = stack_args(stack, 1, line);

  Expr *member = member_from_id(object_expr, method_name, line);
  expr_member_take_args(member, args);

  set_result(result, NONTERM_EXPR, member, CONTEXT_NONE);
}

// Pravidlo E33 (Staticka funkce seznamu bez parametru): E <= List . ID ( )
void precedence_action_list_static_function_no_args(const Stack *stack,
                                                    int line,
                                                    AnnotatedSymbol *result) {
  (void)stack;
  (void)result;
  trace_rule(false, 33, line);

  // Jedina staticka funkce seznamu je List.range, ta vyzaduje 2 nebo 3
  // argumenty (viz E35), jinou obecnou staticku funkci AST nezna
  error_exit(ERR_ARGS, line,
             "Staticka funkce seznamu List.<id> vyzaduje alespon 2 argumenty.");
}

// Pravidlo E34 (Staticka funkce seznamu s parametrem): E <= List . ID ( E )
void precedence_action_list_static_function_one_arg(const Stack *stack,
                                                    int line,
                                                    AnnotatedSymbol *result) {
  (void)stack;
  (void)result;
  trace_rule(false, 34, line);

  // Viz E33, jeden argument nestaci pro zadnou staticku funkci seznamu
  error_exit(ERR_ARGS, line,
             "Staticka funkce seznamu List.<id> vyzaduje alespon 2 argumenty.");
}

// Pravidlo E35 (Staticka funkce seznamu s vice parametry):
// E <= List . ID ( ARGS )
void precedence_action_list_static_function_multiple_args(
    const Stack *stack, int line, AnnotatedSymbol *result) {
  trace_rule(false, 35, line);

  Expr *function_name = stack_expr(stack, 3, line);
  Expr *args = stack_args(stack, 1, line);

  require_id_node(function_name, line);
  if (strcmp(function_name->as.variable.name, "range") != 0)
    error_exit(ERR_UNDEFINED, function_name->line,
               "Neexistujici staticka funkce seznamu List.<id>.");

  // List.range(start, end, [step]) ma vlastni uzel AST
  Expr **items;
  int count;
  expr_list_take_items(args, &items, &count);
  if (count != 2 && count != 3)
    error_exit(ERR_ARGS, line,
               "List.range vyzaduje 2 nebo 3 argumenty (start, end, [step]).");

  Expr *step_expr = count == 3 ? items[2] : NULL;
  Expr *range = expr_new_list_range(items[0], items[1], step_expr, line);

  free(items);
  expr_free(args);
  expr_free(function_name);

  set_result(result, NONTERM_EXPR, range, CONTEXT_NONE);
}

// Pravidlo E36 (Tvorba seznamu parametru): ARGS <= E , E
void precedence_action_args_create(const Stack *stack, int line,
                                   AnnotatedSymbol *result) {
  trace_rule(false, 36, line);

  Expr *first_expr = stack_expr(stack, 2, line);
  Expr *second_expr = stack_expr(stack, 0, line);

  // Seznam argumentu je na zasobniku pomocny EXPR_LIST_LITERAL
  Expr *args = expr_new_list_literal(first_expr->line);
  expr_list_add_item(args, first_expr);
  expr_list_add_item(args, second_expr);

  set_result(result, NONTERM_ARGS, args, CONTEXT_NONE);
}

// Pravidlo E37 (Konsolidace parametru): ARGS <= ARGS , E
void precedence_action_args_append(const Stack *stack, int line,
                                   AnnotatedSymbol *result) {
  trace_rule(false, 37, line);

  Expr *args = stack_args(stack, 2, line);
  Expr *expr = stack_expr(stack, 0, line);

  expr_list_add_item(args, expr);

  set_result(result, NONTERM_ARGS, args, CONTEXT_NONE);
}

// Pravidlo E38 (Likvidace koncove carky (za vyrazem)): ARGS <= E ,
void precedence_action_args_trailing_comma_expr(const Stack *stack, int line,
                                                AnnotatedSymbol *result) {
  trace_rule(false, 38, line);

  // Jediny argument se zabali do seznamu argumentu, aby ARGS bylo na zasobniku
  // vzdy pomocny EXPR_LIST_LITERAL (jinak by nebylo poznat od vyrazu)
  Expr *expr = stack_expr(stack, 1, line);

  Expr *args = expr_new_list_literal(expr->line);
  expr_list_add_item(args, expr);

  set_result(result, NONTERM_ARGS, args, CONTEXT_NONE);
}

// Pravidlo E39 (Likvidace koncove carky (za seznamem)): ARGS <= ARGS ,
void precedence_action_args_trailing_comma_args(const Stack *stack, int line,
                                                AnnotatedSymbol *result) {
  trace_rule(false, 39, line);

  Expr *args = stack_args(stack, 1, line);

  set_result(result, NONTERM_ARGS, args, CONTEXT_NONE);
}

// Pravidlo E40 (Redukce uzavorkovanych vyrazu): E <= ( E )
void precedence_action_parenthesized_expr(const Stack *stack, int line,
                                          AnnotatedSymbol *result) {
  trace_rule(false, 40, line);

  // Zavorky se zahodi, anotace vnitrniho vyrazu se zachova, aby (1 to 5) by 2
  // fungovalo stejne jako 1 to 5 by 2
  ParserStackEntry *inner_entry = parser_stack_peek(stack, 1);
  Expr *inner_expr = stack_expr(stack, 1, line);

  set_result(result, NONTERM_EXPR, inner_expr, inner_entry->annotation);
}

// Pravidlo E41 (Pripojeni else vetve): E <= TIF E
void precedence_action_ternary_else(const Stack *stack, int line,
                                    AnnotatedSymbol *result) {
  trace_rule(false, 41, line);

  Expr *tif = stack_expr(stack, 1, line);
  Expr *else_expr = stack_expr(stack, 0, line);

  // TIF je rozpracovany EXPR_IF bez else vetve
  if (tif->kind != EXPR_IF)
    error_exit(ERR_INTERNAL, line,
               "Precedencni parser: TIF na zasobniku neni uzel if.");

  expr_if_set_else(tif, block_from_expr(else_expr, else_expr->line));

  set_result(result, NONTERM_EXPR, tif, CONTEXT_NONE);
}

// Pravidlo E42 (Infixova metoda): E <= E ID_OPERATOR E
void precedence_action_infix_method(const Stack *stack, int line,
                                    AnnotatedSymbol *result) {
  trace_rule(false, 42, line);

  ParserStackEntry *left_entry = parser_stack_peek(stack, 2);
  Expr *left_expr = stack_expr(stack, 2, line);
  Expr *method_operator = stack_expr(stack, 1, line);
  Expr *right_expr = stack_expr(stack, 0, line);

  require_id_node(method_operator, line);
  const char *name = method_operator->as.variable.name;

  Expr *node;
  ContextAnnotation annotation = CONTEXT_NONE;

  if (strcmp(name, "to") == 0 || strcmp(name, "until") == 0) {
    // 1 to 5, 1 until 5: rozsah bez kroku, krok pripadne doplni "by"
    bool is_inclusive = strcmp(name, "to") == 0;
    node = expr_new_range(left_expr, right_expr, is_inclusive, NULL, line);
    annotation = CONTEXT_RANGE_WITHOUT_STEP;
  } else if (strcmp(name, "by") == 0) {
    // by smi byt jen za rozsahem bez kroku, ktery vznikl vyse
    if (left_entry->annotation != CONTEXT_RANGE_WITHOUT_STEP ||
        left_expr->kind != EXPR_RANGE || left_expr->as.range.step != NULL)
      error_exit(ERR_OTHER, line,
                 "Operator by lze pouzit pouze za rozsahem to nebo until.");
    expr_range_set_step(left_expr, right_expr);
    node = left_expr;
  } else {
    // Ostatni infixove metody jsou clen s jednim argumentem, jeho platnost
    // overi semanticky analyzator
    node = expr_new_member(left_expr, name, method_operator->line);
    expr_member_add_arg(node, right_expr);
  }

  expr_free(method_operator);

  set_result(result, NONTERM_EXPR, node, annotation);
}
