/*
    IFJ projekt 2026

    predictive_parser.c

    Puvodni autor: xbalaka00 za konzultace s ChatGPT
    Seznam zmen:
        - xbalaka00 - pridana funkce predictive_parser (jadro parseru)
          za asistence Claude.
        - xbalaka00 - vyhledani pravidla v LL tabulce vyclenene do funkce
          ll_table_lookup (LL_table.c), jadro nyni pouziva tuto funkci.
        - xbalaka00 - vlozeni statickoho pravidla na zasobnik vyclenene do
          funkce parser_push_static_production.
        - xbalaka00 - jadro predava kontextovou anotaci callbackum
          a parser_push_static_production ji kopiruje do vlozenych symbolu.
        - xbalaka00 - vypis potkaneho ID v jadre nahrazen volanim
          special_action_id (special_actions.h).

    Implementace prediktivniho parseru zalozeneho na LL tabulce a nekterych jeho
    pomocnych funkci.
*/

#include "predictive_parser.h"

#include <stdio.h>

#include "LL_table.h"
#include "error.h"
#include "special_actions.h"

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

    Prompt pro vycleneni vyhledani pravidla v LL tabulce do funkce
    ll_table_lookup a pro poznamku o chybe v navrhu gramatiky. Stejny prompt
    zpusobil zmeny take v LL_table.h a LL_table.c, jeho zneni je pouze zde.

    Prompt pro Claude:
        Pravidla jsem commitnul a otestoval jsem program. Bohužel jsem při
        testování odhalil chybu, která je však dána návrhem gramatiky -
        nevyvstala ze způsobu, jakým jsi pravidla vygeneroval. Gramatiku tedy
        bude nutné znovu přepracovat a přegenerovat, ale teď to nenechám vše
        vykolejit, budeme pokračovat i s chybnými pravidly. Excel ani soubory
        se statickými produkcemi nebo callbacky tedy nebudeme zatím řešit.
        Můžeš o této odhalené chybě přidat poznámku do LL_table.h - chyba
        souvisí se situací, ve které řídící konstrukce if s jednořádkovým tělem
        then a přítomným tělem else vyžaduje, aby příkaz uvnitř jednořádkového
        then nebyl terminován (terminování zde zajistí else). Jak ale říkám,
        prozatím pravidla v kódu necháme být.

        Začneme s úpravami jádra v souboru predictive_parser.c, který jsem
        přiložil. Využij i všechny ostatní soubory, se kterými jsme doteď
        pracovali. První věc, kterou bych udělal, je vyčlenění vyhledání v LL
        tabulce do vlastní metody.

        Pokud to importy dovolí bez problémů s kruhovými vztahy mezi
        hlavičkami, pak tuto metodu přidej do LL_table.*, jinak ji přidej jako
        privátní metodu v predictive_parser.*
        Metoda jen zabalí přístup k LL tabulky do pomocné metody. Zkontroluje se
        v ní rozsah indexů, provede se indexace tabulky, ověření, že pravidlo
        není NONE, a dle typu záznamu také, že získaný callback není NULL,
        případně že statická produkce je validní - stejné kontroly, jaké jsou
        teď přímo v jádře. V jádře však bude muset jistá kontrola indexů zůstat
        také, protože vstupní token, kterým nelze indexovat LL tabulku, není
        automaticky syntax error právě kvůli budoucí implementaci sloupce
        *další. Uvnitř metody tedy kontrola indexů může vyvolat chybu špatného
        použití této metody po vzoru parser_match, a v jádře zůstane ošetření
        rozsahu indexu vstupního terminálu jako jedna z větví rozhodovací
        struktury na místě, kde aktuálně je, s tím, že prozatím druhá větev
        způsobí chybu jako dočasné řešení. Později právě na toto místo přijde
        logika pro *další.

        V souhrnu - indexace tabulky na řádku 218 společně s kontrolami
        chybových stavů související s pokusem vyhledat pravidlo v LL tabulce na
        řádcích 213, 220, 242, a 257. Metoda vrátí pointer na příslušný
        LLTableEntry, o kterém víme, že není prázdné pravidlo a příslušný
        záznam je validní.

        Tok programu uvnitř větve, kdy je vrcholu neterminál, bude takovýto:

        - vytažení neterminálu ze zásobníku do proměnné stejně jako je teď
        - podmínka if pro kontrolu, že vstupní terminál je v rozsahu
          indexování tabulky dle příslušné hodnoty enumu
           - pokud není, prozatím se v této větvi vyhodí chyba
           - pokud je, provede se vyhledání novou metodou
           - získaný záznam se zpracuje tak, jako doposud podle toho, o jaký
             záznam se jedná.

    Prompt pro presun kontroly neznameho typu pravidla do funkce
    ll_table_lookup. Stejny prompt zpusobil zmeny take v LL_table.h
    a LL_table.c, jeho zneni je pouze zde.

    Prompt pro Claude:
        Pěkné. Je pravda, že kontrola neznámého typu pravidla také může přijít
        do této metody, protože sémanticky vzato také spadá pod akci "vyhledej
        v LL tabulce a ověř, že nenastala chyba.". Přesuň ji tedy do této
        metody také, jádro bude zase o kus přehlednější.

        Tato změna bude součástí stejného commitu jako změny po předchozím
        promptu. Proto změnové záznamy uprav tak, aby změny v předchozím commitu
        a tomto commitu byly zaverzovany jako jedna změna mezi commity.
        Deklaruj však normálně i tento prompt.

    Prompt pro vycleneni vlozeni statickeho pravidla na zasobnik do funkce
    parser_push_static_production. Stejny prompt zpusobil zmeny take
    v predictive_parser.h, jeho zneni je pouze zde.

    Prompt pro Claude:
        Dobrá. Nyní obdobným způsobem z jádra vyčleň do pomocné funkce i
        naházení symbolů na zásobník ve statické produkci. Prakticky to znamená
        do funkce přesounout for cyklus na řádcích 298 až 300. Vzhledem k tomu,
        že proměnná production ze řádku 293 byla použita jen pro zjednodušení
        zápisu tohoto cyklu, může v jádře být oddělána a entry->production se
        použije na místě argumentu této nové metody.
        Nová metoda bude v predictive_parser.*, protože je to pomocná metoda
        samotného parseru, ne souboru a LL tabulkou či jiného. Opět se tím
        jádro zpřehlední, navíc stejně bude nutné toto "zpracování statického
        pravidla" použít i ve větvi pro *další, bude tedy hezčí mít to v metodě
        než všude kopírovat ten samý cyklus. Tato změna bude nový commit, tedy
        udělej změnové záznamy.

    Prompt pro predani kontextove anotace callbackum a kopirovani anotace
    ve statickych produkcich je v souboru semantic_types.h. Prompt pro
    specialni akci pro ID je v souboru special_actions.h.
