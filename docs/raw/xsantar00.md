![[logo_cz.png|440]]
# <center>Dokumentace k projektu do předmětu IFJ 2026</center>

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





`generator.c`, `generator.h`: Generátor cílového kódu. Překládá otypovaný AST do instrukcí `IFJcode26`. Tabulku symbolů už nepotřebuje, protože všechny symboly a typy jsou uložené přímo v uzlech stromu.

## Generování kódu

Výrazy se generují zásobníkově, takže každý výraz zanechá na vrcholu datového zásobníku přesně jednu hodnotu, zatímco příkazy zásobník nechávají beze změny. Generátor průběžně sleduje hloubku zásobníku relativně k začátku funkce a po každém příkazu i výrazu ji kontroluje (`gen_check_depth`). Tělo funkce se nejprve zapíše do paměťového bufferu přes `open_memstream`, aby se všechny proměnné zaevidovane během průchodu mohly vypsat jako `DEFVAR` na začátek rámce ještě před samotné instrukce těla. Proměnné mají tvar `LF@<jmeno>_<unique_id>` (díky čemuž nekolidují zastíněné proměnné), dočasné proměnné `LF@$tN`.

Při volání se argumenty uloží na zásobník a volaná funkce je odebere v opačném pořadí. Každá funkce vrací právě jednu hodnotu (typ `Unit` vrací `nil@nil`, který volající zahodí do `GF@$discard`). Pokud při předčasném `return` zůstávají pod návratovou hodnotou rozpracované mezivýsledky, uloží se výsledek do dočasné proměnné, přebytek se odstraní (popne se do `GF@$discard`) a hodnota se vrátí zpět na vrchol před `POPFRAME` a `RETURN`. Jediná implicitní konverze `Int` na `Double` se řeší instrukcí `INT2FLOATS`. Reálné literály se vypisují ve formátu `%a` a speciální znaky v řetězcích se escapují jako `\xyz`.

Podporujeme `if` jako výraz, zkrácené vyhodnocování `&&` a `||` i cykly `while` a `for` (přes rozsahy i seznamy, včetně stráží). Seznam je reprezentován jako pole `IFJcode26`, kde prvek na indexu 0 drží délku a data jsou uložena od indexu 1. Vestavěné funkce (`print`, `toStr`, `readInt`, `subStr`, ...) a pomocné rutiny (`list$get`, `list$range`, `list$slice`, `list$tail`, `list$concat`, `double$floor`) jsou definovány jako funkce v cílovém kódu a na konec výstupu se vypíšou jen ty, které program včetně závislostí skutečně využil. Návěští funkcí a rutin začínají znakem `$`, takže nekolidují s uživatelskými identifikátory. Běhové chyby (index mimo meze, nulový krok rozsahu, prázdný seznam u `tail` či neplatné meze u `subStr`) skáčou na sdílené návěští `$error$40` s ukončením `EXIT int@40`.

`memory.c`, `memory.h`: Pomocné funkce pro správu paměti (`xmalloc`, `xcalloc`, `xrealloc`, `xstrdup`, `xstrdup_opt` a `grow_array`).

Obalují standardní alokační funkce a při selhání alokace okamžitě ukončí program voláním `error_exit` s kódem `ERR_INTERNAL`, aby se kontrola ukazatele `NULL` nemusela opakovat v celém překladači. Funkce `xstrdup_opt` navíc bezpečně propouští `NULL` jako `NULL` a `grow_array` slouží ke zvětšování dynamických polí o jeden prvek (případně k jejich vytvoření z ukazatele `NULL`).




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