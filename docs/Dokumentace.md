![[logo_cz.png|440]]
# <center>Dokumentace K projektu do předmětu IFJ 2026</center>

<center>Jméno, příjmení a xlogin : Os. Číslo: </center>
<center><b>Artur Balák xbalaka00 : 282413</b></center>
<center>Michal Nováček xnovacm00 : 279697 </center>
<center>Radovan Šantavý xsantar00 : 281778</center>
<center>Anna Navrátilová xnavraa00 : 281956</center>

<center>Varianta: vv-BVS</center>
<center>Rozdeleni: xbalaka00: 37% | xnovacm00: 39% | xsantar00: 24% | xnavraa00: 0%</center>
<center>Rozsireni: <code>FUNEXP</code>, <code>BOOLTHEN</code>, <code>CONTROL</code>, <code>LISTS</code>, <code>VALMEM</code></center>
<center>Odkaz na repositář: <b><a href="https://github.com/Dustlightgit/IFJ-Projekt-2026">IFJ-Projekt-2026</a></b></center>
<div style="height: 100px;"></div>

# Rozdělení práce

- Artur Balák
    - Návrh a implementace prediktivního a precedenčního parseru, rozhraní pro stavbu AST a doplnění stavby AST k precedenčnímu parseru.
- Michal Nováček
    - Návrh a implementace scanneru, AST, tabulky symbolů, sémantického analyzátoru a stavba AST u prediktivního parseru.
- Radovan Šantavý
    - Návrh a implementace generátoru kódu, technické zázemí (github, discord...), testovací prostředí, dokumentace a prezentace k obhajobě.

# Odchylky od rovnoměrného rozdělení bodů
- Anna Navrátilová dostala 0 bodů, protože se rozhodla projekt opustit.
- Radovan Šantavý po domluvě obdržel menší podíl než ostatní, protože na projektu vykonal relativně k ostatním méně práce.

# Způsob spolupráce
- Každý člen týmu pracoval na své části samostatně, avšak součinnost jednotlivých komponent byla v reálném čase diskutována a domlouvána na discordu.
- Dělba práce byla domlouvána ve vyhrazeném kanálu.
- Aby mohl být PR sloučen s main, musí získat alespoň 2x kladné stanovisko; výjímkou jsou PR s drobnými úpravami, kde postačuje jedno kladné stanovisko.
- PR neovlivňujíci větev `main` nemusí být nutně někým kontrolován.
- Bylo dohodnuto, že hloubkový code review se při schvalování PR nevyžaduje. Schvalující osoba by se však měla alespoň přehledově seznámit s kódem a případné odhalené problémy uvést ideálně do komentáře na github, avšak postačuje i oznámení nedostatku na discordu.

# Využití AI

Při vývoji projektu byly použity nástroje umělé inteligence _ChatGPT_, _Claude_ a _Gemini_.

Gemini byl využit k dovysvětlení některých pasáží z teorie.

ChatGPT byl využit jako konzultant návrhu parseru. Svým způsobem se jednalo o zveličený zápisník, do kterého byly vypsány nápady na řešení jednotlivých problémů souvisejících se zadáním. ChatGPT obvykle odpovídal analýzou proveditelnosti nebo podněty na uhlazení návrhu. Občas doporučil vlastní řešení, které bylo v některých případech přijato, v některých upraveno a v jiných sloužilo jako dobrý podnět, aby byl návrh lépe vysvětlen.
Například návrh na implementaci prediktoru operandu a operátoru jakožto řešení pro infixový alfanumerický operátor pochází od ChatGPT, avšak jeho původní návrh by prakticky znamenal implementovat kompletní třístavový konečný automat, který na základě přechodů i vyvolává chyby, přitom bohatě dostačuje jednoduchý jednobitový "omylný" prediktor obarvující terminály, jaký je ve výsledné implementaci.

