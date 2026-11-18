#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "generator.h"

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

    Program *program = build_ast();
    generate_program(program, stdout);
    program_free(program);

    dlclose(handle);
    remove(SO_PATH);
    return 0;
}