// xnovacm00
// semantic.h - semanticka analyza

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symtable.h"

// Projde cely program, zkontroluje ho a doplni typy a symboly do AST
SymTable *semantic_analyze(Program *program);

#endif