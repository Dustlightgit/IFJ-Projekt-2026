/*
    IFJ projekt 2026

    xnovacm00 Michal Novacek

    ast.h

    Tento soubor obsahuje datove struktury abstraktniho syntaktickeho stromu
    a deklarace funkci pro jeho vytvareni a uvolnovani.
*/

#ifndef AST_H
#define AST_H

#include <stdbool.h>

//--------------------------------------------------------------------------------//

// Predbezne deklarace
typedef struct Stmt Stmt;
typedef struct Block Block;

//--------------------------------------------------------------------------------//

/**
 * @brief Druh literalu.
 */
typedef enum {
  LIT_INT,    ///< Cele cislo, napr. 5.
  LIT_DOUBLE, ///< Desetinne cislo, napr. 3.14.
  LIT_STRING, ///< Retezec, napr. "ahoj".
  LIT_UNIT,   ///< Hodnota (), nema zadna data.
  LIT_BOOL    ///< Pravdivostni hodnota true/false (rozsireni BOOLTHEN).
} LiteralKind;

/**
 * @brief Literal: druh a hodnota.
 */
typedef struct {
  LiteralKind kind; ///< Urcuje, kterou polozku z value cist.
  union {
    int int_value;       ///< Pro LIT_INT.
    double double_value; ///< Pro LIT_DOUBLE.
    char *string_value;  ///< Pro LIT_STRING, vlastni kopie retezce.
    bool bool_value;     ///< Pro LIT_BOOL.
  } value;
} Literal;

//--------------------------------------------------------------------------------//

/**
 * @brief Druh typu.
 */
typedef enum {
  //TYPE_NONE,    // Typ neni zadany nebo zatim neni znamy
  TYPE_INT,     ///< Int.
  TYPE_DOUBLE,  ///< Double.
  TYPE_STRING,  ///< String.
  TYPE_UNIT,    ///< Unit.
  TYPE_BOOLEAN, ///< Boolean (rozsireni BOOLTHEN).
  TYPE_LIST     ///< List[T] (rozsireni LISTS).
} TypeKind;

/**
 * @brief Typ: druh a u seznamu ukazatel na typ prvku.
 */
typedef struct Type {
  TypeKind kind;        ///< Druh typu.
  struct Type *element; ///< Typ prvku, pouze pro TYPE_LIST, jinak NULL.
} Type;

//--------------------------------------------------------------------------------//

/**
 * @brief Unarni operatory.
 */
typedef enum {
  UNOP_PLUS, ///< Unarni plus, napr. +x.
  UNOP_NEG,  ///< Unarni minus, napr. -x.
  UNOP_NOT   ///< Negace, napr. !x (rozsireni BOOLTHEN).
} UnaryOp;

/**
 * @brief Binarni operatory.
 */
typedef enum {
  BINOP_ADD,        ///< Scitani a konkatenace retezcu, +.
  BINOP_SUB,        ///< Odecitani, -.
  BINOP_MUL,        ///< Nasobeni, *.
  BINOP_DIV,        ///< Deleni, /.
  BINOP_LT,         ///< Mensi, <.
  BINOP_LE,         ///< Mensi nebo rovno, <=.
  BINOP_GT,         ///< Vetsi, >.
  BINOP_GE,         ///< Vetsi nebo rovno, >=.
  BINOP_EQ,         ///< Rovno, ==.
  BINOP_NE,         ///< Nerovno, !=.
  BINOP_AND,        ///< Logicky soucin, && (rozsireni BOOLTHEN).
  BINOP_OR,         ///< Logicky soucet, || (rozsireni BOOLTHEN).
  BINOP_CONCAT_LIST ///< Spojeni seznamu, ++ (rozsireni LISTS).
} BinaryOp;

//--------------------------------------------------------------------------------//

/**
 * @brief Druh vyrazu.
 */
typedef enum {
  EXPR_LITERAL,      ///< Literal, napr. 5.
  EXPR_VARIABLE,     ///< Pouziti promenne, napr. x.
  EXPR_UNARY,        ///< Unarni operator, napr. -x.
  EXPR_BINARY,       ///< Binarni operator, napr. 2 + 3.
  EXPR_CALL,         ///< Volani funkce, napr. print(x).
  EXPR_IF,           ///< Podminka if, napr. if x < 0 then 0 - x else x.
  EXPR_RANGE,        ///< Rozsah, napr. 1 to 10, 0 until n, 1 to 9 by 2.
  EXPR_MEMBER,       ///< Clen hodnoty, napr. x.toInt nebo lst.drop(2).
  EXPR_LIST_LITERAL, ///< Seznam, napr. List(1, 2, 3).
  EXPR_LIST_RANGE,   ///< Cisla v seznamu, napr. List.range(1, 10).
  EXPR_INDEX,        ///< Pristup k prvku seznamu, napr. lst(2).
} ExprKind;

