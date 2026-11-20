/*
    IFJ projekt 2026

    xsantar00 Radovan Santavy
    xnovacm00 Michal Novacek
    xbalaka00 Artur Balak

    token.h

    Puvodni autor: xsantar00 - prvni verze vyctoveho typu enumu
    Seznam zmen:
        - xnovacm00 - finalizace vyctoveho typu enum, struktura token
          a funkce pro praci s nimi.
        - xbalaka00 - zmeneno poradi tokenu tak, aby tokeny relevantni pro
          LL parser byly na zacatku kvuli indexaci LL tabulky.
        - xbalaka00 - pridani TOKEN_LL_TERM_COUNT pro zjisteni poctu
          terminalu kvuli LL tabulce.
        - xbalaka00 - sjednoceni komentaru a doplneni doxygen dokumentace
          za asistence Claude.
        - xbalaka00 - nove rozdeleni tokenu podle toho, kterou tabulku
          indexuji: INDENT, hranate zavorky a TOKEN_DOT uz nejsou
          indexatory LL tabulky (TOKEN_STAR zatim docasne zustava mezi nimi
          kvuli testovacim pravidlum), pridany TOKEN_ID_OPERATOR, TOKEN_DOLLAR,
          oddelovac TOKEN_EXPR_TERM_DELIMITER a koncova hodnota TOKEN_COUNT,
          pridana funkce token_type_validate (prompt je v nonterminal.h).
        - xbalaka00 - text vlastni i TOKEN_ID_OPERATOR, ktery vznika prebarvenim
          TOKEN_ID (prompt je v precedence_helpers.h).
        - xbalaka00 - pridany tokeny TOKEN_PLUS_UNARY a TOKEN_MINUS_UNARY mezi
          unarni operatory, TOKEN_PLUS a TOKEN_MINUS presunuty mezi binarni
          terminaly vyrazove analyzy (prompt je v token.h).
        - xbalaka00 - TOKEN_STAR presunut z indexatoru LL tabulky mezi ostatni
          terminaly vyrazove analyzy (pred TOKEN_SLASH), pridan terminal
          TOKEN_TIF (ternarni if ve vyrazu) za TOKEN_DOLLAR (prompt je zde).

    Definice typu tokenu, struktury tokenu a prototypu funkci pro lexer.
    Pouzivano i jako terminal parseru.

*/

#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>

/*
    Prompt pro pridani tokenu TOKEN_PLUS_UNARY a TOKEN_MINUS_UNARY. Stejny
    prompt zpusobil zmeny take v token.c, reduction_table.c,
    precedence_semantic_actions.c, precedence_semantic_actions.h,
    precedence_table.c, precedence_table.h, precedence_helpers.c
    a precedence_helpers.h, jeho zneni je pouze zde.

    Prompt pro Claude:
        Dělám parser založený na hybridu prediktivní a precedenční analýzy. Přikládám relevantní soubory. Pokud by ti některý soubor chyběl, vyžádej si jej, než začneš pracovat. Kontext návrhu a význam kódu si přečti v komentářích a deklarovaných promptech. Dodržuj jmenné konvence (včetně komentářů v češtině bez diakritiky - používej české názvy všech věcí i přesto, že v promptech někdy věci pojmenuji anglicky; prompty ale cituj doslovně po vzoru souborů), formát hlaviček souborů s popisem souboru, dle vzoru přiložených souborů také deklaruj prompty a aktualizuj změnové seznamy. Kód můžeš zkompilovat, ale podle mě není nutné ho testovat na vlastní implementaci.

        Mohl by jsi prosím provést následující změny:

        * do token.* přidej nové tokeny TOKEN_PLUS_UNARY a TOKEN_MINUS_UNARY - zařaď je mezi unární operátory a stávající TOKEN_PLUS dej mezi ty binární. Přidej i case do výpisové funkce atd.
        * uprav sémantické akce pravidel E7 a E8, aby místo stávajícího plus a mínus používaly tyto nové unární verze.
        * stejně tak uprav i SearchHandle v redukční tabulce pro tato pravidla.
        * Uprav metodu parser_evaluate_predictor tak, aby po vzoru ID_OPERATOR provedla toto:
           * pokud je očekáván operand a přijde + nebo - ze scanneru, je přebarveno na svoji unární variantu
           * po + a - v obou variantách je očekáván operand.
        * Uprav také indexační funkci a konstanty velikostí v precedenční tabulce tak, aby tyto nové tokeny podporovaly. Tabulku zatím nevyplňuj.
*/

