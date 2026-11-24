
Úvodní strana - bude potřeba zformátovat (vedoucí má být tučně takže to tučné zvýraznění není chyba)

**Artur Balák xbalaka00 282413**
Anna Navrátilová xnavraa00 281956
Michal Nováček xnovacm00 279697
Radovan Šantavý xsantar00 281778

Zvolená varianta - vv-BVS

Rozdělení bodů
    - Artur Balák 37 %
    - Anna Navrátilová 0 %
    - Michal Nováček 39 %
    - Radovan Šantavý 24 %

FUNEXP
BOOLTHEN
CONTROL
LISTS
VALMEM

https://github.com/Dustlightgit/IFJ-Projekt-2026





# Rozdělení práce

- Artur Balák
    - Návrh a implementace prediktivního a precedenčního parseru, rozhraní pro stavbu AST a doplnění stavby AST k precedenčnímu parseru.
- Michal Nováček
    - Návrh a implementace scanneru, AST, tabulky symbolů, sémantického analyzátoru a stavba AST u prediktivního parseru.
- Radovan Šantavý
    - Návrh a implementace generátoru kódu, technické zázemí (github, discord...), testovací prostředí, dokumentace a prezentace k obhajobě.

# Odchylky od rovnoměrného rozdělení bodů
- Anna Navrátilová dostala 0 bodů, protože se rozhodla projekt (a pravděpodobně i školu) opustit.
- Radovan Šantavý po domluvě obdržel menší podíl než ostatní, protože na projektu vykonal relativně k ostatním méně práce.

# Způsob spolupráce
- Každý člen týmu pracoval na své části samostatně, avšak součinnost jednotlivých komponent byla v reálném čase diskutována a domlouvána na discordu.
- Dělba práce byla domlouvána v příslušném kanálu.
- Aby mohl být PR sloučen s main, musí získat alespoň 2x kladné stanovisko; výjímkou jsou PR s drobnými úpravami, kde postačuje jedno kladné stanovisko.
- Bylo dohodnuto, že hloubkový code review se při schvalování PR nevyžaduje.
- Schvalující osoba by se však měla alespoň přehledově seznámit s kódem a případné odhalené problémy uvést ideálně do komentáře na github, avšak postačuje i oznámení nedostatku na discordu.

# Využité formy testování
Doplní Radovan


# Konvence projektu
- Formátovací
    - Formátování kódu zajišťuje ručně nastavený clang-format (případně doplnit podrobnosti?)
- Jazykové
    - Kód, commit zprávy, názvy souborů (až na výjímky dané zadáním) anglicky
    - Komentáře česky bez diakritiky s výjímkou promptů, které mohou být doslovně i s diakritikou
- Jmenné
    - Běžné identifikátory, názvy souborů snake_case
    - Konstanty SCREAMING_SNAKE_CASE
    - Názvy datových typů PascalCase
- Jiné
    - Každý soubor začíná hlavičkou s názvem souboru, se jmény osob, které se na souboru podílely, a stručným popisem souboru.
    - Nepovinně (dle uvážení původního autora souboru) lze v hlavičce také udržovat seznam změn.