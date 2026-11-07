// xnovacm00
// token.h - definice typu tokenu, struktury tokenu a prototypu funkci pro lexer

#ifndef TOKEN_H
#define TOKEN_H
 
#include <stdbool.h>
 
typedef enum {
    // Specialni (strukturni)
    TOKEN_EOF,    // Konec vstupu
    TOKEN_EOL,    // Konec logickeho radku
    TOKEN_INDENT, // Zacatek bloku (odsazeni vic)
    TOKEN_DEDENT, // Konec bloku (jedna uzavrena uroven)
 
    // Identifikator a literaly (nesou hodnotu)
    TOKEN_ID,
    TOKEN_INT_LIT,
    TOKEN_DOUBLE_LIT,
    TOKEN_STRING_LIT, // Jednoradkovy i viceradkovy """..."""
 
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
    TOKEN_KW_TRUE,  // BOOLTHEN
    TOKEN_KW_FALSE, // BOOLTHEN
 
    // Nazvy typu (jsou to klicova slova)
    TOKEN_KW_INT,
    TOKEN_KW_DOUBLE,
    TOKEN_KW_STRING,
    TOKEN_KW_UNIT,
    TOKEN_KW_BOOLEAN, // BOOLTHEN
    TOKEN_KW_LIST,    // LISTS
 
 
    // Anotace
    TOKEN_ANNOT_MAIN, // Cele "@main"
 
    // Operatory
    TOKEN_PLUS,     // +
    TOKEN_MINUS,    // -
    TOKEN_STAR,     // * (take v "import ifj26.*")
    TOKEN_SLASH,    // /
    TOKEN_PLUSPLUS, // ++ (LISTS: spojeni seznamu)
    TOKEN_LT,       // <
    TOKEN_LE,       // <=
    TOKEN_GT,       // >
    TOKEN_GE,       // >=
    TOKEN_EQ,       // ==
    TOKEN_NE,       // !=
    TOKEN_NOT,      // ! (BOOLTHEN)
    TOKEN_AND,      // && (BOOLTHEN)
    TOKEN_OR,       // || (BOOLTHEN)
    TOKEN_ASSIGN,   // =
    TOKEN_LARROW,   // <- (CONTROL: generator ve for)
 
    // Oddelovace
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACKET,  // [ (LISTS: List[Int])
    TOKEN_RBRACKET,  // ]
    TOKEN_COMMA,     // ,
    TOKEN_COLON,     // :
    TOKEN_SEMICOLON, // ;
    TOKEN_DOT        // . (VALMEM, LISTS, import)
} TokenType;

typedef struct {
    TokenType type;
    int line;       // Cislo radku, kde token zacina
    bool nl_before; // Pred tokenem byl LF uvnitr zavorek (kontrola koncove carky)
    union {
        long long int_val; // TOKEN_INT_LIT
        double double_val; // TOKEN_DOUBLE_LIT
        char *text;        // TOKEN_ID, TOKEN_STRING_LIT
    } value;
} Token;

// Vrati citelny nazev typu tokenu (pro ladici vypisy a chybova hlaseni)
const char *token_type_name(TokenType type);

// Uvolni pamet, kterou token vlastni (jenom text u TOKEN_ID a TOKEN_STRING_LIT)
void token_free(Token *t);

#endif