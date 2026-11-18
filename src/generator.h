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
 * generator.h - generator cilového kódu IFJcode26 z AST
 *
 * Návrh (shrnutí domluvených rozhodnutí):
 *  - Výrazy se generují zásobníkově: každý výraz nechá svou hodnotu na vrcholu
 *    datového zásobníku (gen_expr), příkazy po sobě zásobník nechávají čistý.
 *  - Tělo funkce se generuje do paměťového bufferu (open_memstream). Až je hotové,
 *    vypíše se nejdřív hlavička s DEFVAR všech proměnných a pak tělo.
 *  - Volání: argumenty se pushnou na datový zásobník, volaná funkce je popne do
 *    parametrů a výsledek vrátí na zásobník. Každá funkce vrací právě jednu hodnotu,
 *    Unit je nil@nil.
 *  - Vestavěné funkce a pomocné rutiny pro seznamy jsou funkce v cílovém kódu,
 *    emitují se jen ty, které program skutečně použil. Pracují s dynamickými typy
 *    IFJcode26 (print a toStr rozliší typ instrukcí TYPE).
 *  - Návěští každé funkce je "$" + jméno. Jméno z IFJ26 nemůže obsahovat '$', takže
 *    pomocné rutiny pojmenované s '$' uvnitř (např. "list$get") nekolidují s
 *    uživatelskými funkcemi. Stejně tak generovaná návěští a dočasné proměnné.
 *  - Seznam je pole IFJcode26: prvek 0 drží délku, prvky jsou od indexu 1.
 *  - Int zůstává 64bitový, přetečení se neřeší.
 *
 * Chyby generátoru (nedostatek paměti, porušená hloubka zásobníku) ukončují program
 * přes error_exit s kódem ERR_INTERNAL, stejně jako ostatní moduly.
 *
 * Funkce gen_* jsou interní. V generator.c je lze označit jako static,
 * v hlavičce jsou jen kvůli přehledu.
 */

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "ast.h"
#include "symtable.h"

/*--------------------------------------------------------------------------------*/
/* Veřejné rozhraní                                                               */
/*--------------------------------------------------------------------------------*/

/*
 * Vygeneruje celý program v IFJcode26 do souboru out (typicky stdout).
 * Program musí projít sémantickou analýzou (typy výrazů a symboly jsou vyplněné).
 * Tabulku symbolů generátor nepotřebuje, všechny symboly jsou přímo v AST.
 * Pořadí výstupu: ".IFJcode26", vstupní bod, všechny funkce, použité rutiny.
 */
void generate_program(Program *program, FILE *out);

/*--------------------------------------------------------------------------------*/
/* Stav generátoru                                                                */
/*--------------------------------------------------------------------------------*/

/*
 * Kontext generátoru, vytváří se jednou na zásobníku v generate_program.
 * Drží: výstupní soubor, aktuální cíl zápisu (out nebo buffer těla funkce),
 * seznam proměnných k deklaraci (DEFVAR) v aktuální funkci, čítače unikátních
 * návěští a dočasných proměnných, aktuální hloubku datového zásobníku
 * (relativně k začátku funkce), aktuální funkci a příznaky použitých pomocných
 * rutin. Definice struktury je v generator.c.
 */
typedef struct Gen Gen;

/*--------------------------------------------------------------------------------*/
/* Základní zápis instrukcí                                                       */
/*--------------------------------------------------------------------------------*/

/*
 * Zapíše jeden řádek (jednu instrukci) do aktuálního cíle, formátování jako printf.
 * Jediné místo, které přímo zapisuje. Hloubku zásobníku nemění.
 */
void gen_emit(Gen *g, const char *fmt, ...);

/* Zapíše PUSHS symb a zvýší hloubku zásobníku o 1. */
void gen_push(Gen *g, const char *symb);

/* Zapíše POPS var a sníží hloubku zásobníku o 1. */
void gen_pop(Gen *g, const char *var);

/* Zahodí hodnotu z vrcholu zásobníku (POPS do odpadní proměnné GF@$discard). */
void gen_discard(Gen *g);

