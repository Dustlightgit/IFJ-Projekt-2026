/*
    IFJ projekt 2026

    generator.c

    Puvodni autor: xsantar00 za asistence Claude
    Seznam zmen:
        

    Tento soubor obsahuje funkce pro generovaní finálního kódu z AST stromu.
    Dále obsahuje statické definice vestavěných funkcí které ma jazyk podporovat.

*/

/*
    Prompt pro Claude:
        Implemetuj kód odpovidající deklaracím v generator.h.
        Styl: principy YAGNI, KISS, DRY. Jména snake_case, konstanty a enum SCREAMING_SNAKE_CASE, typy PascalCase. 
        Výrazy generuj zásobníkově: každý výraz nechá hodnotu na vrcholu datového zásobníku, příkaz zanechá zásobník beze změny. 
        Hloubku zásobníku sleduj v generátoru relativně k začátku funkce a kontroluj ji po příkazech a výrazech.
        Tělo funkce generuj do bufferu přes open_memstream. 
        Nejdřív vypiš hlavičku (LABEL, CREATEFRAME, PUSHFRAME), pak všechny DEFVAR zaevidované během generování, pak tělo. 
        Proměnné jsou LF@<jméno>_<unique_id>, dočasné LF@$tN.
        Každá funkce vrací právě jednu hodnotu, Unit vrací nil@nil a volající ji zahodí do GF@$discard.
        Pokud pod návratovou hodnotou zůstávají rozpracované hodnoty, ulož ji do dočasné proměnné, přebytek popni do GF@$discard a teprve pak POPFRAME a RETURN.
        Vestavěné funkce (print, toStr, readLine, readInt, readDouble, readBoolean, toInt, strLength, subStr, ord, chr) 
        a pomocné rutiny (double$floor, list$get, list$range, list$slice, list$tail, list$concat) jsou funkce v cílovém kódu. 
        Emituj jen ty, které program použil, včetně závislostí.
        print a toStr rozlišují typ za běhu instrukcí TYP.
        Běhové chyby skáčou na sdílené návěští s EXIT int@40
        Seznam je pole: prvek 0 drží délku, prvky od indexu 1.
        ++, take, drop, slice, head, tail a toString řeš přes rutiny nebo toStr.
        Podporuj rozšíření BOOLTHEN, CONTROL (if jako výraz, for, rozsahy), FUNEXP, VALMEM a LISTS.
        Int je 64bitový, přetečení neřeš. Int se převádí na Double jen instrukcí INT2FLOATS v místech, kde se čeká Double.
        Double literály píš ve formátu %a. Řetězcové literály escapuj jako \xyz pro znaky 0–32, 35, 92 a od 127. 
        Záporná čísla generuj jako násobení -1.
*/


/*
 * generator.c - generator cilového kódu IFJcode26 z AST
 *
 * Popis návrhu je na začátku generator.h. Soubor je rozdělen na:
 *  1. rutiny v cílovém kódu (vestavěné funkce a práce se seznamy),
 *  2. stav generátoru a základní zápis instrukcí,
 *  3. jména, literály a konverze,
 *  4. program a funkce,
 *  5. bloky a příkazy,
 *  6. výrazy.
 */

#define _POSIX_C_SOURCE 200809L /* open_memstream a strdup */

#include "generator.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "dstring.h"
#include "error.h"
#include "stack.h"


#define NAME_BUF 256  /* velikost bufferů pro jména proměnných a funkcí */
#define LABEL_BUF 64  /* velikost bufferů pro generovaná návěští */
#define DISCARD_VAR "GF@$discard"
#define ERROR_LABEL "$error$40" /* sdílené návěští pro běhovou chybu 40 */
#define INT_MAX_LITERAL "int@2147483647"


/*--------------------------------------------------------------------------------*/
/* 1. Rutiny v cílovém kódu                                                       */
/*--------------------------------------------------------------------------------*/

/*
 * Každá rutina je funkce s voláním stejným jako u uživatelských funkcí:
 * argumenty jsou na datovém zásobníku (poslední argument nahoře) a výsledek
 * se vrací na zásobník. Návěští "LABEL $jmeno" přidává gen_emit_used_runtime,
 * tělo začíná vytvořením rámce. Všechny DEFVAR jsou hned na začátku, aby se
 * nikdy nevykonaly dvakrát.
 */

#define FN_BEGIN "CREATEFRAME\nPUSHFRAME\n"
#define FN_END "POPFRAME\nRETURN\n"
#define DEF_VAR(v) "DEFVAR LF@" v "\n"
#define POP_VAR(v) "DEFVAR LF@" v "\nPOPS LF@" v "\n"

/* Rutina s jedním parametrem v, která výsledek spočítá jedinou instrukcí. */
#define UNARY_BODY(instr) \
    FN_BEGIN POP_VAR("v") DEF_VAR("r") \
    instr " LF@r LF@v\n" \
    "PUSHS LF@r\n" FN_END

/* Čtení ze vstupu: při nil (chybný vstup nebo EOF) se dosadí výchozí hodnota. */
#define READ_BODY(name, type, default_value) \
    FN_BEGIN DEF_VAR("r") DEF_VAR("t") \
    "READ LF@r " type "\n" \
    "TYPE LF@t LF@r\n" \
    "JUMPIFNEQ $" name "$end LF@t string@nil\n" \
    "MOVE LF@r " default_value "\n" \
    "LABEL $" name "$end\n" \
    "PUSHS LF@r\n" FN_END

static const char BODY_PRINT[] =
    FN_BEGIN POP_VAR("v") DEF_VAR("t")
    "TYPE LF@t LF@v\n"
    "JUMPIFEQ $print$nil LF@t string@nil\n"
    "JUMPIFEQ $print$array LF@t string@array\n"
    "WRITE LF@v\n"
    "JUMP $print$end\n"
    "LABEL $print$nil\n"
    "WRITE string@()\n"
    "JUMP $print$end\n"
    "LABEL $print$array\n"
    "PUSHS LF@v\n"
    "CALL $toStr\n"
    "POPS LF@t\n"
    "WRITE LF@t\n"
    "LABEL $print$end\n"
    "PUSHS nil@nil\n"
    FN_END;

