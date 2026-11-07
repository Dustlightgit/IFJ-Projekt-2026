// xnovacm00
// ast.h

#ifndef AST_H
#define AST_H

#include <stdbool.h>


//--------------------------------------------------------------------------------//


// Block ukazuje na Stmt
typedef struct Stmt Stmt;
typedef struct Block Block;


//--------------------------------------------------------------------------------//


typedef enum {
    LIT_INT,    // Cele cislo, napr. 5
    LIT_DOUBLE, // Desetinne cislo, napr. 3.14
    LIT_STRING, // Retezec, napr. "ahoj"
    LIT_UNIT,   // Hodnota (), nema zadna data
    LIT_BOOL    // Pravdivostni hodnota true/false (rozsireni BOOLTHEN)
} LiteralKind;


typedef struct {
    LiteralKind kind; // Urcuje, kterou polozku z value cist
    union {
        int    int_value;    // Pro LIT_INT
        double double_value; // Pro LIT_DOUBLE
        char  *string_value; // Pro LIT_STRING, vlastni kopie retezce
        bool   bool_value;   // Pro LIT_BOOL
    } value;
} Literal;


//--------------------------------------------------------------------------------//


// Druh typu
typedef enum {
    TYPE_NONE,    // Typ neni zadany nebo zatim neni znamy
    TYPE_INT,     // Int
    TYPE_DOUBLE,  // Double
    TYPE_STRING,  // String
    TYPE_UNIT,    // Unit
    TYPE_BOOLEAN, // Boolean (rozsireni BOOLTHEN)
    TYPE_LIST     // List[T] (rozsireni LISTS)
} TypeKind;

// Typ: druh a u seznamu ukazatel na typ prvku
typedef struct Type {
    TypeKind kind;       // Druh typu
    struct Type *element; // Typ prvku, pouze pro TYPE_LIST, jinak NULL
} Type;


//--------------------------------------------------------------------------------//


typedef enum {
    UNOP_PLUS,  // Unarni plus, napr. +x
    UNOP_NEG,   // Unarni minus, napr. -x
    UNOP_NOT    // Negace, napr. !x (rozsireni BOOLTHEN)
} UnaryOp;

// Binarni operatory
typedef enum {
    BINOP_ADD, // Scitani a konkatenace retezcu, +
    BINOP_SUB, // Odecitani, -
    BINOP_MUL, // Nasobeni, *
    BINOP_DIV, // Deleni, /
    BINOP_LT,  // Mensi, <
    BINOP_LE,  // Mensi nebo rovno, <=
    BINOP_GT,  // Vetsi, >
    BINOP_GE,  // Vetsi nebo rovno, >=
    BINOP_EQ,  // Rovno, ==
    BINOP_NE,  // Nerovno, !=
    BINOP_AND, // Logicky soucin, && (rozsireni BOOLTHEN)
    BINOP_OR,  // Logicky soucet, || (rozsireni BOOLTHEN)
    BINOP_CONCAT_LIST // Spojeni seznamu, ++ (rozsireni LISTS)
} BinaryOp;


//--------------------------------------------------------------------------------//

// Druh vyrazu
typedef enum {
    EXPR_LITERAL,  // Literal, napr. 5
    EXPR_VARIABLE, // Pouziti promenne, napr. x
    EXPR_UNARY,    // Unarni operator, napr. -x
    EXPR_BINARY,   // Binarni operator, napr. 2 + 3
    EXPR_CALL,      // Volani funkce, napr. print(x)
    EXPR_IF,        // Podminka if, napr. if x < 0 then 0 - x else x
    EXPR_RANGE,     // Rozsah, napr. 1 to 10, 0 until n, 1 to 9 by 2
    EXPR_MEMBER,        // Clen hodnoty, napr. x.toInt nebo lst.drop(2)
    EXPR_LIST_LITERAL,  // Seznam, napr. List(1, 2, 3)
    EXPR_LIST_RANGE,    // Cisla v seznamu, napr. List.range(1, 10)
    EXPR_INDEX,         // Pristup k prvku seznamu, napr. lst(2)
} ExprKind;



