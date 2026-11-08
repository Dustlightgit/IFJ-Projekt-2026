// xnovacm00
// ast.h - hlavickovy soubor pro abstraktni syntakticky strom

#ifndef AST_H
#define AST_H

#include <stdbool.h>


//--------------------------------------------------------------------------------//


// Predbezne deklarace
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
    //TYPE_NONE,    // Typ neni zadany nebo zatim neni znamy
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
        struct {
            char *name;            // Jmeno promenne, vlastni kopie
            struct Symbol *symbol; // Symbol z tabulky symbolu, vyplni semanticka analyza
        } variable; // Pro EXPR_VARIABLE, vlastni kopie jmena
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
            struct Symbol *symbol; // Symbol z tabulky symbolu (nevlastni), vyplni semantika
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
            struct Symbol *symbol; // Symbol z tabulky symbolu (nevlastni), vyplni semantika
        } vardef;                // Pro STMT_VARDEF
        struct {
            char *name;          // Jmeno promenne, vlastni kopie
            Expr *value;         // Nova hodnota
            struct Symbol *symbol; // Symbol z tabulky symbolu (nevlastni), vyplni semantika
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
            struct Symbol *symbol; // Symbol z tabulky symbolu (nevlastni), vyplni semantika
        } for_loop;              // Pro STMT_FOR
    } as;
};


//--------------------------------------------------------------------------------//


// Parametr funkce, napr. n: Int
typedef struct {
    char *name;  // Jmeno parametru, vlastni kopie
    Type *type;  // Typ parametru, vzdy zadany
    struct Symbol *symbol; // Symbol z tabulky symbolu (nevlastni), vyplni semantika
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
    struct Symbol *symbol; // Symbol z tabulky symbolu (nevlastni), vyplni semantika
} FuncDef;


// Cely program: seznam definic funkci
typedef struct {
    FuncDef **funcs;    // Pole ukazatelu na funkce
    int func_count;     // Pocet funkci
} Program;


//--------------------------------------------------------------------------------//

// Funkce pro vytvareni a uvolnovani uzlu

//--------------------------------------------------------------------------------//



//--------------------------------------------------------------------------------//
// Vygenerovano AI (zacatek)
// prompt: "Udelej konstruktory a uvolnovaci funkce k ast.h"


// Pravidlo 1: Rodicovsky uzel vlastni sve potomky, uvolneni korene uvolni cely strom.
// Pravidlo 2: Konstruktory prebiraji vlastnictvi ukazatelu na uzly, bloky a typy.
// Pravidlo 3: Predany ukazatel uz volajici nesmi uvolnit ani pouzit na jinem miste.
// Pravidlo 4: Retezce (jmena, literaly) se kopiruji, svuj puvodni retezec uvolnuje volajici.
// Pravidlo 5: Pri nedostatku pameti program konci s navratovym kodem 99.
// Pravidlo 6: Vsechny funkce *_free akceptuji NULL, parser jimi muze pri chybe uvolnit rozpracovane uzly.
// Pravidlo 7: Ukazatele na Symbol AST nevlastni a funkce *_free je neuvolnuji.


// Typy
 
// Vytvori typ daneho druhu (pro TYPE_LIST pouzij type_new_list)
Type *type_new(TypeKind kind);
 
// Vytvori typ List[element], prebere vlastnictvi elementu
Type *type_new_list(Type *element);
 
// Vytvori hlubokou kopii typu (pro semantiku, aby kazdy vyraz mel vlastni typ)
// Resi problem s dvojitym uvolnenim
// (kazdy vyraz uvolnuje svuj typ)
// napr Expr "a" => TYPE_INT (1)
//      Expr "a + b" => TYPE_INT (2)
// neukazuji na stejny TYPE_INT ale kazdy ma svuj
Type *type_clone(const Type *type);
 
// Uvolni typ vcetne typu prvku
void type_free(Type *type);


// Vyrazy
 
Expr *expr_new_int(int value, int line);
Expr *expr_new_double(double value, int line);
Expr *expr_new_string(const char *value, int line);
Expr *expr_new_unit(int line);
Expr *expr_new_bool(bool value, int line);
Expr *expr_new_variable(const char *name, int line);
Expr *expr_new_unary(UnaryOp op, Expr *operand, int line);
Expr *expr_new_binary(BinaryOp op, Expr *left, Expr *right, int line);

// Vytvori volani funkce bez argumentu, argumenty se pridavaji funkci expr_call_add_arg
Expr *expr_new_call(const char *name, int line);
void expr_call_add_arg(Expr *call, Expr *arg);


Expr *expr_new_if(Expr *cond, Block *then_block, Block *else_block, int line);
Expr *expr_new_range(Expr *from, Expr *to, bool is_inclusive, Expr *step, int line);


// Vytvori clen hodnoty bez argumentu, argumenty se pridavaji funkci expr_member_add_arg
Expr *expr_new_member(Expr *object, const char *name, int line);
void expr_member_add_arg(Expr *member, Expr *arg);


// Vytvori prazdny seznam, prvky se pridavaji funkci expr_list_add_item
Expr *expr_new_list_literal(int line);
void expr_list_add_item(Expr *list, Expr *item);
 
Expr *expr_new_list_range(Expr *start, Expr *end, Expr *step, int line);
Expr *expr_new_index(Expr *list, Expr *index, int line);



// Uvolni vyraz a cely jeho podstrom
void expr_free(Expr *expr);



// Bloky a prikazy
 
Block *block_new(void);
 
// Prida prikaz na konec bloku, prebere vlastnictvi prikazu
void block_add(Block *block, Stmt *stmt);

// Uvolni blok a vsechny jeho prikazy
void block_free(Block *block);


Stmt *stmt_new_vardef(bool is_mutable, const char *name, Type *declared_type, Expr *init, int line);
Stmt *stmt_new_assign(const char *name, Expr *value, int line);
Stmt *stmt_new_return(Expr *value, int line);
Stmt *stmt_new_expr(Expr *expr, int line);
Stmt *stmt_new_while(Expr *cond, Block *body, int line);


// Vytvori cyklus for bez strazi a bez tela, doplni se funkcemi nize
Stmt *stmt_new_for(const char *var_name, Expr *source, int line);
void stmt_for_add_guard(Stmt *for_stmt, Expr *guard);
void stmt_for_set_body(Stmt *for_stmt, Block *body);


// Uvolni prikaz a cely jeho podstrom
void stmt_free(Stmt *stmt);


// Funkce a program
 
// Vytvori funkci bez parametru, bez navratoveho typu a bez tela
FuncDef *funcdef_new(const char *name, bool is_main, int line);
void funcdef_add_param(FuncDef *func, const char *name, Type *type);
void funcdef_set_return_type(FuncDef *func, Type *return_type);
void funcdef_set_body(FuncDef *func, Block *body);
void funcdef_free(FuncDef *func);



Program *program_new(void);
void program_add_func(Program *program, FuncDef *func);
 
// Uvolni program a vsechny jeho funkce
void program_free(Program *program);

// Vygenerovano AI (konec)
//--------------------------------------------------------------------------------//

#endif