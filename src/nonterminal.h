/*
    IFJ projekt 2026

    nonterminal.h

    Puvodni autor: xbalaka00 za asistence Claude
    Seznam zmen:
        - Pridana hlavicka pro funkci nonterm_type_name
        - xbalaka00 - pridan NONTERM_ELSE_TERNARY_IF.
        - xbalaka00 - odstranen NONTERM_OPTIONAL_SEMICOLON (nova gramatika).
        - xbalaka00 - odstranen NONTERM_EXPR_NO_ID (chova se stejne jako
          NONTERM_EXPR).
        - xbalaka00 - pridan oddelovac NONTERM_LL_TABLE_COUNT, za nej
          presunut NONTERM_ID_COMMAND a pridany NONTERM_TIF a NONTERM_ARGS
          z vyrazove analyzy, pridana funkce nonterm_type_validate.

    Tento soubor obsahuje vyctovy typ pro neterminaly a deklarace funkci pro
    praci s nimi.

*/

#ifndef NONTERMINAL_H
#define NONTERMINAL_H

/*
    Prompt pro nove rozdeleni terminalu a neterminalu (NONTERM_LL_TABLE_COUNT,
    TOKEN_EXPR_TERM_DELIMITER, terminaly TOKEN_ID_OPERATOR a TOKEN_DOLLAR),
    validacni funkce token_type_validate a nonterm_type_validate a jejich
    pouziti v parser_advance. Stejny prompt zpusobil zmeny take v token.h,
    token.c, nonterminal.c, LL_table.h, LL_table.c a parser_common.c, jeho
    zneni je pouze zde. Zmeny v predictive_parser.c (volani validacnich funkci
    v jadre) jsou popsany v dalsim promptu nize.

    Prompt pro Claude:
        Započnu vývoj precedenčního parseru. V přiloženém excelu na druhém listu
        je návrh pravidel, LL tabulka prozatím není vyplněná.

        Začneme změnama terminálů a neterminálů v souborech nonterminal.* a
        token.*

        NONTERM_COUNT může zůstat jako informativní hodnota, ale nejspíše už
        nebude příliš užitečný. Bude potřeba zavést novou speciální hodnotu
        NONTERM_LL_TABLE_COUNT. Před tímto oddělovačem bude každý neterminál,
        kterým lze indexovat LL tabulku. Změň velikost LL tabulky na tento nový
        oddělovač.
        Za tento oddělovač přesuň současný terminál <IDcommand>. Později za něj
        přijde i <expr>, ale prozatím jej ponech jako indexátor LL tabulky, aby
        fungovala některá testovací pravidla a řešení. Nové neterminály z
        výrazové analýzy TIF (ternary if) a ARGS (arguments) dej za tento
        oddělovač; pro neterminál E se využije prostě záznam NONTERM_EXPR. U
        neterminálů, kterými se neindexuje LL tabulka, nezáleží na jejich
        číselné hodnotě.

        Trochu složitější situace bude u tokenů. V současném stavu je enum
        rozdělen takto:

        * terminály LL gramatiky
        * TOKEN_LL_TERM_COUNT
        * terminály mimo LL gramatiku, kterými smí začínat výraz (tedy literály
          a unární operátory)
        * TOKEN_EXPRESSION_BEGINNING_DELIMITER
        * Ostatní terminály výrazové gramatiky, kterými nesmí začínat výraz
          (tedy binární operátory).

        Chci, aby nové rozdělení bylo

        * indexátory LL tabulky
        * TOKEN_LL_TERM_COUNT (případně přejmenovat, avšak pak ho přejmenuj
          všude, ale asi může zůstat).
        * Terminály výrazové analýzy, kterými smí začínat výraz
        * TOKEN_EXPRESSION_BEGINNING_DELIMITER
        * Ostatní terminály výrazové analýzy, které jsou indexátorem
          precedenční tabulky.
        * Nový TOKEN_EXPR_TERM_COUNT (nebo TOKEN_EXPR_INDEXATOR_DELIMITER nebo
          tak nějak)
        * "Ostatní" terminály, které nebudou indexátory ani jedné z tabulek.
        * Případně koncová speciální hodnota pro zjištění počtu terminálů, i
          když nevím, jestli bude moc užitečný.

        Při výpočtech LL tabulky bylo zjištěno, že terminály hranatých závorek,
        tečka, hvězdička a INDENT nejsou nikdy indexátory LL tabulky (a změny LL
        gramatiky by na tom nic změnit neměly, a i kdyby, tak se to přeskládá až
        bude změněná gramatika převedena do kódu, pokud tedy nesetrváme se
        současnou gramatikou i přes to, že v ní je chyba (naštěstí však taková,
        která možná ani nebude testována)).

        Proto hvězdičku přidej zpět mezi binární operátory jako násobení, tečka
        může přijít do stejné kategorie dle výše uvedeného rozdělení.

        Hranaté závorky a INDENT přijdou do kategorie ostatní.

        V rámci výrazové analýzy pak přibudou i dva speciální terminály $ a
        ID_OPERATOR, které nebude vracet scanner a budou "vytvořeny" interně ve
        výrazovém scanneru. Oba by měly přijít do kategorie, kde jsou i binární
        operátory (neindexují LL tabulku, indexují precedenční tabulku, nezačíná
        jimi výraz).
        Jestli chceš, smíš do souborů token.*, resp. nonterm.* přidat i
        validační funkci, která vyhodí chybu ERR_INTERNAL, pokud token, resp.
        neterminál nebude "skutečný" - tedy funkce zkontroluje, že se nejedná o
        jednu ze speciálních hodnot či hodnotu větší než největší platná
        hodnota, ale dále terminály nebude klasifikovat. Tyto metody pak doplň
        jako dodatečnou (ačkoliv pravděpodobně zbytečnou, ale to asi nevadí)
        interní kontrolu do funkce parser_advance, a potom obě také na vhodné
        místo (např. na místo v jádře, kde je ze stacku vyjmut neterminál, a na
        místo, kdy se pracuje s terminálem na vrcholu zásobníku před
        parser_match a speciální funkcionalitou pro ID). Pokud by tato kontrola
        znamenala, že některé jiné kontroly v jiných funkcích jsou již zbytečné
        (např. ve funkci pro indexaci LL tabulky, kde však samozřejmě musí
        zůstat/přibýt kontrola, že jsou terminály a neterminálu v rozsahu
        indexace tabulky), pak je odstraň.

    Prompt pro Claude (oprava predchoziho kroku, TOKEN_STAR zustava docasne
    v LL tabulce; stejny prompt zpusobil zmeny take v token.h a LL_table.c):
        Počkej, LL tabulku neměň. Pokud jsem řekl, že se musí přemístit
        terminál, který je prozatím potřeba pro testovací účely ponechej v
        tabulce, tak ho ponechej a poznač, že se to změní v budoucnu. Myslím,
        že je to jen případ hvězdičky.

    Prompt pro Claude (doplneni validace do jadra, stejny prompt zpusobil
    zmeny take v predictive_parser.c):
        Zde je predictive_parser.c - pokud bych v budoucnu znova zapoměl
        přiložit nějaký soubor, vyžádej si ho, než začneš pracovat. Můžeš v
        něm upravit ty interní kontroly, jak jsem říkal.
*/

