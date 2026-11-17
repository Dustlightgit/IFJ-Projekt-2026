/*
    IFJ projekt 2026

    parser_common.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - xbalaka00 - pridani funkce parser_context_init za asistence Claude,
          prompt je v predictive_parser.c.
        - xbalaka00 - parser_stack_push prijima ContextAnnotation misto
          IDContextType.
        - xbalaka00 - parser_advance overuje typ nacteneho tokenu funkci
          token_type_validate (prompt je v nonterminal.h).
        - xbalaka00 - parser_stack_push prijima priznak reduction_marker.
        - xbalaka00 - pridana funkce parser_stack_peek (prompt je v
          semantic_types.h).
        - xbalaka00 - pridana funkce trace_rule (puvodne staticka v
          semantic_actions.c), zobecnena pro oba parsery, prompt je zde.
        - xbalaka00 - kontrola koncove carky rozsirena, aby kontrolovala take zakaz dvou po sobe jdoucich carek.

    Tento soubor obsahuje funkce pro praci se zasobnikem parseru a spolecny
    ladici vypis pouzitych pravidel.

*/

#include "parser_common.h"

#include "error.h"
#include "scanner.h"
#include <stdio.h>

bool parser_check_token(const ParserContext *ctx, TokenType expected) {
  return ctx->has_current_token && ctx->current_token.type == expected;
}

void parser_advance(ParserContext *ctx) {

  // Uvolneni stareho tokenu pokud nejaky je
  if (ctx->has_current_token)
    token_free(&(ctx->current_token));

  // Nacteni noveho tokenu
  get_token(&(ctx->current_token));
  ctx->has_current_token = true;

  // Dodatecna vnitrni kontrola, ze scanner vratil skutecny token
  token_type_validate(ctx->current_token.type, ctx->current_token.line);

  // Kontrola koncove carky a dvou carek po sobe
  if (ctx->current_token.type == TOKEN_COMMA) {
    // Pokud je aktualni token carka, a predesly byl take carka, pak nastala chyba
    if (ctx->previous_token_was_comma)
      error_exit(ERR_SYNTAX, ctx->current_token.line,
                 "Parser: Nepovolene dve carky bezprostredne za sebou");

    ctx->previous_token_was_comma = true;
  } else if (ctx->current_token.type == TOKEN_RPAREN) {
    // Pokud je aktualni token ), předesly token byla carka a pred ) nebyl \n, pak nastala chyba
    if (ctx->previous_token_was_comma && !(ctx->current_token.nl_before))
      error_exit(ERR_SYNTAX, ctx->current_token.line,
                 "Parser: Nepovolena koncova carka");

    ctx->previous_token_was_comma = false;
  } else {
    ctx->previous_token_was_comma = false;
  }
}

/*
    Prompt pro funkce parser_stack_init, parser_stack_push, parser_stack_pop
    a parser_stack_top. Stejny prompt zpusobil take pridani deklaraci do
    parser_common.h, jeho zneni je pouze zde.

    Prompt pro Claude:
        Dobrá, zkusil bych udělat už skutečnou práci. Možná bych prvně
        dokonvergoval k vygenerování nějakých testovacích pravidel. Prozatím
        rozhodně ne z celé gramatiky, ta zatím není předělaná do LL tabulky, což
        prý máme udělat sami, ale jen pár jednoduchých, klidně smyšlených
        pravidel. Než se ale k pravidlům dostaneme, bude potřeba funkce
        parser_stack_push. Ta bude v parser_common.h/.c a bude mít deklaraci
        void parser_stack_push(Stack *stack, ...);

        Kde ... je každý z prvků, které jsou uvnitř ParserStackEntry. Funkce
        nejspíš bude potřebovat jen vytvořit lokální proměnnou ParserStackEntry,
        zaplnit ji daty, a zavolat stack_push; ze stack.c to vypadá, že stack si
        tento prvek zkopíruje přes memcopy do svého seznamu, proto nevznikne
        problém, až lokální entry zmizí. Stack si interně nafukuje pole a vše
        hlídá, nemělo by tedy být nutné dělat příliš mnoho kontrol. Dodrž
        konvence, ke kterým jsme došli v minulých promptech.

        Co se životnosti stacku týče, stack jako takový bude lokální proměnná
        uvnitř daného parseru. Obě verze parseru budou používat pravděpodobně tu
        samou formu stack entry (i kdyby pak každý používal svoji vlastní,
        prozatím nebudu výrazovému parseru designovat structy; ale asi bude
        jednodušší mít stejný stack entry s tím, že přinejhorším bude jeden nebo
        druhý parser nepoužívat jednu nebo dvě z hodnot v entry), ale každá
        instance volání parseru bude mít vlastní stack. Parsery se budou moci
        potenciálně až rekurzivně volat (LL parser na hlavní program -> výrazový
        parser na výraz -> při nalezení ternárního ifu se zavolá LL parser, aby
        ho zpracoval -> ten zase zpracuje expr uvnitř ifu -> případně hlubší
        zanoření pro zanořené ternární ify), proto bude každé volání parserů
        nezávislé, ale budou mít společný kontext ParserContext, kde bude např.
        společný stav vstupního tokenu.

        Možná jsem příliš odbočil. Pokud chceš, můžeš přidat i wrappery
        parser_stack_* nad funkcemi stacku, ale možná bude postačovat volat
        přímo funkce stacku, protože mnohdy v nich nic moc extra probíhat
        nebude; i když někdy možná bude potřeba přetypovat pointer na konkrétní
        typ ParserStackEntry. Pokud by opravdu funkce byla čistě jen wrapper s
        předáním argumentů bez čehokoliv jiného, nejspíše wrapper dělat nemusíš.
        U initu případně jako velikost prvku samozřejmě použij
        sizeof(ParserStackEntry) jako velikost prvku.
*/