/*
    Prompt pro presun TOKEN_STAR z indexatoru LL tabulky mezi indexatory
    precedencni tabulky, odstraneni zastupce sloupce *dalsi z LL tabulky,
    zmenu TIF z neterminalu na terminal TOKEN_TIF a zmenu formatu cisel pravidel
    v LL tabulce na L<n>. Stejny prompt zpusobil zmeny take v token.c,
    nonterminal.h, nonterminal.c, LL_table.h, LL_table.c, precedence_table.h
    a precedence_table.c, jeho zneni je pouze zde.

    Prompt pro Claude:
        Dělám parser založený na hybridu prediktivní a precedenční analýzy. Přikládám relevantní soubory. Pokud by ti některý soubor chyběl, vyžádej si jej, než začneš pracovat. Kontext návrhu a význam kódu si přečti v komentářích a deklarovaných promptech. Dodržuj jmenné konvence (včetně komentářů v češtině bez diakritiky - používej české názvy všech věcí i přesto, že v promptech někdy věci pojmenuji anglicky; prompty ale cituj doslovně po vzoru souborů), formát hlaviček souborů s popisem souboru, dle vzoru přiložených souborů také deklaruj prompty a aktualizuj změnové seznamy. Kód můžeš zkompilovat, ale podle mě není nutné ho testovat na vlastní implementaci.

        Potřebuji, aby jsi z části poklidil po testovacích pravidlech (v LL tabulce jsou záznamy pro hvězdičku jakožto zástupce za sloupec *další, který už jsou v jádře udělány finálním způsobem takže je bezpečné aby se smazaly. Přebytečná pravidla odstraním potom.

        A z části je potřeba změna ovlivňující návrh parseru, protože původně jsem ternární if ještě neměl plně domyšlený, ale teď už vím, že TIF musí být terminál a ne neterminál.

        Potřebuji tedy, aby jsi

        * hvězdičku přesunul z indexátorů LL tabulky mezi běžné indexátory precedenční tabulky
        * změnil TIF z neterminálu na terminál a zavedl jej jako nový indexátor precedenční tabulky. Zatím jeho sloupec a řádek vyplň errorovými záznamy. Měl by přijít na okraj tabulky, tedy jeho nové záznamy jsou za $ v řádku i v sloupci.
        * provedl změny, které s tímto souvisí, tedy upravil funkce pro výpis (ne)terminálů, upravil LL tabulku, zohlednil to v indexačních převodech precedenční tabulky apod.

        Teoreticky tyto změny znamenají, že je potřeba i upravit záznamy v redukční tabulce a v sémantickém callbacku na tom jednom místě kde se TIF vyskytuje, avšak to provedu ručně, takže tyto soubory ti nepřikládám (jsou velké a nejsou podle mě nutně potřeba).

        Jo vlastně když už budeš měnit LL tabulku, mohl bys také aktualizovat komentáře, aby čísla pravidel prediktivního parseru uváděly v novém formátu např. L8 s písmenem L aby to zrcadlilo E8 z precedenčních pravidel.
*/

