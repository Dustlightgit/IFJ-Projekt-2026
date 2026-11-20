#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "generator.h"
#include "semantic_analyzer.h"
#include "symtable.h"

#define SO_PATH "/tmp/ifj_ast.so"

typedef Program *(*BuildAstFn)(void);

int main(void) {
    if (system("gcc -shared -fPIC -I. -Isrc -I../src -I../../src -I../../../src -I../../../../src -xc - -o " SO_PATH) != 0) {
        return 1;
    }

    void *handle = dlopen(SO_PATH, RTLD_NOW);
    if (!handle) {
        remove(SO_PATH);
        return 1;
    }

    BuildAstFn build_ast = NULL;
    *(void **)(&build_ast) = dlsym(handle, "build_ast");
    if (!build_ast) {
        dlclose(handle);
        remove(SO_PATH);
        return 1;
    }

    int *run_semantic = dlsym(handle, "run_semantic");

    Program *program = build_ast();
    SymTable *symtable = (run_semantic && *run_semantic) ? semantic_analyze(program) : NULL;
    generate_program(program, stdout);

    symtable_free(symtable);
    program_free(program);

    dlclose(handle);
    remove(SO_PATH);
    return 0;
}