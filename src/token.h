/*
    IFJ projekt 2026

    token.h

    Puvodni autor: xsantar00 - prvni verze vyctoveho typu enumu
    Seznam zmen:
        - xnovacm00 za asistence xyz - finalizace vyctoveho typu enum, struktura token a funkce pro praci s nimi.
        - xbalala00 - zmeneno poradi takenu tak, aby tokeny relevantni pro LL parser byly na zacatku kvuli indexaci LL tabulky.
                    - pridani TOKEN_LL_TERM_COUNT pro zjisteni poctu terminalu kvuli LL tabulce.

    Definice typu tokenu, struktury tokenu a prototypu funkci pro lexer.

*/


#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>

typedef enum {
  // Specialni (strukturni)
  TOKEN_EOF,    // Konec vstupu
  TOKEN_EOL,    // Konec logickeho radku
  TOKEN_INDENT, // Zacatek bloku (odsazeni vic)
  TOKEN_DEDENT, // Konec bloku (jedna uzavrena uroven)

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
  TOKEN_KW_FOR,   // CONTROL

  // Nazvy typu (jsou to klicova slova)
  TOKEN_KW_INT,
  TOKEN_KW_DOUBLE,
  TOKEN_KW_STRING,
  TOKEN_KW_UNIT,
  TOKEN_KW_BOOLEAN, // BOOLTHEN
  TOKEN_KW_LIST,    // LISTS

  // Smisene
  TOKEN_LARROW,    // <- (CONTROL: generator ve for)
  TOKEN_LPAREN,    // (
  TOKEN_RPAREN,    // )
  TOKEN_LBRACKET,  // [ (LISTS: List[Int])
  TOKEN_RBRACKET,  // ]
  TOKEN_COMMA,     // ,
  TOKEN_COLON,     // :
  TOKEN_SEMICOLON, // ;

  // Identifikator a prirazeni
  TOKEN_ID,
  TOKEN_ASSIGN,   // =
  
  // Import a main anotace
  TOKEN_ANNOT_MAIN,   // Cele "@main"
  TOKEN_STAR,         // * (v "import ifj26.*", take je operator)
  TOKEN_DOT,          // . (VALMEM, LISTS, import)

  // Zde konci terminaly LL gramatiky.

  // Pocet terminalu LL gramatiky.
  TOKEN_LL_TERM_COUNT,    // Neni skutecny token

  // Literaly (nesou hodnotu)
  TOKEN_INT_LIT,
  TOKEN_DOUBLE_LIT,
  TOKEN_STRING_LIT, // Jednoradkovy i viceradkovy """..."""
  
  // Pravdivostni hodnoty
  TOKEN_KW_TRUE,  // BOOLTHEN, jsou klicova slova i literaly naraz...
  TOKEN_KW_FALSE, // BOOLTHEN

  // Unarni operatory
  TOKEN_PLUS,     // +
  TOKEN_MINUS,    // -
  TOKEN_NOT,      // ! (BOOLTHEN)

  // Od tohoho bodu dal jsou terminaly, ktere nejsou soucasti LL tabulky
  // a kterymi nesmi zacinat vyraz - lze je rovnou detekovat jako chybu,
  // pokud by se preslo do vyrazoveho parseru na zaklade pristiho tokenu.
  
  // Vyraz nesmi zacinat terminalem s hodnotou vetsi nez tento oddelovac
  TOKEN_EXPRESSION_BEGINNING_DELIMITER,

  // Binarni operatory
  TOKEN_SLASH,    // /
  TOKEN_PLUSPLUS, // ++ (LISTS: spojeni seznamu)
  TOKEN_LT,       // <
  TOKEN_LE,       // <=
  TOKEN_GT,       // >
  TOKEN_GE,       // >=
  TOKEN_EQ,       // ==
  TOKEN_NE,       // !=
  TOKEN_AND,      // && (BOOLTHEN)
  TOKEN_OR,       // || (BOOLTHEN)
  
} TokenType;

typedef union {
    long long int_val; // TOKEN_INT_LIT
    double double_val; // TOKEN_DOUBLE_LIT
    char *text;        // TOKEN_ID, TOKEN_STRING_LIT
  } TokenAttribute; 

typedef struct {
  TokenType type;
  int line;       // Cislo radku, kde token zacina
  bool nl_before; // Pred tokenem byl LF uvnitr zavorek (kontrola koncove carky)
  TokenAttribute value;
} Token;

// Vrati citelny nazev typu tokenu (pro ladici vypisy a chybova hlaseni)
const char *token_type_name(TokenType type);

// Uvolni pamet, kterou token vlastni (jenom text u TOKEN_ID a TOKEN_STRING_LIT)
void token_free(Token *t);

#endif