static const char BODY_TO_STR[] =
    FN_BEGIN POP_VAR("v")
    DEF_VAR("t") DEF_VAR("r") DEF_VAR("n") DEF_VAR("i") DEF_VAR("c")
    DEF_VAR("e") DEF_VAR("s")
    "TYPE LF@t LF@v\n"
    "JUMPIFEQ $toStr$int LF@t string@int\n"
    "JUMPIFEQ $toStr$float LF@t string@float\n"
    "JUMPIFEQ $toStr$bool LF@t string@bool\n"
    "JUMPIFEQ $toStr$nil LF@t string@nil\n"
    "JUMPIFEQ $toStr$array LF@t string@array\n"
    "MOVE LF@r LF@v\n"
    "JUMP $toStr$end\n"
    "LABEL $toStr$int\n"
    "INT2STR LF@r LF@v\n"
    "JUMP $toStr$end\n"
    "LABEL $toStr$float\n"
    "FLOAT2STR LF@r LF@v\n"
    "JUMP $toStr$end\n"
    "LABEL $toStr$bool\n"
    "MOVE LF@r string@false\n"
    "JUMPIFEQ $toStr$end LF@v bool@false\n"
    "MOVE LF@r string@true\n"
    "JUMP $toStr$end\n"
    "LABEL $toStr$nil\n"
    "MOVE LF@r string@()\n"
    "JUMP $toStr$end\n"
    "LABEL $toStr$array\n"
    "ARRAYGET LF@n LF@v int@0\n"
    "MOVE LF@r string@List(\n"
    "MOVE LF@i int@1\n"
    "LABEL $toStr$loop\n"
    "GT LF@c LF@i LF@n\n"
    "JUMPIFEQ $toStr$close LF@c bool@true\n"
    "JUMPIFEQ $toStr$first LF@i int@1\n"
    "CONCAT LF@r LF@r string@,\\032\n"
    "LABEL $toStr$first\n"
    "ARRAYGET LF@e LF@v LF@i\n"
    "PUSHS LF@e\n"
    "CALL $toStr\n"
    "POPS LF@s\n"
    "CONCAT LF@r LF@r LF@s\n"
    "ADD LF@i LF@i int@1\n"
    "JUMP $toStr$loop\n"
    "LABEL $toStr$close\n"
    "CONCAT LF@r LF@r string@)\n"
    "LABEL $toStr$end\n"
    "PUSHS LF@r\n"
    FN_END;

static const char BODY_READ_LINE[] = READ_BODY("readLine", "string", "string@");
static const char BODY_READ_INT[] = READ_BODY("readInt", "int", "int@0");
static const char BODY_READ_DOUBLE[] = READ_BODY("readDouble", "float", "float@0x0p+0");
static const char BODY_READ_BOOLEAN[] = READ_BODY("readBoolean", "bool", "bool@false");

static const char BODY_TO_INT[] = UNARY_BODY("FLOAT2INT");
static const char BODY_STR_LENGTH[] = UNARY_BODY("STRLEN");
static const char BODY_CHR[] = UNARY_BODY("INT2CHAR");

/* subStr: pět podmínek ze specifikace vede na běhovou chybu 40. */
static const char BODY_SUB_STR[] =
    FN_BEGIN POP_VAR("e") POP_VAR("s") POP_VAR("v")
    DEF_VAR("len") DEF_VAR("c") DEF_VAR("r") DEF_VAR("ch")
    "STRLEN LF@len LF@v\n"
    "LT LF@c LF@s int@0\n"
    "JUMPIFEQ " ERROR_LABEL " LF@c bool@true\n"
    "LT LF@c LF@e int@0\n"
    "JUMPIFEQ " ERROR_LABEL " LF@c bool@true\n"
    "GT LF@c LF@s LF@e\n"
    "JUMPIFEQ " ERROR_LABEL " LF@c bool@true\n"
    "LT LF@c LF@s LF@len\n"
    "JUMPIFEQ " ERROR_LABEL " LF@c bool@false\n"
    "GT LF@c LF@e LF@len\n"
    "JUMPIFEQ " ERROR_LABEL " LF@c bool@true\n"
    "MOVE LF@r string@\n"
    "LABEL $subStr$loop\n"
    "JUMPIFEQ $subStr$done LF@s LF@e\n"
    "GETCHAR LF@ch LF@v LF@s\n"
    "CONCAT LF@r LF@r LF@ch\n"
    "ADD LF@s LF@s int@1\n"
    "JUMP $subStr$loop\n"
    "LABEL $subStr$done\n"
    "PUSHS LF@r\n"
    FN_END;

/* ord: prázdný řetězec nebo index mimo rozsah vrací -1. */
static const char BODY_ORD[] =
    FN_BEGIN POP_VAR("i") POP_VAR("v")
    DEF_VAR("len") DEF_VAR("c") DEF_VAR("r")
    "MOVE LF@r int@-1\n"
    "STRLEN LF@len LF@v\n"
    "LT LF@c LF@i int@0\n"
    "JUMPIFEQ $ord$end LF@c bool@true\n"
    "LT LF@c LF@i LF@len\n"
    "JUMPIFEQ $ord$end LF@c bool@false\n"
    "STRI2INT LF@r LF@v LF@i\n"
    "LABEL $ord$end\n"
    "PUSHS LF@r\n"
    FN_END;

/* floor: FLOAT2INT osekává k nule, u záporných necelých čísel se proto odečte 1. */
static const char BODY_FLOOR[] =
    FN_BEGIN POP_VAR("v")
    DEF_VAR("i") DEF_VAR("f") DEF_VAR("c")
    "FLOAT2INT LF@i LF@v\n"
    "INT2FLOAT LF@f LF@i\n"
    "GT LF@c LF@f LF@v\n"
    "JUMPIFEQ $double$floor$done LF@c bool@false\n"
    "SUB LF@i LF@i int@1\n"
    "LABEL $double$floor$done\n"
    "INT2FLOAT LF@f LF@i\n"
    "PUSHS LF@f\n"
    FN_END;

/* list$get(list, index): index mimo meze je běhová chyba 40. */
static const char BODY_LIST_GET[] =
    FN_BEGIN POP_VAR("i") POP_VAR("l")
    DEF_VAR("n") DEF_VAR("c") DEF_VAR("r")
    "ARRAYGET LF@n LF@l int@0\n"
    "LT LF@c LF@i int@0\n"
    "JUMPIFEQ " ERROR_LABEL " LF@c bool@true\n"
    "LT LF@c LF@i LF@n\n"
    "JUMPIFEQ " ERROR_LABEL " LF@c bool@false\n"
    "ADD LF@i LF@i int@1\n"
    "ARRAYGET LF@r LF@l LF@i\n"
    "PUSHS LF@r\n"
    FN_END;

/*
 * list$range(start, end, step, inclusive): seznam od start po end (vyjma end,
 * nebo včetně při inclusive) s krokem step. Pravý okraj se u inclusive posune
 * o jedna ve směru kroku, pak se počet prvků spočítá stejně jako u exkluzivního
 * rozsahu. Nulový krok je běžná chyba 40.
 */
