
## Parser a rozhraní pro stavbu AST

Parser je složený z prediktivního a precedenčního parseru. Oba mají společný vstupní proud tokenů (ve struktuře ParserContext).
Oba parsery se rozhodují na základě svých tabulek a oba na základě provedených pravidel provádějí sémantické akce, které především staví AST.
Část funkcionality stavby AST je vyčleněna do speciálních akcí, které nejsou svázané s konkrétním pravidlem.
Oba parsery se vzájemně rekurzivně volají, aby mohl být zpracován ternární if - každá instance zavolání parseru má tedy svůj vlastní zásobník.

# Prediktivní parser

Jádro prediktivního parseru je implementováno algoritmem z přednášek - jedinou odlišností je, že končí jednoduchým vyprázdněním zásobníku. Parseru je možné specifikovat počáteční neterminál. Obě tyto vlastnosti má kvůli ternárnímu ifu - viz dále.

LL tabulka je prostým dvojrozměrným polem a obsahuje (mimo chybové záznamy) pravidla dvojího druhu - statické a callbackové.

Statické pravidlo je konstanta typu StaticProduction, která přímo obsahuje pravou stranu pravidla pro vložení na zásobník. Takto jsou reprezentována jednoduchá, obvykle čistě syntaktická pravidla nepotřebující vykonávat zvláštní činnost.

Callbackové pravidlo je ukazatel na funkci, tzv. sémantickou akci. Sémantická akce obvykle provádí stavbu AST, případně je potřeba pro fungování některého z implementačních triků či slouží jako rozcestník pro sémantické informace. Odpovědností sémantické akce je vložit na zásobník pravou stranu pravidla, které reprezentuje; k jednotlivým položkám smí přiložit sémantické informace dle svého uvážení. Sémantické akce mohou některé

Sémantické informace se ukládají ke každé položce zásobníku. Jedná se o sémantický ukazatel - libovolný ukazatel typu (void *) na cokoliv dle uvážení sémantické akce, obvykle na uzel AST, ke kterému se má připojit další uzel AST či doplnit informace. Dále se jedná o sémantickou anotaci, kterou je možné specifikovat očekávané chování další akce v pořadí.

Pro šíření sémantických informací skrz pomyslný derivační strom platí jednoduché pravidlo - statické pravidlo vždy zkopíruje ukazatel i anotaci ze svého neterminálu do všech symbolů na pravé straně pravidla. Podrobnější řízení toku informací pak vyžaduje sémantickou akci.

# Precedenční parser

Jádro precedenčního parseru je implementováno algoritmem z přednášek, tentokrát bez odlišností, avšak koná jisté věci navíc. Precedenčnímu parseru je možné specifikovat počáteční terminál, který je využit místo aktuálního terminálu vstupního proudu.

Redukční značka < je reprezentována příznakem u položky zásobníku s významem, že znak < následuje bezprostředně po této položce.

Precedenční tabulka je dvojrozměrné pole obsahující čtyři direktivy z algoritmu z přednášek (včetně chybové direktivy); pro indexaci je využit indexační převod. Jádro na základě direktiv ohraničuje pravé strany pravidel, tzv. handle pravidla. Při redukci vyhledá handle pravidla v redukční tabulce a provede příslušnou sémantickou akci (pokud je nalezena).

Redukční tabulka je statická vyhledávací tabulka s klíčem typu SearchHandle (symbolová reprezentace handle); jeho užitečnou položkou je ukazatel na sémantickou akci. Vzhledem k malému a neměnnému počtu položek je hledání implementováno se složitostí O(n) lineárním vyhledáním s předčasným ukončením při překročení velikosti SearchHandle. Symboly v handle jsou porovnávány kompletně, aby bylo zajištěno, že se nikdy neprovede nesprávná redukce. Neúspěšné vyhledání je syntaktická chyba.

Narozdíl od prediktivního parseru je každé pravidlo precedenčního parseru reprezentováno sémantickou akcí. Akce opět především staví AST. Sémantické akce mají díky mechanismu vyhledání v redukční tabulce zaručeno, že na zásobníku se při jejich zavolání nachází jejich handle. Sémantická akce má právo nahlížet na položky zásobníku, aby si z nich získala potřebné sémantické informace. Akce vrátí symbol anotovaný sémantickými informacemi, na který by měl být zredukován handle. Precedenční akce zásobník nijak nemění.

# Řádný přechod mezi parsery

Prozatím nedokončeno.

# Implementační triky

Prozatím nedokončeno.

Ověření, že byl přečten celý vstupní soubor, je zajištěno tím, že token EOF je součástí gramatiky - bez jeho porovnání není v nejvyšším běhu parseru možné vyprázdnit zásobník.