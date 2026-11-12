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


// Vytvori kopii retezce, NULL zustane NULL (pro casti uzlu, ktere zatim nejsou znamy)
static char *xstrdup_opt(const char *str)
{
    return str == NULL ? NULL : xstrdup(str);
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



// Zkontroluje druh prikazu, nesoulad je chyba parseru a program skonci
static void require_stmt(const Stmt *stmt, StmtKind kind)
{
    if (stmt == NULL || stmt->kind != kind) {
        error_exit(ERR_INTERNAL, stmt == NULL ? 0 : stmt->line, "Spatny druh prikazu pri sestavovani stromu");
    }
}

// Zkontroluje druh vyrazu, nesoulad je chyba parseru a program skonci
static void require_expr(const Expr *expr, ExprKind kind)
{
    if (expr == NULL || expr->kind != kind) {
        error_exit(ERR_INTERNAL, expr == NULL ? 0 : expr->line, "Spatny druh vyrazu pri sestavovani stromu");
    }
}

// Zkontroluje, ze funkce existuje, NULL je chyba parseru a program skonci
static void require_func(const FuncDef *func)
{
    if (func == NULL) {
        error_exit(ERR_INTERNAL, 0, "Chybi definice funkce pri sestavovani stromu");
    }
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
    require_expr(call, EXPR_CALL);
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
    require_expr(member, EXPR_MEMBER);
    member->as.member.args = grow_array(member->as.member.args, member->as.member.arg_count, sizeof(Expr *));
    member->as.member.args[member->as.member.arg_count] = arg;
    member->as.member.arg_count++;
}


// Vytvori prazdny vyraz pro list literal (seznam hodnot zapsany v kodu) => List()
Expr *expr_new_list_literal(int line)
{
    return expr_alloc(EXPR_LIST_LITERAL, line);
}


// Prida polozku do list literalu a prebere jej vlastnictvi => List(1, 2, 3)
void expr_list_add_item(Expr *list, Expr *item)
{
    require_expr(list, EXPR_LIST_LITERAL);
    list->as.list_literal.items = grow_array(list->as.list_literal.items, list->as.list_literal.count, sizeof(Expr *));
    list->as.list_literal.items[list->as.list_literal.count] = item;
    list->as.list_literal.count++;
}

// Vytvori vyraz pro rozsah seznamu a prebere vlastnictvi vyrazu start, end a step => List.range(1, 10, 2)
Expr *expr_new_list_range(Expr *start, Expr *end, Expr *step, int line)
{
    Expr *expr = expr_alloc(EXPR_LIST_RANGE, line);
    expr->as.list_range.start = start;
    expr->as.list_range.end = end;
    expr->as.list_range.step = step;
    return expr;
}

// Vytvori vyraz pro indexovani nebo pristup k prvku podle indexu a prebere vlastnictvi list a index => lst(0)
Expr *expr_new_index(Expr *list, Expr *index, int line)
{
    Expr *expr = expr_alloc(EXPR_INDEX, line);
    expr->as.index.list = list;
    expr->as.index.index = index;
    return expr;
}


//--------------------------------------------------------------------------------//
// Bloky a prikazy
//--------------------------------------------------------------------------------//

// Vytvori novy prazdny blok a alokuje pro jej pamet
Block *block_new(void)
{
    Block *block = xmalloc(sizeof *block);
    block->items = NULL;
    block->count = 0;
    return block;
}

// Prida prikaz do bloku a prebere jeho vlastnictvi
void block_add(Block *block, Stmt *stmt)
{
    block->items = grow_array(block->items, block->count, sizeof(Stmt *));
    block->items[block->count] = stmt;
    block->count++;
}

// Vytvori prazdny prikaz daneho druhu, vsechna ostatni pole jsou vynulovana
static Stmt *stmt_alloc(StmtKind kind, int line)
{
    Stmt *stmt = xmalloc(sizeof *stmt);
    memset(stmt, 0, sizeof *stmt);
    stmt->kind = kind;
    stmt->line = line;
    return stmt;
}

// Vytvori prikaz pro definici promenne, naklonuje jmeno a prebere vlastnictvi datoveho typu
// a inicializacniho vyrazu => val x: Int = 42
Stmt *stmt_new_vardef(bool is_mutable, const char *name, Type *declared_type, Expr *init, int line)
{
    Stmt *stmt = stmt_alloc(STMT_VARDEF, line);
    stmt->as.vardef.is_mutable = is_mutable;
    stmt->as.vardef.name = xstrdup_opt(name);
    stmt->as.vardef.declared_type = declared_type;
    stmt->as.vardef.init = init;
    return stmt;
}

// Vytvori prikaz pro prirazeni hodnoty do promenne, naklonuje jmeno a prebere vlastnictvi vyrazu => x = 42
Stmt *stmt_new_assign(const char *name, Expr *value, int line)
{
    Stmt *stmt = stmt_alloc(STMT_ASSIGN, line);
    stmt->as.assign.name = xstrdup_opt(name);
    stmt->as.assign.value = value;
    return stmt;
}

// Vytvori prikaz pro navrat z funkce a prebere vlastnictvi navratoveho vyrazu => return 42
Stmt *stmt_new_return(Expr *value, int line)
{
    Stmt *stmt = stmt_alloc(STMT_RETURN, line);
    stmt->as.ret.value = value;
    return stmt;
}

// Vytvori prikaz pro samostatne vyhodnoceni vyrazu a prebere vlastnictvi vyrazu => print(42)
Stmt *stmt_new_expr(Expr *expr, int line)
{
    Stmt *stmt = stmt_alloc(STMT_EXPR, line);
    stmt->as.expr_stmt.expr = expr;
    return stmt;
}

// Vytvori prikaz pro cyklus while a prebere vlastnictvi podminky a tela => while i > 0 do print(i)
Stmt *stmt_new_while(Expr *cond, Block *body, int line)
{
    Stmt *stmt = stmt_alloc(STMT_WHILE, line);
    stmt->as.while_loop.cond = cond;
    stmt->as.while_loop.body = body;
    return stmt;
}

// Vytvori prikaz pro cyklus for, naklonuje jmeno promenne a prebere vlastnictvi zdroje => for i <- 1 to 10 by 2 do
Stmt *stmt_new_for(const char *var_name, Expr *source, int line)
{
    Stmt *stmt = stmt_alloc(STMT_FOR, line);
    stmt->as.for_loop.var_name = xstrdup_opt(var_name);
    stmt->as.for_loop.source = source;
    return stmt;
}

// Prida straz do cyklu for a prebere vlastnictvi vyrazu straze => if i != 3
void stmt_for_add_guard(Stmt *for_stmt, Expr *guard)
{
    require_stmt(for_stmt, STMT_FOR);
    for_stmt->as.for_loop.guards = grow_array(for_stmt->as.for_loop.guards,
                                              for_stmt->as.for_loop.guard_count, sizeof(Expr *));
    for_stmt->as.for_loop.guards[for_stmt->as.for_loop.guard_count] = guard;
    for_stmt->as.for_loop.guard_count++;
}

// Priradi telo k cyklu for a prebere vlastnictvi bloku => do print(i)
void stmt_for_set_body(Stmt *for_stmt, Block *body)
{
    require_stmt(for_stmt, STMT_FOR);
    block_free(for_stmt->as.for_loop.body);
    for_stmt->as.for_loop.body = body;
}


//--------------------------------------------------------------------------------//
// Funkce a program
//--------------------------------------------------------------------------------//


// Vytvori definici funkce, naklonuje jmeno a nastavi priznak vstupni funkce => @main def runFoo(): Unit =
FuncDef *funcdef_new(const char *name, bool is_main, int line)
{
    FuncDef *func = xmalloc(sizeof *func);
    func->name = xstrdup_opt(name);
    func->params = NULL;
    func->param_count = 0;
    func->return_type = NULL;
    func->body = NULL;
    func->is_main = is_main;
    func->line = line;
    func->symbol = NULL;
    return func;
}

// Prida formalni parametr do definice funkce, naklonuje jmeno a prebere vlastnictvi datoveho typu => n: Int
void funcdef_add_param(FuncDef *func, const char *name, Type *type)
{
    require_func(func);
    func->params = grow_array(func->params, func->param_count, sizeof(Param));
    func->params[func->param_count].name = xstrdup_opt(name);
    func->params[func->param_count].type = type;
    func->params[func->param_count].symbol = NULL;
    func->param_count++;
}

// Nastavi navratovy typ funkce a prebere vlastnictvi datoveho typu => : Int
void funcdef_set_return_type(FuncDef *func, Type *return_type)
{
    require_func(func);
    type_free(func->return_type);
    func->return_type = return_type;
}

// Nastavi telo funkce a prebere vlastnictvi bloku prikazu => = val x = 42; return x
void funcdef_set_body(FuncDef *func, Block *body)
{
    require_func(func);
    block_free(func->body);
    func->body = body;
}

// Vytvori koren abstraktniho syntaktickeho stromu pro cely program (prazdny seznam funkci) => @main def run(): Unit = print(42)
Program *program_new(void)
{
    Program *program = xmalloc(sizeof *program);
    program->funcs = NULL;
    program->func_count = 0;
    return program;
}

// Prida definici funkce do programu a prebere vlastnictvi jeji struktury => def myFunc(): Unit = print(1)
void program_add_func(Program *program, FuncDef *func)
{
    program->funcs = grow_array(program->funcs, program->func_count, sizeof(FuncDef *));
    program->funcs[program->func_count] = func;
    program->func_count++;
}


//--------------------------------------------------------------------------------//
// Uvolnovani pameti
//--------------------------------------------------------------------------------//


//--------------------------------------------------------------------------------//
// Vygenerovano AI (zacatek)
// prompt: "Napis pametove destruktory a uvolnovaci funkce pro AST (expressions, statements, blocks, functions, program)."


// Uvolni pole ukazatelu na vyrazy i se vsemi vyrazy v nem (napr. pole strazi u cyklu for)
static void free_expr_array(Expr **items, int count)
{
    for (int i = 0; i < count; i++) {
        expr_free(items[i]);
    }
    free(items);
}


// Rekurzivne uvolni uzel vyrazu z pameti podle jeho typu
// (literaly, promenne, unarni/binarni operace, volani, ridici konstrukce)
void expr_free(Expr *expr)
{
    if (expr == NULL) {
        return;
    }

    switch (expr->kind) {
    case EXPR_LITERAL:
        if (expr->as.literal.kind == LIT_STRING) {
            free(expr->as.literal.value.string_value);
        }
        break;
    case EXPR_VARIABLE:
        free(expr->as.variable.name);
        break;
    case EXPR_UNARY:
        expr_free(expr->as.unary.operand);
        break;
    case EXPR_BINARY:
        expr_free(expr->as.binary.left);
        expr_free(expr->as.binary.right);
        break;
    case EXPR_CALL:
        free(expr->as.call.name);
        free_expr_array(expr->as.call.args, expr->as.call.arg_count);
        break;
    case EXPR_IF:
        expr_free(expr->as.if_expr.cond);
        block_free(expr->as.if_expr.then_block);
        block_free(expr->as.if_expr.else_block);
        break;
    case EXPR_RANGE:
        expr_free(expr->as.range.from);
        expr_free(expr->as.range.to);
        expr_free(expr->as.range.step);
        break;
    case EXPR_MEMBER:
        expr_free(expr->as.member.object);
        free(expr->as.member.name);
        free_expr_array(expr->as.member.args, expr->as.member.arg_count);
        break;
    case EXPR_LIST_LITERAL:
        free_expr_array(expr->as.list_literal.items, expr->as.list_literal.count);
        break;
    case EXPR_LIST_RANGE:
        expr_free(expr->as.list_range.start);
        expr_free(expr->as.list_range.end);
        expr_free(expr->as.list_range.step);
        break;
    case EXPR_INDEX:
        expr_free(expr->as.index.list);
        expr_free(expr->as.index.index);
        break;
    }

    type_free(expr->type);
    free(expr);
}

// Rekurzivne uvolni blok prikazu z pameti, vcetne vsech vnitrnich prikazu a dynamicky alokovaneho pole
void block_free(Block *block)
{
    if (block == NULL) {
        return;
    }
    for (int i = 0; i < block->count; i++) {
        stmt_free(block->items[i]);
    }
    free(block->items);
    free(block);
}

// Rekurzivne uvolni uzel prikazu (Stmt) z pameti podle jeho druhu
// (definice, prirazeni, navrat, vyraz, while, for) vcetne vsech vnorenych struktur, poli strazi a bloku
void stmt_free(Stmt *stmt)
{
    if (stmt == NULL) {
        return;
    }
    switch (stmt->kind) {
    case STMT_VARDEF:
        free(stmt->as.vardef.name);
        type_free(stmt->as.vardef.declared_type);
        expr_free(stmt->as.vardef.init);
        break;
    case STMT_ASSIGN:
        free(stmt->as.assign.name);
        expr_free(stmt->as.assign.value);
        break;
    case STMT_RETURN:
        expr_free(stmt->as.ret.value);
        break;
    case STMT_EXPR:
        expr_free(stmt->as.expr_stmt.expr);
        break;
    case STMT_WHILE:
        expr_free(stmt->as.while_loop.cond);
        block_free(stmt->as.while_loop.body);
        break;
    case STMT_FOR:
        free(stmt->as.for_loop.var_name);
        expr_free(stmt->as.for_loop.source);
        free_expr_array(stmt->as.for_loop.guards, stmt->as.for_loop.guard_count);
        block_free(stmt->as.for_loop.body);
        break;
    }
    free(stmt);
}


// Rekurzivne uvolni definici funkce ze zive pameti, vcetne jejiho jmena,
// parametru, datovych typu, navratoveho typu a tela funkce
void funcdef_free(FuncDef *func)
{
    if (func == NULL) {
        return;
    }
    free(func->name);
    for (int i = 0; i < func->param_count; i++) {
        free(func->params[i].name);
        type_free(func->params[i].type);
    }
    free(func->params);
    type_free(func->return_type);
    block_free(func->body);
    free(func);
}


// Rekurzivne uvolni cely program (koren AST) z pameti,
// vcetne pole vsech definic funkci a jejich vnitrnich struktur => @main def run(): Unit = ...
void program_free(Program *program)
{
    if (program == NULL) {
        return;
    }
    for (int i = 0; i < program->func_count; i++) {
        funcdef_free(program->funcs[i]);
    }
    free(program->funcs);
    free(program);
}

// Vygenerovano AI (konec)
//--------------------------------------------------------------------------------//


//--------------------------------------------------------------------------------//
// Postupne sestavovani uzlu
//--------------------------------------------------------------------------------//

// Vygenerovano AI (zacatek)
// prompt: "Uprav ast.h a ast.c tak, aby parser rizeny tabulkou mohl uzel vytvorit prazdny
// a jeho casti doplnovat postupne"


// Nahradi retezec novou kopii a puvodni uvolni (NULL smaze hodnotu)
static void replace_string(char **target, const char *value)
{
    char *copy = xstrdup_opt(value);
    free(*target);
    *target = copy;
}

// Vrati parametr funkce na danem indexu, neplatny index je chyba parseru
static Param *get_param(FuncDef *func, int index)
{
    if (func == NULL || index < 0 || index >= func->param_count) {
        error_exit(ERR_INTERNAL, func == NULL ? 0 : func->line, "Neplatny index parametru pri sestavovani stromu");
    }
    return &func->params[index];
}


// Typy

// Nastavi typ prvku u typu List[T] a prebere jeho vlastnictvi
void type_list_set_element(Type *list, Type *element)
{
    if (list == NULL || list->kind != TYPE_LIST) {
        error_exit(ERR_INTERNAL, 0, "Typ prvku lze nastavit jen u seznamu");
    }
    type_free(list->element);
    list->element = element;
}


// Pomocne funkce pro bloky

// Vytvori blok s jednim prikazem a prebere jeho vlastnictvi
Block *block_from_stmt(Stmt *stmt)
{
    Block *block = block_new();
    block_add(block, stmt);
    return block;
}

// Vytvori blok s jednim prikazem, ktery je vyraz, a prebere vlastnictvi vyrazu
Block *block_from_expr(Expr *expr, int line)
{
    return block_from_stmt(stmt_new_expr(expr, line));
}


// Definice promenne

// Nastavi, jestli je promenna modifikovatelna (true pro var, false pro val)
void stmt_vardef_set_mutable(Stmt *stmt, bool is_mutable)
{
    require_stmt(stmt, STMT_VARDEF);
    stmt->as.vardef.is_mutable = is_mutable;
}

// Nastavi jmeno definovane promenne (vytvori kopii)
void stmt_vardef_set_name(Stmt *stmt, const char *name)
{
    require_stmt(stmt, STMT_VARDEF);
    replace_string(&stmt->as.vardef.name, name);
}

// Nastavi typ napsany programatorem a prebere jeho vlastnictvi
void stmt_vardef_set_type(Stmt *stmt, Type *declared_type)
{
    require_stmt(stmt, STMT_VARDEF);
    type_free(stmt->as.vardef.declared_type);
    stmt->as.vardef.declared_type = declared_type;
}

// Nastavi inicializacni vyraz a prebere jeho vlastnictvi
void stmt_vardef_set_init(Stmt *stmt, Expr *init)
{
    require_stmt(stmt, STMT_VARDEF);
    expr_free(stmt->as.vardef.init);
    stmt->as.vardef.init = init;
}


// Prirazeni, return, samostatny vyraz

// Nastavi jmeno promenne, do ktere se prirazuje (vytvori kopii)
void stmt_assign_set_name(Stmt *stmt, const char *name)
{
    require_stmt(stmt, STMT_ASSIGN);
    replace_string(&stmt->as.assign.name, name);
}

// Nastavi prirazovanou hodnotu a prebere jeji vlastnictvi
void stmt_assign_set_value(Stmt *stmt, Expr *value)
{
    require_stmt(stmt, STMT_ASSIGN);
    expr_free(stmt->as.assign.value);
    stmt->as.assign.value = value;
}

// Nastavi vracenou hodnotu a prebere jeji vlastnictvi
void stmt_return_set_value(Stmt *stmt, Expr *value)
{
    require_stmt(stmt, STMT_RETURN);
    expr_free(stmt->as.ret.value);
    stmt->as.ret.value = value;
}

// Nastavi vyraz samostatneho prikazu a prebere jeho vlastnictvi
void stmt_expr_set_expr(Stmt *stmt, Expr *expr)
{
    require_stmt(stmt, STMT_EXPR);
    expr_free(stmt->as.expr_stmt.expr);
    stmt->as.expr_stmt.expr = expr;
}


// Cykly

// Nastavi podminku cyklu while a prebere jeji vlastnictvi
void stmt_while_set_cond(Stmt *stmt, Expr *cond)
{
    require_stmt(stmt, STMT_WHILE);
    expr_free(stmt->as.while_loop.cond);
    stmt->as.while_loop.cond = cond;
}

// Nastavi telo cyklu while a prebere jeho vlastnictvi
void stmt_while_set_body(Stmt *stmt, Block *body)
{
    require_stmt(stmt, STMT_WHILE);
    block_free(stmt->as.while_loop.body);
    stmt->as.while_loop.body = body;
}

// Nastavi jmeno ridici promenne cyklu for (vytvori kopii)
void stmt_for_set_var_name(Stmt *stmt, const char *var_name)
{
    require_stmt(stmt, STMT_FOR);
    replace_string(&stmt->as.for_loop.var_name, var_name);
}

// Nastavi zdroj cyklu for a prebere jeho vlastnictvi
void stmt_for_set_source(Stmt *stmt, Expr *source)
{
    require_stmt(stmt, STMT_FOR);
    expr_free(stmt->as.for_loop.source);
    stmt->as.for_loop.source = source;
}


// Podminka if

// Nastavi podminku vyrazu if a prebere jeji vlastnictvi
void expr_if_set_cond(Expr *expr, Expr *cond)
{
    require_expr(expr, EXPR_IF);
    expr_free(expr->as.if_expr.cond);
    expr->as.if_expr.cond = cond;
}

// Nastavi vetev then a prebere jeji vlastnictvi
void expr_if_set_then(Expr *expr, Block *then_block)
{
    require_expr(expr, EXPR_IF);
    block_free(expr->as.if_expr.then_block);
    expr->as.if_expr.then_block = then_block;
}

// Nastavi vetev else a prebere jeji vlastnictvi
void expr_if_set_else(Expr *expr, Block *else_block)
{
    require_expr(expr, EXPR_IF);
    block_free(expr->as.if_expr.else_block);
    expr->as.if_expr.else_block = else_block;
}


// Funkce

// Nastavi jmeno funkce (vytvori kopii)
void funcdef_set_name(FuncDef *func, const char *name)
{
    require_func(func);
    replace_string(&func->name, name);
}

// Nastavi priznak vstupni funkce (anotace @main)
void funcdef_set_main(FuncDef *func, bool is_main)
{
    require_func(func);
    func->is_main = is_main;
}

// Nastavi jmeno parametru na danem indexu (vytvori kopii)
void funcdef_param_set_name(FuncDef *func, int index, const char *name)
{
    replace_string(&get_param(func, index)->name, name);
}

// Nastavi typ parametru na danem indexu a prebere jeho vlastnictvi
void funcdef_param_set_type(FuncDef *func, int index, Type *type)
{
    Param *param = get_param(func, index);
    type_free(param->type);
    param->type = type;
}

// Vygenerovano AI (konec)
//--------------------------------------------------------------------------------//