/**
 * @brief Typ tokenu.
 *
 * Poradi hodnot urcuje, kterou tabulku lze tokenem indexovat:
 * - Hodnoty mensi nez TOKEN_LL_TERM_COUNT indexuji LL tabulku. Jsou mezi nimi
 *   i terminaly vyrazove analyzy, ktere se vyskytuji take v LL gramatice
 *   (ID, (, ), carka a List), ty tedy indexuji i precedencni tabulku.
 * - Hodnoty mezi TOKEN_LL_TERM_COUNT a TOKEN_EXPRESSION_BEGINNING_DELIMITER
 *   jsou dalsi terminaly vyrazove analyzy, kterymi smi zacinat vyraz.
 * - Hodnoty mezi TOKEN_EXPRESSION_BEGINNING_DELIMITER a
 *   TOKEN_EXPR_TERM_DELIMITER jsou dalsi terminaly vyrazove analyzy, kterymi
 *   vyraz zacinat nesmi.
 * - Hodnoty za TOKEN_EXPR_TERM_DELIMITER neindexuji zadnou z tabulek.
 *
 * Vyraz nesmi zacinat tokenem, ktery je v enumu za
 * TOKEN_EXPRESSION_BEGINNING_DELIMITER. Hodnoty TOKEN_ID_OPERATOR,
 * TOKEN_PLUS_UNARY, TOKEN_MINUS_UNARY, TOKEN_DOLLAR a TOKEN_TIF scanner nevraci,
 * vznikaji az ve vyrazovem scanneru (scanner vraci TOKEN_PLUS a TOKEN_MINUS,
 * ktere se na miste operandu prebarvi na unarni varianty).
 * Hodnoty TOKEN_LL_TERM_COUNT, TOKEN_EXPRESSION_BEGINNING_DELIMITER,
 * TOKEN_EXPR_TERM_DELIMITER a TOKEN_COUNT nejsou skutecne tokeny.
 */
typedef enum {
  // Specialni (strukturni)
  TOKEN_EOF,    ///< Konec vstupu
  TOKEN_EOL,    ///< Konec logickeho radku
  TOKEN_DEDENT, ///< Konec bloku (jedna uzavrena uroven)

  // Klicova slova
  TOKEN_KW_DEF,
  TOKEN_KW_DO,
  TOKEN_KW_ELSE,
  TOKEN_KW_IF,
  TOKEN_KW_IMPORT,
  TOKEN_KW_RETURN,
  TOKEN_KW_THEN,
  TOKEN_KW_VAL,
  TOKEN_KW_VAR,
  TOKEN_KW_WHILE,
  TOKEN_KW_FOR, ///< CONTROL

  // Nazvy typu (jsou to klicova slova)
  TOKEN_KW_INT,
  TOKEN_KW_DOUBLE,
  TOKEN_KW_STRING,
  TOKEN_KW_UNIT,
  TOKEN_KW_BOOLEAN, ///< BOOLTHEN
  TOKEN_KW_LIST,    ///< LISTS

  // Smisene
  TOKEN_LARROW,    ///< <- (CONTROL: generator ve for)
  TOKEN_LPAREN,    ///< (
  TOKEN_RPAREN,    ///< )
  TOKEN_COMMA,     ///< ,
  TOKEN_COLON,     ///< :
  TOKEN_SEMICOLON, ///< ;

  // Identifikator a prirazeni
  TOKEN_ID,
  TOKEN_ASSIGN, ///< =

  // Main anotace
  TOKEN_ANNOT_MAIN, ///< Cele "@main"

  // Zde konci indexatory LL tabulky.

  TOKEN_LL_TERM_COUNT, ///< Pocet indexatoru LL tabulky (neni skutecny token)

  // Literaly (nesou hodnotu)
  TOKEN_INT_LIT,
  TOKEN_DOUBLE_LIT,
  TOKEN_STRING_LIT, ///< Jednoradkovy i viceradkovy """..."""

  // Pravdivostni hodnoty
  TOKEN_KW_TRUE,  ///< BOOLTHEN, jsou klicova slova i literaly naraz...
  TOKEN_KW_FALSE, ///< BOOLTHEN

  // Unarni operatory
  TOKEN_PLUS_UNARY,  ///< + (unarni, scanner nevraci, vznika prebarvenim)
  TOKEN_MINUS_UNARY, ///< - (unarni, scanner nevraci, vznika prebarvenim)
  TOKEN_NOT,         ///< ! (BOOLTHEN)

  // Od tohoto bodu dal jsou terminaly, kterymi nesmi zacinat vyraz - lze je
  // rovnou detekovat jako chybu, pokud by se preslo do vyrazoveho parseru
  // na zaklade pristiho tokenu.

  // Binarni operatory
  TOKEN_PLUS,  ///< + (binarni, scanner vraci i pro unarni)
  TOKEN_MINUS, ///< - (binarni, scanner vraci i pro unarni)

  /// Vyraz nesmi zacinat terminalem s hodnotou vetsi nez tento oddelovac
  TOKEN_EXPRESSION_BEGINNING_DELIMITER,

  // Ostatni terminaly vyrazove analyzy, indexuji precedencni tabulku
  TOKEN_STAR,     ///< * (take v "import ifj26.*", mimo vyraz neni operator)
  TOKEN_SLASH,    ///< /
  TOKEN_DOT,      ///< . (VALMEM, LISTS, import)
  TOKEN_PLUSPLUS, ///< ++ (LISTS: spojeni seznamu)
  TOKEN_LT,       ///< <
  TOKEN_LE,       ///< <=
  TOKEN_GT,       ///< >
  TOKEN_GE,       ///< >=
  TOKEN_EQ,       ///< ==
  TOKEN_NE,       ///< !=
  TOKEN_AND,      ///< && (BOOLTHEN)
  TOKEN_OR,       ///< || (BOOLTHEN)

  // Specialni terminaly vyrazove analyzy, scanner je nevraci
  TOKEN_ID_OPERATOR, ///< ID prebarvene na operator (nese text jako ID)
  TOKEN_DOLLAR,      ///< $ (konec vyrazu)
  TOKEN_TIF,         ///< Ternarni if ve vyrazu (TIF)

  // Zde konci indexatory precedencni tabulky.

  /// Hodnoty za timto oddelovacem neindexuji zadnou z tabulek (neni to pocet,
  /// precedencni tabulku indexuji i nektere terminaly pred
  /// TOKEN_LL_TERM_COUNT)
  TOKEN_EXPR_TERM_DELIMITER,

  // Terminaly, ktere neindexuji LL ani precedencni tabulku
  TOKEN_INDENT,   ///< Zacatek bloku (odsazeni vic)
  TOKEN_LBRACKET, ///< [ (LISTS: List[Int])
  TOKEN_RBRACKET, ///< ]

  TOKEN_COUNT ///< Pocet hodnot enumu (neni skutecny token)
} TokenType;

