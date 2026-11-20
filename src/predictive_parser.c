/*
    IFJ projekt 2026

    xbalaka00 Artur Balak
    xnovacm00 Michal Novacek

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
        - xbalaka00 - callbackum se predava cislo radku vstupniho terminalu.
        - xbalaka00 - jadro obsahuje chovani pro neterminal <IDcommand>
          (pravidla 32 a 33, zatim bez stavby stromu a vyrazoveho parseru).
        - xnovacm00 - doplnena stavba uzlu AST ke specialnimu chovani pravidla 32
        - xbalaka00 - jadro overuje funkcemi nonterm_type_validate
          a token_type_validate neterminal vyjmuty ze zasobniku a terminal
          na jeho vrcholu (prompt je v nonterminal.h).
        - xbalaka00 - vkladani na zasobnik predava priznak reduction_marker
          false (prompt je v parser_common.c).
        - xbalaka00 - include grammar_symbol.h pro makra GRAMMAR_TERMINAL
          a GRAMMAR_NONTERMINAL (GrammarSymbol presunut do grammar_symbol.h,
          prompt je v reduction_table.h).
        - xbalaka00 - jadro propojeno s precedencnim parserem

    Implementace prediktivniho parseru zalozeneho na LL tabulce a nekterych jeho
    pomocnych funkci.
*/

#include "predictive_parser.h"

#include <stdio.h>