Většina kódu parseru byla vygenerována přes Claude na základě promptů, které jsou uvedeny ve zdrojových souborech. Vývoj z větší části probíhal tak, že v každém promptu byla specifikována přesná změna v projektu odpovídající přibližně jednomu commitu. Claude podle promptu upravil soubory, ty byly zkontrolovány přes funkcionalitu diff ve visual studio code, pokusně zkompilovány, případně i otestovány (pokud to bylo možné) a nakonec commitnuty. Kód vygenerovaný Claude za celou dobu vývoje ani jednou neselhal při kompilaci či testování, což však nejspíše bude z části důsledek i přesné specifikace změn. V některých otázkách byla Claude ponechána větší volnost, například při generování koster sémentických akcí na základě excel souboru, avšak vždy byly vymezeny meze, v jakých se tak dělo.

Gramatika obou parserů i všechny tabulky byly vytvořeny ručně. Claude také zkontroloval ručně navrženou precedenční tabulku a gramatiku a odhalil v ní několik chyb, které ve svém výčtu odůvodnil. Všechny odhalené chyby se zdály být formálně i matematicky opodstatněné, a proto byly aplikovány opravy návrhu.

Claude byl rovněž využit pro generování menších úseků kódu i v jiných částech projektu.

---

# Využité formy testování
## Architektura a princip testování
- Testování řídí skript `test.py`, který postupně spouští jednotlivé testovací moduly (`runner.c`).
- `runner.c` slouží jako náhradní vstupní bod programu (fake `main`) a informační most mezi testovanou částí překladače a testovacím prostředím.
- Vstupy jsou v souborech input.txt a výstupy testovaných modulů se zapisují do souboru `output.txt`, který se následně porovnává s očekávaným výstupem `expected.txt`.
- U testů _generátoru_ a celého _překladače_ (obecně testů vyžadujících interpret) se mezivýstup v cílovém jazyce _ifjcode26_ nejprve uloží do souboru `ifjcode.txt`, vykoná pomocí interpretu a teprve výsledný běh se porovná s očekávaným výstupem.

## Struktura komponent a konfigurace
- Testy jsou rozděleny do izolovaných komponent s vlastním sestavením: `lexer`, `parser`, `generator` a `pipeline`.
	- Všechny zdrojové soubory jsou také překládány pro každou komponentu zvlášť.
	- Každý jednotlivý test nebo celou komponentu lze z testování manuálně vynechat (aktuálně jsou vynechány vybrané testy, pro které nebudeme implementovat rozšíření).
- Seznamy ignorovaných testů (`EXCEPTIONS`) a testů vyžadujících běh v interpretu (`INTERPRET_TESTS`) se definují přímo v `test.py`.
- Podrobnosti k použítí testovacího prostředí jsou popsány v `/test/tests.md`


# Konvence projektu
- **Formátovací**
    - Formátování kódu zajišťuje ručně nastavený clang-format založený na stylu `LLVM`
- **Jazykové**
    - Kód, commit zprávy, názvy souborů (až na výjímky dané zadáním) jsou psány anglicky
    - Komentáře jsou česky bez diakritiky s výjímkou promptů, které mohou být doslovně i s diakritikou
- **Jmenné**
    - Běžné identifikátory, názvy souborů - `snake_case`
    - Konstanty - `SCREAMING_SNAKE_CASE`
    - Názvy datových typů - `PascalCase`
- **Jiné**
    - Každý soubor začíná hlavičkou s názvem souboru, se jmény osob, které se na souboru podílely, a stručným popisem souboru.
    - Nepovinně (dle uvážení původního autora souboru) lze v hlavičce také udržovat seznam změn.

---
# Diagram automatu, který specifikuje lexikální analyzátor
![[fsm_scanner.pdf]]

---

![[BlockScheme.pdf]]

---

# LL Gramatika

![[grammar_whole.png]]
## LL tabulka

![[LL_table.png]]
# Precedenční tabulka
![[precedence_table.png]]

---
# Stručný popis každého modulu

## Lexikální analýza
  
`scanner.c`, `scanner.h`: Lexikální analyzátor. Zvenku je vidět jen funkce `get_token`, která ze vstupu přečte další token. Kromě běžných tokenů vrací i `EOL`, `INDENT` a `DEDENT`, podle kterých parser pozná konce řádků a bloky.

