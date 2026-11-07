<!--
    IFJ projekt 2026

    Původní autor: ...
    Seznam změn:
        - doplneni informaci o priprave prostredi pro scala: xsantar00

    Poskytuje informace o prekladu pomoci scala a o IFJ26_examples 

-->

# Ukázkové programy IFJ26

V tomto archivu najdete několik ukázkových programů v jazyce IFJ26. V adresáři `basic/` jsou ukázky
pro základní zadání, v adresáři `extensions/` je pak jeden příklad pro každé rozšíření. Programy lze
spustit také ve Scale 3.8.4 s využitím souboru (knihovny) [ifj26.scala](ifj26.scala).

Soubory `.in` a `.out` obsahují ukázkový standardní vstup a očekávaný standardní výstup.
Prázdný soubor `.in` znamená, že žádný vstup není potřeba.

V příkladech pro základní zadání jsou k dispozici také soubory `.ifjcode`, které implementují stejný 
algoritmus v IFJcode26. 

| IFJ26 | Protějšek v IFJcode26 | Požadovaná rozšíření | Ukázka |
| --- | --- | --- | --- |
| [basic/factorial.scala](basic/factorial.scala) | [factorial.ifjcode](basic/factorial.ifjcode) | Žádná | Iterativní a rekurzivní faktoriál, vstup, `val`/`var`, podmínky, cykly, explicitní a implicitní `return`. Vstup je omezen na 0–12, aby nedošlo k přetečení faktoriálu; neplatný celočíselný vstup se (podle specifikace) nastaví na nulu. |
| [basic/strings.scala](basic/strings.scala) | [strings.ifjcode](basic/strings.ifjcode) | Žádná | Operace s řetězci a aritmetické operace. **Pozor**, odpovídající kód v IFJcode26 není kompletní (viz komentáře v souboru). |
| [funexp.scala](extensions/funexp.scala) | – | FUNEXP | Vnořená volání ve výrazech a argumentech. |
| [boolthen.scala](extensions/boolthen.scala) | – | BOOLTHEN | Booleovský vstup, hodnoty a návratové typy, negace, zkrácené vyhodnocování AND/OR, `else if` a `if` bez `else`. |
| [control.scala](extensions/control.scala) | – | CONTROL, BOOLTHEN | Podmíněné výrazy, inkluzivní a exkluzivní rozsahy, záporný krok, více stráží a sčítání vybraných hodnot. |
| [valmem.scala](extensions/valmem.scala) | – | VALMEM | `.toString`, `.toInt`, `.floor`, `.length` a řetězené členy. |
| [algebdt.scala](extensions/algebdt.scala) | – | ALGEBDT | Součinové typy, sjednocení typů, vzory a stráže v `match`. |
| [interpolateds.scala](extensions/interpolateds.scala) | – | INTERPOLATEDS | `$id`, `${expression}`, `$$`, víceřádkový interpolovaný řetězec. |
| [lambdas.scala](extensions/lambdas.scala) | – | LAMBDAS, FUNEXP | Funkce vyššího řádu (funkce jako hodnoty), typované lambda výrazy. |
| [lists.scala](extensions/lists.scala) | – | LISTS, CONTROL, LAMBDAS, FUNEXP, BOOLTHEN | Konstrukce seznamu, převod z rozsahu, zřetězení, výřezy, indexování, head/tail, formátování, filtrování, take/dropWhile a iterace se stráží. |
| [extkw.scala](extensions/extkw.scala) | –| EXTKW | Rozšiřující metoda bez parametrů, metoda s prázdným seznamem parametrů a metoda s argumentem. |
| [paramlists.scala](extensions/paramlists.scala) | – | PARAMLISTS | Funkce bez parametrů, více seznamů parametrů a prázdný seznam parametrů. |

<!-- Zacatek editu: Poupravil jsem prikazy pro nase prostredi. Spusteni se dela normalne z rootu projektu -->

## Příprava lokalního prostředí
1. Instalace Java a Scala

Nainstalujte Java Development Kit a příkazový řádek pro Scalu (na merlinu nemusíš):

```sh
sudo apt update
sudo apt install default-jdk
curl -sSLf https://virtuslab.github.io/scala-cli-packages/scala-setup.sh | sh
```
2. Nastavení proměnné prostředí

Před první interakcí s CLI nástrojem scala si nastavte proměnnou prostředí, tato úprava platí jen po dobu života vašeho sezení v otevřeném shellu – po ukončení, bude nutné příkaz provést znovu.

```sh
export JAVA_TOOL_OPTIONS="-Xmx1G"
```

## Spuštění
```sh
# Spuštění programu IFJ26 ve Scale # edit: me osobne nefunguje
cd test/IFJ26_examples      # navigace do slozky se scala soubory
scala run --server=false ifj26.scala basic/factorial.scala <basic/factorial.in

# Spuštění programu IFJcode26 v námi dodaném interpretu (viz e-learning) # edit: tohle normalne funguje
cd test     # navigace do testove slozky
./interpret/ic26int-linux-x86_64 IFJ26_examples/basic/factorial.ifjcode <IFJ26_examples/basic/factorial.in

# Příklad spuštění s využitím vašeho překladače
./vas-prekladac <basic/factorial.scala >/tmp/factorial.ifjcode
./ic26int-linux-x86_64 /tmp/factorial.ifjcode <basic/factorial.in
```
