/*
    IFJ projekt 2026

    predictive_parser.c

    Puvodni autor: xbalaka00 za konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - pridana funkce predictive_parser (jadro parseru)
          za asistence Claude.

    Implementace prediktivniho parseru zalozeneho na LL tabulce a nekterych jeho
    pomocnych funkci.
*/

#include "predictive_parser.h"

#include <stdio.h>

#include "LL_table.h"
#include "error.h"

void parser_match(ParserContext *ctx, Stack *stack) {

  // Overeni, ze vstup ma validni terminal
  if (!ctx->has_current_token)
    error_exit(ERR_INTERNAL, 0,
               "LL Parser: Spatne pouziti parser_match(): Neni nacten token ze "
               "vstupu.");

  // Overeni, ze zasobnik neni prazdny
  if (stack_is_empty(stack))
    error_exit(
        ERR_INTERNAL, ctx->current_token.line,
        "LL Parser: Spatne pouziti parser_match(): Zasobnik je prazdny.");

  // Ziskani vrcholu
  ParserStackEntry *top = parser_stack_top(stack);

  // Pokud vrchol neni terminal, tato funkce nemela byt zavolana
  if (!top->symbol.is_terminal)
    error_exit(ERR_INTERNAL, ctx->current_token.line,
               "LL Parser: Spatne pouziti parser_match(): Na vrcholu zasobniku "
               "neni terminal.");

  // Porovnani a pripadne vyvolani chyby
  if (!parser_check_token(ctx, top->symbol.terminal)) {

    // Token na zasobniku a na vstupu jsou odlisne
    fprintf(stderr,
            "LL Parser: Soucasny vstupni terminal: %s\nSoucasny vrchol "
            "zasobniku: %s\n",
            token_type_name(ctx->current_token.type),
            token_type_name(top->symbol.terminal));

    error_exit(ERR_SYNTAX, ctx->current_token.line,
               "Odlisny terminal na vstupu a na vrcholu zasobniku.");
  }

  // Terminaly jsou v poradku.

  // Odstraneni vrcholu zasobniku.
  parser_stack_pop(stack, NULL);

  // Pokud terminal nebyl EOF, ziskej dalsi token
  if (ctx->current_token.type != TOKEN_EOF)
    parser_advance(ctx);
}