Scanner je konečný automat v jedné funkci `read_token`, kde `switch` podle aktuálního stavu rozhoduje, co se znakem udělá. Znaky se skládají do bufferu a když přijde znak, který už k tokenu nepatří, vrátí se zpátky na vstup. Vrátit umíme i víc znaků najednou, protože u `5.toString` musíme vrátit tečku i písmeno, aby z `5` zůstal celočíselný literál.

Víceznakové operátory (`==`, `<-`, `++`, `&&`, ...) mají vlastní stavy, které po prvním znaku čekají na druhý. Identifikátory a klíčová slova se čtou stejným stavem a až po přečtení celého slova se v tabulce zjistí, jestli je to klíčové slovo. Čísla se nejdřív načtou jako text a na hodnotu se převedou až na konci, kde se také hlídá nepovolená úvodní nula. U řetězců automat zpracovává escape sekvence a umí i víceřádkový zápis `"""`.

Na čtyři věci čistý konečný automat nestačí, proto jsme ho rozšířili:

**Vnořené blokové komentáře:** hloubku vnoření počítá proměnná `comment_depth`, komentář končí až u poslední dvojice `*/`.

**Víceřádkové řetězce:** počítáme uvozovky za sebou. Poslední tři ukončí řetězec, ostatní patří do jeho obsahu.

**Konce řádků:** token `EOL` vznikne jen když na řádku byl nějaký token, prázdné řádky a komentáře se ignorují. Blokový komentář s koncem řádku se chová jako `EOL`. Uvnitř závorek je konec řádku jen mezera, ale scanner ho poznačí do příznaku `nl_before` u dalšího tokenu a hlídá, aby pokračovací řádek byl odsazený víc než začátek příkazu.

**Odsazení:** bloky se určují odsazením, proto scanner drží zásobník úrovní. U prvního tokenu na řádku porovná počet mezer s vrcholem zásobníku. Při větším odsazení vrátí `INDENT`, při menším jeden `DEDENT` za každý uzavřený blok a na konci vstupu uzavře všechny otevřené bloky. Skutečný token si mezitím podrží a vrátí až za nimi.

Neplatný vstup (neznámý znak, neukončený řetězec nebo komentář, tabulátor v odsazení, odsazení neodpovídající žádné předchozí úrovni) skončí chybou `ERR_LEXICAL` s číslem řádku.

## Abstraktní syntaktický strom

`ast.c`, `ast.h`: Datové struktury abstraktního syntaktického stromu (program, funkce, bloky, příkazy, výrazy a typy) a funkce pro jeho vytváření, postupné sestavování a uvolňování. Strom je výstupem parseru a vstupem sémantické analýzy a generátoru kódu.

Program drží pole funkcí, funkce má tělo (`Block`), tělo obsahuje příkazy (`Stmt`) a ty výrazy (`Expr`). Příkazy i výrazy jsou jedna struktura s polem `kind` a unií `as`, kde `kind` říká, kterou variantu můžeme číst. Všechny uzly se tak procházejí jedním `switch`.

Rodič vlastní své potomky, takže `program_free` uvolní celý strom. Konstruktory a settery přebírají vlastnictví uzlů, řetězce si kopírují. Ukazatele `symbol` strom nevlastní, patří tabulce symbolů.

Parser skládá strom až při redukci pravidel, takže uzel často vytváří neúplný (konstruktory berou `NULL`) a zbytek doplňuje settery. Typy výrazů (`Expr.type`) a ukazatele `symbol` nechává na `NULL`, doplňuje je sémantická analýza. Každý výraz dostane vlastní kopii typu (`type_clone`), aby se nic neuvolňovalo dvakrát.
## Tabulka symbolů

`symtable.c`, `symtable.h`: Tabulka symbolů pro sémantickou analýzu. Uchovává proměnné a funkce (i vestavěné) s jejich typy a hlídá, která jména jsou v daném místě programu vidět. Je postavená na AVL stromu.