/**
 * @brief Vyraz: spolecna struktura pro vsechny druhy vyrazu.
 */
typedef struct Expr {
  ExprKind kind; ///< Urcuje, kterou polozku z as cist.
  int line;      ///< Radek ve zdrojovem kodu pro chybova hlaseni.
  Type *type; ///< Typ vyrazu, vyplni semanticka analyza (parser nastavi NULL).
  union {
    Literal literal; ///< Pro EXPR_LITERAL.
    struct {
      char *name; ///< Jmeno promenne, vlastni kopie.
      struct Symbol *
          symbol; ///< Symbol z tabulky symbolu (nevlastni), vyplni semanticka analyza.
    } variable; ///< Pro EXPR_VARIABLE.
    struct {
      UnaryOp op;           ///< Ktery operator.
      struct Expr *operand; ///< Vyraz, na ktery se operator pouzije.
    } unary;                ///< Pro EXPR_UNARY.
    struct {
      BinaryOp op;        ///< Ktery operator.
      struct Expr *left;  ///< Levy operand.
      struct Expr *right; ///< Pravy operand.
    } binary;             ///< Pro EXPR_BINARY.
    struct {
      char *name;         ///< Jmeno volane funkce, vlastni kopie.
      struct Expr **args; ///< Pole ukazatelu na argumenty.
      int arg_count;      ///< Pocet argumentu.
      struct Symbol
          *symbol; ///< Symbol z tabulky symbolu (nevlastni), vyplni semantika.
    } call;        ///< Pro EXPR_CALL.
    struct {
      struct Expr *cond; ///< Podminka.
      Block *then_block; ///< Vetev then.
      Block *else_block; ///< Vetev else, NULL kdyz chybi.
    } if_expr;           ///< Pro EXPR_IF.
    struct {
      struct Expr *from; ///< Zacatek rozsahu.
      struct Expr *to;   ///< Konec rozsahu.
      bool is_inclusive; ///< True pro to, false pro until.
      struct Expr *step; ///< Krok (by), NULL kdyz chybi.
    } range;             ///< Pro EXPR_RANGE.
    struct {
      struct Expr *object; ///< Hodnota, na ktere se clen vola.
      char *name;          ///< Jmeno clenu, napr. toInt, vlastni kopie.
      struct Expr **args;  ///< Argumenty, NULL kdyz zadne nejsou.
      int arg_count;       ///< Pocet argumentu, muze byt 0.
    } member;              ///< Pro EXPR_MEMBER.
    struct {
      struct Expr **items; ///< Prvky seznamu, NULL pro prazdny seznam.
      int count;           ///< Pocet prvku, muze byt 0.
    } list_literal;        ///< Pro EXPR_LIST_LITERAL.
    struct {
      struct Expr *start; ///< Prvni cislo.
      struct Expr *end;   ///< Konec, bez nej.
      struct Expr *step;  ///< Krok, NULL kdyz chybi.
    } list_range;         ///< Pro EXPR_LIST_RANGE.
    struct {
      struct Expr *list;  ///< Seznam, ze ktereho se cte.
      struct Expr *index; ///< Index prvku.
    } index;              ///< Pro EXPR_INDEX.
  } as;
} Expr;

//--------------------------------------------------------------------------------//

/**
 * @brief Blok: posloupnost prikazu.
 */
struct Block {
  Stmt **items; ///< Pole ukazatelu na prikazy.
  int count;    ///< Pocet prikazu v bloku.
};

/**
 * @brief Druh prikazu.
 */
typedef enum {
  STMT_VARDEF, ///< Definice promenne, napr. val x = 5.
  STMT_ASSIGN, ///< Prirazeni, napr. x = 7.
  STMT_RETURN, ///< Navrat z funkce, napr. return x.
  STMT_EXPR,   ///< Samostatny vyraz, napr. print(x).
  STMT_WHILE,  ///< Cyklus while, napr. while i > 1 do ...
  STMT_FOR,    ///< Cyklus for, napr. for i <- 1 to 10 do ...
} StmtKind;