/**
 * @brief Hodnota tokenu, platny clen urcuje typ tokenu.
 */
typedef union {
  long long int_val; ///< TOKEN_INT_LIT
  double double_val; ///< TOKEN_DOUBLE_LIT
  char *text;        ///< TOKEN_ID, TOKEN_ID_OPERATOR, TOKEN_STRING_LIT
} TokenAttribute;

/**
 * @brief Token lexikalni analyzy.
 */
typedef struct {
  TokenType type;
  int line; ///< Cislo radku, kde token zacina
  /// Pred tokenem byl LF uvnitr zavorek (kontrola koncove carky)
  bool nl_before;
  TokenAttribute value;
} Token;

/**
 * @brief Vrati citelny nazev typu tokenu.
 *
 * Pouziva se pro ladici vypisy a chybova hlaseni.
 *
 * @param type Typ tokenu.
 * @return Retezec s nazvem typu, pro tokeny bez nazvu "?".
 */
const char *token_type_name(TokenType type);

/**
 * @brief Overi, ze typ tokenu je skutecny token.
 *
 * Pri neplatne hodnote ukonci program s kodem ERR_INTERNAL. Neplatne jsou
 * specialni hodnoty (TOKEN_LL_TERM_COUNT, TOKEN_EXPRESSION_BEGINNING_DELIMITER,
 * TOKEN_EXPR_TERM_DELIMITER, TOKEN_COUNT) a hodnoty mimo enum. Funkce tokeny
 * dale neklasifikuje, tedy nezjistuje, ktere tabulce slouzi jako indexator.
 *
 * @param type Overovany typ tokenu.
 * @param line Cislo radku pro chybove hlaseni, <= 0 pokud neni znam.
 */
void token_type_validate(TokenType type, int line);

/**
 * @brief Uvolni pamet, kterou token vlastni, a token zneplatni.
 *
 * Uvolnuje se jen text u TOKEN_ID, TOKEN_ID_OPERATOR a TOKEN_STRING_LIT.
 *
 * @param t Token, jehoz pamet se uvolni.
 */
void token_free(Token *t);

#endif // TOKEN_H