static const char BODY_LIST_RANGE[] =
    FN_BEGIN POP_VAR("incl") POP_VAR("step") POP_VAR("end") POP_VAR("start")
    DEF_VAR("c") DEF_VAR("n") DEF_VAR("a") DEF_VAR("i") DEF_VAR("v") DEF_VAR("r")
    "JUMPIFEQ " ERROR_LABEL " LF@step int@0\n"
    "JUMPIFEQ $list$range$count LF@incl bool@false\n"
    "GT LF@c LF@step int@0\n"
    "JUMPIFEQ $list$range$incl_down LF@c bool@false\n"
    "ADD LF@end LF@end int@1\n"
    "JUMP $list$range$count\n"
    "LABEL $list$range$incl_down\n"
    "SUB LF@end LF@end int@1\n"
    "LABEL $list$range$count\n"
    "MOVE LF@n int@0\n"
    "GT LF@c LF@step int@0\n"
    "JUMPIFEQ $list$range$down LF@c bool@false\n"
    "LT LF@c LF@start LF@end\n"
    "JUMPIFEQ $list$range$alloc LF@c bool@false\n"
    "SUB LF@n LF@end LF@start\n"
    "ADD LF@n LF@n LF@step\n"
    "SUB LF@n LF@n int@1\n"
    "IDIV LF@n LF@n LF@step\n"
    "JUMP $list$range$alloc\n"
    "LABEL $list$range$down\n"
    "GT LF@c LF@start LF@end\n"
    "JUMPIFEQ $list$range$alloc LF@c bool@false\n"
    "MUL LF@a LF@step int@-1\n"
    "SUB LF@n LF@start LF@end\n"
    "ADD LF@n LF@n LF@a\n"
    "SUB LF@n LF@n int@1\n"
    "IDIV LF@n LF@n LF@a\n"
    "LABEL $list$range$alloc\n"
    "ADD LF@a LF@n int@1\n"
    "NEWARRAY LF@r LF@a\n"
    "ARRAYSET LF@r int@0 LF@n\n"
    "MOVE LF@i int@1\n"
    "MOVE LF@v LF@start\n"
    "LABEL $list$range$fill\n"
    "GT LF@c LF@i LF@n\n"
    "JUMPIFEQ $list$range$done LF@c bool@true\n"
    "ARRAYSET LF@r LF@i LF@v\n"
    "ADD LF@v LF@v LF@step\n"
    "ADD LF@i LF@i int@1\n"
    "JUMP $list$range$fill\n"
    "LABEL $list$range$done\n"
    "PUSHS LF@r\n"
    FN_END;

/*
 * list$slice(list, from, until): prvky s indexy z intervalu <from, until),
 * meze se oříznou na <0, délka>. Pomocí ní jsou řešené také take a drop.
 */
static const char BODY_LIST_SLICE[] =
    FN_BEGIN POP_VAR("u") POP_VAR("f") POP_VAR("l")
    DEF_VAR("n") DEF_VAR("c") DEF_VAR("m") DEF_VAR("s") DEF_VAR("i")
    DEF_VAR("e") DEF_VAR("x") DEF_VAR("r")
    "ARRAYGET LF@n LF@l int@0\n"
    "LT LF@c LF@f int@0\n"
    "JUMPIFEQ $list$slice$hi LF@c bool@false\n"
    "MOVE LF@f int@0\n"
    "LABEL $list$slice$hi\n"
    "GT LF@c LF@u LF@n\n"
    "JUMPIFEQ $list$slice$size LF@c bool@false\n"
    "MOVE LF@u LF@n\n"
    "LABEL $list$slice$size\n"
    "SUB LF@m LF@u LF@f\n"
    "LT LF@c LF@m int@0\n"
    "JUMPIFEQ $list$slice$alloc LF@c bool@false\n"
    "MOVE LF@m int@0\n"
    "LABEL $list$slice$alloc\n"
    "ADD LF@s LF@m int@1\n"
    "NEWARRAY LF@r LF@s\n"
    "ARRAYSET LF@r int@0 LF@m\n"
    "MOVE LF@i int@1\n"
    "LABEL $list$slice$copy\n"
    "GT LF@c LF@i LF@m\n"
    "JUMPIFEQ $list$slice$done LF@c bool@true\n"
    "ADD LF@e LF@f LF@i\n"
    "ARRAYGET LF@x LF@l LF@e\n"
    "ARRAYSET LF@r LF@i LF@x\n"
    "ADD LF@i LF@i int@1\n"
    "JUMP $list$slice$copy\n"
    "LABEL $list$slice$done\n"
    "PUSHS LF@r\n"
    FN_END;

/* list$tail(list): prázdný seznam je běžná chyba 40, jinak slice od indexu 1. */
static const char BODY_LIST_TAIL[] =
    FN_BEGIN POP_VAR("l") DEF_VAR("n")
    "ARRAYGET LF@n LF@l int@0\n"
    "JUMPIFEQ " ERROR_LABEL " LF@n int@0\n"
    "PUSHS LF@l\n"
    "PUSHS int@1\n"
    "PUSHS " INT_MAX_LITERAL "\n"
    "CALL $list$slice\n"
    FN_END;

/* list$concat(a, b): nový seznam s prvky a a za nimi prvky b. */
static const char BODY_LIST_CONCAT[] =
    FN_BEGIN POP_VAR("b") POP_VAR("a")
    DEF_VAR("na") DEF_VAR("nb") DEF_VAR("m") DEF_VAR("s") DEF_VAR("c")
    DEF_VAR("i") DEF_VAR("j") DEF_VAR("d") DEF_VAR("x") DEF_VAR("r")
    "ARRAYGET LF@na LF@a int@0\n"
    "ARRAYGET LF@nb LF@b int@0\n"
    "ADD LF@m LF@na LF@nb\n"
    "ADD LF@s LF@m int@1\n"
    "NEWARRAY LF@r LF@s\n"
    "ARRAYSET LF@r int@0 LF@m\n"
    "MOVE LF@i int@1\n"
    "LABEL $list$concat$loop_a\n"
    "GT LF@c LF@i LF@na\n"
    "JUMPIFEQ $list$concat$second LF@c bool@true\n"
    "ARRAYGET LF@x LF@a LF@i\n"
    "ARRAYSET LF@r LF@i LF@x\n"
    "ADD LF@i LF@i int@1\n"
    "JUMP $list$concat$loop_a\n"
    "LABEL $list$concat$second\n"
    "MOVE LF@j int@1\n"
    "LABEL $list$concat$loop_b\n"
    "GT LF@c LF@j LF@nb\n"
    "JUMPIFEQ $list$concat$done LF@c bool@true\n"
    "ARRAYGET LF@x LF@b LF@j\n"
    "ADD LF@d LF@na LF@j\n"
    "ARRAYSET LF@r LF@d LF@x\n"
    "ADD LF@j LF@j int@1\n"
    "JUMP $list$concat$loop_b\n"
    "LABEL $list$concat$done\n"
    "PUSHS LF@r\n"
    FN_END;