#include "LL_table.h"
#include "ast.h"
#include "error.h"
#include "grammar_symbol.h"
#include "precedence_parser.h"
#include "predictive_semantic_actions.h"
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
    specialni akci pro ID je v souboru special_actions.h. Prompt pro predani
    cisla radku callbackum je v souboru semantic_types.h.

    Prompt pro doplneni chovani jadra pro neterminal <IDcommand> (pravidla
    32 a 33), anotaci CONTEXT_ID_COMMAND, polozku id_expr v SemanticContext,
    upravu callbacku pravidla 31 a specialni akce pro ID. Stejny prompt
    zpusobil zmeny take v semantic_types.h, predictive_semantic_actions.h,
    predictive_semantic_actions.c, special_actions.h a special_actions.c, jeho zneni je
    pouze zde.

    Prompt pro Claude:
        Dobrá, nyní přidáme do jádra speciální chování pro neterminál
        <IDcommand>. Nejdříve popíšu, co se zhruba stane. Začneme od pravidla
        31:

        - sémantická akce pro pravidlo 31 obdrží sémantický pointer na blok,
          ke kterému se má připojit příkaz.
        - akce pro pravidlo 31 dá do symbolu ID, které přidává na stack nějakou
          sémantickou anotaci, že teď budeme zpracovávat IDcommand speciální
          akci... a do <IDcommand> záznamu dá informace, kam uložit přiřazení
          nebo expr (tedy sémantický pointer jen předá).
        - speciální akce pro ID pro ID vytvoří uzel AST a pointer na něho uloží
          někam, kde k němu bude mít přístup i jádro (takže třeba parser
          context nebo semantic context asi). AST si zařídí, aby s atributem
          názvu bylo z hlediska správy paměti náležitě zacházeno.
        - ID na vstupu se normálně porovná a spotřebuje běžným způsobem po
          vykonání callbacku.
        - pak bude na vrcholu zásobníku <IDcommand>, takže jádro neprovede
          klasickou akci, ale udělá toto:
            - pokud je vstupní terminál =, tak vytvoří uzel přiřazení, dá do
              něj název ID co se uložilo do AST uzlu, ten uzel přiřazení přidá
              do pointeru podle toho co callback 31 uložil do záznamu
              IDcommand, = oddělá ze zásobníku a ručně na něj přidá <expr> s
              pointerem a anotací takovou, aby se ten expr připojil k tomu
              přiřazení. Uzel AST se uvolní, protože už nebude potřeba.
            - pokud je vstupní terminál cokoliv jiného, tak se vstupní token
              nechá na pokoji a zavolá se výrazovka, které bude předaný
              parametrem startovací ID symbol s pointerem na uzel ID AST.
              Výrazovka si pak tenhle startovní symbol přidá na stack a vše tím
              pádem bude stejné, jako kdyby si ID sama zpracovala ze vstupu.
              (Výrazovka totiž nejspíše každé vstupní ID nebo literál s
              atributem okamžitě předělá na uzel AST, aby se nemuselo řešit co
              s atributama po dalším volání scanneru; redukce ID -> E pak jen
              tento pointer předá dále; a pokud by ID nebylo využito jako list
              stromu ale třeba jako název volané funkce, tak se opět vytáhne to
              jméno a uzel pro ID se uvolní; ale toto už říkám extra).

        Takže takhle to bude vypadat v hotovém systému cca. Přikládám ti
        hlavičkový soubor ast.h, který vytvořil kolega. Zatím však nebudeme
        vyplňovat stavbu AST a výrazový parser ještě neexistuje, proto
        prozatím chování vytvoříme jen z části. Bude to vypadat takto:

        * callback pravidla 31 vyplň tak, že do terminálu ID dá kontextovou
          značku CONTEXT_ID_COMMAND, do neterminálu <IDcommand> předá svůj
          pointer a zbytek zůstane null/NONE (myslím že víc nebude potřeba
          dělat). Tímto možná bude callback 31 plně hotový...
        * do semantic_context přidej položku pointeru na AST uzel pro ID, což
          zmanená na strukturu typu Expr. Jmenovat se bude třeba helper_pointer
          nebo tak, název můžeš vymyslet i jiný. Možná se časem stejně
          semantic_context zruší pokud nebude mít jiné využití než toto;
          potom by se tento pointer přesunul do parser context a do speciální
          akce pro ID by se místo semantic context dal parser context, ale
          prozatím necháme takto.
        * ve speciálním callbacku pro ID zahaj switch case přes kontextové
          anotace, který ve větvi pro CONTEXT_ID_COMMAND vytvoří ten AST uzel
          pro ID (tedy Expr s typem položky EXPR_VARIABLE a do něj se předá
          název ID a další informace, které uzel požaduje) a pointer na něj dá
          do té položky v semantic_context. V běžném případě by callback pro
          neznámou kontextovou anotaci pravděpodobně vyhodil interní chybu,
          ale zatím to udělej tak, aby se jednoduše neudělalo nic kromě
          vypsání, že bylo potkáno ID, pokud akce nedostala ten kontextový
          token CONTEXT_ID_COMMAND. To potkání vypiš v obou případech.
        * Tolik k úpravám existujících callbacků. Nyní ke změnám v jádře:
           * do větve, kdy je na vrcholu zásobníku neterminál, přidej kontrolu,
             zda je na vrcholu <IDcommand>. Pokud se vejde do této větve,
             běžné vyhledání v LL tabulce ani jiné akce se neprovádí! Jedná se
             o akce pro celou iteraci smyčky while jádra by se dalo říct. Pokud
             chceš, můžeš akce pro tuto větev vyčlenit do speciálního callbacku
             v special_actions.h pokud předáš relevantní informace přes
             argument, ale zatím to klidně nechej přímo v jádře. Terminál
             IDcommand bude normálně ze stacku vyjmut. Akce pro tuto větev:
           * Pokud je vstupním tokenem =, pak se provede následující
              * prozatím stavbu stromu nedoplňujeme; proto se AST uzel ID ze
                semantic_context zatím jen uvolní. Před uvolněním zde později
                proběhne tvorba uzlu přiřazení se jménem z toho uzlu AST, které
                se přidá k bloku dle sémantického pointeru u položky <IDcommand>
                na vrcholu. Můžeš vypsat na sdterr, že by zde bylo provedeno
                pravidlo přiřazení a vypiš i jméno ID.
              * Zavolá se parser_advance, aby byl terminál = přepsán novým
                vstupním terminálem.
              * Na zásobník se vloží <expr>, prozatím bez pointeru (později
                pointer na vytvořený uzel přiřazení) a s kontextovou značkou
                vyjadřující, že se má výraz připojit k přiřazení (zatím ale
                může být NONE).
              * toto celé se rovná pravidlu 32.
           * Pokud není vstupním tokenem =, pak se nehledě na vstupní token
             později přejde do výrazovky (přímo, nikoliv přes neterminál <expr>
             na zásobníku) s předáním počátečního AST uzlu a dalších informací
             apod a ta by sama vyhodnotila, jestli je ten druhý terminál
             smysluplný. Jelikož však prozatím výrazový parser ještě neexistuje,
             pouze se uvolní uzel AST v parser_context a na stack se přidá
             <expr>, aby bylo možné jej přepsat na * jakožto testovací akce.
             Před tím uvolněním vypiš na stderr zprávu o tom, že zde by se
             přešlo do výrazové analýzy a vypiš jméno ID jako důkaz, že se v
             pořádku dostalo až sem. Toto odpovídá pravidlu 33.

        Takto budou prakticky pravidla 32 a 33 v odpovědnosti jádra
        implementována do takové míry, jaká je možná s přihlédnutím k faktu, že
        ještě výrazový parser ani stavba stromu neexistuje. Toto speciální
        chování jádra by nemělo být ovlivněno změnami gramatiky, které budu
        muset provést pro opravení té chyby s else větví ifu.
        ast.h neměň a jinou funkcionalitu nepřidávej, pouze využij jeho funkce
        pro vytvoření ID uzlu expr_new_variable a jeho uvolnění expr_free.
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
                      annotation, false);
}

