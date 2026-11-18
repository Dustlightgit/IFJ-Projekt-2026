/*
    IFJ projekt 2026

    xsantar00 Radovan Santar

    generator.c        

    Tento soubor obsahuje funkce pro generovani finalniho kodu z AST stromu.
    Dale obsahuje staticke definice vestavenych funkci ktere ma jazyk podporovat.

*/

/*
    Prompt pro Claude:
        Styl: principy YAGNI, KISS, DRY. Jmena snake_case, konstanty a enum SCREAMING_SNAKE_CASE, typy PascalCase. 
        Vyrazy generuj zasobnikove: kazdy vyraz necha hodnotu na vrcholu datoveho zasobniku, prikaz zanecha zasobnik beze zmeny. 
        Hloubku zasobniku sleduj v generatoru relativne k zacatku funkce a kontroluj ji po prikazech a vyrazech.
        Telo funkce generuj do bufferu pres open_memstream. 
        Nejdriv vypis hlavicku (LABEL, CREATEFRAME, PUSHFRAME), pak vsechny DEFVAR zaevidovane behem generovani, pak telo. 
        Promenne jsou LF@<jmeno>_<unique_id>, docasne LF@$tN.
        Kazda funkce vraci presne jednu hodnotu, Unit vraci nil@nil a volajici ji zahodi do GF@$discard.
        Pokud pod navratovou hodnotou zustavaji rozpracovane hodnoty, uloz ji do docasne promenne, prebytek popni do GF@$discard a teprve pak POPFRAME a RETURN.
        Vestavene funkce (print, toStr, readLine, readInt, readDouble, readBoolean, toInt, strLength, subStr, ord, chr) 
        a pomocne rutiny (double$floor, list$get, list$range, list$slice, list$tail, list$concat) jsou funkce v cilovem kodu. 
        Emituj jen ty, ktere program pouzil, vcetne zavislosti.
        print a toStr rozlisuji typ za behu instrukci TYP.
        Behove chyby skaci na sdilene navesti s EXIT int@40
        Seznam je pole: prvek 0 drzi delku, prvky od indexu 1.
        ++, take, drop, slice, head, tail a toString res pres rutiny nebo toStr.
        Podporuj rozsireni BOOLTHEN, CONTROL (if jako vyraz, for, rozsahy), FUNEXP, VALMEM a LISTS.
        Int je 64bitovy, preteceni nereš. Int se prevadi na Double jen instrukci INT2FLOATS v mistech, kde se ceka Double.
        Double literaly pis ve formatu %a. Retezcove literaly escapuj jako \xyz pro znaky 0–32, 35, 92 a od 127. 
        Zaporna cisla generuj jako nasobeni -1.
*/

/*
 * generator.h - generator ciloveho kodu IFJcode26 z AST
 *
 * Navrh (shrnuti domluvenych rozhodnuti):
 *  - Vyrazy se generuji zasobnikove: kazdy vyraz necha svou hodnotu na vrcholu
 *    datoveho zasobniku (gen_expr), prikazy po sobe zasobnik nechavaji cisty.
 *  - Telo funkce se generuje do pametoveho bufferu (open_memstream). Az je hotove,
 *    vypise se nejdriv hlavicka s DEFVAR vsech promennych a pak telo.
 *  - Volani: argumenty se pushnou na datovy zasobnik, volana funkce je popne do
 *    parametru a vysledek vrati na zasobnik. Kazda funkce vraci presne jednu hodnotu,
 *    Unit je nil@nil.
 *  - Vestavene funkce a pomocne rutiny pro seznamy jsou funkce v cilovem kodu,
 *    emituji se jen ty, ktere program skutecne pouzil. Pracuji s dynamickymi typy
 *    IFJcode26 (print a toStr rozlisi typ instrukci TYPE).
 *  - Navesti kazde funkce je "$" + jmeno. Jmeno z IFJ26 nemuze obsahovat '$', takze
 *    pomocne rutiny pojmenovane s '$' uvnitr (napr. "list$get") nekoliduji s
 *    uzivatelskymi funkcemi. Stejne tak generovana navesti a docasne promenne.
 *  - Seznam je pole IFJcode26: prvek 0 drzi delku, prvky jsou od indexu 1.
 *  - Int zustava 64bitovy, preteceni se nereší.
 *
 * Chyby generatoru (nedostatek pameti, porusena hloubka zasobniku) ukoncuji program
 * pres error_exit s kodem ERR_INTERNAL, stejne jako ostatni moduly.
 *
 * Funkce gen_* jsou interni. V generator.c je lze oznacit jako static,
 * v hlavicce jsou jen kvuli prehledu.
 */

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "ast.h"
#include "symtable.h"

