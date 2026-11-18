/*
    IFJ projekt 2026

    xnovacm00 Michal Novacek

    scanner.h

    Tento soubor obsahuje deklaraci rozhrani lexikalniho analyzatoru.
*/

#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"

/**
 * @brief Precte dalsi token ze standardniho vstupu.
 *
 * @param t Vystup, do ktereho se ulozi nacteny token.
 */
void get_token(Token *t);

#endif