/*
    Prompt pro zobecneni IDContextType na ContextAnnotation je v souboru
    semantic_types.h.
*/

/*
    Prompt pro pridani priznaku reduction_marker do ParserStackEntry
    a jeho predani funkci parser_stack_push. Stejny prompt zpusobil zmeny
    take v parser_common.h, semantic_actions.c a predictive_parser.c, jeho
    zneni je pouze zde.

    Prompt pro Claude:
        Dobrá. Do StackEntry doplň boolean reduction_marker. Význam této
        značky je, že bezprostředně po prvku s touto značkou jakože následuje
        symbol <, který v matematickém modelu označuje začátek redukce. Handle
        je tedy vždy tvořen všemi prvky od vrcholu zásobníku až po první
        symbol opatřen touto značkou vyjma tohoto symbolu. Defaultně bude
        nastaven na false a LL parser jej nebude využívat. Rozšiř nadstavbové
        funkce nad stackem parser_stack_* o tento nový parametr.
*/

void parser_stack_init(Stack *stack) {
  stack_init(stack, sizeof(ParserStackEntry));
}

void parser_stack_push(Stack *stack, GrammarSymbol symbol,
                       void *semantic_pointer, ContextAnnotation annotation,
                       bool reduction_marker) {
  ParserStackEntry entry = {
      .symbol = symbol,
      .semantic_pointer = semantic_pointer,
      .annotation = annotation,
      .reduction_marker = reduction_marker,
  };

  // Zasobnik si zaznam zkopiruje, lokalni entry tedy muze po volani zaniknout
  stack_push(stack, &entry);
}

bool parser_stack_pop(Stack *stack, ParserStackEntry *out) {
  return stack_pop(stack, out);
}

ParserStackEntry *parser_stack_top(const Stack *stack) {
  return stack_top(stack);
}

ParserStackEntry *parser_stack_peek(const Stack *stack, size_t depth) {
  return stack_peek(stack, depth);
}

/*
    Prompt pro funkci parser_stack_peek je v souboru semantic_types.h.
    Prompt pro funkci parser_context_init je v souboru predictive_parser.c.
*/

void parser_context_init(ParserContext *ctx) {
  // Vynulovani vsech hodnot, has_current_token je false, takze parser_advance
  // zadny stary token neuvolnuje
  *ctx = (ParserContext){0};

  // Ziskani prvniho vstupniho tokenu
  parser_advance(ctx);
}

/*
    Prompt pro funkci trace_rule. Stejny prompt zpusobil zmeny take
    v parser_common.h a semantic_actions.c, jeho zneni je pouze zde.

    Prompt pro Claude:
        Mohl by jsi funkci trace_rule zobecnit tak, aby mohla vypisovat
        pravidla jak LL parseru, tak prediktivního parseru?
        Přidej do ní parametr bool is_predictive_parser na základě kterého
        vypíše buď
        "LL Parser: Pravidlo L%d (radek %d)\n"
        nebo
        "Precedencni parser: Pravidlo E%d (radek %d)\n"

        Funkci přesuň do parser_common.* a uprav stávající volání v
        semantic_actions.c
*/

void trace_rule(bool is_predictive_parser, int rule_number, int line) {
  if (is_predictive_parser)
    fprintf(stderr, "LL Parser: Pravidlo L%d (radek %d)\n", rule_number, line);
  else
    fprintf(stderr, "Precedencni parser: Pravidlo E%d (radek %d)\n",
            rule_number, line);
}