/*--------------------------------------------------------------------------------*/
/* Verejne rozhrani                                                               */
/*--------------------------------------------------------------------------------*/

/*
 * Vygeneruje cely program v IFJcode26 do souboru out (typicky stdout).
 * Program musi projit semantickou analyzou (typy vyrazu a symboly jsou vyplnene).
 * Tabulku symbolu generator nepotrebuje, vsechny symboly jsou primo v AST.
 * Poradi vystupu: ".IFJcode26", vstupni bod, vsechny funkce, pouzite rutiny.
 */
void generate_program(Program *program, FILE *out);

/*--------------------------------------------------------------------------------*/
/* Stav generatoru                                                                */
/*--------------------------------------------------------------------------------*/

/*
 * Kontext generatoru, vytvari se jednou na zasobniku v generate_program.
 * Drzi: vystupni soubor, aktualni cil zapisu (out nebo buffer tela funkce),
 * seznam promennych k deklaraci (DEFVAR) v aktualni funkci, citace unikatnich
 * navesti a docasnych promennych, aktualni hloubku datoveho zasobniku
 * (relativne k zacatku funkce), aktualni funkci a priznaky pouzitych pomocnych
 * rutin. Definice struktury je v generator.c.
 */
typedef struct Gen Gen;

/*--------------------------------------------------------------------------------*/
/* Zakladni zapis instrukci                                                       */
/*--------------------------------------------------------------------------------*/

/*
 * Zapise jeden radek (jednu instrukci) do aktualniho cile, formatovani jako printf.
 * Jedine misto, ktere primo zapisuje. Hloubku zasobniku nemeni.
 */
void gen_emit(Gen *g, const char *fmt, ...);

/* Zapise PUSHS symb a zvysi hloubku zasobniku o 1. */
void gen_push(Gen *g, const char *symb);

/* Zapise POPS var a snizi hloubku zasobniku o 1. */
void gen_pop(Gen *g, const char *var);

/* Zahodi hodnotu z vrcholu zasobniku (POPS do odpadni promenne GF@$discard). */
void gen_discard(Gen *g);

/*
 * Zapise zasobnikovou instrukci (napr. "ADDS") a upravi hloubku zasobniku:
 * consumed hodnot ubude, produced hodnot pribude.
 */
void gen_stack_op(Gen *g, const char *op, int consumed, int produced);

/*
 * Vnitrni kontrola: pokud se aktualni hloubka zasobniku nerovna expected,
 * skonci s ERR_INTERNAL. Vola se po kazdem prikazu (hloubka jako pred nim)
 * a po kazdem vyrazu (hloubka pred vyrazem + 1).
 */
void gen_check_depth(const Gen *g, int expected, int line);

/*--------------------------------------------------------------------------------*/
/* Jmena, navesti, promenne                                                       */
/*--------------------------------------------------------------------------------*/

/* Vytvori unikatni navesti tvaru "$prefix$N" (N je citac) do bufferu buf. */
void gen_make_label(Gen *g, char *buf, size_t size, const char *prefix);