Všechny symboly ze všech rozsahů jsou v jednom AVL stromu. Klíčem je dvojice (jméno, číslo rozsahu), takže vedle sebe můžou bez problému existovat dvě proměnné stejného jména v různých blocích. Aktuálně otevřené rozsahy si tabulka drží na zásobníku jejich čísel. Při vstupu do bloku (`symtable_enter_scope`) dostane rozsah nové číslo, které se nikdy nepoužije znovu, a při výstupu se jen sníží hloubka zásobníku. Symboly se tím neuvolňují, jen přestanou být vidět, takže ukazatele na ně v AST zůstávají platné až do `symtable_free`.

Hledání (`symtable_lookup`) prochází rozsahy od nejvnitřnějšího ven a vrací první nalezený symbol, díky čemuž funguje zastínění proměnných. Pro kontrolu redefinice se hledá jen v aktuálním rozsahu. Každá proměnná má navíc `unique_id`, podle kterého se zastíněné proměnné se stejným jménem rozliší.

Funkce jsou vždy v globálním rozsahu. Před analýzou těl se musí zaregistrovat všechny, aby se šlo volat funkce definované později. Navratový typ funkce může být zpočátku neznámý (`NULL`) a odvodí se až při analýze těla. Příznak `being_inferred` slouží k detekci rekurze při odvozování návratového typu. Vestavěné funkce se registrují při startu (`symtable_register_builtins`) a `NULL` v typech parametrů u nich znamená `Any`.

Tabulka vlastní všechny symboly, jména i typy si kopíruje a uvolní je jedním `symtable_free`. Odkaz na definici funkce v AST tabulka nevlastní, uvolňuje se spolu se stromem.
## Sémantická analýza

`Semantic_analyzer.c`, `semantic_analyzer.h`: Sémantická analýza. Projde AST, zkontroluje typy a pravidla jazyka (definice, počty argumentů, `@main`, ...) a doplní do stromu typy výrazů a odkazy na symboly. Vrací tabulku symbolů, kterou dál používá generátor kódu.

Vstupem je AST od parseru, hlavní funkce `semantic_analyze` nejdřív vytvoří tabulku symbolů a zaregistruje do ní vestavěné funkce. Potom zapíše všechny uživatelské funkce, aby šlo volat i funkce definované později v souboru. Tady se také kontroluje `@main`: musí být právě jedna, bez parametrů a s typem `Unit`. Teprve potom se analyzují těla funkcí.

Analýza je rekurzivní průchod stromem. `analyze_expr` zjistí typ výrazu, uloží ho do `Expr.type` (každý výraz má vlastní kopii) a vrátí ho. Zároveň doplní ukazatele `symbol` u proměnných, volání, přiřazení a cyklů, které parser nechal prázdné. Bloky a těla funkcí a cyklů se analyzují v novém rozsahu tabulky symbolů. Při definici proměnné se nejdřív analyzuje inicializace a teprve potom se proměnná zapíše do tabulky. Funkci `analyze_expr` jde předat i očekávaný typ (z deklarace, z přiřazení, z návratového typu), který potřebujeme třeba pro `List()` bez prvků, jinak by se typ prázdného seznamu nedal odvodit.

Pravidla typů jsou jednoduchá. Jediná implicitní konverze je `Int` na `Double` (`type_assignable`). Aritmetika dvou `Int` dává `Int`, jinak `Double`. Prvky seznamu se sjednocují do společného typu (`Int` a `Double` dají `Double`). Zápis `lst(i)` parser nepozná od volání funkce, takže semantická analýza podle symbolu rozhodne, jestli jde o indexování seznamu. Členy hodnot (`toString`, `length`, `drop`, ...) se kontrolují podle typu objektu a počtu a typu argumentů.

Když funkce nemá uvedený návratový typ, odvodí se z typu těla. Pokud se taková funkce zavolá dřív, než na ni dojde řada, analyzuje se na požádání (`analyze_func` se zavolá přímo z volání). Příznak `being_inferred` přitom pozná rekurzi, která musí mít typ uvedený. Funkce, které už byly odvozeny, se v hlavním průchodu přeskočí. U funkcí s explicitním typem se kontroluje, že tělo vrací správný typ, nebo že vždy skončí `return` (`block_returns`).

