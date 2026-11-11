// xnovacm00
// semantic.c - semanticka analyza

#include "semantic.h"
#include "ast.h"
#include "symtable.h"
#include "error.h"

#include <stdbool.h>
#include <stddef.h>


// Vrati true, kdyz jsou dva typy uplne stejne (u seznamu se porovnava i typ prvku)
static bool type_equals(const Type *a, const Type *b)
{
    if (a == NULL || b == NULL) {
        return a == b; // Stejne jsou jen kdyz jsou oba NULL
    }
    if (a->kind != b->kind) {
        return false;
    }
    if (a->kind == TYPE_LIST) {
        return type_equals(a->element, b->element);
    }
    return true;
}

// Vrati true, kdyz je typ Int nebo Double
static bool type_is_numeric(const Type *type)
{
    return type != NULL && (type->kind == TYPE_INT || type->kind == TYPE_DOUBLE);
}

// Vrati true, kdyz lze hodnotu typu src ulozit tam, kde se ceka typ target
// Povolena je jedina implicitni konverze: Int -> Double
static bool type_assignable(const Type *target, const Type *src)
{
    if (type_equals(target, src)) {
        return true;
    }
    return target != NULL && src != NULL
        && target->kind == TYPE_DOUBLE && src->kind == TYPE_INT;
}


// Sdileny stav pri analyze (zatim jen tabulka symbolu)
typedef struct {
    SymTable *table;
} Ctx;

// Predbezne deklarace (funkce se volaji navzajem)
static Type *analyze_expr(Ctx *ctx, Expr *e, const Type *expected);
static Type *analyze_block(Ctx *ctx, Block *b, const Type *expected);
static void analyze_stmt(Ctx *ctx, Stmt *s);


// Vytvori novy typ Unit (vlastni ho volajici)
static Type *unit_type(void)
{
    return type_new(TYPE_UNIT);
}


// Zapise vsechny funkce do tabulky symbolu a zkontroluje funkci s anotaci @main
static void register_functions(Ctx *ctx, Program *program)
{
    int main_count = 0;

    for (int i = 0; i < program->func_count; i++) {
        FuncDef *func = program->funcs[i];

        // Zapsani funkce do globalniho rozsahu, NULL znamena, ze jmeno uz existuje
        Symbol *symbol = symtable_define_func(ctx->table, func->name, func);
        if (symbol == NULL) {
            error_exit(ERR_REDEFINED, func->line, "Redefinice funkce");
        }
        func->symbol = symbol;

        if (func->is_main) {
            main_count++;

            if (func->param_count != 0) {
                error_exit(ERR_ARGS, func->line, "Funkce @main nesmi mit parametry");
            }
            // Navratovy typ muze chybet (pak se odvodi), ale kdyz je uveden, musi to byt Unit
            if (func->return_type != NULL && func->return_type->kind != TYPE_UNIT) {
                error_exit(ERR_TYPE, func->line, "Funkce @main musi vracet Unit");
            }
        }
    }

    if (main_count == 0) {
        error_exit(ERR_UNDEFINED, 0, "Chybi funkce s anotaci @main");
    }
    if (main_count > 1) {
        error_exit(ERR_REDEFINED, 0, "Vice funkci s anotaci @main");
    }
}

// Nastavi vyrazu typ daneho druhu a vrati ukazatel na nej (typ patri vyrazu)
static Type *set_expr_type(Expr *e, TypeKind kind)
{
    e->type = type_new(kind);
    return e->type;
}


// Vrati typ vysledku aritmetiky (+ - * /): Int,Int -> Int, jinak Double
// Oba operandy musi byt ciselne, jinak chyba 6
static TypeKind arithmetic_result(const Type *l, const Type *r, int line)
{
    if (!type_is_numeric(l) || !type_is_numeric(r)) {
        error_exit(ERR_TYPE, line, "Aritmeticky operator vyzaduje ciselne operandy");
    }
    if (l->kind == TYPE_INT && r->kind == TYPE_INT) {
        return TYPE_INT;
    }
    return TYPE_DOUBLE;
}