/*
 * Vytvori navesti funkce ("$" + jmeno), spolecne pro uzivatelske funkce,
 * vestavene funkce i pomocne rutiny.
 */
void gen_function_label(char *buf, size_t size, const char *name);

/*
 * Vytvori jmeno promenne v lokalnim ramci ve tvaru "LF@jmeno_unique_id".
 * Diky unique_id se zastinene promenne nepletou.
 */
void gen_var_name(char *buf, size_t size, const Symbol *symbol);

/*
 * Zaeviduje promennou, ktera se vypise jako DEFVAR do hlavicky aktualni funkce.
 * Vola se u kazde promenne, parametru, ridici promenne for i docasne promenne.
 */
void gen_declare_var(Gen *g, const char *var);

/* Vytvori a zaeviduje novou docasnou promennou "LF@$tN". */
void gen_new_temp(Gen *g, char *buf, size_t size);

/*--------------------------------------------------------------------------------*/
/* Literaly a konverze                                                            */
/*--------------------------------------------------------------------------------*/

/*
 * Pushne literal na zasobnik. Int jako int@, Double ve formatu %a, String se
 * zakodovanymi escape sekvencemi (znaky 0-32, 35, 92 a od 127 jako \xyz),
 * Bool, Unit jako nil@nil. Predpoklada, ze lexer uz escape sekvence z IFJ26
 * prevedl na skutecne znaky.
 */
void gen_push_literal(Gen *g, const Literal *literal);

/*
 * Prevede hodnotu na vrcholu zasobniku z typu from na typ to.
 * Jedina implicitni konverze v jazyce je Int -> Double (INT2FLOATS),
 * ve vsech ostatnich pripadech nic nezapise. NULL typ znamena "bez konverze".
 */
void gen_convert(Gen *g, const Type *from, const Type *to);

/*--------------------------------------------------------------------------------*/
/* Program a funkce                                                               */
/*--------------------------------------------------------------------------------*/

/*
 * Zapise uvod programu: ".IFJcode26", definici GF@$discard, CALL na vstupni
 * funkci, zahozeni jejiho vysledku, EXIT int@0 a sdilene navesti $error$40
 * (EXIT int@40), na ktere skaci vsechny behove kontroly.
 */
void gen_entry(Gen *g, const FuncDef *main_func);

/*
 * Vygeneruje jednu funkci: telo do bufferu, pak LABEL, CREATEFRAME, PUSHFRAME,
 * DEFVAR vsech zaevidovanych promennych a nakonec telo. Telo zacina popnutim
 * parametru ze zasobniku (v obracenem poradi). Hodnota tela (konvertovana na
 * navratovy typ) je vysledek. U funkce s typem Unit se hodnota tela zahodi
 * a vysledkem je nil@nil.
 */
void gen_function(Gen *g, const FuncDef *func);

/*
 * Zapise spolecny konec navratu z funkce. Navratova hodnota je na vrcholu zasobniku.
 * Pokud pod ni lezi rozpracovane hodnoty vyrazu (hloubka > 1), ulozi ji do docasne
 * promenne, popne prebytek do odpadni promenne a hodnotu vrati na zasobnik.
 * Pak zapise POPFRAME a RETURN. Hloubku zasobniku nemeni, protoze kod za navratem
 * pokracuje dal (jde o jiny tok rizeni).
 */
void gen_return_sequence(Gen *g);

/*--------------------------------------------------------------------------------*/
/* Pomocne rutiny v cilovem kodu (vestavene funkce, prace se seznamy)             */
/*--------------------------------------------------------------------------------*/

/*
 * Oznaci rutinu jako pouzitou (i rutiny, ktere vola), aby se na konci vypsala jeji
 * definice. Jmena odpovidaji vestavenym funkcim (print, readInt, subStr, ...)
 * a pomocnym rutinam ("list$get", "list$range", "list$slice", "list$tail",
 * "list$concat", "double$floor").
 */