/* Záznam v tabulce rutin. */
typedef struct {
    const char *name;  /* jméno rutiny, návěští je "$" + jméno */
    const char *needs; /* rutina, kterou tato rutina volá, jinak NULL */
    const char *body;  /* tělo v IFJcode26 včetně vytvoření a ukončení rámce */
} Runtime;

static const Runtime RUNTIME[] = {
    {"print", "toStr", BODY_PRINT},
    {"toStr", NULL, BODY_TO_STR},
    {"readLine", NULL, BODY_READ_LINE},
    {"readInt", NULL, BODY_READ_INT},
    {"readDouble", NULL, BODY_READ_DOUBLE},
    {"readBoolean", NULL, BODY_READ_BOOLEAN},
    {"toInt", NULL, BODY_TO_INT},
    {"strLength", NULL, BODY_STR_LENGTH},
    {"subStr", NULL, BODY_SUB_STR},
    {"ord", NULL, BODY_ORD},
    {"chr", NULL, BODY_CHR},
    {"double$floor", NULL, BODY_FLOOR},
    {"list$get", NULL, BODY_LIST_GET},
    {"list$range", NULL, BODY_LIST_RANGE},
    {"list$slice", NULL, BODY_LIST_SLICE},
    {"list$tail", "list$slice", BODY_LIST_TAIL},
    {"list$concat", NULL, BODY_LIST_CONCAT},
};

#define RUNTIME_COUNT (sizeof RUNTIME / sizeof RUNTIME[0])


/*--------------------------------------------------------------------------------*/
/* 2. Stav generátoru a základní zápis instrukcí                                  */
/*--------------------------------------------------------------------------------*/

struct Gen {
    FILE *out;                    /* výstup programu */
    FILE *cur;                    /* aktuální cíl zápisu: out nebo buffer těla funkce */
    Stack vars;                   /* proměnné aktuální funkce k deklaraci (char *) */
    int depth;                    /* hloubka datového zásobníku vůči začátku funkce */
    int label_counter;
    int temp_counter;
    const FuncDef *func;          /* funkce, jejíž tělo se právě generuje */
    bool used[RUNTIME_COUNT];     /* které rutiny program použil */
};

/* Typ Double pro konverze, aby nebylo nutné ho alokovat. */
static const Type DOUBLE_TYPE = {TYPE_DOUBLE, NULL};

void gen_emit(Gen *g, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(g->cur, fmt, args);
    va_end(args);
    fputc('\n', g->cur);
}

void gen_push(Gen *g, const char *symb) {
    gen_emit(g, "PUSHS %s", symb);
    g->depth++;
}

void gen_pop(Gen *g, const char *var) {
    gen_emit(g, "POPS %s", var);
    g->depth--;
}

void gen_discard(Gen *g) {
    gen_pop(g, DISCARD_VAR);
}

void gen_stack_op(Gen *g, const char *op, int consumed, int produced) {
    gen_emit(g, "%s", op);
    g->depth += produced - consumed;
}

void gen_check_depth(const Gen *g, int expected, int line) {
    if (g->depth != expected) {
        error_exit(ERR_INTERNAL, line, "Nesouhlasi hloubka datoveho zasobniku v generatoru");
    }
}

/* Zapíše JUMPIFEQS na label, který odebere ze zásobníku dvě hodnoty. */
static void emit_jumpifeqs(Gen *g, const char *label) {
    char op[LABEL_BUF + 16];
    snprintf(op, sizeof op, "JUMPIFEQS %s", label);
    gen_stack_op(g, op, 2, 0);
}


/*--------------------------------------------------------------------------------*/
/* 3. Jména, literály a konverze                                                  */
/*--------------------------------------------------------------------------------*/

void gen_make_label(Gen *g, char *buf, size_t size, const char *prefix) {
    g->label_counter++;
    snprintf(buf, size, "$%s$%d", prefix, g->label_counter);
}

void gen_function_label(char *buf, size_t size, const char *name) {
    snprintf(buf, size, "$%s", name);
}

void gen_var_name(char *buf, size_t size, const Symbol *symbol) {
    snprintf(buf, size, "LF@%s_%d", symbol->name, symbol->as.var.unique_id);
}

void gen_declare_var(Gen *g, const char *var) {
    char *copy = strdup(var);
    if (copy == NULL) {
        error_exit(ERR_INTERNAL, 0, "Nedostatek pameti");
    }
    stack_push(&g->vars, &copy);
}

void gen_new_temp(Gen *g, char *buf, size_t size) {
    g->temp_counter++;
    snprintf(buf, size, "LF@$t%d", g->temp_counter);
    gen_declare_var(g, buf);
}

/* Přidá celý řetězec na konec dynamického řetězce. */
static void dstr_append_str(DString *s, const char *text) {
    for (; *text != '\0'; text++) {
        dstr_append_char(s, *text);
    }
}

/* Pushne řetězcový literál se zakódovanými escape sekvencemi. */
static void push_string_literal(Gen *g, const char *text) {
    DString literal;
    dstr_init(&literal);
    dstr_append_str(&literal, "string@");

    for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; p++) {
        if (*p <= 32 || *p == 35 || *p == 92 || *p >= 127) {
            char escape[8];
            snprintf(escape, sizeof escape, "\\%03d", *p);
            dstr_append_str(&literal, escape);
        } else {
            dstr_append_char(&literal, (char)*p);
        }
    }

    gen_push(g, literal.data);
    dstr_free(&literal);
}

void gen_push_literal(Gen *g, const Literal *literal) {
    char buf[64];

    switch (literal->kind) {
    case LIT_INT:
        snprintf(buf, sizeof buf, "int@%d", literal->value.int_value);
        gen_push(g, buf);
        break;
    case LIT_DOUBLE:
        snprintf(buf, sizeof buf, "float@%a", literal->value.double_value);
        gen_push(g, buf);
        break;
    case LIT_STRING:
        push_string_literal(g, literal->value.string_value);
        break;
    case LIT_BOOL:
        gen_push(g, literal->value.bool_value ? "bool@true" : "bool@false");
        break;
    case LIT_UNIT:
        gen_push(g, "nil@nil");
        break;
    }
}

void gen_convert(Gen *g, const Type *from, const Type *to) {
    if (from != NULL && to != NULL && from->kind == TYPE_INT && to->kind == TYPE_DOUBLE) {
        gen_stack_op(g, "INT2FLOATS", 1, 1);
    }
}


/*--------------------------------------------------------------------------------*/
/* 4. Program a funkce                                                            */
/*--------------------------------------------------------------------------------*/

