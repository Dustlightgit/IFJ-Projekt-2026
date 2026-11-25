**scanner.c, scanner.h:** Lexikální analyzátor. Zvenku je vidět jen funkce `get_token`, která ze vstupu přečte další token. Kromě běžných tokenů vrací i `EOL`, `INDENT` a `DEDENT`, podle kterých parser pozná konce řádků a bloky.

**Lexikální analýza**

Scanner je konečný automat v jedné funkci `read_token`, kde `switch` podle aktuálního stavu rozhoduje, co se znakem udělá. Znaky se skládají do bufferu a když přijde znak, který už k tokenu nepatří, vrátí se zpátky na vstup. Vrátit umíme i víc znaků najednou, protože u `5.toString` musíme vrátit tečku i písmeno, aby z `5` zůstal celočíselný literál.

Víceznakové operátory (`==`, `<-`, `++`, `&&`, ...) mají vlastní stavy, které po prvním znaku čekají na druhý. Identifikátory a klíčová slova se čtou stejným stavem a až po přečtení celého slova se v tabulce zjistí, jestli je to klíčové slovo. Čísla se nejdřív načtou jako text a na hodnotu se převedou až na konci, kde se také hlídá nepovolená úvodní nula. U řetězců automat zpracovává escape sekvence a umí i víceřádkový zápis `"""`.

Na čtyři věci čistý konečný automat nestačí, proto jsme ho rozšířili:

**Vnořené blokové komentáře:** hloubku vnoření počítá proměnná `comment_depth`, komentář končí až u poslední dvojice `*/`.

**Víceřádkové řetězce:** počítáme uvozovky za sebou. Poslední tři ukončí řetězec, ostatní patří do jeho obsahu.

**Konce řádků:** token `EOL` vznikne jen když na řádku byl nějaký token, prázdné řádky a komentáře se ignorují. Blokový komentář s koncem řádku se chová jako `EOL`. Uvnitř závorek je konec řádku jen mezera, ale scanner ho poznačí do příznaku `nl_before` u dalšího tokenu a hlídá, aby pokračovací řádek byl odsazený víc než začátek příkazu.

**Odsazení:** bloky se určují odsazením, proto scanner drží zásobník úrovní. U prvního tokenu na řádku porovná počet mezer s vrcholem zásobníku. Při větším odsazení vrátí `INDENT`, při menším jeden `DEDENT` za každý uzavřený blok a na konci vstupu uzavře všechny otevřené bloky. Skutečný token si mezitím podrží a vrátí až za nimi.

Neplatný vstup (neznámý znak, neukončený řetězec nebo komentář, tabulátor v odsazení, odsazení neodpovídající žádné předchozí úrovni) skončí chybou `ERR_LEXICAL` s číslem řádku.

**ast.c, ast.h:** Datové struktury abstraktního syntaktického stromu (program, funkce, bloky, příkazy, výrazy a typy) a funkce pro jeho vytváření, postupné sestavování a uvolňování. Strom je výstupem parseru a vstupem sémantické analýzy a generátoru kódu.

**Abstraktní syntaktický strom**

Program drží pole funkcí, funkce má tělo (`Block`), tělo obsahuje příkazy (`Stmt`) a ty výrazy (`Expr`). Příkazy i výrazy jsou jedna struktura s polem `kind` a unií `as`, kde `kind` říká, kterou variantu můžeme číst. Všechny uzly se tak procházejí jedním `switch`.

Rodič vlastní své potomky, takže `program_free` uvolní celý strom. Konstruktory a settery přebírají vlastnictví uzlů, řetězce si kopírují. Ukazatele `symbol` strom nevlastní, patří tabulce symbolů.

Parser skládá strom až při redukci pravidel, takže uzel často vytváří neúplný (konstruktory berou `NULL`) a zbytek doplňuje settery. Typy výrazů (`Expr.type`) a ukazatele `symbol` nechává na `NULL`, doplňuje je sémantická analýza. Každý výraz dostane vlastní kopii typu (`type_clone`), aby se nic neuvolňovalo dvakrát.

**symtable.c, symtable.h:** Tabulka symbolů pro sémantickou analýzu. Uchovává proměnné a funkce (i vestavěné) s jejich typy a hlídá, která jména jsou v daném místě programu vidět. Je postavená na AVL stromu.

**Tabulka symbolů**

Všechny symboly ze všech rozsahů jsou v jednom AVL stromu. Klíčem je dvojice (jméno, číslo rozsahu), takže vedle sebe můžou bez problému existovat dvě proměnné stejného jména v různých blocích. Aktuálně otevřené rozsahy si tabulka drží na zásobníku jejich čísel. Při vstupu do bloku (`symtable_enter_scope`) dostane rozsah nové číslo, které se nikdy nepoužije znovu, a při výstupu se jen sníží hloubka zásobníku. Symboly se tím neuvolňují, jen přestanou být vidět, takže ukazatele na ně v AST zůstávají platné až do `symtable_free`.