void gen_use_runtime(Gen *g, const char *name);

/*
 * Zapise CALL na navesti. Hloubku zasobniku upravi o -argc + 1
 * (argumenty ubydou, vysledek pribude).
 */
void gen_call(Gen *g, const char *label, int argc);

/* Oznaci rutinu jako pouzitou a zavola ji. Argumenty uz musi byt na zasobniku. */
void gen_call_runtime(Gen *g, const char *name, int argc);

/*
 * Vypise definice vsech pouzitych rutin (vola se jednou za vsemi funkcemi).
 * Rutiny jsou zaznamy v tabulce v generator.c.
 */
void gen_emit_used_runtime(Gen *g);

/*--------------------------------------------------------------------------------*/
/* Bloky a prikazy (po prikazu je hloubka zasobniku stejna jako pred nim)         */
/*--------------------------------------------------------------------------------*/

/*
 * Vygeneruje blok. Pokud want neni NULL, blok necha na zasobniku presne jednu hodnotu
 * (hodnotu posledniho vyrazoveho prikazu, jinak nil@nil) konvertovanou na typ want.
 * Pokud je want NULL, nenecha na zasobniku nic. Jedna funkce pro tela funkci,
 * vetve if i tela cyklu.
 */
void gen_block(Gen *g, const Block *block, const Type *want);

/*
 * Vygeneruje jeden prikaz podle druhu (rozdeli na funkce nize).
 * STMT_EXPR se vyresi primo: gen_expr a gen_discard.
 */
void gen_stmt(Gen *g, const Stmt *stmt);

/*
 * val/var: zaeviduje promennou, vyhodnoti inicializaci konvertovanou na typ promenne
 * a popne ji do promenne.
 */
void gen_stmt_vardef(Gen *g, const Stmt *stmt);

/* Prirazeni: vyhodnoti hodnotu konvertovanou na typ promenne a popne ji do promenne. */
void gen_stmt_assign(Gen *g, const Stmt *stmt);

/*
 * return: vyhodnoti hodnotu (nebo pushne nil@nil), konvertuje na navratovy typ funkce
 * (u explicitniho Unit hodnotu zahodi a vrati nil@nil) a zapise gen_return_sequence.
 */
void gen_stmt_return(Gen *g, const Stmt *stmt);

/* while: navesti zacatku, gen_jump_if_false na konec, telo, skok zpet, navesti konce. */
void gen_stmt_while(Gen *g, const Stmt *stmt);

/* for: podle zdroje zavola gen_for_range (EXPR_RANGE) nebo gen_for_list (seznam). */
void gen_stmt_for(Gen *g, const Stmt *stmt);

/*
 * for pres rozsah: meze a krok se vyhodnoti prave jednou do docasnych promennych.
 * Nulovy krok (literal i vyraz) ukonci program kodem 40. Smer porovnani se urci
 * podle znamenka kroku, rozdil to/until podle is_inclusive.
 */
void gen_for_range(Gen *g, const Stmt *stmt);

/* for pres seznam: seznam se vyhodnoti jednou, prochazi se indexem od 1 (index 0 drzi delku). */
void gen_for_list(Gen *g, const Stmt *stmt);

/*
 * Straze cyklu for: kazdou vyhodnoti a pri nepravde skoci na skip_label
 * (konec iterace, kde se posune citac).
 */
void gen_for_guards(Gen *g, const Stmt *stmt, const char *skip_label);

/*--------------------------------------------------------------------------------*/
/* Vyrazy (kazdy necha na zasobniku presne jednu hodnotu)                         */
/*--------------------------------------------------------------------------------*/

/* Vygeneruje vyraz podle druhu (rozdeli na funkce nize). */
void gen_expr(Gen *g, const Expr *expr);