/**
 * @brief Prikaz: spolecna obalka pro vsechny druhy prikazu.
 */
struct Stmt {
  StmtKind kind; ///< Urcuje, kterou polozku z as cist.
  int line;      ///< Radek ve zdrojovem kodu pro chybova hlaseni.
  union {
    struct {
      bool is_mutable;     ///< True pro var, false pro val.
      char *name;          ///< Jmeno promenne, vlastni kopie.
      Type *declared_type; ///< Typ napsany programatorem, NULL kdyz chybi.
      Expr *init;          ///< Pocatecni hodnota.
      struct Symbol
          *symbol; ///< Symbol z tabulky symbolu (nevlastni), vyplni semantika.
    } vardef;      ///< Pro STMT_VARDEF.
    struct {
      char *name;  ///< Jmeno promenne, vlastni kopie.
      Expr *value; ///< Nova hodnota.
      struct Symbol
          *symbol; ///< Symbol z tabulky symbolu (nevlastni), vyplni semantika.
    } assign;      ///< Pro STMT_ASSIGN.
    struct {
      Expr *value; ///< Vracena hodnota, NULL pro prazdny return.
    } ret;         ///< Pro STMT_RETURN.
    struct {
      Expr *expr; ///< Vyraz, ktery se vyhodnoti a zahodi.
    } expr_stmt;  ///< Pro STMT_EXPR.
    struct {
      Expr *cond;  ///< Podminka cyklu.
      Block *body; ///< Telo cyklu.
    } while_loop;  ///< Pro STMT_WHILE.
    struct {
      char *var_name;  ///< Jmeno ridici promenne, vlastni kopie.
      Expr *source;    ///< Zdroj: rozsah nebo seznam.
      Expr **guards;   ///< Pole strazi (vyrazy typu Boolean).
      int guard_count; ///< Pocet strazi, muze byt 0.
      Block *body;     ///< Telo cyklu.
      struct Symbol
          *symbol; ///< Symbol z tabulky symbolu (nevlastni), vyplni semantika.
    } for_loop;    ///< Pro STMT_FOR.
  } as;
};

//--------------------------------------------------------------------------------//

/**
 * @brief Parametr funkce, napr. n: Int.
 */
typedef struct {
  char *name; ///< Jmeno parametru, vlastni kopie.
  Type *type; ///< Typ parametru, vzdy zadany.
  struct Symbol
      *symbol; ///< Symbol z tabulky symbolu (nevlastni), vyplni semantika.
} Param;

/**
 * @brief Definice funkce.
 */
typedef struct {
  char *name;        ///< Jmeno funkce, vlastni kopie.
  Param *params;     ///< Pole parametru.
  int param_count;   ///< Pocet parametru, muze byt 0.
  Type *return_type; ///< Navratovy typ, NULL kdyz ho programator nenapsal.
  Block *body;       ///< Telo funkce.
  bool is_main;      ///< True pro funkci s anotaci @main.
  int line;          ///< Radek ve zdrojovem kodu pro chybova hlaseni.
  struct Symbol
      *symbol; ///< Symbol z tabulky symbolu (nevlastni), vyplni semantika.
} FuncDef;

/**
 * @brief Cely program: seznam definic funkci.
 */
typedef struct {
  FuncDef **funcs; ///< Pole ukazatelu na funkce.
  int func_count;  ///< Pocet funkci.
} Program;

//--------------------------------------------------------------------------------//

// Funkce pro vytvareni a uvolnovani uzlu

//--------------------------------------------------------------------------------//

//--------------------------------------------------------------------------------//
/* ----- Zacatek casti generovane AI (Claude) -----
 * Prompt: "Udelej konstruktory a uvolnovaci funkce k ast.h
    // Pravidlo 1: Rodicovsky uzel vlastni sve potomky, uvolneni korene uvolni cely strom.
    // Pravidlo 2: Konstruktory prebiraji vlastnictvi ukazatelu na uzly, bloky a typy.
    // Pravidlo 3: Predany ukazatel uz volajici nesmi uvolnit ani pouzit na jinem miste.
    // Pravidlo 4: Retezce (jmena, literaly) se kopiruji, svuj puvodni retezec uvolnuje volajici.
    // Pravidlo 5: Pri nedostatku pameti program konci s navratovym kodem 99.
    // Pravidlo 6: Vsechny funkce *_free akceptuji NULL, parser jimi muze pri chybe uvolnit rozpracovane uzly.
    // Pravidlo 7: Ukazatele na Symbol AST nevlastni a funkce *_free je neuvolnuji."
 */

