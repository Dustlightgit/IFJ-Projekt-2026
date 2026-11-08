// xnovacm00
// ast.c - implementace abstraktniho syntaktickeho stromu

#include "ast.h"
#include "error.h"

#include <stdlib.h>
#include <string.h>


// Alokuje pamet, pri neuspechu ukonci program s kodem interni chyby
static void *xmalloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == NULL) {
        error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
    }
    return ptr;
}

// Vytvori kopii retezce
static char *xstrdup(const char *str)
{
    size_t len = strlen(str) + 1; // Delka retezce + nullovy znak
    char *copy = xmalloc(len);
    
    // Zkopiruju obsah retezce do nove pameti (vytvori se nezavisla kopie retezce)
    memcpy(copy, str, len); 
    return copy;
}


// Zmeni velikost pameti, pri neuspechu ukonci program s kodem interni chyby
static void *xrealloc(void *old, size_t size)
{
    void *ptr = realloc(old, size);
    if (ptr == NULL) {
        error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
    }
    return ptr;
}


// Zvetsi pole o jedno misto (pro prazdne pole s ukazatelem NULL ho vytvori)
static void *grow_array(void *array, int count, size_t elem_size)
{
    return xrealloc(array, (size_t)(count + 1) * elem_size);
}

//--------------------------------------------------------------------------------//
// Typy
//--------------------------------------------------------------------------------//

// Vytvori novy typ daneho druhu (pro TYPE_LIST pouzij type_new_list)
Type *type_new(TypeKind kind)
{
    Type *type = xmalloc(sizeof *type);
    type->kind = kind;
    type->element = NULL;
    return type;
}

// Vytvori typ seznamu List[element] a prebere vlastnictvi prvku
Type *type_new_list(Type *element)
{
    Type *type = type_new(TYPE_LIST);
    type->element = element;
    return type;
}

// Vytvori hlubokou kopii typu (rekurzivne zkopiruje i vnitrni prvky)
Type *type_clone(const Type *type)
{
    if (type == NULL) {
        return NULL;
    }
    Type *copy = type_new(type->kind);
    copy->element = type_clone(type->element);
    return copy;
}

// Rekurzivne uvolni typ vcetne jeho vnitrnich prvku z pameti
void type_free(Type *type)
{
    if (type == NULL) {
        return;
    }
    type_free(type->element);
    free(type);
}


//--------------------------------------------------------------------------------//
// Vyrazy
//--------------------------------------------------------------------------------//

// Vytvori prazdny vyraz daneho druhu, vsechna ostatni pole jsou vynulovana
static Expr *expr_alloc(ExprKind kind, int line)
{
    Expr *expr = xmalloc(sizeof *expr);
    memset(expr, 0, sizeof *expr);
    expr->kind = kind;
    expr->line = line;
    return expr;
}

// Vytvori vyraz pro celociselny literal
Expr *expr_new_int(int value, int line)
{
    Expr *expr = expr_alloc(EXPR_LITERAL, line);
    expr->as.literal.kind = LIT_INT;
    expr->as.literal.value.int_value = value;
    return expr;
}

// Vytvori vyraz pro desetinny literal
Expr *expr_new_double(double value, int line)
{
    Expr *expr = expr_alloc(EXPR_LITERAL, line);
    expr->as.literal.kind = LIT_DOUBLE;
    expr->as.literal.value.double_value = value;
    return expr;
}

// Vytvori vyraz pro retezcovy literal (vytvori vlastni kopii retezce)
Expr *expr_new_string(const char *value, int line)
{
    Expr *expr = expr_alloc(EXPR_LITERAL, line);
    expr->as.literal.kind = LIT_STRING;
    expr->as.literal.value.string_value = xstrdup(value);
    return expr;
}

// Vytvori vyraz pro jednotkovy literal ()
Expr *expr_new_unit(int line)
{
    Expr *expr = expr_alloc(EXPR_LITERAL, line);
    expr->as.literal.kind = LIT_UNIT;
    return expr;
}

