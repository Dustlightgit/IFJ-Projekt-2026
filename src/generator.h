/* 
    IFJ projekt 2026

    Původní autor: xsantar00
    Seznam změn:
        - pridany cykly a nejake aritmeticke operace: xsantar00

    knihovna pro generator kodu

*/

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// // instrukce (PUSHFRAME)
void emit0(const char *op) {
    printf("%s\n", op);
}

// instrukce s jednim argumentem (DEFVAR ⟨var⟩)
void emit1(const char *op, const char *arg1) {
    printf("%s %s\n", op, arg1);
}

// instrukce s dvema argumenty (MOVE ⟨var⟩ ⟨symb⟩)
void emit2(const char *op, const char *arg1, const char *arg2) {
    printf("%s %s %s\n", op, arg1, arg2);
}
// instrukce s tremi argumenty (ADD ⟨var⟩ ⟨symb1⟩ ⟨symb2⟩)
void emit3(const char *op, const char *arg1, const char *arg2, const char *arg3) {
    printf("%s %s %s %s\n", op, arg1, arg2, arg3);
}

// 1. Generování hlavičky funkce
void function_header(const char *func_name) {
    emit1("LABEL", func_name);
    emit0("CREATEFRAME");
    emit0("PUSHFRAME");
}
// 2. Generování paticky funkce
void function_footer(const char *return_var) {
    pushs(return_var);
    emit0("POPFRAME");
    emit0("RETURN");
}

// Deklarace promenne
void var_def(const char *var_name) {
    emit1("DEFVAR", var_name);
}
// pop promenne ze zasobniku
void pops(const char *var_name) {
    emit1("POPS", var_name);
}
// push promenne na zasobnik
void pushs(const char *var_name) {
    emit1("PUSHS", var_name);
}
// priradi dest hodnotu src
void assign(const char *dest, const char *src) {
    emit2("MOVE", dest, src);
}
// 3. Řídicí konstrukce cyklu (while)
void while_header(const char *loop_label) {
    emit1("LABEL", loop_label);
}

void while_condition(const char *arg1, const char *arg2, const char *check, const char *done_label) {
    // Podmínka i > n
    emit3("GT", check, arg1, arg2);
    // Skok na konec, pokud je check true
    emit3("JUMPIFEQ", done_label, check, "bool@true");
}

void while_footer(const char *loop_label, const char *done_label) {
    emit1("JUMP", loop_label);
    emit1("LABEL", done_label);
}

// nasobi arg1 a arg2 a zapise ho na dest
void add(const char *dest, const char *arg1, const char *arg2) {
   emit3("MUL", dest, arg1, arg2);
}
// scita arg1 a arg2 a zapise ho na dest
void add(const char *dest, const char *arg1, const char *arg2) {
   emit3("ADD", dest, arg1, arg2);
}
// odcita arg2 od arg1 a zapise ho na dest SUB ⟨var⟩ ⟨symb1⟩ ⟨symb2⟩
void sub(const char *dest, const char *arg1, const char *arg2) {
   emit3("SUB", dest, arg1, arg2);
}
// Podělí hodnotu ze arg1 druhou hodnotou ze arg2 (oba musí být typu float)
void div(const char *dest, const char *arg1, const char *arg2) {
   emit3("DIV", dest, arg1, arg2);
}
// Podělí hodnotu ze arg1 druhou hodnotou ze arg2 (musí být oba typu int) (výsledek zaokrouhlený k nule)
void idiv(const char *dest, const char *arg1, const char *arg2) {
   emit3("IDIV", dest, arg1, arg2);
}



#endif