---

## Parser a rozhraní pro stavbu AST

`nonterminal.*`: Výčtový typ `NonterminalType` reprezentující neterminály obou parserů a obslužné metody pro práci s nimi.
`grammar_symbol.h`: Struktura `GrammarSymbol` reprezentující buď terminál, nebo neterminál, a makra pro zápis jejích hodnot.

`semantic_types.h`: Datové typy pro sémantické akce a deklarace datových typů ukazatelů na sémantické akce.
`parser_common.*`: Struktura `ParserStackEntry` reprezentující položku zásobníku parseru a metody obalující zásobník ze `stack.*`, struktura `ParserContext` obsahující především stav sdíleného vstupního proudu terminálů a metody pro práci s ním.

`production.*`: Datový typ `StaticProduction` reprezentující statické pravidlo a definice samotných statických pravidel.
`predictive_semantic_actions.*`: Deklarace a definice sémantických akcí prediktivního parseru.
`LL_table.*`: LL tabulka obsahující statická a callbacková pravidla prediktivního parseru a metoda pro získání její položky.
`predictive_parser.*`: Implementace jádra prediktivního parseru.

`precedence_table.*`: Precedenční tabulka obsahující direktivy, indexační převod z terminálu na index a metoda pro získání direktivy z tabulky.
`precedence_semantic_actions.*`: Deklarace a definice sémantických akcí precedenčního parseru.
`reduction_table.*`: Struktura `SearchHandle` sloužící jako klíč do tabulky, redukční tabulka a metoda pro vyhledání položky v tabulce.
`precedence_helpers.*`: Pomocné metody využívané jádrem precedenčního parseru.
`precedence_parser.*`: Implementace jádra precedenčního parseru.

`special_actions.*`: Speciální sémantické akce obou parserů, které nejsou vázány na pravidlo. Používají je interně sémantické akce jednotlivých parserů nebo jádra parserů přímo.


Parser je složený z **prediktivního** a **precedenčního** parseru. Oba mají společný vstupní proud _tokenů_ (ve struktuře `ParserContext`).
Oba parsery se rozhodují na základě svých tabulek a oba na základě provedených pravidel provádějí _sémantické akce_, které především staví AST.
Část funkcionality stavby AST je vyčleněna do _speciálních akcí_, které nejsou svázané s konkrétním pravidlem.
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
### Prediktivní parser

Jádro precedenčního parseru je implementováno algoritmem z přednášek, tentokrát v zásadě bez odlišností, avšak koná jisté věci navíc. Precedenčnímu parseru je možné specifikovat počáteční terminál, který je využit místo aktuálního terminálu vstupního proudu.

**LL tabulka** je prostým dvojrozměrným polem a obsahuje (mimo chybové záznamy) pravidla dvojího druhu - _statické_ a _callbackové_.

**Statické pravidlo** je konstanta typu `StaticProduction`, která přímo obsahuje pravou stranu pravidla pro vložení na zásobník. Takto jsou reprezentována jednoduchá, obvykle čistě syntaktická pravidla nepotřebující vykonávat zvláštní činnost.

**Callbackové pravidlo** je ukazatel na funkci, tzv. sémantickou akci. Sémantická akce obvykle provádí stavbu AST, případně je potřeba pro fungování některého z implementačních triků či slouží jako rozcestník pro sémantické informace. Odpovědností sémantické akce je vložit na zásobník pravou stranu pravidla, které reprezentuje; k jednotlivým položkám smí přiložit sémantické informace dle svého uvážení.

**Sémantické informace** se ukládají ke každé položce zásobníku. Jedná se o _sémantický ukazatel_ - libovolný ukazatel typu `(void *)` na cokoliv dle uvážení _sémantické akce_, obvykle na uzel AST, ke kterému se má připojit další uzel AST či doplnit informace. Dále se jedná o _sémantickou anotaci_, kterou je možné specifikovat očekávané chování další akce v pořadí.