void gen_entry(Gen *g, const FuncDef *main_func) {
    char label[NAME_BUF];
    gen_function_label(label, sizeof label, main_func->name);

    gen_emit(g, ".IFJcode26");
    gen_emit(g, "");
    gen_emit(g, "DEFVAR %s", DISCARD_VAR);
    gen_emit(g, "CALL %s", label);
    gen_emit(g, "POPS %s", DISCARD_VAR);
    gen_emit(g, "EXIT int@0");
    gen_emit(g, "");
    gen_emit(g, "LABEL %s", ERROR_LABEL);
    gen_emit(g, "EXIT int@40");
    gen_emit(g, "");
}

void gen_return_sequence(Gen *g) {
    int excess = g->depth - 1; /* hodnoty pod návratovou hodnotou */

    if (excess > 0) {
        char value[NAME_BUF];
        gen_new_temp(g, value, sizeof value);
        gen_emit(g, "POPS %s", value);
        for (int i = 0; i < excess; i++) {
            gen_emit(g, "POPS %s", DISCARD_VAR);
        }
        gen_emit(g, "PUSHS %s", value);
    }

    gen_emit(g, "POPFRAME");
    gen_emit(g, "RETURN");
}

/* Vypíše DEFVAR všech zaevidovaných proměnných funkce a uvolní seznam. */
static void flush_declarations(Gen *g) {
    char **vars = g->vars.data;

    for (size_t i = 0; i < g->vars.count; i++) {
        gen_emit(g, "DEFVAR %s", vars[i]);
        free(vars[i]);
    }
    stack_clear(&g->vars);
}

void gen_function(Gen *g, const FuncDef *func) {
    char *body = NULL;
    size_t body_size = 0;
    char label[NAME_BUF];

    g->func = func;
    g->depth = func->param_count; /* argumenty leží na zásobníku */
    g->cur = open_memstream(&body, &body_size);
    if (g->cur == NULL) {
        error_exit(ERR_INTERNAL, func->line, "Nedostatek pameti");
    }

    /* Poslední argument je nahoře, proto se parametry popují od posledního. */
    for (int i = func->param_count - 1; i >= 0; i--) {
        char name[NAME_BUF];
        gen_var_name(name, sizeof name, func->params[i].symbol);
        gen_declare_var(g, name);
        gen_pop(g, name);
    }

    const Type *ret = func->symbol->as.func.return_type;
    if (ret->kind == TYPE_UNIT) {
        gen_block(g, func->body, NULL);
        gen_push(g, "nil@nil");
    } else {
        gen_block(g, func->body, ret);
    }
    gen_check_depth(g, 1, func->line);
    gen_return_sequence(g);

    fclose(g->cur);
    g->cur = g->out;

    gen_function_label(label, sizeof label, func->name);
    gen_emit(g, "\nLABEL %s", label);
    gen_emit(g, "CREATEFRAME");
    gen_emit(g, "PUSHFRAME");
    flush_declarations(g);
    fwrite(body, 1, body_size, g->out);
    free(body);
}


/*--------------------------------------------------------------------------------*/
/* Pomocné rutiny                                                                 */
/*--------------------------------------------------------------------------------*/

/* Vrátí index rutiny v tabulce. Neznámé jméno je chyba generátoru. */
static size_t find_runtime(const char *name) {
    for (size_t i = 0; i < RUNTIME_COUNT; i++) {
        if (strcmp(RUNTIME[i].name, name) == 0) {
            return i;
        }
    }
    error_exit(ERR_INTERNAL, 0, "Neznama pomocna rutina v generatoru");
    return 0;
}

void gen_use_runtime(Gen *g, const char *name) {
    size_t index = find_runtime(name);

    if (g->used[index]) {
        return;
    }
    g->used[index] = true;
    if (RUNTIME[index].needs != NULL) {
        gen_use_runtime(g, RUNTIME[index].needs);
    }
}

void gen_call(Gen *g, const char *label, int argc) {
    gen_emit(g, "CALL %s", label);
    g->depth += 1 - argc;
}

void gen_call_runtime(Gen *g, const char *name, int argc) {
    char label[NAME_BUF];

    gen_use_runtime(g, name);
    gen_function_label(label, sizeof label, name);
    gen_call(g, label, argc);
}

void gen_emit_used_runtime(Gen *g) {
    for (size_t i = 0; i < RUNTIME_COUNT; i++) {
        if (g->used[i]) {
            char label[NAME_BUF];
            gen_function_label(label, sizeof label, RUNTIME[i].name);
            gen_emit(g, "\nLABEL %s", label);
            fputs(RUNTIME[i].body, g->cur);
        }
    }
}


/*--------------------------------------------------------------------------------*/
/* 5. Bloky a příkazy                                                             */
/*--------------------------------------------------------------------------------*/

void gen_block(Gen *g, const Block *block, const Type *want) {
    for (int i = 0; i < block->count; i++) {
        const Stmt *stmt = block->items[i];
        bool is_last = (i == block->count - 1);

        if (is_last && want != NULL && stmt->kind == STMT_EXPR) {
            gen_expr_as(g, stmt->as.expr_stmt.expr, want);
            continue;
        }

        gen_stmt(g, stmt);
        if (is_last && want != NULL) {
            gen_push(g, "nil@nil"); /* blok nekončí výrazem, výsledek je Unit */
        }
    }
}

void gen_stmt(Gen *g, const Stmt *stmt) {
    int depth = g->depth;

    switch (stmt->kind) {
    case STMT_EXPR:
        gen_expr(g, stmt->as.expr_stmt.expr);
        gen_discard(g);
        break;
    case STMT_VARDEF:
        gen_stmt_vardef(g, stmt);
        break;
    case STMT_ASSIGN:
        gen_stmt_assign(g, stmt);
        break;
    case STMT_RETURN:
        gen_stmt_return(g, stmt);
        break;
    case STMT_WHILE:
        gen_stmt_while(g, stmt);
        break;
    case STMT_FOR:
        gen_stmt_for(g, stmt);
        break;
    default:
        error_exit(ERR_INTERNAL, stmt->line, "Nepodporovany prikaz v generatoru");
    }

    gen_check_depth(g, depth, stmt->line);
}

void gen_stmt_vardef(Gen *g, const Stmt *stmt) {
    const Symbol *symbol = stmt->as.vardef.symbol;
    char name[NAME_BUF];

    gen_var_name(name, sizeof name, symbol);
    gen_declare_var(g, name);
    gen_expr_as(g, stmt->as.vardef.init, symbol->as.var.type);
    gen_pop(g, name);
}