/*
    Prompty pro funkci predictive_parser a funkci parser_context_init
    v parser_common.h/.c. Funkce predictive_parser byla puvodne navrzena do
    souboru LL_parser.h/.c, na zaklade druheho promptu byla presunuta sem.
    Stejne prompty zpusobily zmeny take v predictive_parser.h a
    parser_common.h/.c, jejich zneni je pouze zde.

    Prvni prompt, vychozi kostra jadra parseru.

    Prompt pro Claude:
        Rád bych, kdyby jsi napsal jádro prediktivního parseru. Jazyk je C.
        Přikládám hlavičkové soubory připravených funkcí. Jádro později bude
        muset být složitejší, ale pro začátek bude stačit hlavní algoritmus,
        který specifikuji níže. Pravidla budou doplněna později. Prozatím jen
        tvoříme jádro, které bude tuto tabulku využívat.
        Většinu předpřipravených podpůrných funkcí bys měl mít pořádně popsanou
        v hlavičkových souborech a deklarovaných promptech uvnitř nich.
        Zmíním však tuto trojici:
        parser_check_token umožňuje zeptat se, jestli je na pozici vstupního
        tokenu konkrétní token.
        parser_advance získá další token od scanneru (a kontroluje jedno
        globálně platící pravidlo o čárkách).
        parser_match porovná vstupní terminál s terminálem na vrcholu zásobníku
        a vyvolá chybu, pokud jsou odlišné.

        Hlavičkové soubory jsou rozděleny takto proto, že později vznikne i
        výrazový parser, který bude jisté věci sdílet. Hlavičkové soubory
        neměň. Výrazový parser vznikne a tedy bude napojen na tento parser až
        později.
        Bude nutné, aby se oba parsery mohly rekurzivně volat, proto uvnitř
        jádra parseru nepoužívej statické proměnné ani jiné věci, které by mohly
        rekurzi překážet. Každá instance volání parseru bude mít vlastní stack.
        Vstupní proud bude společný, proto je jeho správa ve struktuře
        ParserContext. SemanticContext je struktura pro interní využití
        callbacků, prozatím ji pouze předávej callbackům.

        Kdekoliv to jde, vyvaruj se použití dynamické alokace.

        Dodržuj formátování hlavičky souboru, doxygen dokumentace, komentáře,
        naming convention (včetně komentářů v češtině bez diakritiky) a
        deklarování přikládání promptů a změn, dle přiložených souborů. Tento
        prompt tedy zahrň taky; stačí jednou, nejspíše k nějvětší  a na ostatní
        místa odkazy.

        Každá chyba v tuto chvíli alespoň prozatimně ukončí program "natvrdo"
        přes exit, proto prozatím případné poklízení dynamické paměti při
        chybovém stavu není nutné řešit. Používej funkci error_exit a pokud je
        to možné, dodej jí i číslo řádku ze vstupního tokenu.

        Vygeneruj v parser_common.h/.c inicializační funkci pro ParserContext,
        která inicializuje všechny hodnoty na nějaké nulové hodnoty a získá
        první vstupní terminál voláním parser_advance.

        Dále vytvoř funkci v LL_parser.h/.c
        void predictive_parser(ParserContext *ctx, NonterminalType
        starting_nonterm, void *starting_semantic_pointer);

        Která bude obsahovat samotný parser, který bude implementovat následující
        algoritmus:

        - Vytvoří se stack jako lokální proměnná a inicializuje se.
        - Pushne se počáteční neterminál a starting_semantic_pointer na stack pomocí
          parser_stack_push, id_type bude ID_CONTEXT_NONE; toto políčko bude
          obvykle vyplněno callbacky a používá se především pro terminál ID,
          který bude zpracován speciálním callbackem. Toto prozatím není
          potřeba řešit, tento callback zatím není připravený.
        - Vejde se do smyčky, která poběží, dokud se nevyprázdní zásobník.
          - Získá se ukazatel na vrchol zásobníku pomocí parser_stack_top.
            - Pokud se jedná o neterminál, potom:
              - Pokud je to hodnota NONTERM_COUNT, vyvolá se interní chyba.
                Tato hodnota by nikdy neměla být na zásobníku, není to
                skutečný neterminál.
              - Ověří se, že aktuální token na vstupu má svoji hodnotu menší
                než TOKEN_LL_TERM_COUNT. Prozatím se vyvolá interní chyba,
                pokud toto nastane, později nejspíše bude doplněna další
                logika pro některé situace, např. pro přechod do výrazového
                parseru.
              - Vyhledá se pravidlo v LL tabulce:
                - Pokud je pravidlo, LL_RULE_ERROR, vyhodí se chyba syntaxe
                  "LL Parser: Neni mozne zvolit pravidlo."
                - Pokud chybové není, pak se uchová sémantický pointer v
                  dočasné proměnné a neterminál se odstraní ze zásobníku.
                - Pokud je typu LL_RULE_STATIC, bude jeho statická produkce
                  vložena na zásobník v pořadí snižujícího se indexu.
                  Sémantický pointer a typ ID budou NULL a none.
                - Pokud je typu LL_RULE_SEMANTIC, zavolá se callback, kterému
                  bude předán sémantický pointer z právě zpracovaného
                  neterminálu a sémantický kontext (kdyby si potřebovaly
                  předávat data mezi sebou).
            - Pokud se jedná o terminál, potom:
              - Pokud se jedná o TOKEN_ID, prozatím se pouze vypíše na stderr
                informace o tom, že byl token potkán a jeho stringový atribut.
                Později se sem doplní další zpracování.
              - Zavolá se funkce parser_match, která porovná vrchol zásobníku
                se vstupním terminálem a oba odstraní.
        - Po ukončení hlavní smyčky se příslušnými funkcemi zlikviduje stack a
          poklidí se případná dynamická paměť; jak ale říkám, vyvaruj se jejímu
          používání. Obvykle by měla stačit lokální proměnná, případně
          proměnné uvnitř ParserContext. Stack interně ukládá vše hodnotou.

        Pokud ti cokoliv není jasné, zeptej se dříve, než začneš generovat kód.

    Druhy prompt, presun funkce predictive_parser.

    Prompt pro Claude:
        Tvé předpoklady jsou povětšinou správně. U ID jsem skutečně myslel
        atribut ze vstupního tokenu před jeho porovnáním. Vše vypadá v pořádku.
        Jen jsem spletl ten název, skutečně jsem myslel predictive_parser.h/.c
        jako místo, kam má přijít implementace hlavní funkce parseru. Mohl by
        jsi to ještě přesunout a upravit reference v komentářích apod.? Tento
        prompt nejspíše zahrň také, avšak v seznamech změn stačí jen jeden
        záznam o všech těchto změnách.
*/