// Typy

/**
 * @brief Vytvori typ daneho druhu (pro TYPE_LIST pouzij type_new_list).
 *
 * @param kind Druh typu.
 * @return Novy typ.
 */
Type *type_new(TypeKind kind);

/**
 * @brief Vytvori typ List[element], prebere vlastnictvi elementu.
 *
 * @param element Typ prvku seznamu.
 * @return Novy typ seznamu.
 */
Type *type_new_list(Type *element);

/**
 * @brief Vytvori hlubokou kopii typu.
 *
 * Pouziva se v semantice, aby kazdy vyraz mel vlastni typ. Resi problem
 * s dvojitym uvolnenim (kazdy vyraz uvolnuje svuj typ).
 * Napr. Expr "a" => TYPE_INT (1), Expr "a + b" => TYPE_INT (2).
 * Neukazuji na stejny TYPE_INT, ale kazdy ma svuj.
 *
 * @param type Typ ke zkopirovani.
 * @return Nova kopie typu.
 */
Type *type_clone(const Type *type);

/**
 * @brief Uvolni typ vcetne typu prvku.
 *
 * @param type Typ k uvolneni (muze byt NULL).
 */
void type_free(Type *type);

// Vyrazy

/**
 * @brief Vytvori celociselny literal.
 *
 * @param value Hodnota.
 * @param line  Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_int(int value, int line);

/**
 * @brief Vytvori desetinny literal.
 *
 * @param value Hodnota.
 * @param line  Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_double(double value, int line);

/**
 * @brief Vytvori retezcovy literal, retezec se zkopiruje.
 *
 * @param value Hodnota.
 * @param line  Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_string(const char *value, int line);

/**
 * @brief Vytvori literal Unit, tedy hodnotu ().
 *
 * @param line Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_unit(int line);

/**
 * @brief Vytvori pravdivostni literal.
 *
 * @param value Hodnota.
 * @param line  Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_bool(bool value, int line);

/**
 * @brief Vytvori pouziti promenne, jmeno se zkopiruje.
 *
 * @param name Jmeno promenne.
 * @param line Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_variable(const char *name, int line);

/**
 * @brief Vytvori unarni vyraz, prebere vlastnictvi operandu.
 *
 * @param op      Operator.
 * @param operand Operand.
 * @param line    Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_unary(UnaryOp op, Expr *operand, int line);

/**
 * @brief Vytvori binarni vyraz, prebere vlastnictvi obou operandu.
 *
 * @param op    Operator.
 * @param left  Levy operand.
 * @param right Pravy operand.
 * @param line  Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_binary(BinaryOp op, Expr *left, Expr *right, int line);

/**
 * @brief Vytvori volani funkce bez argumentu.
 *
 * Argumenty se pridavaji funkci expr_call_add_arg.
 *
 * @param name Jmeno volane funkce.
 * @param line Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_call(const char *name, int line);

/**
 * @brief Prida argument na konec volani funkce, prebere jeho vlastnictvi.
 *
 * @param call Volani funkce.
 * @param arg  Pridavany argument.
 */
void expr_call_add_arg(Expr *call, Expr *arg);

/**
 * @brief Vytvori podminku if, prebere vlastnictvi vsech casti.
 *
 * @param cond       Podminka.
 * @param then_block Vetev then.
 * @param else_block Vetev else, NULL kdyz chybi.
 * @param line       Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_if(Expr *cond, Block *then_block, Block *else_block, int line);

/**
 * @brief Vytvori rozsah, prebere vlastnictvi vsech casti.
 *
 * @param from         Zacatek rozsahu.
 * @param to           Konec rozsahu.
 * @param is_inclusive True pro to, false pro until.
 * @param step         Krok, NULL kdyz chybi.
 * @param line         Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_range(Expr *from, Expr *to, bool is_inclusive, Expr *step,
                     int line);

/**
 * @brief Vytvori clen hodnoty bez argumentu.
 *
 * Argumenty se pridavaji funkci expr_member_add_arg.
 *
 * @param object Hodnota, na ktere se clen vola.
 * @param name   Jmeno clenu.
 * @param line   Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_member(Expr *object, const char *name, int line);

/**
 * @brief Prida argument na konec clenu, prebere jeho vlastnictvi.
 *
 * @param member Vyraz clenu.
 * @param arg    Pridavany argument.
 */