void gen_stmt_assign(Gen *g, const Stmt *stmt) {
    const Symbol *symbol = stmt->as.assign.symbol;
    char name[NAME_BUF];

    gen_var_name(name, sizeof name, symbol);
    gen_expr_as(g, stmt->as.assign.value, symbol->as.var.type);
    gen_pop(g, name);
}

void gen_stmt_return(Gen *g, const Stmt *stmt) {
    const Type *ret = g->func->symbol->as.func.return_type;
    const Expr *value = stmt->as.ret.value;
    int depth = g->depth;

    if (ret->kind == TYPE_UNIT) {
        /* Hodnota se vyhodnotí a zahodí, funkce vrací nil@nil. */
        if (value != NULL) {
            gen_expr(g, value);
            gen_discard(g);
        }
        gen_push(g, "nil@nil");
    } else {
        gen_expr_as(g, value, ret);
    }

    gen_return_sequence(g);
    g->depth = depth; /* kód za returnem je jiný tok řízení */
}

void gen_stmt_while(Gen *g, const Stmt *stmt) {
    char start[LABEL_BUF], end[LABEL_BUF];

    gen_make_label(g, start, sizeof start, "while");
    gen_make_label(g, end, sizeof end, "endwhile");

    gen_emit(g, "LABEL %s", start);
    gen_jump_if_false(g, stmt->as.while_loop.cond, end);
    gen_block(g, stmt->as.while_loop.body, NULL);
    gen_emit(g, "JUMP %s", start);
    gen_emit(g, "LABEL %s", end);
}

void gen_stmt_for(Gen *g, const Stmt *stmt) {
    if (stmt->as.for_loop.source->kind == EXPR_RANGE) {
        gen_for_range(g, stmt);
    } else {
        gen_for_list(g, stmt);
    }
}

void gen_for_guards(Gen *g, const Stmt *stmt, const char *skip_label) {
    for (int i = 0; i < stmt->as.for_loop.guard_count; i++) {
        gen_jump_if_false(g, stmt->as.for_loop.guards[i], skip_label);
    }
}

/*
 * Zapíše test konce rozsahu: porovná čítač s mezí instrukcí op a skočí na done,
 * když výsledek porovnání je expect.
 */
static void emit_range_exit(Gen *g, const char *op, bool expect, const char *var,
                            const char *end, const char *cond, const char *done) {
    gen_emit(g, "%s %s %s %s", op, cond, var, end);
    gen_emit(g, "JUMPIFEQ %s %s bool@%s", done, cond, expect ? "true" : "false");
}

void gen_for_range(Gen *g, const Stmt *stmt) {
    const Expr *range = stmt->as.for_loop.source;
    bool inclusive = range->as.range.is_inclusive;
    char var[NAME_BUF], end[NAME_BUF], step[NAME_BUF], cond[NAME_BUF];
    char check[LABEL_BUF], up[LABEL_BUF], body[LABEL_BUF], next[LABEL_BUF], done[LABEL_BUF];

    gen_var_name(var, sizeof var, stmt->as.for_loop.symbol);
    gen_declare_var(g, var);
    gen_new_temp(g, end, sizeof end);
    gen_new_temp(g, step, sizeof step);
    gen_new_temp(g, cond, sizeof cond);
    gen_make_label(g, check, sizeof check, "for_check");
    gen_make_label(g, up, sizeof up, "for_up");
    gen_make_label(g, body, sizeof body, "for_body");
    gen_make_label(g, next, sizeof next, "for_next");
    gen_make_label(g, done, sizeof done, "for_done");

    /* Meze a krok se vyhodnotí jednou, zleva doprava. Řídicí proměnná je zároveň čítač. */
    gen_expr(g, range->as.range.from);
    gen_pop(g, var);
    gen_expr(g, range->as.range.to);
    gen_pop(g, end);
    if (range->as.range.step != NULL) {
        gen_expr(g, range->as.range.step);
        gen_pop(g, step);
    } else {
        gen_emit(g, "MOVE %s int@1", step);
    }
    gen_emit(g, "JUMPIFEQ %s %s int@0", ERROR_LABEL, step);

    /* Směr se určí za běhu podle znaménka kroku. */
    gen_emit(g, "LABEL %s", check);
    gen_emit(g, "GT %s %s int@0", cond, step);
    gen_emit(g, "JUMPIFEQ %s %s bool@true", up, cond);
    emit_range_exit(g, inclusive ? "LT" : "GT", inclusive, var, end, cond, done);
    gen_emit(g, "JUMP %s", body);
    gen_emit(g, "LABEL %s", up);
    emit_range_exit(g, inclusive ? "GT" : "LT", inclusive, var, end, cond, done);

    gen_emit(g, "LABEL %s", body);
    gen_for_guards(g, stmt, next);
    gen_block(g, stmt->as.for_loop.body, NULL);
    gen_emit(g, "LABEL %s", next);
    gen_emit(g, "ADD %s %s %s", var, var, step);
    gen_emit(g, "JUMP %s", check);
    gen_emit(g, "LABEL %s", done);
}

void gen_for_list(Gen *g, const Stmt *stmt) {
    char var[NAME_BUF], list[NAME_BUF], size[NAME_BUF], index[NAME_BUF], cond[NAME_BUF];
    char check[LABEL_BUF], next[LABEL_BUF], done[LABEL_BUF];

    gen_var_name(var, sizeof var, stmt->as.for_loop.symbol);
    gen_declare_var(g, var);
    gen_new_temp(g, list, sizeof list);
    gen_new_temp(g, size, sizeof size);
    gen_new_temp(g, index, sizeof index);
    gen_new_temp(g, cond, sizeof cond);
    gen_make_label(g, check, sizeof check, "for_check");
    gen_make_label(g, next, sizeof next, "for_next");
    gen_make_label(g, done, sizeof done, "for_done");

    gen_expr(g, stmt->as.for_loop.source);
    gen_pop(g, list);
    gen_emit(g, "ARRAYGET %s %s int@0", size, list);
    gen_emit(g, "MOVE %s int@1", index);

    gen_emit(g, "LABEL %s", check);
    gen_emit(g, "GT %s %s %s", cond, index, size);
    gen_emit(g, "JUMPIFEQ %s %s bool@true", done, cond);
    gen_emit(g, "ARRAYGET %s %s %s", var, list, index);
    gen_for_guards(g, stmt, next);
    gen_block(g, stmt->as.for_loop.body, NULL);
    gen_emit(g, "LABEL %s", next);
    gen_emit(g, "ADD %s %s int@1", index, index);
    gen_emit(g, "JUMP %s", check);
    gen_emit(g, "LABEL %s", done);
}


/*--------------------------------------------------------------------------------*/
/* 6. Výrazy                                                                      */
/*--------------------------------------------------------------------------------*/