void predictive_parser(ParserContext *ctx, NonterminalType starting_nonterm,
                       void *starting_semantic_pointer) {

  // Kazda instance parseru ma vlastni zasobnik (kvuli rekurzi)
  Stack stack;
  parser_stack_init(&stack);

  // Pocatecni neterminal
  GrammarSymbol start_symbol = GRAMMAR_NONTERMINAL(starting_nonterm);
  parser_stack_push(&stack, start_symbol, starting_semantic_pointer,
                    CONTEXT_NONE, false);

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

      // Dodatecna vnitrni kontrola, ze neterminal ze zasobniku je skutecny
      nonterm_type_validate(nonterm, ctx->current_token.line);

      if (nonterm == NONTERM_EXPR) {
        // Specialni neterminal <expr> reprezentujici prechod do vyrazove analyzy

        // Zavolani precedencniho parseru
        Expr *result = precedence_parser(ctx, NULL);

        // Zavolani specialni akce pro pripojeni vyrazu k AST
        special_action_expr(semantic_pointer, annotation, result,
                            ctx->current_token.line);

      } else if (nonterm == NONTERM_ID_COMMAND) {
        // Neterminal <IDcommand> (pravidla 32 a 33) je v odpovednosti jadra,
        // LL tabulka ani zadna dalsi akce se v teto iteraci neprovadi.
        // Uzel ID vytvorila special_action_id pri zpracovani ID na zacatku
        // prikazu, ID je uz porovnano a spotrebovano.
        Expr *id_expr = ctx->semantic_context.id_expr;

        if (id_expr == NULL)
          error_exit(ERR_INTERNAL, ctx->current_token.line,
                     "LL Parser: Pro <IDcommand> neni vytvoren AST uzel ID.");

        GrammarSymbol expr_symbol = GRAMMAR_NONTERMINAL(NONTERM_EXPR);

        if (parser_check_token(ctx, TOKEN_ASSIGN)) {
          // Pravidlo 32: <IDcommand> => = <expr>
          fprintf(stderr, "LL Parser: Pravidlo 32, prirazeni do ID: %s\n",
                  id_expr->as.variable.name);

          // Prirazeni se prida do bloku (nebo do tela cyklu ci vetve if)
          Block *block = special_action_block(semantic_pointer, annotation,
                                              ctx->current_token.line);
          Stmt *assign =
              stmt_new_assign(id_expr->as.variable.name, NULL, id_expr->line);
          block_add(block, assign);

          // AST uzel ID uz neni potreba, jmeno si prirazeni zkopirovalo
          expr_free(id_expr);
          ctx->semantic_context.id_expr = NULL;

          // Terminal = se nahradi dalsim vstupnim terminalem
          parser_advance(ctx);

          // Vyraz za = se pripoji jako hodnota prirazeni
          parser_stack_push(&stack, expr_symbol, assign, CONTEXT_ASSIGN_VALUE,
                            false);
        }

        else {
          // Pravidlo 33: <IDcommand> => prechod do vyrazove analyzy. Vstupni
          // token se nemeni. Do vyrazoveho parseru je predan pocatecni
          // symbol ID s ukazatelem na uzel ID.
          fprintf(stderr,
                  "LL Parser: Pravidlo 33, prechod do vyrazove analyzy, "
                  "pocatecni ID: %s\n",
                  id_expr->as.variable.name);

          // Priprava pocatecniho terminalu
          AnnotatedSymbol starting_id = {.symbol = GRAMMAR_TERMINAL(TOKEN_ID),
                                         .semantic_pointer = id_expr};

          // Zavolani precedencniho parseru
          Expr *result = precedence_parser(ctx, &starting_id);

          // Zavolani specialni akce pro pripojeni vyrazu k AST
          special_action_expr(semantic_pointer, annotation, result,
                              ctx->current_token.line);
        }
      }

      // Do LL tabulky lze indexovat jen terminaly LL gramatiky
      else if (ctx->current_token.type < TOKEN_LL_TERM_COUNT) {
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
                          annotation, ctx->current_token.line, &stack);
        }
      }

      else if (ctx->current_token.type < TOKEN_EXPRESSION_BEGINNING_DELIMITER) {
        // Objevil se terminal, ktery neindexuje LL tabulku, avsak je platnym zacatkem vyrazu

        // Provedeni pravidla dle aktualniho neterminalu
        AnnotatedSymbol current_nonterm_info = {
            .symbol = GRAMMAR_NONTERMINAL(nonterm),
            .semantic_pointer = semantic_pointer,
            .annotation = annotation};

        special_action_execute_expr_rule(ctx, &stack, &current_nonterm_info);
      }

      else {
        // Vstupni token neni terminal LL tabulky ani zacatek vyrazu.
        error_exit(ERR_SYNTAX, ctx->current_token.line,
                   "LL Parser: Token na vstupu neni terminal LL tabulky ani "
                   "zacatek vyrazu.");
      }
    }

    else {
      // Terminal na vrcholu zasobniku

      // Dodatecna vnitrni kontrola, ze terminal na vrcholu je skutecny token
      token_type_validate(top->symbol.terminal, ctx->current_token.line);

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
