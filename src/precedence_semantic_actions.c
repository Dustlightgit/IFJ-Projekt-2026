/*
    IFJ projekt 2026

    precedence_semantic_actions.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        -

    Tento soubor obsahuje semanticke akce pravidel precedencniho parseru. Akce
    zatim pouze vypisuji cislo pravidla na stderr a nastavuji neterminal, na
    ktery se handle redukuje. Atributy pro stavbu stromu zatim nepridavaji.

*/

#include "precedence_semantic_actions.h"

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

// Zakomentovane radky u akci ziskavaji zaznamy handle ze zasobniku. Hloubka 0
// je vrchol zasobniku, tedy posledni symbol prave strany pravidla, nejlevejsi
// symbol prave strany je v hloubce o jednu mensi, nez je delka handle.

// Pravidlo E0 (Term ID): E <= ID
void precedence_action_term_id(const Stack *stack, int line,
                               AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 0, line);

  // ParserStackEntry *id = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E1 (Term integer): E <= INT_LIT
void precedence_action_term_int(const Stack *stack, int line,
                                AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 1, line);

  // ParserStackEntry *int_literal = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E2 (Term double): E <= DOUBLE_LIT
void precedence_action_term_double(const Stack *stack, int line,
                                   AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 2, line);

  // ParserStackEntry *double_literal = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E3 (Term string): E <= STRING_LIT
void precedence_action_term_string(const Stack *stack, int line,
                                   AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 3, line);

  // ParserStackEntry *string_literal = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E4 (Term true): E <= true
void precedence_action_term_true(const Stack *stack, int line,
                                 AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 4, line);

  // ParserStackEntry *true_literal = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E5 (Term false): E <= false
void precedence_action_term_false(const Stack *stack, int line,
                                  AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 5, line);

  // ParserStackEntry *false_literal = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E6 (Term unit): E <= ( )
void precedence_action_term_unit(const Stack *stack, int line,
                                 AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 6, line);

  // ParserStackEntry *lparen = parser_stack_peek(stack, 1);
  // ParserStackEntry *rparen = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E7 (Operator + unarni): E <= + E
void precedence_action_unary_plus(const Stack *stack, int line,
                                  AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 7, line);

  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *operand_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E8 (Operator - unarni): E <= - E
void precedence_action_unary_minus(const Stack *stack, int line,
                                   AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 8, line);

  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *operand_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E9 (Operator ! unarni): E <= ! E
void precedence_action_unary_not(const Stack *stack, int line,
                                 AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 9, line);

  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *operand_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E10 (Operator *): E <= E * E
void precedence_action_multiply(const Stack *stack, int line,
                                AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 10, line);

  // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *right_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E11 (Operator /): E <= E / E
void precedence_action_divide(const Stack *stack, int line,
                              AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 11, line);

  // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *right_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E12 (Operator +): E <= E + E
void precedence_action_add(const Stack *stack, int line,
                           AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 12, line);

  // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *right_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E13 (Operator -): E <= E - E
void precedence_action_subtract(const Stack *stack, int line,
                                AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 13, line);

  // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *right_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E14 (Operator ++): E <= E ++ E
void precedence_action_concat(const Stack *stack, int line,
                              AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 14, line);

  // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *right_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E15 (Operator <): E <= E < E
void precedence_action_less(const Stack *stack, int line,
                            AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 15, line);

  // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *right_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E16 (Operator <=): E <= E <= E
void precedence_action_less_equal(const Stack *stack, int line,
                                  AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 16, line);

  // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *right_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E17 (Operator >): E <= E > E
void precedence_action_greater(const Stack *stack, int line,
                               AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 17, line);

  // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *right_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E18 (Operator >=): E <= E >= E
void precedence_action_greater_equal(const Stack *stack, int line,
                                     AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 18, line);

  // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *right_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E19 (Operator ==): E <= E == E
