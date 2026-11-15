/*
    IFJ projekt 2026

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

    Definice typu tokenu, struktury tokenu a prototypu funkci pro lexer.

*/

#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>

/**
 * @brief Typ tokenu.
 *
 * Poradi hodnot urcuje, kterou tabulku lze tokenem indexovat:
 * - Hodnoty mensi nez TOKEN_LL_TERM_COUNT indexuji LL tabulku. Jsou mezi nimi
 *   i terminaly vyrazove analyzy, ktere se vyskytuji take v LL gramatice
 *   (ID, (, ), carka a List), ty tedy indexuji i precedencni tabulku.
 *   Docasne je mezi nimi i TOKEN_STAR (testovaci pravidla), ktery v budoucnu
 *   LL tabulku indexovat prestane a bude mezi ostatnimi terminaly vyrazove
 *   analyzy.
 * - Hodnoty mezi TOKEN_LL_TERM_COUNT a TOKEN_EXPRESSION_BEGINNING_DELIMITER
 *   jsou dalsi terminaly vyrazove analyzy, kterymi smi zacinat vyraz.
 * - Hodnoty mezi TOKEN_EXPRESSION_BEGINNING_DELIMITER a
 *   TOKEN_EXPR_TERM_DELIMITER jsou dalsi terminaly vyrazove analyzy, kterymi
 *   vyraz zacinat nesmi.
 * - Hodnoty za TOKEN_EXPR_TERM_DELIMITER neindexuji zadnou z tabulek.
 *
 * Vyraz nesmi zacinat tokenem, ktery je v enumu za
 * TOKEN_EXPRESSION_BEGINNING_DELIMITER. Hodnoty TOKEN_ID_OPERATOR a
 * TOKEN_DOLLAR scanner nevraci, vznikaji az ve vyrazovem scanneru.
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

  // DOCASNE: TOKEN_STAR je zde jen kvuli testovacim pravidlum v LL tabulce
  // (zastupce sloupce *dalsi). V budoucnu se presune mezi ostatni terminaly
  // vyrazove analyzy (pred TOKEN_SLASH) a z LL tabulky se odstrani.
  TOKEN_STAR, ///< * (take v "import ifj26.*", mimo vyraz neni operator)

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
  TOKEN_PLUS,  ///< +
  TOKEN_MINUS, ///< -
  TOKEN_NOT,   ///< ! (BOOLTHEN)

  // Od tohoto bodu dal jsou terminaly, kterymi nesmi zacinat vyraz - lze je
  // rovnou detekovat jako chybu, pokud by se preslo do vyrazoveho parseru
  // na zaklade pristiho tokenu.

  /// Vyraz nesmi zacinat terminalem s hodnotou vetsi nez tento oddelovac
  TOKEN_EXPRESSION_BEGINNING_DELIMITER,

  // Ostatni terminaly vyrazove analyzy, indexuji precedencni tabulku
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
  TOKEN_ID_OPERATOR, ///< ID prebarvene na operator (infixova metoda)
  TOKEN_DOLLAR,      ///< $ (konec vyrazu)

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
  char *text;        ///< TOKEN_ID, TOKEN_STRING_LIT
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
 * Uvolnuje se jen text u TOKEN_ID a TOKEN_STRING_LIT.
 *
 * @param t Token, jehoz pamet se uvolni.
 */
void token_free(Token *t);

#endif // TOKEN_H