/*
 * Zapíše zásobníkovou instrukci (např. "ADDS") a upraví hloubku zásobníku:
 * consumed hodnot ubude, produced hodnot přibude.
 */
void gen_stack_op(Gen *g, const char *op, int consumed, int produced);

/*
 * Vnitřní kontrola: pokud se aktuální hloubka zásobníku nerovná expected,
 * skončí s ERR_INTERNAL. Volá se po každém příkazu (hloubka jako před ním)
 * a po každém výrazu (hloubka před výrazem + 1).
 */
void gen_check_depth(const Gen *g, int expected, int line);

/*--------------------------------------------------------------------------------*/
/* Jména, návěští, proměnné                                                       */
/*--------------------------------------------------------------------------------*/

/* Vytvoří unikátní návěští tvaru "$prefix$N" (N je čítač) do bufferu buf. */
void gen_make_label(Gen *g, char *buf, size_t size, const char *prefix);

/*
 * Vytvoří návěští funkce ("$" + jméno), společné pro uživatelské funkce,
 * vestavěné funkce i pomocné rutiny.
 */
void gen_function_label(char *buf, size_t size, const char *name);

/*
 * Vytvoří jméno proměnné v lokálním rámci ve tvaru "LF@jmeno_unique_id".
 * Díky unique_id se zastíněné proměnné nepletou.
 */
void gen_var_name(char *buf, size_t size, const Symbol *symbol);

/*
 * Zaeviduje proměnnou, která se vypíše jako DEFVAR do hlavičky aktuální funkce.
 * Volá se u každé proměnné, parametru, řídicí proměnné for i dočasné proměnné.
 */
void gen_declare_var(Gen *g, const char *var);

/* Vytvoří a zaeviduje novou dočasnou proměnnou "LF@$tN". */
void gen_new_temp(Gen *g, char *buf, size_t size);

/*--------------------------------------------------------------------------------*/
/* Literály a konverze                                                            */
/*--------------------------------------------------------------------------------*/

/*
 * Pushne literál na zásobník. Int jako int@, Double ve formátu %a, String se
 * zakódovanými escape sekvencemi (znaky 0-32, 35, 92 a od 127 jako \xyz),
 * Bool, Unit jako nil@nil. Předpokládá, že lexer už escape sekvence z IFJ26
 * převedl na skutečné znaky.
 */
void gen_push_literal(Gen *g, const Literal *literal);

/*
 * Převede hodnotu na vrcholu zásobníku z typu from na typ to.
 * Jediná implicitní konverze v jazyce je Int -> Double (INT2FLOATS),
 * ve všech ostatních případech nic nezapíše. NULL typ znamená "bez konverze".
 */
void gen_convert(Gen *g, const Type *from, const Type *to);

/*--------------------------------------------------------------------------------*/
/* Program a funkce                                                               */
/*--------------------------------------------------------------------------------*/

/*
 * Zapíše úvod programu: ".IFJcode26", definici GF@$discard, CALL na vstupní
 * funkci, zahození jejího výsledku, EXIT int@0 a sdílené návěští $error$40
 * (EXIT int@40), na které skáčou všechny běhové kontroly.
 */
void gen_entry(Gen *g, const FuncDef *main_func);

/*
 * Vygeneruje jednu funkci: tělo do bufferu, pak LABEL, CREATEFRAME, PUSHFRAME,
 * DEFVAR všech zaevidovaných proměnných a nakonec tělo. Tělo začíná popnutím
 * parametrů ze zásobníku (v obráceném pořadí). Hodnota těla (konvertovaná na
 * návratový typ) je výsledek. U funkce s typem Unit se hodnota těla zahodí
 * a výsledkem je nil@nil.
 */
void gen_function(Gen *g, const FuncDef *func);

/*
 * Zapíše společný konec návratu z funkce. Návratová hodnota je na vrcholu zásobníku.
 * Pokud pod ní leží rozpracované hodnoty výrazu (hloubka > 1), uloží ji do dočasné
 * proměnné, popne přebytek do odpadní proměnné a hodnotu vrátí na zásobník.
 * Pak zapíše POPFRAME a RETURN. Hloubku zásobníku nemění, protože kód za návratem
 * pokračuje dál (jde o jiný tok řízení).
 */
