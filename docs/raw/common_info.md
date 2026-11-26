
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



# Využití AI
Při vývoji projektu byly použity nástroje umělé inteligence ChatGPT, Claude a Gemini.

Gemini byl využit k dovysvětlení některých pasáží z teorie.

ChatGPT byl využit jako konzultant návrhu parseru. Svým způsobem se jednalo o zveličený zápisník, do kterého byly vypsány nápady na řešení jednotlivých problémů souvisejících se zadáním. ChatGPT obvykle odpovídal analýzou proveditelnosti nebo podněty na uhlazení návrhu. Občas doporučil vlastní řešení, které bylo v některých případech přijato, v některých upraveno a v jiných sloužilo jako dobrý podnět, aby byl návrh lépe vysvětlen.
Například návrh na implementaci prediktoru operandu a operátoru jakožto řešení pro infixový alfanumerický operátor pochází od ChatGPT, avšak jeho původní návrh by prakticky znamenal implementovat kompletní třístavový konečný automat, který na základě přechodů i vyvolává chyby, přitom bohatě dostačuje jednoduchý jednobitový "omylný" prediktor obarvující terminály, jaký je ve výsledné implementaci.

Většina kódu parseru byla vygenerována přes Claude na základě promptů, které jsou uvedeny ve zdrojových souborech. Vývoj z větší části probíhal tak, že v každém promptu byla specifikována přesná změna v projektu odpovídající přibližně jednomu commitu. Claude podle promptu upravil soubory, ty byly zkontrolovány přes funkcionalitu diff ve visual studio code, pokusně zkompilovány, případně i otestovány (pokud to bylo možné) a nakonec commitnuty. Kód vygenerovaný Claude za celou dobu vývoje ani jednou neselhal při kompilaci či testování, což však nejspíše bude z části důsledek i přesné specifikace změn. V některých otázkách byla Claude ponechána větší volnost, například při generování koster sémentických akcí na základě excel souboru, avšak vždy byly vymezeny meze, v jakých se tak dělo.

Gramatika obou parserů i všechny tabulky byly vytvořeny ručně. Jedinou výjímku tvoří nejnovější verze LL tabulky a jejích množin. Původní verze LL gramatiky musela během vývoje být dvakrát přepracována kvůli chybám, které odhalilo testování. V původní verzi i v první revizi byly výpočty množin provedeny ručně. Při odhalení nutnosti druhé revize byla úprava gramatiky provedena ručně, z časových důvodů však byly výpočty množin a vyplnění LL tabulky přenechány Claude, jehož práce byla poté zkontrolována a testována. Claude také zkontroloval ručně navrženou precedenční tabulku a gramatiku a odhalil v ní několik chyb, které ve svém výčtu odůvodnil. Všechny odhalené chyby se zdály být formálně i matematicky opodstatněné, a proto byly aplikovány opravy návrhu.

Claude byl rovněž využit pro generování menších úseků kódu i v jiných částech projektu.