*/

void parser_push_static_production(Stack *stack,
                                   const StaticProduction *production,
                                   void *semantic_pointer,
                                   ContextAnnotation annotation) {

  // Symboly se vkladaji od posledniho, aby byl na vrcholu prvni
  // Staticka produkce jednoduse zkopiruje semanticky ukazatel i kontextovou
  // anotaci do vsech vlozenych symbolu
  for (size_t i = production->length; i > 0; i--)
    parser_stack_push(stack, production->production[i - 1], semantic_pointer,
                      annotation);
}

void predictive_parser(ParserContext *ctx, NonterminalType starting_nonterm,
                       void *starting_semantic_pointer) {

  // Kazda instance parseru ma vlastni zasobnik (kvuli rekurzi)
  Stack stack;
  parser_stack_init(&stack);

  // Pocatecni neterminal
  GrammarSymbol start_symbol = GRAMMAR_NONTERMINAL(starting_nonterm);
  parser_stack_push(&stack, start_symbol, starting_semantic_pointer,
                    CONTEXT_NONE);

  while (!stack_is_empty(&stack)) {

    // Na vstupu musi byt nacteny token
    if (!ctx->has_current_token)
      error_exit(ERR_INTERNAL, 0, "LL Parser: Neni nacten token ze vstupu.");

    ParserStackEntry *top = parser_stack_top(&stack);

    if (!top->symbol.is_terminal) {
      // Neterminal na vrcholu zasobniku, vytazeni ze zasobniku. Ukazatel top
      // po odebrani neni platny, proto se dale pouziji jen hodnoty z kopie
      // odebraneho zaznamu.
      ParserStackEntry nonterm_entry;
      parser_stack_pop(&stack, &nonterm_entry);

      NonterminalType nonterm = nonterm_entry.symbol.nonterminal;
      void *semantic_pointer = nonterm_entry.semantic_pointer;
      ContextAnnotation annotation = nonterm_entry.annotation;

      // Do LL tabulky lze indexovat jen terminaly LL gramatiky
      if (ctx->current_token.type < TOKEN_LL_TERM_COUNT) {
        // Vyhledani pravidla, pri neexistenci pravidla ukonci program
        const LLTableEntry *entry = ll_table_lookup(
            nonterm, ctx->current_token.type, ctx->current_token.line);

        if (entry->type == LL_RULE_STATIC) {
          // Pravidlo je staticke
          parser_push_static_production(&stack, entry->production,
                                        semantic_pointer, annotation);
        }

        else {
          // Pravidlo je semanticke (ll_table_lookup zaruci, ze je zaznam
          // staticky nebo semanticky), callback si produkci vlozi na zasobnik
          // sam
          entry->callback(&(ctx->semantic_context), semantic_pointer,
                          annotation, &stack);
        }
      }

      else {
        // Docasne reseni: vstupni token neni terminal LL tabulky. Zde pozdeji
        // pribude logika pro sloupec *dalsi (tokeny, kterymi muze zacinat
        // vyraz).
        error_exit(ERR_INTERNAL, ctx->current_token.line,
                   "LL Parser: Token na vstupu neni terminal LL tabulky.");
      }
    }

    else {
      // Terminal na vrcholu zasobniku

      // Na vrcholu je ID a na vstupu je ID: specialni akce zpracuje token ID
      // vcetne atributu. Musi probehnout pred parser_match, ktery token
      // uvolni. Pri odlisnosti tokenu by parser_match vyvolal chybu.
      if (top->symbol.terminal == TOKEN_ID && parser_check_token(ctx, TOKEN_ID))
        special_action_id(&(ctx->semantic_context), top->semantic_pointer,
                          top->annotation, &(ctx->current_token));

      // Porovna terminaly a oba odstrani
      parser_match(ctx, &stack);
    }
  }

  // Uklid zasobniku
  stack_free(&stack);
}