void precedence_action_equal(const Stack *stack, int line,
                             AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 19, line);

  // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *right_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E20 (Operator !=): E <= E != E
void precedence_action_not_equal(const Stack *stack, int line,
                                 AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 20, line);

  // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *right_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E21 (Operator &&): E <= E && E
void precedence_action_and(const Stack *stack, int line,
                           AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 21, line);

  // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *right_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E22 (Operator ||): E <= E || E
void precedence_action_or(const Stack *stack, int line,
                          AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 22, line);

  // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *operator_token = parser_stack_peek(stack, 1);
  // ParserStackEntry *right_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E23 (Volani funkce bez parametru): E <= ID ( )
void precedence_action_call_no_args(const Stack *stack, int line,
                                    AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 23, line);

  // ParserStackEntry *function_name = parser_stack_peek(stack, 2);
  // ParserStackEntry *lparen = parser_stack_peek(stack, 1);
  // ParserStackEntry *rparen = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E24 (Volani funkce s parametrem): E <= ID ( E )
void precedence_action_call_one_arg(const Stack *stack, int line,
                                    AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 24, line);

  // ParserStackEntry *function_name = parser_stack_peek(stack, 3);
  // ParserStackEntry *lparen = parser_stack_peek(stack, 2);
  // ParserStackEntry *argument_expr = parser_stack_peek(stack, 1);
  // ParserStackEntry *rparen = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E25 (Volani funkce s vice parametry): E <= ID ( ARGS )
void precedence_action_call_multiple_args(const Stack *stack, int line,
                                          AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 25, line);

  // ParserStackEntry *function_name = parser_stack_peek(stack, 3);
  // ParserStackEntry *lparen = parser_stack_peek(stack, 2);
  // ParserStackEntry *args = parser_stack_peek(stack, 1);
  // ParserStackEntry *rparen = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E26 (Prazdny seznam): E <= List ( )
void precedence_action_list_empty(const Stack *stack, int line,
                                  AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 26, line);

  // ParserStackEntry *list_keyword = parser_stack_peek(stack, 2);
  // ParserStackEntry *lparen = parser_stack_peek(stack, 1);
  // ParserStackEntry *rparen = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E27 (Jednoprvkovy seznam): E <= List ( E )
void precedence_action_list_one_element(const Stack *stack, int line,
                                        AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 27, line);

  // ParserStackEntry *list_keyword = parser_stack_peek(stack, 3);
  // ParserStackEntry *lparen = parser_stack_peek(stack, 2);
  // ParserStackEntry *element_expr = parser_stack_peek(stack, 1);
  // ParserStackEntry *rparen = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E28 (Viceprvkovy seznam): E <= List ( ARGS )
void precedence_action_list_multiple_elements(const Stack *stack, int line,
                                              AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 28, line);

  // ParserStackEntry *list_keyword = parser_stack_peek(stack, 3);
  // ParserStackEntry *lparen = parser_stack_peek(stack, 2);
  // ParserStackEntry *args = parser_stack_peek(stack, 1);
  // ParserStackEntry *rparen = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E29 (Clenska notace bez parametru): E <= E . ID
void precedence_action_member_access(const Stack *stack, int line,
                                     AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 29, line);

  // ParserStackEntry *object_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *dot = parser_stack_peek(stack, 1);
  // ParserStackEntry *member_name = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E30 (Clenska metoda s parametrem): E <= E . ID ( E )
void precedence_action_method_one_arg(const Stack *stack, int line,
                                      AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 30, line);

  // ParserStackEntry *object_expr = parser_stack_peek(stack, 5);
  // ParserStackEntry *dot = parser_stack_peek(stack, 4);
  // ParserStackEntry *method_name = parser_stack_peek(stack, 3);
  // ParserStackEntry *lparen = parser_stack_peek(stack, 2);
  // ParserStackEntry *argument_expr = parser_stack_peek(stack, 1);
  // ParserStackEntry *rparen = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E31 (Clenska metoda s vice parametry): E <= E . ID ( ARGS )