/*
    Prompty pro vychozi kostru souboru a enum neterminalu. Vysledny kod byl
    upraven.

    Prompt pro Claude:
        Ahoj, potřeboval bych pomoci s přetavením LL gramatiky do parseru. Zatím
        ještě není přechroustaná do LL tabulky, to si pak chci zkusit sám.
        Přikládám ti excel s gramatikou, možná ještě není bez much, ale to
        nebrání s parserem pokročit. Zvolil jsem přístup prediktivního parseru,
        takže změny by měly být levné. Samotné jádro parseru chci napsat
        převážně sám.

        Pro začátek mi jen vygeneruj enum se všemi neterminály. Zkus reflektovat
        název, který je v tabulce, ale jestli tě občas napadne lepší a
        výstižnější název pro neterminál, asi ho můžeš lehce upravit, ale vypiš
        mi seznam změn. Gramatiku neměň. Dej neterminálům prefix NONTERM. Jestli
        chceš, můžeš občas přidat komentář dle kontextu tokenu nebo neterminály
        rozdělit do kategorií, avšak nemusíš. Případně si to pak upravím.

        Tady je naše naming konvence:
        - Proměnné a funkce - snake_case
        - Konstanty a položky enum - SCREAMING_SNAKE_CASE
        - Datové typy - PascalCase
        - Komentáře česky
        - Kód, git messages anglicky

        Formátování neřeš, potom to proženu clang-formatem. Využij tyto konvence
        i v případných dalších úkolech.

    Prompt pro Claude:
        Aha, já ho zapomněl nahrát. Tady je. Píši v jazyce C.

    Prompt pro Claude:
        Mohl by jsi komentáře přepsat tak, aby byly jednoduché? Zde je úryvek
        kódu kolegy, ideálně tedy zkus držet formu s ním, ať je vše
        konzistentní. Před samotným enumem a obecně při hlavní dokumentaci
        nějaké funkce v hlavičkovém souboru však můžeš komentář udělat takový,
        aby byl ve tvaru doxygen dokumentace.

        Inspirativní úryvek ze scanneru:
        <zde jsem prekopiroval nahodny usek z kodu scanner.c>

*/

