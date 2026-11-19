/*
    IFJ projekt 2026

    xbalaka00 Artur Balak

    precedence_parser.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - pridana prazdna hlavicka funkce precedence_parser
          (prompt je v precedence_parser.h).
        - xbalaka00 - napsana implementace precedencniho parseru
        - xbalaka00 - kontrola a opravy jadra: parser_execute_directive vraci,
          zda byl spotrebovan vstupni token, predikce a predzpracovani se
          provadi jednou pro kazdy token, opravena ukoncovaci podminka
          a kontroly prazdneho vyrazu, terminalu v pocatecnim zaznamu
          a zacykleni (prompt je v precedence_parser.h).
        - xbalaka00 - ReductionResult prejmenovan na AnnotatedSymbol (prompt
          je v semantic_types.h).

    Tento soubor obsahuje implementaci precedencniho parseru vyrazu a jeho
    pomocnych funkci. Prompt pro vznik souboru je v souboru
    precedence_parser.h.
*/

#include "precedence_parser.h"

#include "error.h"
#include "nonterminal.h"
#include "precedence_helpers.h"
#include "precedence_table.h"
#include "reduction_table.h"
#include "semantic_types.h"
#include "special_actions.h"

/*
    Prompty pro makro, ktere prikaze prekladaci, aby nevypisovalo varovani u umyslneho propadnuti do dalsi vetve konstrukce case.

    Prompt pro ChatGPT:
        Jak překladači řeknu, že chci, aby ve switch case statement propadl do další větve?

    Prompt pro ChatGPT:
        Myslím C. break jsem vynechal, a překladač teď vypisuje varování - zde ale chci, aby se v dané větvi provedla akce,
        a pak další akce, které se provedou stejné i v další větvi v pořadí - fall through je úmyslný, ale překladač vypisuje warning.
*/
#if defined(__GNUC__) && __GNUC__ >= 7
#define FALLTHROUGH __attribute__((fallthrough))
#else
#define FALLTHROUGH ((void)0)
#endif

void parser_execute_reduction(const ParserContext *ctx, Stack *stack) {
  int line = ctx->current_token.line;

  // Sestaveni handle ze zasobniku a vyhledani semanticke akce pravidla
  SearchHandle handle;
  parser_build_handle(stack, &handle, line);
  PrecedenceSemanticAction action = reduction_table_lookup(&handle, line);

  // Provedeni akce, handle je v tu chvili stale na zasobniku
  AnnotatedSymbol result = {0};
  action(stack, line, &result);

  // Redukci musi vzniknout neterminal, jinak je chyba v akci
  if (result.symbol.is_terminal)
    error_exit(ERR_INTERNAL, line,
               "Precedencni parser: Semanticka akce nevratila neterminal.");
  nonterm_type_validate(result.symbol.nonterminal, line);

  // Vyklizeni handle ze zasobniku
  for (size_t i = 0; i < handle.length; i++)
    parser_stack_pop(stack, NULL);

  // Smazani znacky redukce, redukce, kterou ohranicovala, byla provedena
  ParserStackEntry *top = parser_stack_top(stack);
  if (top == NULL)
    error_exit(ERR_INTERNAL, line,
               "Precedencni parser: Po vyklizeni handle je zasobnik prazdny.");
  top->reduction_marker = false;

  // Vlozeni redukovaneho neterminalu
  parser_stack_push(stack, result.symbol, result.semantic_pointer,
                    result.annotation, false);
}