void gen_return_sequence(Gen *g);

/*--------------------------------------------------------------------------------*/
/* Pomocné rutiny v cílovém kódu (vestavěné funkce, práce se seznamy)             */
/*--------------------------------------------------------------------------------*/

/*
 * Označí rutinu jako použitou (i rutiny, které volá), aby se na konci vypsala její
 * definice. Jména odpovídají vestavěným funkcím (print, readInt, subStr, ...)
 * a pomocným rutinám ("list$get", "list$range", "list$slice", "list$tail",
 * "list$concat", "double$floor").
 */
void gen_use_runtime(Gen *g, const char *name);

/*
 * Zapíše CALL na návěští. Hloubku zásobníku upraví o -argc + 1
 * (argumenty ubydou, výsledek přibude).
 */
void gen_call(Gen *g, const char *label, int argc);

/* Označí rutinu jako použitou a zavolá ji. Argumenty už musí být na zásobníku. */
void gen_call_runtime(Gen *g, const char *name, int argc);

/*
 * Vypíše definice všech použitých rutin (volá se jednou za všemi funkcemi).
 * Rutiny jsou záznamy v tabulce v generator.c.
 */
void gen_emit_used_runtime(Gen *g);

/*--------------------------------------------------------------------------------*/
/* Bloky a příkazy (po příkazu je hloubka zásobníku stejná jako před ním)         */
/*--------------------------------------------------------------------------------*/

/*
 * Vygeneruje blok. Pokud want není NULL, blok nechá na zásobníku právě jednu hodnotu
 * (hodnotu posledního výrazového příkazu, jinak nil@nil) konvertovanou na typ want.
 * Pokud je want NULL, nenechá na zásobníku nic. Jedna funkce pro těla funkcí,
 * větve if i těla cyklů.
 */
void gen_block(Gen *g, const Block *block, const Type *want);

/*
 * Vygeneruje jeden příkaz podle druhu (rozdělí na funkce níže).
 * STMT_EXPR se vyřeší přímo: gen_expr a gen_discard.
 */
void gen_stmt(Gen *g, const Stmt *stmt);

/*
 * val/var: zaeviduje proměnnou, vyhodnotí inicializaci konvertovanou na typ proměnné
 * a popne ji do proměnné.
 */
void gen_stmt_vardef(Gen *g, const Stmt *stmt);

/* Přiřazení: vyhodnotí hodnotu konvertovanou na typ proměnné a popne ji do proměnné. */
void gen_stmt_assign(Gen *g, const Stmt *stmt);

/*
 * return: vyhodnotí hodnotu (nebo pushne nil@nil), konvertuje na návratový typ funkce
 * (u explicitního Unit hodnotu zahodí a vrátí nil@nil) a zapíše gen_return_sequence.
 */
void gen_stmt_return(Gen *g, const Stmt *stmt);

/* while: návěští začátku, gen_jump_if_false na konec, tělo, skok zpět, návěští konce. */
void gen_stmt_while(Gen *g, const Stmt *stmt);

/* for: podle zdroje zavolá gen_for_range (EXPR_RANGE) nebo gen_for_list (seznam). */
void gen_stmt_for(Gen *g, const Stmt *stmt);

/*
 * for přes rozsah: meze a krok se vyhodnotí právě jednou do dočasných proměnných.
 * Nulový krok (literál i výraz) ukončí program kódem 40. Směr porovnání se určí
 * podle znaménka kroku, rozdíl to/until podle is_inclusive.
 */
void gen_for_range(Gen *g, const Stmt *stmt);

/* for přes seznam: seznam se vyhodnotí jednou, prochází se indexem od 1 (index 0 drží délku). */
void gen_for_list(Gen *g, const Stmt *stmt);

/*
 * Stráže cyklu for: každou vyhodnotí a při nepravdě skočí na skip_label
 * (konec iterace, kde se posune čítač).
 */
void gen_for_guards(Gen *g, const Stmt *stmt, const char *skip_label);

/*--------------------------------------------------------------------------------*/
/* Výrazy (každý nechá na zásobníku právě jednu hodnotu)                          */
/*--------------------------------------------------------------------------------*/