/*
 * Vygeneruje vyraz a zkonvertuje ho na typ target (NULL = bez konverze).
 * Jedine misto, kde se resi implicitni Int -> Double u argumentu, prirazeni,
 * navratovych hodnot a operandu.
 */
void gen_expr_as(Gen *g, const Expr *expr, const Type *target);

/*
 * Vyhodnoti podminku a pri nepravde skoci na label (JUMPIFEQS proti bool@false).
 * Spolecne pro if, while a straze for. Hloubka zasobniku po skoku zustane beze zmeny.
 */
void gen_jump_if_false(Gen *g, const Expr *cond, const char *label);

/* Unarni operatory: minus jako nasobeni -1, plus nic, ! jako NOTS. */
void gen_expr_unary(Gen *g, const Expr *expr);

/* Binarni operatory: rozdeli na aritmetiku, porovnani, logicke operatory a spojeni seznamu. */
void gen_expr_binary(Gen *g, const Expr *expr);

/*
 * + - * / nad cisly (IDIVS pro Int, DIVS pro Double, operandy konvertovane na
 * spolecny typ) a + nad retezci (CONCAT pres docasne promenne, protoze nema
 * zasobnikovou verzi).
 */
void gen_arithmetic(Gen *g, const Expr *expr);

/*
 * < <= > >= == != : LTS, GTS, EQS, <= jako GTS + NOTS, >= jako LTS + NOTS,
 * != jako EQS + NOTS. Smisene Int a Double se prevedou na Double.
 */
void gen_comparison(Gen *g, const Expr *expr);

/* && a || se zkratovym vyhodnocenim pres podminene skoky a navesti. */
void gen_logical(Gen *g, const Expr *expr);

/*
 * EXPR_CALL ma tri podoby:
 *  - volani uzivatelske funkce: argumenty zleva doprava konvertovane na typy
 *    parametru, pak gen_call,
 *  - volani vestavene funkce: totez, ale pres gen_call_runtime (NULL typ parametru
 *    = Any, bez konverze),
 *  - indexovani seznamu zapsane jako lst(i): semantika u nej nastavi symbol druhu
 *    SYM_VAR, generator pushne promennou a index a zavola "list$get".
 */
void gen_expr_call(Gen *g, const Expr *expr);

/*
 * if jako vyraz: obe vetve pres gen_block s typem vysledku (u typu Unit se hodnoty
 * vetvi zahodi a vysledkem je nil@nil). Po obou vetvich musi byt hloubka zasobniku
 * stejna. Chybejici else se chova jako vetev s nil@nil.
 */
void gen_expr_if(Gen *g, const Expr *expr);

/*
 * Cleny hodnot (toString, toInt, floor, length, drop, take, slice, head, tail, toList):
 * vyhodnoti objekt a argumenty a zavola odpovidajici rutinu pres gen_call_runtime.
 * toList na rozsahu nic nepridava, rozsah uz vznikl jako seznam.
 */
void gen_expr_member(Gen *g, const Expr *expr);

/* List(a, b, c): vytvori pole o velikosti n + 1, do prvku 0 ulozi delku, prvky na indexy od 1. */
void gen_expr_list_literal(Gen *g, const Expr *expr);

/*
 * List.range(start, end[, step]): vyhodnoti argumenty a zavola rutinu "list$range",
 * ktera pri nulovem kroku skonci kodem 40.
 */
void gen_expr_list_range(Gen *g, const Expr *expr);

/*
 * Rozsah mimo cyklus for (1 to 5, 0 until n, 1 to 9 by 2): stejna rutina "list$range"
 * jako u List.range, jen s priznakem, jestli je pravy okraj zahrnut.
 */
void gen_expr_range(Gen *g, const Expr *expr);

/*
 * lst(i) zapsane jako EXPR_INDEX: vyhodnoti seznam a index a zavola rutinu "list$get",
 * ktera pri indexu mimo meze skonci kodem 40.
 */
void gen_expr_index(Gen *g, const Expr *expr);

#endif
