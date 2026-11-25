
## Parser a rozhraní pro stavbu AST

Parser je složený z prediktivního a precedenčního parseru. Oba mají společný vstupní proud tokenů (ve struktuře ParserContext).
Oba parsery se rozhodují na základě svých tabulek a oba na základě provedených pravidel provádějí sémantické akce, které především staví AST.
Část funkcionality stavby AST je vyčleněna do speciálních akcí, které nejsou svázané s konkrétním pravidlem.
Oba parsery se vzájemně rekurzivně volají, aby mohl být zpracován ternární if - každá instance zavolání parseru má tedy svůj vlastní zásobník.

Parser nad rámec zadání podporuje infixový zápis členských metod, tedy například
```scala
List(2, 3, 5, 7, 11) drop 2
```
se vyhodnotí ekvivalentně se zápisem
```scala
List(2, 3, 5, 7, 11).drop(2)
```
Tato funkcionalita byla implementována především jako způsob syntaktického zpracování zápisu generátoru.
Parser naopak nepodporuje zápis běžného řídícího if s přítomnou větví else, jehož větev else by byla jednořádkový řídící příkaz, tedy například situaci
```scala
if w then while x do y else z
```

Plný detail návrhu parseru je v souboru ./formalisms/parser_formalisms.xlsx na githubu.

# Prediktivní parser

Jádro prediktivního parseru je implementováno algoritmem z přednášek - jedinou odlišností je, že končí jednoduchým vyprázdněním zásobníku. Parseru je možné specifikovat počáteční neterminál. Obě tyto vlastnosti má kvůli ternárnímu ifu - viz dále.

LL tabulka je prostým dvojrozměrným polem a obsahuje (mimo chybové záznamy) pravidla dvojího druhu - statické a callbackové.

Statické pravidlo je konstanta typu StaticProduction, která přímo obsahuje pravou stranu pravidla pro vložení na zásobník. Takto jsou reprezentována jednoduchá, obvykle čistě syntaktická pravidla nepotřebující vykonávat zvláštní činnost.

Callbackové pravidlo je ukazatel na funkci, tzv. sémantickou akci. Sémantická akce obvykle provádí stavbu AST, případně je potřeba pro fungování některého z implementačních triků či slouží jako rozcestník pro sémantické informace. Odpovědností sémantické akce je vložit na zásobník pravou stranu pravidla, které reprezentuje; k jednotlivým položkám smí přiložit sémantické informace dle svého uvážení. Sémantické akce mohou některé

Sémantické informace se ukládají ke každé položce zásobníku. Jedná se o sémantický ukazatel - libovolný ukazatel typu (void *) na cokoliv dle uvážení sémantické akce, obvykle na uzel AST, ke kterému se má připojit další uzel AST či doplnit informace. Dále se jedná o sémantickou anotaci, kterou je možné specifikovat očekávané chování další akce v pořadí.

Pro šíření sémantických informací skrz pomyslný derivační strom platí jednoduché pravidlo - statické pravidlo vždy zkopíruje ukazatel i anotaci ze svého neterminálu do všech symbolů na pravé straně pravidla. Podrobnější řízení toku informací pak vyžaduje sémantickou akci.

Pokud je na vrcholu zásobníku i na vstupu terminál ID, je nad ním zavolána příslušná speciální akce.

# Precedenční parser

Jádro precedenčního parseru je implementováno algoritmem z přednášek, tentokrát v zásadě bez odlišností, avšak koná jisté věci navíc. Precedenčnímu parseru je možné specifikovat počáteční terminál, který je využit místo aktuálního terminálu vstupního proudu.

Redukční značka < je reprezentována příznakem u položky zásobníku s významem, že znak < následuje bezprostředně po této položce.

V parseru je využit jednobitový prediktor, který za základě terminálu na vstupu předpoví, jestli příští terminál bude operand, nebo operátor. Prediktor je využit pro "přebarvení" vstupních terminálů na jejich varianty v některých situacích, aby byly především pro účely precedenční tabulky odlišeny od původních terminálů. Terminály + a - jsou v příslušných situacích přebarveny na své unární varianty, a terminál ID je přebarven na operátorovou verzi.

Precedenční tabulka je dvojrozměrné pole obsahující čtyři direktivy z algoritmu z přednášek (včetně chybové direktivy); pro indexaci je využit indexační převod. Jádro na základě direktiv ohraničuje pravé strany pravidel, tzv. handle pravidla. Při redukci vyhledá handle pravidla v redukční tabulce a provede příslušnou sémantickou akci (pokud je nalezena).

Redukční tabulka je statická vyhledávací tabulka s klíčem typu SearchHandle (symbolová reprezentace handle); jeho užitečnou položkou je ukazatel na sémantickou akci. Vzhledem k malému a neměnnému počtu položek je hledání implementováno se složitostí O(n) lineárním vyhledáním s předčasným ukončením při překročení velikosti SearchHandle. Symboly v handle jsou porovnávány kompletně, aby bylo zajištěno, že se nikdy neprovede nesprávná redukce. Neúspěšné vyhledání je syntaktická chyba.

Narozdíl od prediktivního parseru je každé pravidlo precedenčního parseru reprezentováno sémantickou akcí. Akce opět především staví AST. Sémantické akce mají díky mechanismu vyhledání v redukční tabulce zaručeno, že na zásobníku se při jejich zavolání nachází jejich handle. Sémantická akce má právo nahlížet na položky zásobníku, aby si z nich získala potřebné sémantické informace. Akce vrátí symbol anotovaný sémantickými informacemi, na který by měl být zredukován handle. Precedenční akce zásobník nijak nemění.

# Řádný přechod mezi parsery

