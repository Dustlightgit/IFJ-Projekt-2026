/*
    IFJ projekt 2026

    xnovacm00 Michal Novacek

    semantic_analyzer.h

    Tento soubor obsahuje deklaraci funkce pro semantickou analyzu.
*/

#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include "symtable.h"

/**
 * @brief Provede semantickou analyzu celeho programu.
 *
 * Projde cely program, zkontroluje ho a doplni typy a symboly do AST.
 *
 * @param program Program (AST) k analyze.
 * @return Tabulka symbolu vytvorena behem analyzy.
 */
SymTable *semantic_analyze(Program *program);

#endif