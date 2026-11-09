/*
    IFJ projekt 2026

    parser_common.c

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        -

    Tento soubor obsahuje funkce pro praci se zasobnikem parseru.

*/

#include "parser_common.h"

#include "error.h"
#include "scanner.h"

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

  // Kontrola koncove carky
  if (ctx->current_token.type == TOKEN_COMMA) {
    ctx->previous_token_was_comma = true;
  } else if (ctx->current_token.type == TOKEN_RPAREN) {
    // Pokud je aktualni token ), předesly token byla carka a pred ) nebyl \n, pak nastala chyba
    if (ctx->previous_token_was_comma && !(ctx->current_token.nl_before))
      error_exit(ERR_SYNTAX, ctx->current_token.line,
                 "Nepovolena koncova carka");

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

void parser_stack_init(Stack *stack) {
  stack_init(stack, sizeof(ParserStackEntry));
}

void parser_stack_push(Stack *stack, GrammarSymbol symbol,
                       void *semantic_pointer, IDContextType id_type) {
  ParserStackEntry entry = {
      .symbol = symbol,
      .semantic_pointer = semantic_pointer,
      .id_type = id_type,
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
