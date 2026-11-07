/* 
    IFJ projekt 2026

    Původní autor: xsantar00
    Seznam změn:

    knihovna pro generator kodu

*/

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// PUSHFRAME
void emit0(const char *op) {
    printf("%s\n", op);
}

// DEFVAR ⟨var⟩
void emit1(const char *op, const char *arg1) {
    printf("%s %s\n", op, arg1);
}

// ADD ⟨var⟩ ⟨symb1⟩ ⟨symb2⟩
void emit2(const char *op, const char *arg1, const char *arg2) {
    printf("%s %s %s\n", op, arg1, arg2);
}

void emit3(const char *op, const char *arg1, const char *arg2, const char *arg3) {
    printf("%s %s %s %s\n", op, arg1, arg2, arg3);
}

// 1. Generování hlavičky a těla funkce
void function_header(const char *func_name) {
    emit1("LABEL", func_name);
    emit0("CREATEFRAME");
    emit0("PUSHFRAME");
}

void function_footer(const char *return_var) {
    char buf[64];
    snprintf(buf, sizeof(buf), "LF@%s", return_var);
    emit1("PUSHS", buf);
    emit0("POPFRAME");
    emit0("RETURN");
}

#endif