/*
    Prompt pro pridani NONTERM_ELSE_TERNARY_IF je v souboru
    semantic_actions.c. Prompt pro odstraneni
    NONTERM_OPTIONAL_SEMICOLON je v souboru production.c.
    Prompt pro odstraneni NONTERM_EXPR_NO_ID. Stejny prompt zpusobil zmeny
    take v nonterminal.c, production.c, production.h a LL_table.c, jeho zneni
    je pouze zde.

    Prompt pro Claude:
        Nová gramatika sice ještě není hotová, ale už teď můžeme poupravit
        jednu drobnost - myslím, že neterminál NONTERM_EXPR_NO_ID nejspíše
        nebude v implementaci existovat. V excelu byl odlišen pouze kvůli
        výpočtům množin a LL tabulky. Čistě formálně a matematicky tedy sice
        možná existuje, ale má naprosto totožné chování jako NONTERM_EXPR v
        tom smyslu, že také způsobí přechod do výrazové analýzy. Proto jej v
        kódu nebudeme odlišovat.

        Smaž tedy NONTERM_EXPR_NO_ID z nonterminal.* a v pravidlech každý jeho
        výskyt nahraď NONTERM_EXPR; myslím, že se jedná pouze o statické
        pravidlo 30.
*/

/**
 * @brief Neterminaly LL gramatiky a vyrazove analyzy.
 *
 * Hodnoty mensi nez NONTERM_LL_TABLE_COUNT indexuji LL tabulku. Hodnoty za
 * timto oddelovacem LL tabulku neindexuji a na jejich cislene hodnote nezalezi.
 * Poradi neterminalu pred oddelovacem odpovida prvnimu vyskytu v pravidlech
 * gramatiky.
 */