void expr_member_add_arg(Expr *member, Expr *arg);

/**
 * @brief Vytvori prazdny seznam.
 *
 * Prvky se pridavaji funkci expr_list_add_item.
 *
 * @param line Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_list_literal(int line);

/**
 * @brief Prida prvek na konec seznamu, prebere jeho vlastnictvi.
 *
 * @param list Seznam.
 * @param item Pridavany prvek.
 */
void expr_list_add_item(Expr *list, Expr *item);

/**
 * @brief Vytvori List.range, prebere vlastnictvi vsech casti.
 *
 * @param start Prvni cislo.
 * @param end   Konec, bez nej.
 * @param step  Krok, NULL kdyz chybi.
 * @param line  Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_list_range(Expr *start, Expr *end, Expr *step, int line);

/**
 * @brief Vytvori pristup k prvku seznamu, prebere vlastnictvi obou casti.
 *
 * @param list  Seznam, ze ktereho se cte.
 * @param index Index prvku.
 * @param line  Radek ve zdrojovem kodu.
 * @return Novy vyraz.
 */
Expr *expr_new_index(Expr *list, Expr *index, int line);

/**
 * @brief Uvolni vyraz a cely jeho podstrom.
 *
 * @param expr Vyraz k uvolneni (muze byt NULL).
 */
void expr_free(Expr *expr);

// Bloky a prikazy

/**
 * @brief Vytvori prazdny blok.
 *
 * @return Novy blok.
 */
Block *block_new(void);

/**
 * @brief Prida prikaz na konec bloku, prebere vlastnictvi prikazu.
 *
 * @param block Blok.
 * @param stmt  Pridavany prikaz.
 */
void block_add(Block *block, Stmt *stmt);

/**
 * @brief Uvolni blok a vsechny jeho prikazy.
 *
 * @param block Blok k uvolneni (muze byt NULL).
 */
void block_free(Block *block);

/**
 * @brief Vytvori definici promenne, prebere vlastnictvi typu a inicializace.
 *
 * @param is_mutable    True pro var, false pro val.
 * @param name          Jmeno promenne.
 * @param declared_type Uvedeny typ, NULL kdyz chybi.
 * @param init          Pocatecni hodnota.
 * @param line          Radek ve zdrojovem kodu.
 * @return Novy prikaz.
 */
Stmt *stmt_new_vardef(bool is_mutable, const char *name, Type *declared_type,
                      Expr *init, int line);

/**
 * @brief Vytvori prirazeni, prebere vlastnictvi hodnoty.
 *
 * @param name  Jmeno promenne.
 * @param value Nova hodnota.
 * @param line  Radek ve zdrojovem kodu.
 * @return Novy prikaz.
 */
Stmt *stmt_new_assign(const char *name, Expr *value, int line);

/**
 * @brief Vytvori prikaz return, prebere vlastnictvi hodnoty.
 *
 * @param value Vracena hodnota, NULL pro prazdny return.
 * @param line  Radek ve zdrojovem kodu.
 * @return Novy prikaz.
 */
Stmt *stmt_new_return(Expr *value, int line);

/**
 * @brief Vytvori prikaz ze samostatneho vyrazu, prebere vlastnictvi vyrazu.
 *
 * @param expr Vyraz.
 * @param line Radek ve zdrojovem kodu.
 * @return Novy prikaz.
 */
Stmt *stmt_new_expr(Expr *expr, int line);

/**
 * @brief Vytvori cyklus while, prebere vlastnictvi podminky a tela.
 *
 * @param cond Podminka.
 * @param body Telo cyklu.
 * @param line Radek ve zdrojovem kodu.
 * @return Novy prikaz.
 */
Stmt *stmt_new_while(Expr *cond, Block *body, int line);

/**
 * @brief Vytvori cyklus for bez strazi a bez tela.
 *
 * Straze a telo se doplni funkcemi stmt_for_add_guard a stmt_for_set_body.
 *
 * @param var_name Jmeno ridici promenne.
 * @param source   Zdroj: rozsah nebo seznam.
 * @param line     Radek ve zdrojovem kodu.
 * @return Novy prikaz.
 */
Stmt *stmt_new_for(const char *var_name, Expr *source, int line);

/**
 * @brief Prida straz cyklu for, prebere jeho vlastnictvi.
 *
 * @param for_stmt Cyklus for.
 * @param guard    Straz (vyraz typu Boolean).
 */