void predictive_parser(ParserContext *ctx, NonterminalType starting_nonterm,
                       void *starting_semantic_pointer) {

  // Kazda instance parseru ma vlastni zasobnik (kvuli rekurzi)
  Stack stack;
  parser_stack_init(&stack);

  // Pocatecni neterminal
  GrammarSymbol start_symbol = GRAMMAR_NONTERMINAL(starting_nonterm);
  parser_stack_push(&stack, start_symbol, starting_semantic_pointer,
                    ID_CONTEXT_NONE);

  while (!stack_is_empty(&stack)) {

    // Na vstupu musi byt nacteny token
    if (!ctx->has_current_token)
      error_exit(ERR_INTERNAL, 0, "LL Parser: Neni nacten token ze vstupu.");

    ParserStackEntry *top = parser_stack_top(&stack);

    if (!top->symbol.is_terminal) {
      // Neterminal na vrcholu zasobniku
      NonterminalType nonterm = top->symbol.nonterminal;

      // NONTERM_COUNT neni skutecny neterminal
      if (nonterm >= NONTERM_COUNT)
        error_exit(ERR_INTERNAL, ctx->current_token.line,
                   "LL Parser: Na zasobniku je neplatny neterminal.");

      // Do LL tabulky lze indexovat jen terminaly LL gramatiky
      if (ctx->current_token.type >= TOKEN_LL_TERM_COUNT)
        error_exit(ERR_INTERNAL, ctx->current_token.line,
                   "LL Parser: Token na vstupu neni terminal LL tabulky.");

      // Vyhledani pravidla
      const LLTableEntry *entry = &LL_TABLE[nonterm][ctx->current_token.type];

      if (entry->type == LL_RULE_ERROR) {
        fprintf(stderr,
                "LL Parser: Soucasny vstupni terminal: %s\nSoucasny vrchol "
                "zasobniku: %s\n",
                token_type_name(ctx->current_token.type),
                nonterm_type_name(nonterm));

        error_exit(ERR_SYNTAX, ctx->current_token.line,
                   "LL Parser: Neni mozne zvolit pravidlo.");
      }

      // Uchovani semantickeho ukazatele a odstraneni neterminalu. Ukazatel top
      // po odebrani neni platny, proto se pouzije dale jen docasna promenna.
      void *semantic_pointer = top->semantic_pointer;
      parser_stack_pop(&stack, NULL);

      if (entry->type == LL_RULE_STATIC) {
        // Pravidlo je staticke

        // Ziskame statickou produkci a zkontrolujeme ji
        const StaticProduction *production = entry->production;

        if (production == NULL ||
            production->length > STATIC_PRODUCTION_MAX_LENGTH)
          error_exit(ERR_INTERNAL, ctx->current_token.line,
                     "LL Parser: Neplatna staticka produkce v LL tabulce.");

        // Symboly se vkladaji od posledniho, aby byl na vrcholu prvni
        // Staticka produkce jednoduse zkopiruje semanticky ukazatel do vlozenych symbolu
        for (size_t i = production->length; i > 0; i--)
          parser_stack_push(&stack, production->production[i - 1],
                            semantic_pointer, ID_CONTEXT_NONE);
      }

      else if (entry->type == LL_RULE_SEMANTIC) {
        // Pravidlo je semanticke

        if (entry->callback == NULL)
          error_exit(ERR_INTERNAL, ctx->current_token.line,
                     "LL Parser: Callback v LL tabulce je NULL.");

        // Callback si produkci vlozi na zasobnik sam
        entry->callback(&(ctx->semantic_context), semantic_pointer, &stack);
      }

      else {
        // Neznamy typ pravidla
        error_exit(ERR_INTERNAL, ctx->current_token.line,
                   "LL Parser: Neznamy typ pravidla v LL tabulce.");
      }
    }

    else {
      // Terminal na vrcholu zasobniku

      // Prozatim jen vypis ID, pozdeji bude doplneno dalsi zpracovani.
      // Vypis musi probehnout pred parser_match, ktery token uvolni.
      if (top->symbol.terminal == TOKEN_ID && parser_check_token(ctx, TOKEN_ID))
        fprintf(stderr, "LL Parser: Potkan token ID, atribut: %s\n",
                ctx->current_token.value.text);

      // Porovna terminaly a oba odstrani
      parser_match(ctx, &stack);
    }
  }

  // Uklid zasobniku
  stack_free(&stack);
}