bool parser_execute_directive(ParserContext *ctx, Stack *stack,
                              bool use_specified_terminal, TokenType terminal,
                              void *semantic_pointer) {
  int line = ctx->current_token.line;

  // Ziskani terminalu nejblize vrcholu
  ParserStackEntry *top_terminal = parser_top_terminal(stack, line);

  // Urceni terminalu, kterym se bude indexovat
  TokenType input_index;

  if (use_specified_terminal)
    input_index = terminal;
  else
    input_index = ctx->current_token.type;

  // Vyhledani direktivy v precedencni tabulce
  PrecedenceDirective directive =
      precedence_table_lookup(top_terminal->symbol.terminal, input_index, line);

  // Priznak, zda byl spotrebovan vstupni token
  bool input_consumed = false;

  // Rozhodnuti o provedene akci
  switch (directive) {
  case DIRECTIVE_REDUCTION_EXECUTE:
    // Provedeni redukce
    parser_execute_reduction(ctx, stack);
    break;

  case DIRECTIVE_REDUCTION_MARKER:
    // Vlozi se znacka redukce, dale se postupuje stejne jako u direktivy pro vlozeni symbolu
    top_terminal->reduction_marker = true;
    FALLTHROUGH;

  case DIRECTIVE_SHIFT_SYMBOL:
    // Vstupni symbol je vlozen na zasobnik

    parser_stack_push(stack, (GrammarSymbol)GRAMMAR_TERMINAL(input_index),
                      semantic_pointer, CONTEXT_NONE, false);

    // Pokud byl misto vstupniho tokenu vyuzit specifikovany terminal, vstup nesmi byt zmenen
    if (!use_specified_terminal) {
      parser_advance(ctx);
      input_consumed = true;
    }
    break;

  // DIRECTIVE_ERROR byl jiz osetren pri vyhledavani direktivy
  default:
    break;
  }

  return input_consumed;
}