/* Vygeneruje výraz podle druhu (rozdělí na funkce níže). */
void gen_expr(Gen *g, const Expr *expr);

/*
 * Vygeneruje výraz a zkonvertuje ho na typ target (NULL = bez konverze).
 * Jediné místo, kde se řeší implicitní Int -> Double u argumentů, přiřazení,
 * návratových hodnot a operandů.
 */
void gen_expr_as(Gen *g, const Expr *expr, const Type *target);

/*
 * Vyhodnotí podmínku a při nepravdě skočí na label (JUMPIFEQS proti bool@false).
 * Společné pro if, while a stráže for. Hloubka zásobníku po skoku zůstane beze změny.
 */
void gen_jump_if_false(Gen *g, const Expr *cond, const char *label);

/* Unární operátory: mínus jako násobení -1, plus nic, ! jako NOTS. */
void gen_expr_unary(Gen *g, const Expr *expr);

/* Binární operátory: rozdělí na aritmetiku, porovnání, logické operátory a spojení seznamů. */
void gen_expr_binary(Gen *g, const Expr *expr);

/*
 * + - * / nad čísly (IDIVS pro Int, DIVS pro Double, operandy konvertované na
 * společný typ) a + nad řetězci (CONCAT přes dočasné proměnné, protože nemá
 * zásobníkovou verzi).
 */
void gen_arithmetic(Gen *g, const Expr *expr);

/*
 * < <= > >= == != : LTS, GTS, EQS, <= jako GTS + NOTS, >= jako LTS + NOTS,
 * != jako EQS + NOTS. Smíšené Int a Double se převedou na Double.
 */
void gen_comparison(Gen *g, const Expr *expr);

/* && a || se zkratovým vyhodnocením přes podmíněné skoky a návěští. */
void gen_logical(Gen *g, const Expr *expr);

/*
 * EXPR_CALL má tři podoby:
 *  - volání uživatelské funkce: argumenty zleva doprava konvertované na typy
 *    parametrů, pak gen_call,
 *  - volání vestavěné funkce: totéž, ale přes gen_call_runtime (NULL typ parametru
 *    = Any, bez konverze),
 *  - indexování seznamu zapsané jako lst(i): sémantika u něj nastaví symbol druhu
 *    SYM_VAR, generátor pushne proměnnou a index a zavolá "list$get".
 */
void gen_expr_call(Gen *g, const Expr *expr);

/*
 * if jako výraz: obě větve přes gen_block s typem výsledku (u typu Unit se hodnoty
 * větví zahodí a výsledkem je nil@nil). Po obou větvích musí být hloubka zásobníku
 * stejná. Chybějící else se chová jako větev s nil@nil.
 */
void gen_expr_if(Gen *g, const Expr *expr);

/*
 * Členy hodnot (toString, toInt, floor, length, drop, take, slice, head, tail, toList):
 * vyhodnotí objekt a argumenty a zavolá odpovídající rutinu přes gen_call_runtime.
 * toList na rozsahu nic nepřidává, rozsah už vznikl jako seznam.
 */
void gen_expr_member(Gen *g, const Expr *expr);

/* List(a, b, c): vytvoří pole o velikosti n + 1, do prvku 0 uloží délku, prvky na indexy od 1. */
void gen_expr_list_literal(Gen *g, const Expr *expr);

/*
 * List.range(start, end[, step]): vyhodnotí argumenty a zavolá rutinu "list$range",
 * která při nulovém kroku skončí kódem 40.
 */
void gen_expr_list_range(Gen *g, const Expr *expr);

/*
 * Rozsah mimo cyklus for (1 to 5, 0 until n, 1 to 9 by 2): stejná rutina "list$range"
 * jako u List.range, jen s příznakem, jestli je pravý okraj zahrnut.
 */
void gen_expr_range(Gen *g, const Expr *expr);

/*
 * lst(i) zapsané jako EXPR_INDEX: vyhodnotí seznam a index a zavolá rutinu "list$get",
 * která při indexu mimo meze skončí kódem 40.
 */
void gen_expr_index(Gen *g, const Expr *expr);

#endif