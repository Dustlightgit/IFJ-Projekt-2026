//xsantar00


// Typy tokenů rozpoznávané lexikálním analyzátorem
typedef enum {
    // Speciální a řídicí tokeny
    TOKEN_EOF,              // Konec souboru
    TOKEN_EOL,              // Konec logického řádku
    TOKEN_INDENT,           // Zvýšení úrovně odsazení (otevření bloku)
    TOKEN_DEDENT,           // Snížení úrovně odsazení (uzavření bloku)

    // Identifikátory a literály
    TOKEN_ID,               // Název proměnné, funkce
    TOKEN_INT_LITERAL,      // Celočíselný literál (např. 42, 1_000)
    TOKEN_DOUBLE_LITERAL,   // Desetinný literál (např. 3.14, 1e-10)
    TOKEN_STRING_LITERAL,   // Řetězec ("...")
    TOKEN_MULTILINE_STRING, // Víceřádkový řetězec ("""...""")

    // Klíčová slova základního zadání
    TOKEN_DEF,           // def
    TOKEN_DO,            // do
    TOKEN_ELSE,          // else
    TOKEN_IF,            // if
    TOKEN_IMPORT,        // import
    TOKEN_RETURN,        // return
    TOKEN_THEN,          // then
    TOKEN_VAL,           // val
    TOKEN_VAR,           // var
    TOKEN_WHILE,         // while
    TOKEN_INT,           // Int
    TOKEN_DOUBLE,        // Double
    TOKEN_STRING,        // String
    TOKEN_UNIT,          // Unit

    // Klíčová slova rozšíření
    // TOKEN_BOOLEAN,       // Boolean
    // TOKEN_CASE,          // case
    // TOKEN_CLASS,         // class
    // TOKEN_EXTENSION,     // extension
    // TOKEN_FOR,           // for
    // TOKEN_LIST,          // List
    // TOKEN_MATCH,         // match
    // TOKEN_TYPE,          // type

    // Operátory
    TOKEN_PLUS,             // +
    TOKEN_MINUS,            // -
    TOKEN_MUL,              // *
    TOKEN_DIV,              // /
    TOKEN_ASSIGN,           // =
    TOKEN_EQ,               // ==
    TOKEN_NEQ,              // !=
    TOKEN_LT,               // <
    TOKEN_LTE,              // <=
    TOKEN_GT,               // >
    TOKEN_GTE,              // >=
    TOKEN_AND,              // &&
    TOKEN_OR,               // ||
    TOKEN_NOT,              // !

    // Separátory a závorky
    TOKEN_LPAREN,           // (
    TOKEN_RPAREN,           // )
    TOKEN_COLON,            // :
    TOKEN_COMMA,            // ,
    TOKEN_SEMICOLON,        // ;
    TOKEN_DOT,              // .
    TOKEN_AT_MAIN,          // @main
    TOKEN_ARROW             // =>
} TokenType;