void gen_expr(Gen *g, const Expr *expr) {
    int depth = g->depth;

    switch (expr->kind) {
    case EXPR_LITERAL:
        gen_push_literal(g, &expr->as.literal);
        break;
    case EXPR_VARIABLE: {
        char name[NAME_BUF];
        gen_var_name(name, sizeof name, expr->as.variable.symbol);
        gen_push(g, name);
        break;
    }
    case EXPR_UNARY:
        gen_expr_unary(g, expr);
        break;
    case EXPR_BINARY:
        gen_expr_binary(g, expr);
        break;
    case EXPR_CALL:
        gen_expr_call(g, expr);
        break;
    case EXPR_IF:
        gen_expr_if(g, expr);
        break;
    case EXPR_MEMBER:
        gen_expr_member(g, expr);
        break;
    case EXPR_LIST_LITERAL:
        gen_expr_list_literal(g, expr);
        break;
    case EXPR_LIST_RANGE:
        gen_expr_list_range(g, expr);
        break;
    case EXPR_RANGE:
        gen_expr_range(g, expr);
        break;
    case EXPR_INDEX:
        gen_expr_index(g, expr);
        break;
    default:
        error_exit(ERR_INTERNAL, expr->line, "Nepodporovany vyraz v generatoru");
    }

    gen_check_depth(g, depth + 1, expr->line);
}

void gen_expr_as(Gen *g, const Expr *expr, const Type *target) {
    gen_expr(g, expr);
    gen_convert(g, expr->type, target);
}

void gen_jump_if_false(Gen *g, const Expr *cond, const char *label) {
    gen_expr(g, cond);
    gen_push(g, "bool@false");
    emit_jumpifeqs(g, label);
}

void gen_expr_unary(Gen *g, const Expr *expr) {
    gen_expr(g, expr->as.unary.operand);

    switch (expr->as.unary.op) {
    case UNOP_PLUS:
        break;
    case UNOP_NEG:
        gen_push(g, expr->type->kind == TYPE_INT ? "int@-1" : "float@-0x1p+0");
        gen_stack_op(g, "MULS", 2, 1);
        break;
    case UNOP_NOT:
        gen_stack_op(g, "NOTS", 1, 1);
        break;
    }
}

/* Vrátí Double, když je některý operand Double (druhý se převede), jinak NULL. */
static const Type *operand_target(const Expr *left, const Expr *right) {
    if (left->type->kind == TYPE_DOUBLE || right->type->kind == TYPE_DOUBLE) {
        return &DOUBLE_TYPE;
    }
    return NULL;
}

void gen_expr_binary(Gen *g, const Expr *expr) {
    switch (expr->as.binary.op) {
    case BINOP_AND:
    case BINOP_OR:
        gen_logical(g, expr);
        break;
    case BINOP_CONCAT_LIST:
        gen_expr(g, expr->as.binary.left);
        gen_expr(g, expr->as.binary.right);
        gen_call_runtime(g, "list$concat", 2);
        break;
    case BINOP_LT:
    case BINOP_LE:
    case BINOP_GT:
    case BINOP_GE:
    case BINOP_EQ:
    case BINOP_NE:
        gen_comparison(g, expr);
        break;
    default:
        gen_arithmetic(g, expr);
    }
}

void gen_arithmetic(Gen *g, const Expr *expr) {
    const Expr *left = expr->as.binary.left;
    const Expr *right = expr->as.binary.right;

    if (expr->type->kind == TYPE_STRING) {
        char left_var[NAME_BUF], right_var[NAME_BUF];
        gen_new_temp(g, left_var, sizeof left_var);
        gen_new_temp(g, right_var, sizeof right_var);
        gen_expr(g, left);
        gen_expr(g, right);
        gen_pop(g, right_var);
        gen_pop(g, left_var);
        gen_emit(g, "CONCAT %s %s %s", left_var, left_var, right_var);
        gen_push(g, left_var);
        return;
    }

    const Type *target = operand_target(left, right);
    gen_expr_as(g, left, target);
    gen_expr_as(g, right, target);

    switch (expr->as.binary.op) {
    case BINOP_ADD:
        gen_stack_op(g, "ADDS", 2, 1);
        break;
    case BINOP_SUB:
        gen_stack_op(g, "SUBS", 2, 1);
        break;
    case BINOP_MUL:
        gen_stack_op(g, "MULS", 2, 1);
        break;
    case BINOP_DIV:
        gen_stack_op(g, expr->type->kind == TYPE_INT ? "IDIVS" : "DIVS", 2, 1);
        break;
    default:
        error_exit(ERR_INTERNAL, expr->line, "Neznamy aritmeticky operator v generatoru");
    }
}

void gen_comparison(Gen *g, const Expr *expr) {
    const Expr *left = expr->as.binary.left;
    const Expr *right = expr->as.binary.right;
    const Type *target = operand_target(left, right);

    gen_expr_as(g, left, target);
    gen_expr_as(g, right, target);

    /* <= a >= se skládají z opačného porovnání a negace, != z rovnosti a negace. */
    switch (expr->as.binary.op) {
    case BINOP_LT:
        gen_stack_op(g, "LTS", 2, 1);
        break;
    case BINOP_GT:
        gen_stack_op(g, "GTS", 2, 1);
        break;
    case BINOP_LE:
        gen_stack_op(g, "GTS", 2, 1);
        gen_stack_op(g, "NOTS", 1, 1);
        break;
    case BINOP_GE:
        gen_stack_op(g, "LTS", 2, 1);
        gen_stack_op(g, "NOTS", 1, 1);
        break;
    case BINOP_EQ:
        gen_stack_op(g, "EQS", 2, 1);
        break;
    case BINOP_NE:
        gen_stack_op(g, "EQS", 2, 1);
        gen_stack_op(g, "NOTS", 1, 1);
        break;
    default:
        error_exit(ERR_INTERNAL, expr->line, "Neznamy relacni operator v generatoru");
    }
}

void gen_logical(Gen *g, const Expr *expr) {
    /* && skončí hned na false, || hned na true. */
    const char *shortcut = (expr->as.binary.op == BINOP_AND) ? "bool@false" : "bool@true";
    char short_label[LABEL_BUF], end_label[LABEL_BUF];
    int depth = g->depth;

    gen_make_label(g, short_label, sizeof short_label, "short");
    gen_make_label(g, end_label, sizeof end_label, "endlogic");

    gen_expr(g, expr->as.binary.left);
    gen_push(g, shortcut);
    emit_jumpifeqs(g, short_label);
    gen_expr(g, expr->as.binary.right);
    gen_emit(g, "JUMP %s", end_label);

    gen_emit(g, "LABEL %s", short_label);
    g->depth = depth;
    gen_push(g, shortcut);
    gen_emit(g, "LABEL %s", end_label);
}