V gramatice prediktivního parseru je zaveden speciální neterminál <expr>. Pokud se tento neterminál objeví na vrcholu zásobníku, je zahájen přechod do výrazové analýzy. Zavolá se precedenční parser, který vrátí výsledný výrazový podstrom; ten je připojen ke zbytku stromu příslušnou speciální akcí.

Pro účely výpočtů množin pro LL tabulku byly ručně definovány následující množiny:
- $ empty(<expr>) = \emptyset $
- $ first(<expr>) = {(, List, if, ID, *další} $
přičemž *další zastupuje symboly, které neindexují LL tabulku, ale smí jimi začínat výraz. Jedná se o literály a unární operátory. Pro tyto symboly je v návrhu LL tabulky vyhrazen zvláštní sloupec. V případě, že je na vstupu jeden z těchto terminálů, je příslušné pravidlo provedeno jádrem prediktivního parseru mimo rámec LL tabulky.

Výsledek výpočtu množiny follow(<expr>) byl využit jako množina ukončovačů výrazové analýzy. Přijde-li na vstup jeden z těchto terminálů, precedenční parser až do svého ukončení místo vstupního terminálu využívá ukončovač $ v souladu s matematickým modelem.

# Implementační triky

Mezi hlavní implementační triky patří rozlišení situace příkazu přiřazení a příkazu tvořeného výrazem, který začíná ID; a dále také ternární if, vyžadující spolupráci obou parserů.

Na začátku příkazu je nutné od sebe odlišit následující situace:
```scala
5 * 2 + example
example = 5 * 2
example + 5 * 2
```

Tyto situace jsou řešeny v pravidlech L30-L33 (viz obrázek). Byl zaveden neterminál <exprNoID>, který má stejný význam jako <expr> s tím rozdílem, že jeho množina first neobsahuje ID a if, jednak pro odlišení výše uvedené situace, ale také jako vyřešení otázky, zde je samostatně položený if výrazový, či běžný řídící. <exprNoID> je skutečným neterminálem formální gramatiky, v implementaci však není odlišen od <expr>, protože by jeho chování bylo stejné.

Ze situací přiřazení vs. výraz začínající ID je terminál ID vytknut (pravidlo L31), přičemž sémantická akce tohoto pravidla uschová ID v podobě uzlu AST. Pravidla L32 a L33 provádí jádro mimo rámec běžného běhu. Pokud je na vstupu =, běžně provede tvorbu uzlu přiřazení. Pokud je na vstupu cokoliv jiného, je spuštěn precedenční parser s uchovaným počátečním terminálem ID.

Dostane-li prediktivní parser na vstup terminál if, zavolá prediktivní parser s počátečním neterminálem <ternaryIf>. Prediktivní parser zpracuje ternární if, přičemž jeho sémantické akce rozhodnou o tom, v jaké podobě se ternární if vrátí.

Prediktivní parser narozdíl od matematického modelu končí prostým vyprázdněním zásobníku bez kontroly EOF, aby zpracování ternárního ifu skončilo korektně. Ověření, že byl přečten celý vstupní soubor, je zajištěno tím, že terminál EOF je součástí pravidla L0. Bez korektního porovnání EOF není v nejvyšším běhu parseru možné vyprázdnit zásobník.

Pokud ternární if končí odsazeným výrazem, je do výrazové analýzy vrácen jako neterminál E s hotovým uzlem AST. V této situaci precedenční parser zahájí ukončení svého běhu.

Jestliže ternární if nekončí odsazeným výrazem, je prediktivním parserem zpracován bez větve else a navrácen je jako terminál TIF s nedokončeným uzlem AST. Ten je na zásobník zařazen standardním způsobem přes precedenční tabulku a větev else připojí příslušná redukce (pravidlo E41).

Tento přístup vyžadoval zavedení "virtuálních pravidel" ilustrujících umístění ternárního ifu v programu IFJ26 pro účely výpočtu množiny follow(<ternaryIf>) a množin, které na tomto výpočtu zavisí.







Popis modulů parseru (neměly by všechny být na konci na jednom místě ze všech částí?)

nonterminal.*: Výčtový typ NonterminalType reprezentující neterminály obou parserů a obslužné metody pro práci s nimi.
grammar_symbol.h: Struktura GrammarSymbol reprezentující buď terminál, nebo neterminál, a makra pro zápis jejích hodnot.

semantic_types.h: Datové typy pro sémantické akce a deklarace datových typů ukazatelů na sémantické akce.
parser_common.\*: Struktura ParserStackEntry reprezentující položku zásobníku parseru a metody obalující zásobník ze stack.\*, struktura ParserContext obsahující především stav sdíleného vstupního proudu terminálů a metody pro práci s ním.

production.*: Datový typ StaticProduction reprezentující statické pravidlo a definice samotných statických pravidel.
predictive_semantic_actions.*: Deklarace a definice sémantických akcí prediktivního parseru.
LL_table.*: LL tabulka obsahující statická a callbacková pravidla prediktivního parseru a metoda pro získání její položky.
predictive_parser.* Implementace jádra prediktivního parseru.

precedence_table.* Precedenční tabulka obsahující direktivy, indexační převod z terminálu na index a metoda pro získání direktivy z tabulky.
precedence_semantic_actions.* Deklarace a definice sémantických akcí precedenčního parseru.
reduction_table.* Struktura SearchHandle sloužící jako klíč do tabulky, redukční tabulka a metoda pro vyhledání položky v tabulce.
precedence_helpers.* Pomocné metody využívané jádrem precedenčního parseru.
precedence_parser.* Implementace jádra precedenčního parseru.

special_actions.* Speciální sémantické akce obou parserů, které nejsou vázány na pravidlo. Používají je interně sémantické akce jednotlivých parserů nebo jádra parserů přímo.