Expr *precedence_parser(ParserContext *ctx,
                        AnnotatedSymbol *starting_terminal) {

  // Priprava stacku
  Stack stack;
  parser_stack_init(&stack);

  // Priprava priznaku a prediktoru (na miste prvniho symbolu je vzdy ocekavan operand)
  bool parser_running = !parser_is_expression_terminator(&ctx->current_token);
  PredictorValue predictor = EXPECT_OPERAND;

  // V pripade specifikovaneho prvniho terminalu priznak behu parseru plati pro druhy terminal vyrazu.

  // Vlozeni symbolu dna zasobniku
  parser_stack_push(&stack, (GrammarSymbol)GRAMMAR_TERMINAL(TOKEN_DOLLAR), NULL,
                    CONTEXT_NONE, false);

  if (starting_terminal != NULL) {
    // Pokud byl specifikovany pocatecni terminal, je pro nej provedena akce, jako kdyby prisel normalni cestou na vstup.
    // Toto se provede bez ohledu na hodnotu priznaku behu parseru - tento terminal se povazuje za prvni symbol vstupu.
    // Pokud se nejedna o validni znak vyrazove analyzy, bude vyhozena chyba pri indexaci tabulky.
    // Jeho metadata jsou pripadne vlozena na zasobnik spolu se symbolem.

    // Pocatecni zaznam musi nest terminal
    if (!starting_terminal->symbol.is_terminal)
      error_exit(ERR_INTERNAL, ctx->current_token.line,
                 "Precedencni parser: Pocatecni zaznam neni terminal.");

    // Vyhodnoceni prediktoru
    // Vstupni token musi zustat beze zmeny, proto se vytvori falesny vstupni token
    Token fake_token = {0};
    fake_token.type = starting_terminal->symbol.terminal;

    predictor = parser_evaluate_predictor(&fake_token, predictor);

    token_free(&fake_token);

    // Neni nutne zohlednit prebarveni ID na ID_OPERATOR, protoze u prvniho symbolu se nikdy neprovede

    // Provedeni direktivy dle tabulky
    parser_execute_directive(ctx, &stack, true,
                             starting_terminal->symbol.terminal,
                             starting_terminal->semantic_pointer);
  }

  // Hlavni smycka precedencniho parseru, kazda jeji iterace zpracuje jeden vstupni token
  while (parser_running) {

    if (ctx->current_token.type == TOKEN_KW_IF) {
      // Na vstupu je token if

      // Pozdeji zde bude zavolani precedencniho parseru, aby zpracoval ternarni if,
      // a zapojeni vysledku na stack. Tato vetev take rucne nastavi priznak a prediktor.

      // Prozatim ternarni if neni podporovan
      error_exit(ERR_INTERNAL, ctx->current_token.line,
                 "Precedencni parser: Ternarni if prozatim neni podporovan.");
    } else {

      // Vyhodnoceni prediktoru, vstupni token se pripadne prebarvi z ID na ID_OPERATOR.
      // Provadi se prave jednou pro kazdy vstupni token, a to pred jeho zpracovanim,
      // aby byl pro indexaci tabulky pouzit uz prebarveny token.
      predictor = parser_evaluate_predictor(&ctx->current_token, predictor);

      // Provedeni specialni akce pro predzpracovani vstupniho terminalu, take jednou pro kazdy token
      void *preprocessing_result =
          special_action_precedence_preprocessing(&ctx->current_token);

      // Provadeni direktiv dle precedencni tabulky, dokud neni vstupni token
      // spotrebovan (jedna z direktiv muze byt redukce, ktera vstup nemeni).
      // Kazda platna redukce odstrani ze zasobniku aspon jeden terminal, pocet
      // pruchodu je proto omezen velikosti zasobniku (+ 1 pro vlozeni tokenu).
      size_t directives_left = stack_size(&stack) + 1;
      bool input_consumed = false;

      while (!input_consumed) {
        if (directives_left-- == 0)
          error_exit(ERR_INTERNAL, ctx->current_token.line,
                     "Precedencni parser: Zacyklene provadeni redukci.");

        // Ctvrty argument je ignorovan
        input_consumed = parser_execute_directive(
            ctx, &stack, false, TOKEN_KW_IF, preprocessing_result);
      }

      // Vyhodnoceni priznaku behu, na vstupu je uz dalsi token
      parser_running = !parser_is_expression_terminator(&ctx->current_token);
    }
  }

  // Ukoncovaci smycka precedencniho parseru
  // Smycka bezi, dokud na zasobniku nezustane pouze $ a neterminal. Pokud je na zasobniku
  // jen $, je vyraz prazdny, smycka se neprovede.
  size_t reductions_left = stack_size(&stack);

  while (stack_size(&stack) > 1 &&
         (stack_size(&stack) > 2 ||
          parser_stack_top(&stack)->symbol.is_terminal)) {

    // Kazda platna redukce odstrani ze zasobniku aspon jeden terminal
    if (reductions_left-- == 0)
      error_exit(ERR_INTERNAL, ctx->current_token.line,
                 "Precedencni parser: Zacyklene provadeni redukci.");

    // Za vstupni terminal je povazovan $,
    parser_execute_directive(ctx, &stack, true, TOKEN_DOLLAR, NULL);

    // Neprovadi se vyhodnoceni prediktoru (bylo by zbytecne)
  }

  // Na zasobniku zustalo jen $, vyraz je prazdny
  if (stack_size(&stack) < 2)
    error_exit(ERR_SYNTAX, ctx->current_token.line,
               "Precedencni parser: Vyraz je prazdny.");

  // Overeni, ze dno zasobniku zustalo neporuseno
  ParserStackEntry *stack_floor = parser_stack_peek(&stack, 1);

  if (!stack_floor->symbol.is_terminal ||
      stack_floor->symbol.terminal != TOKEN_DOLLAR)
    error_exit(ERR_INTERNAL, ctx->current_token.line,
               "Precedencni parser: Naruseno dno zasobniku.");

  // Ziskani neterminalu
  ParserStackEntry *result = parser_stack_top(&stack);

  // Overeni, ze se skutecne jedna o neterminal E
  if (result->symbol.is_terminal || result->symbol.nonterminal != NONTERM_EXPR)
    error_exit(ERR_SYNTAX, ctx->current_token.line,
               "Precedencni parser: Beh neskoncil neterminalem E.");

  void *resulting_expr = result->semantic_pointer;

  // Uklid zasobniku
  stack_free(&stack);

  // Navraceni vytvoreneho stromu
  return resulting_expr;
}