void gen_expr_call(Gen *g, const Expr *expr) {
    const Symbol *symbol = expr->as.call.symbol;
    int argc = expr->as.call.arg_count;

    /* lst(i): sémantika u indexování nastaví symbol proměnné. */
    if (symbol->kind == SYM_VAR) {
        char name[NAME_BUF];
        gen_var_name(name, sizeof name, symbol);
        gen_push(g, name);
        gen_expr(g, expr->as.call.args[0]);
        gen_call_runtime(g, "list$get", 2);
        return;
    }

    for (int i = 0; i < argc; i++) {
        gen_expr_as(g, expr->as.call.args[i], symbol->as.func.param_types[i]);
    }

    if (symbol->as.func.is_builtin) {
        gen_call_runtime(g, symbol->name, argc);
    } else {
        char label[NAME_BUF];
        gen_function_label(label, sizeof label, symbol->name);
        gen_call(g, label, argc);
    }
}

/* Větev if jako hodnota: chybějící větev i větev s want == NULL dají nil@nil. */
static void gen_branch(Gen *g, const Block *block, const Type *want) {
    if (block == NULL) {
        gen_push(g, "nil@nil");
        return;
    }
    gen_block(g, block, want);
    if (want == NULL) {
        gen_push(g, "nil@nil");
    }
}

void gen_expr_if(Gen *g, const Expr *expr) {
    const Type *want = (expr->type->kind == TYPE_UNIT) ? NULL : expr->type;
    char else_label[LABEL_BUF], end_label[LABEL_BUF];
    int depth = g->depth;

    gen_make_label(g, else_label, sizeof else_label, "else");
    gen_make_label(g, end_label, sizeof end_label, "endif");

    gen_jump_if_false(g, expr->as.if_expr.cond, else_label);
    gen_branch(g, expr->as.if_expr.then_block, want);
    gen_emit(g, "JUMP %s", end_label);

    gen_emit(g, "LABEL %s", else_label);
    g->depth = depth;
    gen_branch(g, expr->as.if_expr.else_block, want);
    gen_emit(g, "LABEL %s", end_label);
}

/* Členy bez argumentů, které jsou jen voláním rutiny nad objektem. */
typedef struct {
    const char *member;
    const char *runtime;
} SimpleMember;

static const SimpleMember SIMPLE_MEMBERS[] = {
    {"toString", "toStr"},
    {"toInt", "toInt"},
    {"length", "strLength"},
    {"floor", "double$floor"},
    {"tail", "list$tail"},
};

void gen_expr_member(Gen *g, const Expr *expr) {
    const char *name = expr->as.member.name;
    Expr **args = expr->as.member.args;

    gen_expr(g, expr->as.member.object);

    if (strcmp(name, "toList") == 0) {
        return; /* rozsah už vznikl jako seznam */
    }

    for (size_t i = 0; i < sizeof SIMPLE_MEMBERS / sizeof SIMPLE_MEMBERS[0]; i++) {
        if (strcmp(name, SIMPLE_MEMBERS[i].member) == 0) {
            gen_call_runtime(g, SIMPLE_MEMBERS[i].runtime, 1);
            return;
        }
    }

    /* head, take, drop a slice se převedou na list$get a list$slice. */
    if (strcmp(name, "head") == 0) {
        gen_push(g, "int@0");
        gen_call_runtime(g, "list$get", 2);
    } else if (strcmp(name, "take") == 0) {
        gen_push(g, "int@0");
        gen_expr(g, args[0]);
        gen_call_runtime(g, "list$slice", 3);
    } else if (strcmp(name, "drop") == 0) {
        gen_expr(g, args[0]);
        gen_push(g, INT_MAX_LITERAL);
        gen_call_runtime(g, "list$slice", 3);
    } else if (strcmp(name, "slice") == 0) {
        gen_expr(g, args[0]);
        gen_expr(g, args[1]);
        gen_call_runtime(g, "list$slice", 3);
    } else {
        error_exit(ERR_INTERNAL, expr->line, "Nepodporovany clen v generatoru");
    }
}

void gen_expr_list_literal(Gen *g, const Expr *expr) {
    int count = expr->as.list_literal.count;
    const Type *element = expr->type->element;
    char array[NAME_BUF], item[NAME_BUF];

    gen_new_temp(g, array, sizeof array);
    gen_new_temp(g, item, sizeof item);

    gen_emit(g, "NEWARRAY %s int@%d", array, count + 1);
    gen_emit(g, "ARRAYSET %s int@0 int@%d", array, count);
    for (int i = 0; i < count; i++) {
        gen_expr_as(g, expr->as.list_literal.items[i], element);
        gen_pop(g, item);
        gen_emit(g, "ARRAYSET %s int@%d %s", array, i + 1, item);
    }
    gen_push(g, array);
}

/* Společná část rozsahů: argumenty a volání rutiny list$range. */
static void gen_range_call(Gen *g, const Expr *from, const Expr *to, const Expr *step,
                           bool inclusive) {
    gen_expr(g, from);
    gen_expr(g, to);
    if (step != NULL) {
        gen_expr(g, step);
    } else {
        gen_push(g, "int@1");
    }
    gen_push(g, inclusive ? "bool@true" : "bool@false");
    gen_call_runtime(g, "list$range", 4);
}

void gen_expr_list_range(Gen *g, const Expr *expr) {
    gen_range_call(g, expr->as.list_range.start, expr->as.list_range.end,
                   expr->as.list_range.step, false);
}

void gen_expr_range(Gen *g, const Expr *expr) {
    gen_range_call(g, expr->as.range.from, expr->as.range.to, expr->as.range.step,
                   expr->as.range.is_inclusive);
}

void gen_expr_index(Gen *g, const Expr *expr) {
    gen_expr(g, expr->as.index.list);
    gen_expr(g, expr->as.index.index);
    gen_call_runtime(g, "list$get", 2);
}


/*--------------------------------------------------------------------------------*/
/* Veřejné rozhraní                                                               */
/*--------------------------------------------------------------------------------*/

void generate_program(Program *program, FILE *out) {
    Gen g;
    const FuncDef *main_func = NULL;

    memset(&g, 0, sizeof g);
    g.out = out;
    g.cur = out;
    stack_init(&g.vars, sizeof(char *));

    for (int i = 0; i < program->func_count; i++) {
        if (program->funcs[i]->is_main) {
            main_func = program->funcs[i];
        }
    }
    if (main_func == NULL) {
        error_exit(ERR_INTERNAL, 0, "Program nema vstupni funkci");
    }

    gen_entry(&g, main_func);
    for (int i = 0; i < program->func_count; i++) {
        gen_function(&g, program->funcs[i]);
    }
    gen_emit_used_runtime(&g);

    stack_free(&g.vars);
}