void precedence_action_method_multiple_args(const Stack *stack, int line,
                                            AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 31, line);

  // ParserStackEntry *object_expr = parser_stack_peek(stack, 5);
  // ParserStackEntry *dot = parser_stack_peek(stack, 4);
  // ParserStackEntry *method_name = parser_stack_peek(stack, 3);
  // ParserStackEntry *lparen = parser_stack_peek(stack, 2);
  // ParserStackEntry *args = parser_stack_peek(stack, 1);
  // ParserStackEntry *rparen = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E32 (Staticka funkce seznamu): E <= List . ID ( ARGS )
void precedence_action_list_static_function(const Stack *stack, int line,
                                            AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 32, line);

  // ParserStackEntry *list_keyword = parser_stack_peek(stack, 5);
  // ParserStackEntry *dot = parser_stack_peek(stack, 4);
  // ParserStackEntry *function_name = parser_stack_peek(stack, 3);
  // ParserStackEntry *lparen = parser_stack_peek(stack, 2);
  // ParserStackEntry *args = parser_stack_peek(stack, 1);
  // ParserStackEntry *rparen = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E33 (Tvorba seznamu parametru): ARGS <= E , E
void precedence_action_args_create(const Stack *stack, int line,
                                   AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 33, line);

  // ParserStackEntry *first_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *comma = parser_stack_peek(stack, 1);
  // ParserStackEntry *second_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_ARGS, NULL, CONTEXT_NONE);
}

// Pravidlo E34 (Konsolidace parametru): ARGS <= ARGS , E
void precedence_action_args_append(const Stack *stack, int line,
                                   AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 34, line);

  // ParserStackEntry *args = parser_stack_peek(stack, 2);
  // ParserStackEntry *comma = parser_stack_peek(stack, 1);
  // ParserStackEntry *expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_ARGS, NULL, CONTEXT_NONE);
}

// Pravidlo E35 (Likvidace koncove carky (za vyrazem)): ARGS <= E ,
void precedence_action_args_trailing_comma_expr(const Stack *stack, int line,
                                                AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 35, line);

  // ParserStackEntry *expr = parser_stack_peek(stack, 1);
  // ParserStackEntry *comma = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_ARGS, NULL, CONTEXT_NONE);
}

// Pravidlo E36 (Likvidace koncove carky (za seznamem)): ARGS <= ARGS ,
void precedence_action_args_trailing_comma_args(const Stack *stack, int line,
                                                AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 36, line);

  // ParserStackEntry *args = parser_stack_peek(stack, 1);
  // ParserStackEntry *comma = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_ARGS, NULL, CONTEXT_NONE);
}

// Pravidlo E37 (Redukce uzavorkovanych vyrazu): E <= ( E )
void precedence_action_parenthesized_expr(const Stack *stack, int line,
                                          AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 37, line);

  // ParserStackEntry *lparen = parser_stack_peek(stack, 2);
  // ParserStackEntry *inner_expr = parser_stack_peek(stack, 1);
  // ParserStackEntry *rparen = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E38 (Pripojeni else vetve): E <= TIF E
void precedence_action_ternary_else(const Stack *stack, int line,
                                    AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 38, line);

  // ParserStackEntry *tif = parser_stack_peek(stack, 1);
  // ParserStackEntry *else_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}

// Pravidlo E39 (Infixova metoda): E <= E ID_OPERATOR E
void precedence_action_infix_method(const Stack *stack, int line,
                                    AnnotatedSymbol *result) {
  (void)stack;
  trace_rule(false, 39, line);

  // ParserStackEntry *left_expr = parser_stack_peek(stack, 2);
  // ParserStackEntry *method_operator = parser_stack_peek(stack, 1);
  // ParserStackEntry *right_expr = parser_stack_peek(stack, 0);

  set_result(result, NONTERM_EXPR, NULL, CONTEXT_NONE);
}