// Vytvori vyraz pro pravdivostni hodnotu (true/false)
Expr *expr_new_bool(bool value, int line)
{
    Expr *expr = expr_alloc(EXPR_LITERAL, line);
    expr->as.literal.kind = LIT_BOOL;
    expr->as.literal.value.bool_value = value;
    return expr;
}

// Vytvori vyraz pro pouziti promenne (vytvori vlastni kopii jmena)
Expr *expr_new_variable(const char *name, int line)
{
    Expr *expr = expr_alloc(EXPR_VARIABLE, line);
    expr->as.variable.name = xstrdup(name);
    return expr;
}

// Vytvori vyraz pro unarni operator a prebere vlastnictvi operandu
Expr *expr_new_unary(UnaryOp op, Expr *operand, int line)
{
    Expr *expr = expr_alloc(EXPR_UNARY, line);
    expr->as.unary.op = op;
    expr->as.unary.operand = operand;
    return expr;
}

// Vytvori vyraz pro binarni operator a prebere vlastnictvi leveho i praveho operandu
// Funkce dostane ukazatel na uz alokovany podvyraz (napr. levy operand)
// Od teto chvile za nej bere plnou zodpovednost
// Az se bude v budoucnu mazat cely vysledny vyraz pres expr_free, automaticky se vnitrne uvolni i tento operand
Expr *expr_new_binary(BinaryOp op, Expr *left, Expr *right, int line)
{
    Expr *expr = expr_alloc(EXPR_BINARY, line);
    expr->as.binary.op = op;
    expr->as.binary.left = left;
    expr->as.binary.right = right;
    return expr;
}




// Vytvori vyraz pro volani funkce (bez argumentu), jmeno funkce zkopiruje
Expr *expr_new_call(const char *name, int line)
{
    Expr *expr = expr_alloc(EXPR_CALL, line);
    expr->as.call.name = xstrdup(name);
    return expr;
}

// Prida argument k existujicimu volani funkce a prebere jeho vlastnictvi
void expr_call_add_arg(Expr *call, Expr *arg)
{
    call->as.call.args = grow_array(call->as.call.args, call->as.call.arg_count, sizeof(Expr *));
    call->as.call.args[call->as.call.arg_count] = arg;
    call->as.call.arg_count++;
}


// Vytvori vyraz pro podminku if (cond pak then_block jinak else_block) a prebere vlastnictvi vsech casti
Expr *expr_new_if(Expr *cond, Block *then_block, Block *else_block, int line)
{
    Expr *expr = expr_alloc(EXPR_IF, line);
    expr->as.if_expr.cond = cond;
    expr->as.if_expr.then_block = then_block;
    expr->as.if_expr.else_block = else_block;
    return expr;
}

// Vytvori vyraz pro rozsah (range) a prebere vlastnictvi vyrazu from, to a step
Expr *expr_new_range(Expr *from, Expr *to, bool is_inclusive, Expr *step, int line)
{
    Expr *expr = expr_alloc(EXPR_RANGE, line);
    expr->as.range.from = from;
    expr->as.range.to = to;
    expr->as.range.is_inclusive = is_inclusive;
    expr->as.range.step = step;
    return expr;
}

// Vytvori vyraz pro pristup ke clenu/metode (bez argumentu), prebere vlastnictvi objektu a zkopiruje jmeno clenu
Expr *expr_new_member(Expr *object, const char *name, int line)
{
    Expr *expr = expr_alloc(EXPR_MEMBER, line);
    expr->as.member.object = object;
    expr->as.member.name = xstrdup(name);
    return expr;
}

// Prida argument k existujicimu clenu/volani metody a prebere jeho vlastnictvi
void expr_member_add_arg(Expr *member, Expr *arg)
{
    member->as.member.args = grow_array(member->as.member.args, member->as.member.arg_count, sizeof(Expr *));
    member->as.member.args[member->as.member.arg_count] = arg;
    member->as.member.arg_count++;
}