void stmt_for_add_guard(Stmt *for_stmt, Expr *guard);

/**
 * @brief Nastavi telo cyklu for, prebere jeho vlastnictvi.
 *
 * @param for_stmt Cyklus for.
 * @param body     Telo cyklu.
 */
void stmt_for_set_body(Stmt *for_stmt, Block *body);

/**
 * @brief Uvolni prikaz a cely jeho podstrom.
 *
 * @param stmt Prikaz k uvolneni (muze byt NULL).
 */
void stmt_free(Stmt *stmt);

// Funkce a program

/**
 * @brief Vytvori funkci bez parametru, bez navratoveho typu a bez tela.
 *
 * @param name    Jmeno funkce.
 * @param is_main True pro funkci s anotaci @main.
 * @param line    Radek ve zdrojovem kodu.
 * @return Nova definice funkce.
 */
FuncDef *funcdef_new(const char *name, bool is_main, int line);

/**
 * @brief Prida parametr na konec seznamu parametru, prebere vlastnictvi typu.
 *
 * @param func Funkce.
 * @param name Jmeno parametru.
 * @param type Typ parametru.
 */
void funcdef_add_param(FuncDef *func, const char *name, Type *type);

/**
 * @brief Nastavi navratovy typ funkce, prebere jeho vlastnictvi.
 *
 * @param func        Funkce.
 * @param return_type Navratovy typ.
 */
void funcdef_set_return_type(FuncDef *func, Type *return_type);

/**
 * @brief Nastavi telo funkce, prebere jeho vlastnictvi.
 *
 * @param func Funkce.
 * @param body Telo funkce.
 */
void funcdef_set_body(FuncDef *func, Block *body);

/**
 * @brief Uvolni definici funkce vcetne parametru a tela.
 *
 * @param func Funkce k uvolneni (muze byt NULL).
 */
void funcdef_free(FuncDef *func);

/**
 * @brief Vytvori prazdny program.
 *
 * @return Novy program.
 */
Program *program_new(void);

/**
 * @brief Prida funkci do programu, prebere vlastnictvi funkce.
 *
 * @param program Program.
 * @param func    Pridavana funkce.
 */
void program_add_func(Program *program, FuncDef *func);

/**
 * @brief Uvolni program a vsechny jeho funkce.
 *
 * @param program Program k uvolneni (muze byt NULL).
 */
void program_free(Program *program);

/* ----- Konec casti generovane AI ----- */
//--------------------------------------------------------------------------------//

//--------------------------------------------------------------------------------//
// Postupne sestavovani uzlu
//--------------------------------------------------------------------------------//

/* ----- Zacatek casti generovane AI (Claude) -----
 * Prompt: "Parser rizeny tabulkou (precedencni parser + LL tabulka) sklada
 * AST postupne pri redukci pravidel, takze v okamziku vytvoreni uzlu jeste
 * nezna vsechny jeho casti. Uprav ast.h tak, aby se uzel dal
 * vytvorit neuplny a jeho casti se doplnovaly pozdeji pomoci setteru.
 * Pozadavky:
 *   - konstruktory (stmt_new_vardef, stmt_new_assign, stmt_new_for,
 *     funcdef_new, funcdef_add_param) akceptuji NULL u jmena, ktere
 *     zatim neni zname; expr_new_if, stmt_new_while, stmt_new_return
 *     a stmt_new_expr akceptuji NULL u casti uzlu,
 *   - settery prebiraji vlastnictvi predaneho ukazatele (retezce kopiruji)
 *     a puvodni hodnotu uvolni,
 *   - setter zavolany na uzel jineho druhu ukonci program s chybou
 *     ERR_INTERNAL,
 *   - parametry funkce se adresuji indexem, ne ukazatelem, protoze se
 *     pole pri pridani presouva (realloc),
 *   - pridej pomocne funkce block_from_stmt a block_from_expr pro
 *     jednoradkova tela a vetve ternarniho if"
 *   
 */

// Typy

/**
 * @brief Nastavi typ prvku u typu List[T].
 *
 * @param list    Typ seznamu.
 * @param element Typ prvku, prebere se jeho vlastnictvi.
 */
void type_list_set_element(Type *list, Type *element);

// Pomocne funkce pro bloky

/**
 * @brief Vytvori blok s jednim prikazem (jednoradkove telo).
 *
 * @param stmt Prikaz, prebere se jeho vlastnictvi.
 * @return Novy blok.
 */