typedef enum {
  // Struktura programu
  NONTERM_PROGRAM,         ///< Startovni neterminal
  NONTERM_DEF_LIST,        ///< Seznam deklaraci funkci
  NONTERM_DEF,             ///< Deklarace funkce
  NONTERM_MAIN_ANNOTATION, ///< Anotace @main
  NONTERM_OPTIONAL_EOL,    ///< Nepovinny EOL

  // Parametry a typy
  NONTERM_PARAM_LIST,      ///< Parametr nebo konec parametru
  NONTERM_PARAM_LIST_TAIL, ///< Dalsi parametr za carkou nebo konec
  NONTERM_OPTIONAL_TYPE,   ///< Nepovinny typ
  NONTERM_TYPE,            ///< Datovy typ

  // Bloky a prikazy
  NONTERM_BLOCK_OR_EXPR,    ///< Telo funkce, vyraz nebo blok
  NONTERM_BLOCK,            ///< Odsazeny blok
  NONTERM_COMMAND_LIST,     ///< Dalsi prikazy v bloku
  NONTERM_COMMAND_END,      ///< Konec prikazu, strednik nebo EOL
  NONTERM_COMMAND,          ///< Prikaz
  NONTERM_COMMAND_SIMPLE,   ///< Jednoduchy prikaz (val/var, return, ID, vyraz)
  NONTERM_COMMAND_CONTROL,  ///< Ridici prikaz (while, for, if)
  NONTERM_VAL_VAR,          ///< val nebo var
  NONTERM_RETURN_VALUE,     ///< Nepovinna hodnota za return
  NONTERM_BLOCK_OR_COMMAND, ///< Telo cyklu nebo podminky

  // Cyklus for
  NONTERM_FOR_HEADER, ///< Hlavicka for
  NONTERM_GUARD_LIST, ///< Seznam strazi

  // Podminky
  NONTERM_OPTIONAL_ELSE,       ///< Nepovinna vetev else
  NONTERM_THEN_BRANCH,         ///< Vetev then za podminkou
  NONTERM_ELSE_OR_COMMAND_END, ///< Else za inline then, nebo konec prikazu
  NONTERM_TERNARY_IF,          ///< Ternarni if
  NONTERM_EXPR_OR_INDENT_EXPR, ///< Vyraz, pripadne odsazeny
  NONTERM_ELSE_TERNARY_IF,     ///< Else vetev ternarniho ifu, pripadne odsazena

  // Pseudoneterminal, predani do vyrazove analyzy. Prozatim indexuje LL
  // tabulku kvuli testovacim pravidlum, pozdeji bude presunut za oddelovac.
  NONTERM_EXPR, ///< Vyraz (v precedencni gramatice neterminal E)

  NONTERM_LL_TABLE_COUNT, ///< Pocet neterminalu indexujicich LL tabulku

  // Neterminaly, ktere LL tabulku neindexuji
  NONTERM_ID_COMMAND, ///< Prikaz zacinajici na ID (v odpovednosti jadra)
  NONTERM_TIF,        ///< Ternarni if ve vyrazove analyze (TIF)
  NONTERM_ARGS,       ///< Seznam argumentu ve vyrazove analyze (ARGS)

  NONTERM_COUNT ///< Pocet hodnot enumu, pouze informativni
} NonterminalType;

/**
 * @brief Vrati citelny nazev neterminalu pro chybove vypisy.
 *
 * @param type Typ neterminalu.
 * @return Retezec s nazvem neterminalu, pro neplatnou hodnotu "?".
 */
const char *nonterm_type_name(NonterminalType type);

/**
 * @brief Overi, ze typ neterminalu je skutecny neterminal.
 *
 * Pri neplatne hodnote ukonci program s kodem ERR_INTERNAL. Neplatne jsou
 * specialni hodnoty (NONTERM_LL_TABLE_COUNT, NONTERM_COUNT) a hodnoty mimo
 * enum. Funkce neterminaly dale neklasifikuje, tedy nezjistuje, zda indexuji
 * LL tabulku.
 *
 * @param type Overovany typ neterminalu.
 * @param line Cislo radku pro chybove hlaseni, <= 0 pokud neni znam.
 */
void nonterm_type_validate(NonterminalType type, int line);

#endif // NONTERMINAL_H