Hledání (`symtable_lookup`) prochází rozsahy od nejvnitřnějšího ven a vrací první nalezený symbol, díky čemuž funguje zastínění proměnných. Pro kontrolu redefinice se hledá jen v aktuálním rozsahu. Každá proměnná má navíc `unique_id`, podle kterého se zastíněné proměnné se stejným jménem rozliší.

Funkce jsou vždy v globálním rozsahu. Před analýzou těl se musí zaregistrovat všechny, aby se šlo volat funkce definované později. Navratový typ funkce může být zpočátku neznámý (`NULL`) a odvodí se až při analýze těla. Příznak `being_inferred` slouží k detekci rekurze při odvozování návratového typu. Vestavěné funkce se registrují při startu (`symtable_register_builtins`) a `NULL` v typech parametrů u nich znamená `Any`.

Tabulka vlastní všechny symboly, jména i typy si kopíruje a uvolní je jedním `symtable_free`. Odkaz na definici funkce v AST tabulka nevlastní, uvolňuje se spolu se stromem.

**Semantic_analyzator.c, semantic_analyzator.h:** Sémantická analýza. Projde AST, zkontroluje typy a pravidla jazyka (definice, počty argumentů, `@main`, ...) a doplní do stromu typy výrazů a odkazy na symboly. Vrací tabulku symbolů, kterou dál používá generátor kódu.

**Sémantická analýza**

Vstupem je AST od parseru, hlavní funkce `semantic_analyze` nejdřív vytvoří tabulku symbolů a zaregistruje do ní vestavěné funkce. Potom zapíše všechny uživatelské funkce, aby šlo volat i funkce definované později v souboru. Tady se také kontroluje `@main`: musí být právě jedna, bez parametrů a s typem `Unit`. Teprve potom se analyzují těla funkcí.

Analýza je rekurzivní průchod stromem. `analyze_expr` zjistí typ výrazu, uloží ho do `Expr.type` (každý výraz má vlastní kopii) a vrátí ho. Zároveň doplní ukazatele `symbol` u proměnných, volání, přiřazení a cyklů, které parser nechal prázdné. Bloky a těla funkcí a cyklů se analyzují v novém rozsahu tabulky symbolů. Při definici proměnné se nejdřív analyzuje inicializace a teprve potom se proměnná zapíše do tabulky. Funkci `analyze_expr` jde předat i očekávaný typ (z deklarace, z přiřazení, z návratového typu), který potřebujeme třeba pro `List()` bez prvků, jinak by se typ prázdného seznamu nedal odvodit.

Pravidla typů jsou jednoduchá. Jediná implicitní konverze je `Int` na `Double` (`type_assignable`). Aritmetika dvou `Int` dává `Int`, jinak `Double`. Prvky seznamu se sjednocují do společného typu (`Int` a `Double` dají `Double`). Zápis `lst(i)` parser nepozná od volání funkce, takže semantická analýza podle symbolu rozhodne, jestli jde o indexování seznamu. Členy hodnot (`toString`, `length`, `drop`, ...) se kontrolují podle typu objektu a počtu a typu argumentů.

Když funkce nemá uvedený návratový typ, odvodí se z typu těla. Pokud se taková funkce zavolá dřív, než na ni dojde řada, analyzuje se na požádání (`analyze_func` se zavolá přímo z volání). Příznak `being_inferred` přitom pozná rekurzi, která musí mít typ uvedený. Funkce, které už byly odvozeny, se v hlavním průchodu přeskočí. U funkcí s explicitním typem se kontroluje, že tělo vrací správný typ, nebo že vždy skončí `return` (`block_returns`).

**dstring.c, dstring.h:** Dynamický řetězec, který si sám zvětšuje paměť podle potřeby. Scanner ho používá na skládání identifikátorů, čísel a řetězců po jednom znaku.

`DString` si pamatuje text, jeho délku a kolik má alokované paměti. Když se při přidání znaku paměť zaplní, zdvojnásobí se, takže se nemusí zvětšovat po každém znaku. Text je pořád ukončený `'\0'`, takže ho jde normálně předat funkcím z `<string.h>`. Až je řetězec hotový, zkopíruje se přes `dstr_copy` a tu kopii pak musí volající uvolnit sám.

**stack.c, stack.h:** Obecný zásobník prvků pevné velikosti, který si sám zvětšuje paměť. Používá se na uložení úrovní odsazení ve scanneru a v syntaktické analýze. Funkce `stack_top` a `stack_peek` vrací ukazatel přímo na prvek, takže ho volající musí přetypovat na správný typ.

**error.c, error.h:** Chybové návratové kódy podle zadání a funkce `error_exit`, která vypíše chybu na `stderr` a ukončí program s odpovídajícím kódem.