Block *block_from_stmt(Stmt *stmt);

/**
 * @brief Vytvori blok s jednim prikazem, ktery je vyraz.
 *
 * Pouziva se pro vetev ternarniho if a telo funkce.
 *
 * @param expr Vyraz, prebere se jeho vlastnictvi.
 * @param line Radek ve zdrojovem kodu.
 * @return Novy blok.
 */
Block *block_from_expr(Expr *expr, int line);

// Definice promenne

/**
 * @brief Nastavi, zda je promenna modifikovatelna.
 *
 * @param stmt       Prikaz STMT_VARDEF.
 * @param is_mutable True pro var, false pro val.
 */
void stmt_vardef_set_mutable(Stmt *stmt,
                             bool is_mutable); // True pro var, false pro val

/**
 * @brief Nastavi jmeno definovane promenne, jmeno se zkopiruje.
 *
 * @param stmt Prikaz STMT_VARDEF.
 * @param name Jmeno promenne.
 */
void stmt_vardef_set_name(Stmt *stmt, const char *name);

/**
 * @brief Nastavi uvedeny typ promenne, prebere jeho vlastnictvi.
 *
 * @param stmt          Prikaz STMT_VARDEF.
 * @param declared_type Uvedeny typ.
 */
void stmt_vardef_set_type(Stmt *stmt, Type *declared_type);

/**
 * @brief Nastavi pocatecni hodnotu promenne, prebere jeji vlastnictvi.
 *
 * @param stmt Prikaz STMT_VARDEF.
 * @param init Pocatecni hodnota.
 */
void stmt_vardef_set_init(Stmt *stmt, Expr *init);

// Prirazeni, return, samostatny vyraz

/**
 * @brief Nastavi jmeno promenne v prirazeni, jmeno se zkopiruje.
 *
 * @param stmt Prikaz STMT_ASSIGN.
 * @param name Jmeno promenne.
 */
void stmt_assign_set_name(Stmt *stmt, const char *name);

/**
 * @brief Nastavi novou hodnotu v prirazeni, prebere jeji vlastnictvi.
 *
 * @param stmt  Prikaz STMT_ASSIGN.
 * @param value Nova hodnota.
 */
void stmt_assign_set_value(Stmt *stmt, Expr *value);

/**
 * @brief Nastavi vracenou hodnotu prikazu return, prebere jeji vlastnictvi.
 *
 * @param stmt  Prikaz STMT_RETURN.
 * @param value Vracena hodnota.
 */
void stmt_return_set_value(Stmt *stmt, Expr *value);

/**
 * @brief Nastavi vyraz samostatneho prikazu, prebere jeho vlastnictvi.
 *
 * @param stmt Prikaz STMT_EXPR.
 * @param expr Vyraz.
 */
void stmt_expr_set_expr(Stmt *stmt, Expr *expr);

// Cykly

/**
 * @brief Nastavi podminku cyklu while, prebere jeji vlastnictvi.
 *
 * @param stmt Prikaz STMT_WHILE.
 * @param cond Podminka.
 */
void stmt_while_set_cond(Stmt *stmt, Expr *cond);

/**
 * @brief Nastavi telo cyklu while, prebere jeho vlastnictvi.
 *
 * @param stmt Prikaz STMT_WHILE.
 * @param body Telo cyklu.
 */
void stmt_while_set_body(Stmt *stmt, Block *body);

/**
 * @brief Nastavi jmeno ridici promenne cyklu for, jmeno se zkopiruje.
 *
 * @param stmt     Prikaz STMT_FOR.
 * @param var_name Jmeno ridici promenne.
 */
void stmt_for_set_var_name(Stmt *stmt, const char *var_name);

/**
 * @brief Nastavi zdroj cyklu for, prebere jeho vlastnictvi.
 *
 * @param stmt   Prikaz STMT_FOR.
 * @param source Zdroj: rozsah nebo seznam.
 */
void stmt_for_set_source(Stmt *stmt, Expr *source);

// Podminka if (pouzije se expr_new_if(NULL, NULL, NULL, line))

/**
 * @brief Nastavi podminku vyrazu if, prebere jeji vlastnictvi.
 *
 * @param expr Vyraz EXPR_IF.
 * @param cond Podminka.
 */
void expr_if_set_cond(Expr *expr, Expr *cond);

