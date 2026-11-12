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
    FuncDef *current_func; // Funkce, jejiz telo se prave analyzuje
} Ctx;

// Predbezne deklarace (funkce se volaji navzajem)
static Type *analyze_expr(Ctx *ctx, Expr *e, const Type *expected);
static Type *analyze_block(Ctx *ctx, Block *b, const Type *expected);
static void analyze_stmt(Ctx *ctx, Stmt *s);
static void analyze_func(Ctx *ctx, FuncDef *func);


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

    case EXPR_CALL: {
        Symbol *sym = symtable_lookup(ctx->table, e->as.call.name);
        if (sym == NULL || sym->kind != SYM_FUNC) {
            error_exit(ERR_UNDEFINED, e->line, "Volani nedefinovane funkce");
        }
        e->as.call.symbol = sym;

        if (e->as.call.arg_count != sym->as.func.param_count) {
            error_exit(ERR_ARGS, e->line, "Spatny pocet argumentu");
        }

        for (int i = 0; i < e->as.call.arg_count; i++) {
            const Type *param_type = sym->as.func.param_types[i]; // NULL = Any
            Type *arg_type = analyze_expr(ctx, e->as.call.args[i], param_type);

            if (param_type != NULL && !type_assignable(param_type, arg_type)) {
                error_exit(ERR_ARGS, e->line, "Spatny typ argumentu");
            }
        }

        // Funkce bez uvedeneho typu: odvodime ho ted (rekurze) nebo je to chyba
        if (sym->as.func.return_type == NULL) {
            if (sym->as.func.def == NULL || sym->as.func.being_inferred) {
                error_exit(ERR_TYPE, e->line, "Rekurze vyzaduje explicitni navratovy typ");
            }
            analyze_func(ctx, sym->as.func.def);
        }
        e->type = type_clone(sym->as.func.return_type);
        return e->type;
    }

    case EXPR_IF: {
        Type *cond = analyze_expr(ctx, e->as.if_expr.cond, NULL);
        if (cond->kind != TYPE_BOOLEAN) {
            error_exit(ERR_TYPE, e->line, "Podminka if musi byt Boolean");
        }

        Type *then_type = analyze_block(ctx, e->as.if_expr.then_block, expected);

        // if bez else nema hodnotu, jeho typ je Unit
        if (e->as.if_expr.else_block == NULL) {
            type_free(then_type);
            return set_expr_type(e, TYPE_UNIT);
        }

        Type *else_type = analyze_block(ctx, e->as.if_expr.else_block, expected);

        if (type_equals(then_type, else_type)) {
            e->type = then_type;      // typ preberu, druhy uvolnim
            type_free(else_type);
        } else if (type_is_numeric(then_type) && type_is_numeric(else_type)) {
            e->type = type_new(TYPE_DOUBLE); // Int a Double -> Double
            type_free(then_type);
            type_free(else_type);
        } else {
            // Vetve maji ruzne typy: zatim se bere jako Unit
            e->type = type_new(TYPE_UNIT);
            type_free(then_type);
            type_free(else_type);
        }
        return e->type;
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

        case STMT_WHILE: {
        Type *cond = analyze_expr(ctx, s->as.while_loop.cond, NULL);
        if (cond->kind != TYPE_BOOLEAN) {
            error_exit(ERR_TYPE, s->line, "Podminka while musi byt Boolean");
        }
        // Hodnota tela cyklu se zahazuje
        type_free(analyze_block(ctx, s->as.while_loop.body, NULL));
        break;
    }

    case STMT_RETURN: {
        const Type *ret = ctx->current_func->return_type;

        // return je povolen jen ve funkci s explicitnim navratovym typem
        if (ret == NULL) {
            error_exit(ERR_TYPE, s->line, "return vyzaduje explicitni navratovy typ funkce");
        }

        Expr *value = s->as.ret.value;
        if (value == NULL) {
            // Samostatny return vraci () a je jen ve funkci s typem Unit
            if (ret->kind != TYPE_UNIT) {
                error_exit(ERR_TYPE, s->line, "Prazdny return ve funkci, ktera nevraci Unit");
            }
            break;
        }

        Type *t = analyze_expr(ctx, value, ret);
        // U explicitniho Unit se hodnota vyhodnoti a zahodi, jinak musi sedet typ
        if (ret->kind != TYPE_UNIT && !type_assignable(ret, t)) {
            error_exit(ERR_TYPE, s->line, "Typ return nesouhlasi s navratovym typem");
        }
        break;
    }

    default:
        error_exit(ERR_INTERNAL, s->line, "Tento druh prikazu se zatim neanalyzuje");
    }
}



