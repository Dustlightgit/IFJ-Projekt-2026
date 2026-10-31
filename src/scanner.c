// xnovacm00
// scanner.c - lexikalni analyzator implementace


#include "scanner.h"

#include <stdio.h>
#include <string.h>
#include "error.h"


// Cislo aktualniho radku vstupu
static int line = 1;

// Stavy konecneho automatu
typedef enum {
    STATE_START, // Pocatecni stav
} State;


// Precte dalsi znak ze vstupu a pocita radky
static int next_char(void)
{
    int character = getchar();
    if (character == '\n')
        line++;
    return character;
}

// Precte dalsi token
void get_token(Token *token) {
    memset(token, 0, sizeof(*token));
    State state = STATE_START;

    for (;;) {
        int character = next_char();

        switch (state) {
        case STATE_START:
            // Bile znaky preskakujeme, zustavame ve stavu START
            if (character == ' ' || character == '\t' || character == '\r' || character == '\n')
                continue;

            token->line = line;

            // Konec vstupu
            if (character == EOF) {
                token->type = TOKEN_EOF;
                return;
            }

            // nic jineho zatim neumim
            error_exit(ERR_LEXICAL, token->line, "Zatim neumim tento znak");
            break;
        }
    }

}