Pro šíření sémantických informací skrz pomyslný derivační strom platí jednoduché pravidlo - statické pravidlo vždy zkopíruje _ukazatel_ i _anotaci_ ze svého _neterminálu_ do všech symbolů na pravé straně pravidla. Podrobnější řízení toku informací pak vyžaduje _sémantickou akci_.

Pokud je na vrcholu zásobníku i na vstupu terminál ID, je nad ním zavolána příslušná speciální akce.

### Precedenční parser

Jádro precedenčního parseru je implementováno algoritmem z přednášek, tentokrát bez odlišností, avšak koná jisté věci navíc. Precedenčnímu parseru je možné specifikovat počáteční _terminál_, který je využit místo aktuálního terminálu vstupního proudu.

**Redukční značka** `<` je reprezentována příznakem u položky zásobníku s významem, že znak `<` následuje bezprostředně po této položce.

V parseru je využit **jednobitový prediktor**, který za základě terminálu na vstupu předpoví, jestli příští terminál bude operand, nebo operátor. _Prediktor_ je využit pro "přebarvení" vstupních terminálů na jejich varianty v některých situacích, aby byly především pro účely _precedenční tabulky_ odlišeny od původních terminálů. Terminály _+_ a _-_ jsou v příslušných situacích přebarveny na své unární varianty, a terminál _ID_ je přebarven na operátorovou verzi.

**Precedenční tabulka** je dvojrozměrné pole obsahující čtyři direktivy z algoritmu z přednášek (včetně chybové direktivy); pro indexaci je využit indexační převod. Jádro na základě direktiv ohraničuje pravé strany pravidel, tzv. _handle_ pravidla. Při redukci vyhledá _handle_ pravidla v _redukční tabulce_ a provede příslušnou _sémantickou akci_ (pokud je nalezena).

**Redukční tabulka** je statická vyhledávací tabulka s klíčem typu `SearchHandle` (symbolová reprezentace _handle_); jeho užitečnou položkou je _ukazatel na sémantickou akci_. Vzhledem k malému a neměnnému počtu položek je hledání implementováno se složitostí $O(n)$ lineárním vyhledáním s předčasným ukončením při překročení velikosti `SearchHandle`. Symboly v _handle_ jsou porovnávány kompletně, aby bylo zajištěno, že se nikdy neprovede nesprávná redukce. Neúspěšné vyhledání je syntaktická chyba.

Narozdíl od prediktivního parseru je každé pravidlo precedenčního parseru reprezentováno _sémantickou akcí_. Akce opět především staví AST. _Sémantické akce_ mají díky mechanismu vyhledání v _redukční tabulce_ zaručeno, že na zásobníku se při jejich zavolání nachází jejich _handle_. _Sémantická akce_ má právo nahlížet na položky zásobníku, aby si z nich získala potřebné _sémantické informace_. Akce vrátí symbol anotovaný sémantickými informacemi, na který by měl být zredukován _handle_. Precedenční akce zásobník nijak nemění.

### Řádný přechod mezi parsery

V gramatice prediktivního parseru je zaveden speciální neterminál $\langle\text{expr}\rangle$. Pokud se tento neterminál objeví na vrcholu zásobníku, je zahájen přechod do výrazové analýzy. Zavolá se precedenční parser, který vrátí výsledný výrazový podstrom; ten je připojen ke zbytku stromu příslušnou speciální akcí.