// Vrati true, kdyz blok vzdy skonci prikazem return
// (posledni prikaz je return, nebo if s else, jehoz obe vetve vzdy konci returnem)
static bool block_returns(const Block *b)
{
    if (b == NULL || b->count == 0) {
        return false;
    }
    const Stmt *last = b->items[b->count - 1];

    if (last->kind == STMT_RETURN) {
        return true;
    }


    if (last->kind == STMT_EXPR) {
        const Expr *x = last->as.expr_stmt.expr;
        if (x->kind == EXPR_IF && x->as.if_expr.else_block != NULL) {
            // Vraci true pouze tehdy, pokud block_returns vrati true jak pro pak vetev, tak pro else vetev (&&)
            // Pokud by alespon jedna vetev return nemela, celkove to vrati false
            return block_returns(x->as.if_expr.then_block)
                && block_returns(x->as.if_expr.else_block);
        }
    }
    return false;
}






// Zanalyzuje definici funkce: parametry, telo a kontrola navratoveho typu
static void analyze_func(Ctx *ctx, FuncDef *func)
{
    Symbol *fsym = func->symbol;
    
    // Navratovy typ se musi odvodit z toho co funkce vraci
    bool infer = (func->return_type == NULL); // typ nebyl uveden

    FuncDef *saved = ctx->current_func; // analyza muze zacit uprostred jine funkce
    ctx->current_func = func;
    if (infer) {
        fsym->as.func.being_inferred = true;
    }

    symtable_enter_scope(ctx->table);

    for (int i = 0; i < func->param_count; i++) {
        Param *p = &func->params[i];
        Symbol *sym = symtable_define_var(ctx->table, p->name, p->type, false);
        if (sym == NULL) {
            error_exit(ERR_REDEFINED, func->line, "Duplicitni jmeno parametru");
        }
        p->symbol = sym;
    }

    Type *body_type = analyze_block(ctx, func->body, NULL);

    if (infer) {
        // Navratovy typ je typ vysledku tela (symbol si ho zkopiruje)
        symbol_set_return_type(fsym, body_type);
        fsym->as.func.being_inferred = false;
    } else if (func->return_type->kind != TYPE_UNIT) {
        // Explicitni typ: bud sedi vysledek tela, nebo telo vzdy konci returnem
        if (!type_assignable(func->return_type, body_type) && !block_returns(func->body)) {
            error_exit(ERR_TYPE, func->line, "Telo funkce nevraci navratovy typ");
        }
    }
    type_free(body_type);

    symtable_exit_scope(ctx->table);
    ctx->current_func = saved;
}





SymTable *semantic_analyze(Program *program)
{
    Ctx ctx;
    ctx.table = symtable_new();
    ctx.current_func = NULL;
    symtable_register_builtins(ctx.table);


    register_functions(&ctx, program);

    for (int i = 0; i < program->func_count; i++) {
        FuncDef *func = program->funcs[i];
        // Preskoc funkci, jejiz typ uz byl odvozen pri volani z jine funkce
        if (func->return_type == NULL && func->symbol->as.func.return_type != NULL) {
            continue;
        }
        analyze_func(&ctx, func);
    }

    // @main bez uvedeneho typu musi mit odvozeny Unit
    for (int i = 0; i < program->func_count; i++) {
        FuncDef *func = program->funcs[i];
        if (func->is_main && func->symbol->as.func.return_type->kind != TYPE_UNIT) {
            error_exit(ERR_TYPE, func->line, "Funkce @main musi vracet Unit");
        }
    }

    return ctx.table;
}