/**
 * @brief Nastavi vetev then vyrazu if, prebere jeji vlastnictvi.
 *
 * @param expr       Vyraz EXPR_IF.
 * @param then_block Vetev then.
 */
void expr_if_set_then(Expr *expr, Block *then_block);

/**
 * @brief Nastavi vetev else vyrazu if, prebere jeji vlastnictvi.
 *
 * @param expr       Vyraz EXPR_IF.
 * @param else_block Vetev else.
 */
void expr_if_set_else(Expr *expr, Block *else_block);

// Funkce (parametry se adresuji indexem, ne ukazatelem, protoze se pole presouva)

/**
 * @brief Nastavi jmeno funkce, jmeno se zkopiruje.
 *
 * @param func Funkce.
 * @param name Jmeno funkce.
 */
void funcdef_set_name(FuncDef *func, const char *name);

/**
 * @brief Nastavi, zda ma funkce anotaci @main.
 *
 * @param func    Funkce.
 * @param is_main True pro funkci s anotaci @main.
 */
void funcdef_set_main(FuncDef *func, bool is_main);

/**
 * @brief Nastavi jmeno parametru na danem indexu, jmeno se zkopiruje.
 *
 * @param func  Funkce.
 * @param index Index parametru.
 * @param name  Jmeno parametru.
 */
void funcdef_param_set_name(FuncDef *func, int index, const char *name);

/**
 * @brief Nastavi typ parametru na danem indexu, prebere jeho vlastnictvi.
 *
 * @param func  Funkce.
 * @param index Index parametru.
 * @param type  Typ parametru.
 */
void funcdef_param_set_type(FuncDef *func, int index, Type *type);

/* ----- Konec casti generovane AI ----- */
//--------------------------------------------------------------------------------//

//--------------------------------------------------------------------------------//
// Pomocne funkce pro precedencni parser
//--------------------------------------------------------------------------------//

/* ----- Zacatek casti generovane AI (Claude) -----
 * Prompt:
 *   Do precedencniho parseru (precedence_semantic_actions.c) potrebuji
 *   pomocne funkce pro napojeni semantickych akci na AST (typ Expr).
 *   Parser pri redukci pravidel sklada vyrazy po castech a nektere
 *   uzly (kroky rozsahu, argumenty volani) vznikaji az dodatecne, proto
 *   je potreba:
 *     - nastavit krok u uzlu EXPR_RANGE,
 *     - doplnit argumenty do volani funkce (EXPR_CALL) a do vyrazu
 *       clenu (EXPR_MEMBER),
 *     - vyjmout prvky z pomocneho uzlu EXPR_LIST_LITERAL.
 *   Pomocny list literal slouzi jen jako docasny kontejner argumentu
 *   a po presunu prvku se uvolni.
 *   Pozadavky: jasne vlastnictvi pameti (funkce prvky prebiraji),
 *   zadne uniky pameti
 */

/**
 * @brief Nastavi krok rozsahu a prebere jeho vlastnictvi.
 *
 * Puvodni krok se uvolni.
 *
 * @param range Vyraz EXPR_RANGE.
 * @param step  Novy krok.
 */
void expr_range_set_step(Expr *range, Expr *step);

/**
 * @brief Presune prvky list literalu na konec argumentu volani funkce.
 *
 * Prebere vlastnictvi prvku. List literal (pomocny uzel) uvolni.
 *
 * @param call Volani funkce.
 * @param args Pomocny uzel EXPR_LIST_LITERAL s argumenty.
 */
void expr_call_take_args(Expr *call, Expr *args);

/**
 * @brief Presune prvky list literalu na konec argumentu clenu.
 *
 * Prebere vlastnictvi prvku. List literal (pomocny uzel) uvolni.
 *
 * @param member Vyraz clenu.
 * @param args   Pomocny uzel EXPR_LIST_LITERAL s argumenty.
 */
void expr_member_take_args(Expr *member, Expr *args);

/**
 * @brief Vyjme prvky z list literalu.
 *
 * Do *items da pole ukazatelu (volajici ho uvolni funkci free, jeho prvky
 * prebira) a do *count jejich pocet. List literal zustane prazdny a uvolni
 * ho volajici pomoci expr_free.
 *
 * @param list  List literal.
 * @param items Vystup: pole ukazatelu na prvky.
 * @param count Vystup: pocet prvku.
 */
void expr_list_take_items(Expr *list, Expr ***items, int *count);

/* ----- Konec casti generovane AI ----- */
//--------------------------------------------------------------------------------//

#endif