Pro účely výpočtů množin pro LL tabulku byly ručně definovány následující množiny:
- $\text{empty}(\langle\text{expr}\rangle) = \emptyset$
- $\text{first}(\langle\text{expr}\rangle) = \{ (, \text{List}, \text{if}, \text{ID}, \text{*další} \}$
přičemž *další zastupuje symboly, které neindexují LL tabulku, ale smí jimi začínat výraz. Jedná se o literály a unární operátory. Pro tyto symboly je v návrhu LL tabulky vyhrazen zvláštní sloupec. V případě, že je na vstupu jeden z těchto terminálů, je příslušné pravidlo provedeno jádrem prediktivního parseru mimo rámec LL tabulky.

Výsledek výpočtu množiny $\text{follow}(\langle\text{expr}\rangle)$ byl využit jako množina ukončovačů výrazové analýzy. Přijde-li na vstup jeden z těchto terminálů, _precedenční parser_ až do svého ukončení místo vstupního terminálu využívá ukončovač $\$$ v souladu s matematickým modelem.

### Implementační triky

Mezi hlavní implementační triky patří rozlišení situace příkazu přiřazení a příkazu tvořeného výrazem, který začíná ID; a dále také ternární if, vyžadující spolupráci obou parserů.

Na začátku příkazu je nutné od sebe odlišit následující situace:
```scala
5 * 2 + example
example = 5 * 2
example + 5 * 2
```

Tyto situace jsou řešeny v pravidlech L30-L33 (viz obrázek). Byl zaveden neterminál $\langle\text{exprNoID}\rangle$, který má stejný význam jako $\langle\text{expr}\rangle$ s tím rozdílem, že jeho množina first neobsahuje ID a if, jednak pro odlišení výše uvedené situace, ale také jako vyřešení otázky, zde je samostatně položený if výrazový, či běžný řídící. $\langle\text{exprNoID}\rangle$ je skutečným neterminálem formální gramatiky, v implementaci však není odlišen od $\langle\text{expr}\rangle$, protože by jeho chování bylo stejné.

Ze situací přiřazení vs. výraz začínající ID je terminál ID vytknut (pravidlo L31), přičemž sémantická akce tohoto pravidla uschová ID v podobě uzlu AST. Pravidla L32 a L33 provádí jádro mimo rámec běžného běhu. Pokud je na vstupu =, běžně provede tvorbu uzlu přiřazení. Pokud je na vstupu cokoliv jiného, je spuštěn precedenční parser s uchovaným počátečním terminálem ID.

Dostane-li prediktivní parser na vstup terminál if, zavolá prediktivní parser s počátečním neterminálem $\langle\text{ternaryIf}\rangle$. Prediktivní parser zpracuje ternární if, přičemž jeho sémantické akce rozhodnou o tom, v jaké podobě se ternární if vrátí.

Prediktivní parser narozdíl od matematického modelu končí prostým vyprázdněním zásobníku bez kontroly EOF, aby zpracování ternárního ifu skončilo korektně. Ověření, že byl přečten celý vstupní soubor, je zajištěno tím, že terminál EOF je součástí pravidla L0. Bez korektního porovnání EOF není v nejvyšším běhu parseru možné vyprázdnit zásobník.

Pokud ternární if končí odsazeným výrazem, je do výrazové analýzy vrácen jako neterminál E s hotovým uzlem AST. V této situaci precedenční parser zahájí ukončení svého běhu.

Jestliže ternární if nekončí odsazeným výrazem, je prediktivním parserem zpracován bez větve else a navrácen je jako terminál TIF s nedokončeným uzlem AST. Ten je na zásobník zařazen standardním způsobem přes precedenční tabulku a větev else připojí příslušná redukce (pravidlo E41).

Tento přístup vyžadoval zavedení "virtuálních pravidel" ilustrujících umístění ternárního ifu v programu IFJ26 pro účely výpočtu množiny $\text{follow}(\langle\text{ternaryIf}\rangle)$ a množin, které na tomto výpočtu zavisí.

## Generování kódu

`generator.c`, `generator.h`: Generátor cílového kódu. Překládá otypovaný AST do instrukcí `IFJcode26`. _Tabulku symbolů_ už nepotřebuje, protože všechny symboly a typy jsou uložené přímo v uzlech stromu.

Výrazy se generují zásobníkově, takže každý _výraz_ zanechá na vrcholu datového zásobníku přesně jednu hodnotu, zatímco _příkazy_ zásobník nechávají beze změny. Generátor průběžně sleduje hloubku zásobníku relativně k začátku funkce a po každém příkazu i výrazu ji kontroluje (`gen_check_depth`). Tělo funkce se nejprve zapíše do paměťového bufferu přes `open_memstream`, aby se všechny proměnné zaevidované během průchodu mohly vypsat jako `DEFVAR` na začátek rámce ještě před samotné instrukce těla. Proměnné mají tvar `LF@<jmeno>_<unique_id>` (díky čemuž nekolidují zastíněné proměnné), dočasné proměnné mají tvar `LF@$tN`.

Při volání se argumenty uloží na zásobník a volaná funkce je odebere v opačném pořadí (obdoba konvence volání _pascal_). Každá funkce vrací právě jednu hodnotu (typ `Unit` vrací `nil@nil`, který volající zahodí do `GF@$discard`). Pokud při předčasném `return` zůstávají pod návratovou hodnotou rozpracované mezivýsledky, uloží se výsledek do dočasné proměnné, přebytek se odstraní (popne se do `GF@$discard`) a hodnota se vrátí zpět na vrchol před `POPFRAME` a `RETURN`. Jediná implicitní konverze `Int` na `Double` se řeší instrukcí `INT2FLOATS`. Reálné literály se vypisují ve formátu `%a` a speciální znaky v řetězcích se escapují jako `\xyz`.

Podporujeme `if` jako výraz, zkrácené vyhodnocování `&&` a `||` i cykly `while` a `for` (přes rozsahy i seznamy, včetně stráží). Seznam je reprezentován jako pole `IFJcode26`, kde prvek na indexu 0 drží délku a data jsou uložena od indexu 1. Vestavěné funkce (`print`, `toStr`, `readInt`, `subStr`, ...) a pomocné rutiny (`list$get`, `list$range`, `list$slice`, `list$tail`, `list$concat`, `double$floor`) jsou definovány jako funkce v cílovém kódu a na konec výstupu se vypíšou jen když je program včetně závislostí skutečně využil. Návěští funkcí a rutin začínají znakem `$`, takže nekolidují s uživatelskými identifikátory. Běhové chyby (index mimo meze, nulový krok rozsahu, prázdný seznam u `tail` či neplatné meze u `subStr`) skáčou na sdílené návěští `$error$40` s ukončením `EXIT int@40`.


## Pomocné funkce

`dstring.c`, `dstring.h`: Dynamický řetězec, který si sám zvětšuje paměť podle potřeby. _Scanner_ ho používá na skládání identifikátorů, čísel a řetězců po jednom znaku.

`DString` si pamatuje text, jeho délku a kolik má alokované paměti. Když se při přidání znaku paměť zaplní, zdvojnásobí se, takže se nemusí zvětšovat po každém znaku. Text je pořád ukončený `'\0'`, takže ho jde normálně předat funkcím z `<string.h>`. Až je řetězec hotový, zkopíruje se přes `dstr_copy` a tu kopii pak musí _volající_ uvolnit sám.

`stack.c`, `stack.h`: Obecný zásobník prvků pevné velikosti, který si sám zvětšuje paměť. Používá se na uložení úrovní odsazení ve _scanneru_ a v _syntaktické analýze_. Funkce `stack_top` a `stack_peek` vrací ukazatel přímo na prvek, takže ho _volající_ musí přetypovat na správný typ.

`error.c`, `error.h`: Chybové návratové kódy podle zadání a funkce `error_exit`, která vypíše chybu na `stderr` a ukončí program s odpovídajícím kódem.

`memory.c`, `memory.h`: Pomocné funkce pro správu paměti (`xmalloc`, `xcalloc`, `xrealloc`, `xstrdup`, `xstrdup_opt` a `grow_array`). Obalují standardní alokační funkce a při selhání alokace okamžitě ukončí program voláním `error_exit` s kódem `ERR_INTERNAL`, aby se kontrola ukazatele `NULL` nemusela opakovat v celém _překladači_. Funkce `xstrdup_opt` navíc bezpečně propouští `NULL` jako `NULL` a `grow_array` slouží ke zvětšování dynamických polí o jeden prvek (případně k jejich vytvoření z ukazatele `NULL`).


