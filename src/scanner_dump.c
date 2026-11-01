#include <stdio.h>

#include "scanner.h"


static void print_escaped(const char *text)
{
    for (; *text; text++) {
        switch (*text) {
        case '\n': printf("\\n"); break;
        case '\t': printf("\\t"); break;
        case '\r': printf("\\r"); break;
        case '"': printf("\\\""); break;
        case '\\': printf("\\\\"); break;
        default: putchar(*text);
        }
    }
}

int main(void)
{
    Token t;
    do {
        get_token(&t);
        printf("%d  %s", t.line, token_type_name(t.type));
        if (t.nl_before)
            printf("  [nl_before]");
        printf("\n");
        token_free(&t);
    } while (t.type != TOKEN_EOF);
    return 0;
}