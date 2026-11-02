
#include <stdio.h>

#include "scanner.h"
#include "token.h"

int main(void)
{
    Token token;

    while (1) {
        get_token(&token);

        printf("%s\n", token_type_name(token.type));

        int is_eof = (token.type == TOKEN_EOF);

        token_free(&token);

        if (is_eof) {
            break;
        }
    }

    return 0;
}