// Vyraz = pro vsechny druhy vyrazu
typedef struct Expr {
    ExprKind kind; // Urcuje, kterou polozku z as cist
    int line;      // Radek ve zdrojovem kodu pro chybova hlaseni
    Type *type;    // Typ vyrazu, vyplni semanticka analyza (parser nastavi NULL)
    union {
        Literal literal;                 // Pro EXPR_LITERAL
        struct { char *name; } variable; // Pro EXPR_VARIABLE, vlastni kopie jmena
        struct {
            UnaryOp op;           // Ktery operator
            struct Expr *operand; // Vyraz, na ktery se operator pouzije
        } unary;                  // Pro EXPR_UNARY
        struct {
            BinaryOp op;         // Ktery operator
            struct Expr *left;   // Levy operand
            struct Expr *right;  // Pravy operand
        } binary;                // Pro EXPR_BINARY
        struct {
            char *name;          // Jmeno volane funkce, vlastni kopie
            struct Expr **args;  // Pole ukazatelu na argumenty
            int arg_count;       // Pocet argumentu
        } call;                  // Pro EXPR_CALL
        struct {
            struct Expr *cond;     // Podminka
            Block *then_block;     // Vetev then
            Block *else_block;     // Vetev else, NULL kdyz chybi
        } if_expr;                 // Pro EXPR_IF
        struct {
            struct Expr *from;     // Zacatek rozsahu
            struct Expr *to;       // Konec rozsahu
            bool is_inclusive;     // True pro to, false pro until
            struct Expr *step;     // Krok (by), NULL kdyz chybi
        } range;                   // Pro EXPR_RANGE
        struct {
            struct Expr *object;   // Hodnota, na ktere se clen vola
            char *name;            // Jmeno clenu, napr. toInt, vlastni kopie
            struct Expr **args;    // Argumenty, NULL kdyz zadne nejsou
            int arg_count;         // Pocet argumentu, muze byt 0
        } member;                  // Pro EXPR_MEMBER
        struct {
            struct Expr **items;   // Prvky seznamu, NULL pro prazdny seznam
            int count;             // Pocet prvku, muze byt 0
        } list_literal;            // Pro EXPR_LIST_LITERAL
        struct {
            struct Expr *start;    // Prvni cislo
            struct Expr *end;      // Konec, bez nej
            struct Expr *step;     // Krok, NULL kdyz chybi
        } list_range;              // Pro EXPR_LIST_RANGE
        struct {
            struct Expr *list;     // Seznam, ze ktereho se cte
            struct Expr *index;    // Index prvku
        } index;                   // Pro EXPR_INDEX
    } as;
} Expr;


//--------------------------------------------------------------------------------//


// Blok: posloupnost prikazu
struct Block {
    Stmt **items; // Pole ukazatelu na prikazy
    int count;    // Pocet prikazu v bloku
};


// Druh prikazu
typedef enum {
    STMT_VARDEF, // Definice promenne, napr. val x = 5
    STMT_ASSIGN, // Prirazeni, napr. x = 7
    STMT_RETURN, // Navrat z funkce, napr. return x
    STMT_EXPR,    // Samostatny vyraz, napr. print(x)
    STMT_WHILE,  // Cyklus while, napr. while i > 1 do ...
    STMT_FOR,    // Cyklus for, napr. for i <- 1 to 10 do ...
} StmtKind;


// Prikaz: spolecna obalka pro vsechny druhy prikazu
struct Stmt {
    StmtKind kind; // Urcuje, kterou polozku z as cist
    int line;      // Radek ve zdrojovem kodu pro chybova hlaseni
    union {
        struct {
            bool is_mutable;     // True pro var, false pro val
            char *name;          // Jmeno promenne, vlastni kopie
            Type *declared_type; // Typ napsany programatorem, NULL kdyz chybi
            Expr *init;          // Pocatecni hodnota
        } vardef;                // Pro STMT_VARDEF
        struct {
            char *name;          // Jmeno promenne, vlastni kopie
            Expr *value;         // Nova hodnota
        } assign;                // Pro STMT_ASSIGN
        struct {
            Expr *value;         // Vracena hodnota, NULL pro prazdny return
        } ret;                   // Pro STMT_RETURN
        struct {
            Expr *expr;          // Vyraz, ktery se vyhodnoti a zahodi
        } expr_stmt;             // Pro STMT_EXPR
        struct {
            Expr *cond;          // Podminka cyklu
            Block *body;         // Telo cyklu
        } while_loop;            // Pro STMT_WHILE
        struct {
            char *var_name;      // Jmeno ridici promenne, vlastni kopie
            Expr *source;        // Zdroj: rozsah nebo seznam
            Expr **guards;       // Pole strazi (vyrazy typu Boolean)
            int guard_count;     // Pocet strazi, muze byt 0
            Block *body;         // Telo cyklu
        } for_loop;              // Pro STMT_FOR
    } as;
};


//--------------------------------------------------------------------------------//


// Parametr funkce, napr. n: Int
typedef struct {
    char *name;  // Jmeno parametru, vlastni kopie
    Type *type;  // Typ parametru, vzdy zadany
} Param;

// Definice funkce
typedef struct {
    char *name;         // Jmeno funkce, vlastni kopie
    Param *params;      // Pole parametru
    int param_count;    // Pocet parametru, muze byt 0
    Type *return_type;  // Navratovy typ, NULL kdyz ho programator nenapsal
    Block *body;        // Telo funkce
    bool is_main;       // True pro funkci s anotaci @main
    int line;           // Radek ve zdrojovem kodu pro chybova hlaseni
} FuncDef;


// Cely program: seznam definic funkci
typedef struct {
    FuncDef **funcs;    // Pole ukazatelu na funkce
    int func_count;     // Pocet funkci
} Program;


//--------------------------------------------------------------------------------//



#endif