// Zjisti typ vyrazu, ulozi ho do e->type a vrati jej (nevlastni ukazatel)
// Parametr expected je typ, ktery se od vyrazu ocekava (zatim nepouzity, pozdeji pro List())
static Type *analyze_expr(Ctx *ctx, Expr *e, const Type *expected)
{
    (void)expected;

    switch (e->kind) {
    case EXPR_LITERAL:
        switch (e->as.literal.kind) {
        case LIT_INT:    return set_expr_type(e, TYPE_INT);
        case LIT_DOUBLE: return set_expr_type(e, TYPE_DOUBLE);
        case LIT_STRING: return set_expr_type(e, TYPE_STRING);
        case LIT_UNIT:   return set_expr_type(e, TYPE_UNIT);
        case LIT_BOOL:   return set_expr_type(e, TYPE_BOOLEAN);
        }
        break;

    case EXPR_VARIABLE: {
        Symbol *symbol = symtable_lookup(ctx->table, e->as.variable.name);
        if (symbol == NULL || symbol->kind != SYM_VAR) {
            error_exit(ERR_UNDEFINED, e->line, "Pouziti nedefinovane promenne");
        }
        e->as.variable.symbol = symbol;
        e->type = type_clone(symbol->as.var.type);
        return e->type;
    }

    case EXPR_UNARY: {
        Type *t = analyze_expr(ctx, e->as.unary.operand, NULL);
        if (e->as.unary.op == UNOP_NOT) {
            if (t->kind != TYPE_BOOLEAN) {
                error_exit(ERR_TYPE, e->line, "Operator ! vyzaduje Boolean");
            }
            return set_expr_type(e, TYPE_BOOLEAN);
        }
        // UNOP_PLUS a UNOP_NEG: jen cisla, typ se nemeni
        if (!type_is_numeric(t)) {
            error_exit(ERR_TYPE, e->line, "Unarni +/- vyzaduje cislo");
        }
        return set_expr_type(e, t->kind);
    }

    case EXPR_BINARY: {
        Type *l = analyze_expr(ctx, e->as.binary.left, NULL);
        Type *r = analyze_expr(ctx, e->as.binary.right, NULL);

        switch (e->as.binary.op) {
        case BINOP_ADD:
            // String + String je konkatenace
            if (l->kind == TYPE_STRING && r->kind == TYPE_STRING) {
                return set_expr_type(e, TYPE_STRING);
            }
            return set_expr_type(e, arithmetic_result(l, r, e->line));

        case BINOP_SUB:
        case BINOP_MUL:
        case BINOP_DIV:
            return set_expr_type(e, arithmetic_result(l, r, e->line));

        case BINOP_LT:
        case BINOP_LE:
        case BINOP_GT:
        case BINOP_GE:
            // dve cisla, nebo dva stringy
            if (!((type_is_numeric(l) && type_is_numeric(r))
                  || (l->kind == TYPE_STRING && r->kind == TYPE_STRING))) {
                error_exit(ERR_TYPE, e->line, "Spatne operandy relacniho operatoru");
            }
            return set_expr_type(e, TYPE_BOOLEAN);

        case BINOP_EQ:
        case BINOP_NE:
            // stejne typy, nebo dve cisla
            if (!(type_equals(l, r) || (type_is_numeric(l) && type_is_numeric(r)))) {
                error_exit(ERR_TYPE, e->line, "Porovnani hodnot ruznych typu");
            }
            return set_expr_type(e, TYPE_BOOLEAN);

        case BINOP_AND:
        case BINOP_OR:
            if (l->kind != TYPE_BOOLEAN || r->kind != TYPE_BOOLEAN) {
                error_exit(ERR_TYPE, e->line, "&& a || vyzaduji Boolean");
            }
            return set_expr_type(e, TYPE_BOOLEAN);

        case BINOP_CONCAT_LIST:
            error_exit(ERR_INTERNAL, e->line, "++ zatim neni implementovano");
        }
        break;
    }


    default:
        error_exit(ERR_INTERNAL, e->line, "Tento druh vyrazu se zatim neanalyzuje");
    }

    return NULL; // Sem se program nedostane
}


// Zanalyzuje blok v novem rozsahu a vrati typ jeho vysledku (vlastni volajici)
// Vysledek je typ posledniho prikazu, je-li to vyraz, jinak Unit
static Type *analyze_block(Ctx *ctx, Block *b, const Type *expected)
{
    (void)expected; // Zatim nepouzito

    symtable_enter_scope(ctx->table);

    Type *result = NULL;
    for (int i = 0; i < b->count; i++) {
        Stmt *s = b->items[i];
        analyze_stmt(ctx, s);

        if (i == b->count - 1) {
            if (s->kind == STMT_EXPR) {
                result = type_clone(s->as.expr_stmt.expr->type);
            }
        }
    }

    symtable_exit_scope(ctx->table);

    return result != NULL ? result : unit_type();
}


// Zanalyzuje jeden prikaz
static void analyze_stmt(Ctx *ctx, Stmt *s)
{
    switch (s->kind) {
    case STMT_EXPR:
        analyze_expr(ctx, s->as.expr_stmt.expr, NULL);
        break;

    case STMT_VARDEF: {
        Type *declared = s->as.vardef.declared_type; // muze byt NULL
        Type *init_type = analyze_expr(ctx, s->as.vardef.init, declared);

        Type *final_type;
        if (declared != NULL) {
            if (!type_assignable(declared, init_type)) {
                error_exit(ERR_TYPE, s->line, "Inicializace nesouhlasi s uvedenym typem");
            }
            final_type = declared;
        } else {
            final_type = init_type;
        }

        // Definice az PO analyze initu, aby val x = x neprosel
        Symbol *sym = symtable_define_var(ctx->table, s->as.vardef.name,
                                          final_type, s->as.vardef.is_mutable);
        if (sym == NULL) {
            error_exit(ERR_REDEFINED, s->line, "Redefinice promenne");
        }
        s->as.vardef.symbol = sym;
        break;
    }

    case STMT_ASSIGN: {
        Symbol *sym = symtable_lookup(ctx->table, s->as.assign.name);
        if (sym == NULL || sym->kind != SYM_VAR) {
            error_exit(ERR_UNDEFINED, s->line, "Prirazeni do nedefinovane promenne");
        }
        if (!sym->as.var.is_mutable) {
            error_exit(ERR_IMMUTABLE, s->line, "Prirazeni do nemodifikovatelne promenne");
        }
        Type *value_type = analyze_expr(ctx, s->as.assign.value, sym->as.var.type);
        if (!type_assignable(sym->as.var.type, value_type)) {
            error_exit(ERR_TYPE, s->line, "Prirazeni hodnoty nekompatibilniho typu");
        }
        s->as.assign.symbol = sym;
        break;
    }

    default:
        error_exit(ERR_INTERNAL, s->line, "Tento druh prikazu se zatim neanalyzuje");
    }
}








SymTable *semantic_analyze(Program *program)
{
    Ctx ctx;
    ctx.table = symtable_new();
    symtable_register_builtins(ctx.table);

    register_functions(&ctx, program);

    // Dalsi kroky

    return ctx.table; // Uvolni az main.c po generovani kodu
}